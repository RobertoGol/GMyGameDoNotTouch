#include "../include/World.hpp"
#include "../include/GameplayDescriptorRegistry.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <string_view>
#include <unordered_map>

namespace bunker {

namespace {

constexpr std::array<std::string_view, 11> kKnownEditorLayers = {
    "Terrain",
    "Structures",
    "Gameplay",
    "Loot",
    "Service",
    "Fey",
    "Spawn",
    "Debug",
    "Markers",
    "Rail",
    "Industrial"
};

void WriteString(std::ofstream& file, const std::string& value) {
    const auto length = static_cast<std::uint32_t>(value.size());
    file.write(reinterpret_cast<const char*>(&length), sizeof(length));
    file.write(value.data(), static_cast<std::streamsize>(value.size()));
}

bool ReadString(std::ifstream& file, std::string& value) {
    std::uint32_t length = 0;
    file.read(reinterpret_cast<char*>(&length), sizeof(length));
    if (!file) {
        return false;
    }

    value.resize(length);
    file.read(value.data(), static_cast<std::streamsize>(length));
    return static_cast<bool>(file);
}

std::string TrimLayerCopy(std::string_view value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return std::string(value.substr(first, last - first + 1));
}

std::string ToLowerCopy(std::string_view value) {
    std::string lower(value);
    for (char& ch : lower) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return lower;
}

bool IsRailGameplayTag(std::string_view normalizedTag) {
    return normalizedTag == "rail_depot" || normalizedTag == "rail_fortress_hub";
}

bool IsIndustrialGameplayTag(std::string_view normalizedTag) {
    return normalizedTag == "industrial_gate" ||
        normalizedTag == "industrial_survey" ||
        normalizedTag == "industrial_outpost" ||
        normalizedTag == "assembly_cell" ||
        normalizedTag == "foundry_line" ||
        normalizedTag == "reactor_yard" ||
        normalizedTag == "capacitor_bank";
}

bool IsServiceGameplayTag(std::string_view normalizedTag) {
    return normalizedTag == "tower_sync" ||
        normalizedTag == "workshop_service" ||
        normalizedTag == "power_pylon" ||
        normalizedTag == "drone_station" ||
        normalizedTag == "recovery_fabricator" ||
        normalizedTag == "relay_substation" ||
        normalizedTag == "service_bay" ||
        normalizedTag == "water_reclaimer" ||
        normalizedTag == "lanline_service_hub" ||
        normalizedTag == "medical_support" ||
        normalizedTag == "tank_service" ||
        normalizedTag == "remote_link" ||
        normalizedTag == "orbital_uplink";
}

bool IsSpawnObject(const MapObject& object) {
    const std::string lowerRegistryId = ToLowerCopy(object.registryId);
    const std::string lowerDisplayName = ToLowerCopy(object.displayName);
    return lowerRegistryId.find("spawn") != std::string::npos ||
        lowerDisplayName.find("spawn") != std::string::npos;
}

void AppendUniqueLayer(std::vector<std::string>& layers, std::string layerName) {
    if (layerName.empty()) {
        return;
    }
    if (std::find(layers.begin(), layers.end(), layerName) == layers.end()) {
        layers.push_back(std::move(layerName));
    }
}

void NormalizeLoadedObject(MapObject& object) {
    object.scriptTag = std::string(NormalizeGameplayDescriptorTag(object.scriptTag));
    object.editorLayer = NormalizeEditorLayerName(object.editorLayer);
    if (object.editorLayer.empty()) {
        object.editorLayer = DefaultEditorLayerName(object);
    }
}

bool LooksLikeLegacySemanticAutoAnchor(const MapObject& object) {
    return object.registryId.find("_auto_") != std::string::npos &&
        !object.scriptTag.empty();
}

bool LooksLikeRegistryStyleReference(std::string_view value) {
    return value.size() >= 2 && value.front() == '[' && value.back() == ']';
}

}  // namespace

const char* WorldObjectReferenceFieldLabel(WorldObjectReferenceField field) {
    switch (field) {
    case WorldObjectReferenceField::LinkTarget:
    default:
        return "linkTarget";
    }
}

const char* CurrentWorldBinaryFormatLabel() {
    return "BWL5";
}

std::string NormalizeEditorLayerName(std::string_view layerName) {
    std::string trimmed = TrimLayerCopy(layerName);
    if (trimmed.empty()) {
        return {};
    }

    const std::string lowerLayer = ToLowerCopy(trimmed);
    for (std::string_view knownLayer : kKnownEditorLayers) {
        if (lowerLayer == ToLowerCopy(knownLayer)) {
            return std::string(knownLayer);
        }
    }

    trimmed[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(trimmed[0])));
    return trimmed;
}

std::string DefaultEditorLayerName(const MapObject& object) {
    const std::string normalizedTag = std::string(NormalizeGameplayDescriptorTag(object.scriptTag));
    if (normalizedTag == "fey_ring") {
        return "Fey";
    }
    if (IsRailGameplayTag(normalizedTag)) {
        return "Rail";
    }
    if (IsIndustrialGameplayTag(normalizedTag)) {
        return "Industrial";
    }
    if (IsServiceGameplayTag(normalizedTag)) {
        return "Service";
    }
    if (IsSpawnObject(object)) {
        return "Spawn";
    }
    if (normalizedTag.starts_with("debug_")) {
        return "Debug";
    }

    switch (object.category) {
    case ObjectCategory::Structure:
    case ObjectCategory::Hangar:
        return "Structures";
    case ObjectCategory::ResourceNode:
        return "Terrain";
    case ObjectCategory::Container:
        return "Loot";
    case ObjectCategory::Landmark:
        return "Markers";
    case ObjectCategory::Terminal:
    case ObjectCategory::Vehicle:
    case ObjectCategory::Hostile:
    default:
        return "Gameplay";
    }
}

void World::Clear() {
    objects.clear();
}

void World::AddObject(const MapObject& obj) {
    MapObject normalizedObject = obj;
    NormalizeLoadedObject(normalizedObject);
    objects.push_back(std::move(normalizedObject));
}

bool World::Load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    char header[4]{};
    file.read(header, 4);
    const std::string format(header, 4);
    const bool hasExtendedObjectData = (format == "BWL2" || format == "BWL3" || format == "BWL4" || format == "BWL5");
    const bool hasSemanticAuthoringState = (format == "BWL3" || format == "BWL4" || format == "BWL5");
    const bool hasEditorLayerData = (format == "BWL4" || format == "BWL5");
    const bool hasPrefabSourceData = (format == "BWL5");
    if (format != "BWLD" && format != "BWL2" && format != "BWL3" && format != "BWL4" && format != "BWL5") {
        return false;
    }

    Clear();
    if (!ReadString(file, metadata.name) || !ReadString(file, metadata.biome) ||
        !ReadString(file, metadata.objective)) {
        return false;
    }
    file.read(reinterpret_cast<char*>(&metadata.playerSpawnX), sizeof(metadata.playerSpawnX));
    file.read(reinterpret_cast<char*>(&metadata.playerSpawnY), sizeof(metadata.playerSpawnY));
    if (!file) {
        return false;
    }

    std::uint32_t count = 0;
    file.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!file) {
        return false;
    }

    for (std::uint32_t index = 0; index < count; ++index) {
        MapObject object;
        std::uint32_t interaction = 0;
        std::uint32_t category = 0;

        if (!ReadString(file, object.registryId) || !ReadString(file, object.displayName)) {
            return false;
        }
        if (hasExtendedObjectData) {
            if (!ReadString(file, object.scriptTag) || !ReadString(file, object.linkTarget)) {
                return false;
            }
            if (hasEditorLayerData && !ReadString(file, object.editorLayer)) {
                return false;
            }
            if (hasPrefabSourceData && !ReadString(file, object.prefabSourceId)) {
                return false;
            }
        }

        file.read(reinterpret_cast<char*>(&interaction), sizeof(interaction));
        file.read(reinterpret_cast<char*>(&category), sizeof(category));
        file.read(reinterpret_cast<char*>(&object.x), sizeof(object.x));
        file.read(reinterpret_cast<char*>(&object.y), sizeof(object.y));
        file.read(reinterpret_cast<char*>(&object.z), sizeof(object.z));
        file.read(reinterpret_cast<char*>(&object.width), sizeof(object.width));
        file.read(reinterpret_cast<char*>(&object.depth), sizeof(object.depth));
        file.read(reinterpret_cast<char*>(&object.height), sizeof(object.height));
        file.read(reinterpret_cast<char*>(&object.health), sizeof(object.health));
        file.read(reinterpret_cast<char*>(&object.blocksMovement), sizeof(object.blocksMovement));
        file.read(reinterpret_cast<char*>(&object.discovered), sizeof(object.discovered));
        file.read(reinterpret_cast<char*>(&object.manualLoot), sizeof(object.manualLoot));
        if (hasSemanticAuthoringState) {
            file.read(reinterpret_cast<char*>(&object.semanticAutoCreated), sizeof(object.semanticAutoCreated));
            file.read(reinterpret_cast<char*>(&object.semanticLayoutPinned), sizeof(object.semanticLayoutPinned));
        }

        if (!file) {
            return false;
        }

        object.interaction = static_cast<InteractionType>(interaction);
        object.category = static_cast<ObjectCategory>(category);

        for (auto& lootId : object.manualLootIds) {
            if (!ReadString(file, lootId)) {
                return false;
            }
        }

        if (!hasSemanticAuthoringState && hasExtendedObjectData && LooksLikeLegacySemanticAutoAnchor(object)) {
            object.semanticAutoCreated = true;
        }
        NormalizeLoadedObject(object);
        objects.push_back(std::move(object));
    }

    return true;
}

bool World::Save(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(CurrentWorldBinaryFormatLabel(), 4);
    WriteString(file, metadata.name);
    WriteString(file, metadata.biome);
    WriteString(file, metadata.objective);
    file.write(reinterpret_cast<const char*>(&metadata.playerSpawnX), sizeof(metadata.playerSpawnX));
    file.write(reinterpret_cast<const char*>(&metadata.playerSpawnY), sizeof(metadata.playerSpawnY));

    const auto count = static_cast<std::uint32_t>(objects.size());
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& object : objects) {
        const auto interaction = static_cast<std::uint32_t>(object.interaction);
        const auto category = static_cast<std::uint32_t>(object.category);

        WriteString(file, object.registryId);
        WriteString(file, object.displayName);
        WriteString(file, object.scriptTag);
        WriteString(file, object.linkTarget);
        const std::string normalizedLayer = NormalizeEditorLayerName(object.editorLayer);
        WriteString(file, normalizedLayer.empty() ? DefaultEditorLayerName(object) : normalizedLayer);
        WriteString(file, object.prefabSourceId);
        file.write(reinterpret_cast<const char*>(&interaction), sizeof(interaction));
        file.write(reinterpret_cast<const char*>(&category), sizeof(category));
        file.write(reinterpret_cast<const char*>(&object.x), sizeof(object.x));
        file.write(reinterpret_cast<const char*>(&object.y), sizeof(object.y));
        file.write(reinterpret_cast<const char*>(&object.z), sizeof(object.z));
        file.write(reinterpret_cast<const char*>(&object.width), sizeof(object.width));
        file.write(reinterpret_cast<const char*>(&object.depth), sizeof(object.depth));
        file.write(reinterpret_cast<const char*>(&object.height), sizeof(object.height));
        file.write(reinterpret_cast<const char*>(&object.health), sizeof(object.health));
        file.write(reinterpret_cast<const char*>(&object.blocksMovement), sizeof(object.blocksMovement));
        file.write(reinterpret_cast<const char*>(&object.discovered), sizeof(object.discovered));
        file.write(reinterpret_cast<const char*>(&object.manualLoot), sizeof(object.manualLoot));
        file.write(reinterpret_cast<const char*>(&object.semanticAutoCreated), sizeof(object.semanticAutoCreated));
        file.write(reinterpret_cast<const char*>(&object.semanticLayoutPinned), sizeof(object.semanticLayoutPinned));

        for (const auto& lootId : object.manualLootIds) {
            WriteString(file, lootId);
        }
    }

    return static_cast<bool>(file);
}

void World::GeneratePrototypeZone() {
    Clear();
    metadata.name = "Cryo Sector // Shelter 17";
    metadata.biome = "Bunker Interior";
    metadata.objective = "Wake from cryostasis, recover the Pip-Pad, restore BT-72, and force open the first recovery route.";
    metadata.playerSpawnX = -12.0f;
    metadata.playerSpawnY = -8.0f;

    AddObject({
        "[%cryo_0001]",
        "Cryo Capsule 14",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        -12.0f,
        -8.0f,
        0.0f,
        2.0f,
        1.4f,
        2.2f,
        100.0f,
        false,
        true,
        false,
        {}
    });

    AddObject({
        "[%pip_0001]",
        "Pip-Pad Recovery Locker",
        InteractionType::Container,
        ObjectCategory::Container,
        -9.0f,
        -6.0f,
        0.0f,
        1.4f,
        1.4f,
        1.6f,
        100.0f,
        false,
        true,
        true,
        {"#%it_pippad", "cryo_medkit", "", ""}
    });

    AddObject({
        "[%archive_0001]",
        "Missing Personnel Archive",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        -7.0f,
        -1.0f,
        0.0f,
        2.2f,
        1.4f,
        2.0f,
        100.0f,
        false,
        true,
        false,
        {}
    });

    AddObject({
        "[%enemy_laska_0001]",
        "Feral Laska",
        InteractionType::Hostile,
        ObjectCategory::Hostile,
        -4.5f,
        -1.5f,
        0.0f,
        1.1f,
        1.1f,
        1.1f,
        35.0f,
        true,
        true,
        false,
        {}
    });

    AddObject({
        "[%core_0001]",
        "Central Core Rack",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        -1.5f,
        -2.0f,
        0.0f,
        4.0f,
        2.0f,
        3.0f,
        100.0f,
        false,
        true,
        {}
    });

    AddObject({
        "[%garage_0001]",
        "Garage Lift",
        InteractionType::Transition,
        ObjectCategory::Landmark,
        2.0f,
        2.0f,
        0.0f,
        3.5f,
        2.0f,
        2.0f,
        100.0f,
        false,
        true,
        false,
        {}
    });

    AddObject({
        "[#tr_hull_0001]",
        "First Tank Hull",
        InteractionType::VehicleAnchor,
        ObjectCategory::Vehicle,
        4.0f,
        -1.5f,
        0.0f,
        3.5f,
        2.0f,
        2.0f,
        180.0f,
        true,
        true,
        false,
        {}
    });

    AddObject({
        "[%workshop_0001]",
        "Field Workshop",
        InteractionType::Workshop,
        ObjectCategory::Terminal,
        7.0f,
        -1.0f,
        0.0f,
        2.5f,
        2.0f,
        2.0f,
        100.0f,
        false,
        true,
        false,
        {}
    });

    AddObject({
        "[%workshop_cache_0001]",
        "Workshop Supply Cache",
        InteractionType::Container,
        ObjectCategory::Container,
        8.9f,
        0.8f,
        0.0f,
        1.4f,
        1.2f,
        1.1f,
        70.0f,
        false,
        true,
        true,
        {"repair_patch", "power_cell", "#%it_ptrs_ammo", ""}
    });

    AddObject({
        "[%echo_0001]",
        "Echo Residue // Maintenance Ghost",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        5.8f,
        1.4f,
        0.0f,
        1.0f,
        1.0f,
        1.4f,
        100.0f,
        false,
        true,
        false,
        {},
        "echo_trace",
        "[%workshop_cache_0001]"
    });

    AddObject({
        "[%spec_eng_0001]",
        "Cryo Capsule // Senior Engineer",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        -10.4f,
        -10.4f,
        0.0f,
        1.8f,
        1.4f,
        2.2f,
        100.0f,
        false,
        true,
        false,
        {},
        "specialist_cryo",
        "engineer"
    });

    AddObject({
        "#%it_bucket_0001",
        "Bucket Plow Rack",
        InteractionType::Container,
        ObjectCategory::Container,
        8.5f,
        -2.0f,
        0.0f,
        1.2f,
        1.2f,
        1.2f,
        80.0f,
        false,
        true,
        true,
        {"scrap_steel", "hydraulic_seal", "", ""}
    });

    AddObject({
        "[%bulkhead_0001]",
        "Outer Bulkhead",
        InteractionType::Transition,
        ObjectCategory::Landmark,
        12.0f,
        0.5f,
        0.0f,
        2.8f,
        1.8f,
        2.4f,
        100.0f,
        false,
        true,
        false,
        {}
    });

    AddObject({
        "#%res_scrap_0001",
        "Outer Debris Barrier",
        InteractionType::Resource,
        ObjectCategory::ResourceNode,
        16.5f,
        1.5f,
        0.0f,
        4.0f,
        2.5f,
        1.0f,
        60.0f,
        true,
        true,
        true,
        {"steel_scrap", "copper_wire", "old_plate", ""}
    });

    AddObject({
        "#%term_0001",
        "Outskirts Relay Terminal",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        20.0f,
        -4.0f,
        0.0f,
        1.5f,
        1.5f,
        2.0f,
        90.0f,
        false,
        true,
        false,
        {}
    });

    AddObject({
        "[%pylon_0001]",
        "North Service Pylon",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        18.0f,
        4.5f,
        0.0f,
        1.4f,
        1.2f,
        3.2f,
        90.0f,
        false,
        true,
        false,
        {},
        "power_pylon",
        "regional_grid_north"
    });

    AddObject({
        "[%pylon_0002]",
        "South Service Pylon",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        25.0f,
        -5.0f,
        0.0f,
        1.4f,
        1.2f,
        3.2f,
        90.0f,
        false,
        true,
        false,
        {},
        "power_pylon",
        "regional_grid_south"
    });

    AddObject({
        "[%drone_0001]",
        "Field Drone Station",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        21.0f,
        -8.0f,
        0.0f,
        1.8f,
        1.5f,
        2.4f,
        85.0f,
        false,
        true,
        false,
        {},
        "drone_station",
        "salvage_sweep"
    });

    AddObject({
        "[%rail_0001]",
        "Industrial Rail Depot",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        28.5f,
        -7.5f,
        0.0f,
        2.6f,
        1.8f,
        2.8f,
        90.0f,
        false,
        true,
        false,
        {},
        "rail_depot",
        "industrial_spur_alpha"
    });

    AddObject({
        "[%orbit_0001]",
        "Orbital Uplink Mast",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        30.5f,
        -2.5f,
        0.0f,
        2.2f,
        1.6f,
        4.2f,
        100.0f,
        false,
        true,
        false,
        {},
        "orbital_uplink",
        "low_orbit_scan"
    });

    AddObject({
        "[%fortress_0001]",
        "Rail Fortress Depot",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        33.0f,
        -7.2f,
        0.0f,
        3.0f,
        1.9f,
        3.0f,
        100.0f,
        false,
        true,
        false,
        {},
        "rail_fortress_hub",
        "magistral_anchor_alpha"
    });

    AddObject({
        "[%fabricator_0001]",
        "Recovery Fabricator Node",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        8.5f,
        -7.8f,
        0.0f,
        2.0f,
        1.6f,
        2.4f,
        100.0f,
        false,
        true,
        false,
        {},
        "recovery_fabricator",
        "shelter17_refinery"
    });

    AddObject({
        "[%industrial_gate_0001]",
        "Industrial Gate Override",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        36.5f,
        -3.2f,
        0.0f,
        2.4f,
        1.8f,
        2.8f,
        100.0f,
        false,
        true,
        false,
        {},
        "industrial_gate",
        "inner_spur_alpha"
    });

    AddObject({
        "[%survey_0001]",
        "Industrial Survey Beacon",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        39.5f,
        -5.6f,
        0.0f,
        2.0f,
        1.6f,
        3.0f,
        100.0f,
        false,
        true,
        false,
        {},
        "industrial_survey",
        "inner_spur_survey"
    });

    AddObject({
        "[%outpost_0001]",
        "Inner Spur Outpost Hub",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        43.0f,
        -7.0f,
        0.0f,
        2.4f,
        1.8f,
        2.6f,
        100.0f,
        false,
        true,
        false,
        {},
        "industrial_outpost",
        "inner_spur_outpost"
    });

    AddObject({
        "[%assembly_0001]",
        "Inner Spur Assembly Cell",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        46.0f,
        -4.8f,
        0.0f,
        2.6f,
        1.8f,
        2.8f,
        100.0f,
        false,
        true,
        false,
        {},
        "assembly_cell",
        "inner_spur_assembly"
    });

    AddObject({
        "[%foundry_0001]",
        "Inner Spur Foundry Line",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        48.8f,
        -7.2f,
        0.0f,
        3.0f,
        2.0f,
        3.2f,
        100.0f,
        false,
        true,
        false,
        {},
        "foundry_line",
        "inner_spur_foundry"
    });

    AddObject({
        "[%reactor_0001]",
        "Inner Spur Reactor Yard",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        52.0f,
        -5.5f,
        0.0f,
        3.2f,
        2.1f,
        3.4f,
        100.0f,
        false,
        true,
        false,
        {},
        "reactor_yard",
        "inner_spur_reactor"
    });

    AddObject({
        "[%capacitor_0001]",
        "Inner Spur Capacitor Bank",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        55.2f,
        -7.0f,
        0.0f,
        3.0f,
        2.0f,
        3.0f,
        100.0f,
        false,
        true,
        false,
        {},
        "capacitor_bank",
        "inner_spur_capacitor"
    });

    AddObject({
        "[%substation_0001]",
        "Inner Spur Relay Substation",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        58.3f,
        -5.8f,
        0.0f,
        3.1f,
        2.0f,
        3.2f,
        100.0f,
        false,
        true,
        false,
        {},
        "relay_substation",
        "shelter17_backbone"
    });

    AddObject({
        "[%servicebay_0001]",
        "Inner Spur Service Bay",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        61.1f,
        -7.2f,
        0.0f,
        3.3f,
        2.1f,
        3.3f,
        100.0f,
        false,
        true,
        false,
        {},
        "service_bay",
        "inner_spur_service"
    });

    AddObject({
        "[%water_0001]",
        "Inner Spur Water Reclaimer",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        64.0f,
        -5.6f,
        0.0f,
        3.0f,
        2.0f,
        3.1f,
        100.0f,
        false,
        true,
        false,
        {},
        "water_reclaimer",
        "inner_spur_water"
    });

    AddObject({
        "[%services_0001]",
        "Shelter 17 Lanline Service Hub",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        66.8f,
        -6.8f,
        0.0f,
        2.6f,
        1.8f,
        2.8f,
        100.0f,
        false,
        true,
        false,
        {},
        "lanline_service_hub",
        "shelter17_services"
    });

    AddObject({
        "[%fey_0001]",
        "Shelter 17 Fey Ring Gate",
        InteractionType::Terminal,
        ObjectCategory::Landmark,
        69.4f,
        -5.6f,
        0.0f,
        3.0f,
        2.0f,
        3.2f,
        100.0f,
        false,
        true,
        false,
        {},
        "fey_ring",
        "intercity_ring"
    });

    AddObject({
        "[%med_0001]",
        "Field Medical Relay",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        63.5f,
        -8.7f,
        0.0f,
        1.8f,
        1.4f,
        2.1f,
        100.0f,
        false,
        true,
        false,
        {},
        "medical_support",
        "field_medical"
    });

    AddObject({
        "[%tankservice_0001]",
        "BT-72 Tank Service Anchor",
        InteractionType::Workshop,
        ObjectCategory::Hangar,
        61.7f,
        -9.5f,
        0.0f,
        3.4f,
        2.2f,
        3.0f,
        100.0f,
        false,
        true,
        false,
        {},
        "tank_service",
        "bt72_service"
    });

    AddObject({
        "[%debrief_0001]",
        "Shelter 17 Debrief Console",
        InteractionType::Terminal,
        ObjectCategory::Terminal,
        -2.5f,
        3.2f,
        0.0f,
        2.0f,
        1.6f,
        2.0f,
        100.0f,
        false,
        true,
        false,
        {}
    });

    AddObject({
        "[%enemy_ghoul_0001]",
        "Outer Ghoul",
        InteractionType::Hostile,
        ObjectCategory::Hostile,
        18.5f,
        -1.0f,
        0.0f,
        1.3f,
        1.1f,
        1.6f,
        55.0f,
        true,
        true,
        false,
        {}
    });

    AddObject({
        "[%gate_0001]",
        "Outer Gate Wall",
        InteractionType::Static,
        ObjectCategory::Structure,
        24.0f,
        2.0f,
        0.0f,
        8.0f,
        2.0f,
        4.0f,
        500.0f,
        true,
        true,
        false,
        {}
    });

    AddObject({
        "[%camp_0001]",
        "Forward Camp Marker",
        InteractionType::Transition,
        ObjectCategory::Landmark,
        22.0f,
        -6.0f,
        0.0f,
        2.5f,
        2.5f,
        1.0f,
        100.0f,
        false,
        true,
        false,
        {}
    });
}

void World::EnsureStarterInfrastructure() {
    if (objects.empty()) {
        GeneratePrototypeZone();
        return;
    }

    if (IsStarterScenarioWorld() && !HasObject("[%echo_0001]")) {
        AddObject({
            "[%echo_0001]",
            "Echo Residue // Maintenance Ghost",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            5.8f,
            1.4f,
            0.0f,
            1.0f,
            1.0f,
            1.4f,
            100.0f,
            false,
            true,
            false,
            {},
            "echo_trace",
            "[%workshop_cache_0001]"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%spec_eng_0001]")) {
        AddObject({
            "[%spec_eng_0001]",
            "Cryo Capsule // Senior Engineer",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            -10.4f,
            -10.4f,
            0.0f,
            1.8f,
            1.4f,
            2.2f,
            100.0f,
            false,
            true,
            false,
            {},
            "specialist_cryo",
            "engineer"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%pylon_0001]")) {
        AddObject({
            "[%pylon_0001]",
            "North Service Pylon",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            18.0f,
            4.5f,
            0.0f,
            1.4f,
            1.2f,
            3.2f,
            90.0f,
            false,
            true,
            false,
            {},
            "power_pylon",
            "regional_grid_north"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%pylon_0002]")) {
        AddObject({
            "[%pylon_0002]",
            "South Service Pylon",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            25.0f,
            -5.0f,
            0.0f,
            1.4f,
            1.2f,
            3.2f,
            90.0f,
            false,
            true,
            false,
            {},
            "power_pylon",
            "regional_grid_south"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%drone_0001]")) {
        AddObject({
            "[%drone_0001]",
            "Field Drone Station",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            21.0f,
            -8.0f,
            0.0f,
            1.8f,
            1.5f,
            2.4f,
            85.0f,
            false,
            true,
            false,
            {},
            "drone_station",
            "salvage_sweep"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%rail_0001]")) {
        AddObject({
            "[%rail_0001]",
            "Industrial Rail Depot",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            28.5f,
            -7.5f,
            0.0f,
            2.6f,
            1.8f,
            2.8f,
            90.0f,
            false,
            true,
            false,
            {},
            "rail_depot",
            "industrial_spur_alpha"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%orbit_0001]")) {
        AddObject({
            "[%orbit_0001]",
            "Orbital Uplink Mast",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            30.5f,
            -2.5f,
            0.0f,
            2.2f,
            1.6f,
            4.2f,
            100.0f,
            false,
            true,
            false,
            {},
            "orbital_uplink",
            "low_orbit_scan"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%fortress_0001]")) {
        AddObject({
            "[%fortress_0001]",
            "Rail Fortress Depot",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            33.0f,
            -7.2f,
            0.0f,
            3.0f,
            1.9f,
            3.0f,
            100.0f,
            false,
            true,
            false,
            {},
            "rail_fortress_hub",
            "magistral_anchor_alpha"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%fabricator_0001]")) {
        AddObject({
            "[%fabricator_0001]",
            "Recovery Fabricator Node",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            8.5f,
            -7.8f,
            0.0f,
            2.0f,
            1.6f,
            2.4f,
            100.0f,
            false,
            true,
            false,
            {},
            "recovery_fabricator",
            "shelter17_refinery"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%industrial_gate_0001]")) {
        AddObject({
            "[%industrial_gate_0001]",
            "Industrial Gate Override",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            36.5f,
            -3.2f,
            0.0f,
            2.4f,
            1.8f,
            2.8f,
            100.0f,
            false,
            true,
            false,
            {},
            "industrial_gate",
            "inner_spur_alpha"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%survey_0001]")) {
        AddObject({
            "[%survey_0001]",
            "Industrial Survey Beacon",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            39.5f,
            -5.6f,
            0.0f,
            2.0f,
            1.6f,
            3.0f,
            100.0f,
            false,
            true,
            false,
            {},
            "industrial_survey",
            "inner_spur_survey"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%outpost_0001]")) {
        AddObject({
            "[%outpost_0001]",
            "Inner Spur Outpost Hub",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            43.0f,
            -7.0f,
            0.0f,
            2.4f,
            1.8f,
            2.6f,
            100.0f,
            false,
            true,
            false,
            {},
            "industrial_outpost",
            "inner_spur_outpost"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%assembly_0001]")) {
        AddObject({
            "[%assembly_0001]",
            "Inner Spur Assembly Cell",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            46.0f,
            -4.8f,
            0.0f,
            2.6f,
            1.8f,
            2.8f,
            100.0f,
            false,
            true,
            false,
            {},
            "assembly_cell",
            "inner_spur_assembly"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%foundry_0001]")) {
        AddObject({
            "[%foundry_0001]",
            "Inner Spur Foundry Line",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            48.8f,
            -7.2f,
            0.0f,
            3.0f,
            2.0f,
            3.2f,
            100.0f,
            false,
            true,
            false,
            {},
            "foundry_line",
            "inner_spur_foundry"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%reactor_0001]")) {
        AddObject({
            "[%reactor_0001]",
            "Inner Spur Reactor Yard",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            52.0f,
            -5.5f,
            0.0f,
            3.2f,
            2.1f,
            3.4f,
            100.0f,
            false,
            true,
            false,
            {},
            "reactor_yard",
            "inner_spur_reactor"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%capacitor_0001]")) {
        AddObject({
            "[%capacitor_0001]",
            "Inner Spur Capacitor Bank",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            55.2f,
            -7.0f,
            0.0f,
            3.0f,
            2.0f,
            3.0f,
            100.0f,
            false,
            true,
            false,
            {},
            "capacitor_bank",
            "inner_spur_capacitor"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%substation_0001]")) {
        AddObject({
            "[%substation_0001]",
            "Inner Spur Relay Substation",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            58.3f,
            -5.8f,
            0.0f,
            3.1f,
            2.0f,
            3.2f,
            100.0f,
            false,
            true,
            false,
            {},
            "relay_substation",
            "shelter17_backbone"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%servicebay_0001]")) {
        AddObject({
            "[%servicebay_0001]",
            "Inner Spur Service Bay",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            61.1f,
            -7.2f,
            0.0f,
            3.3f,
            2.1f,
            3.3f,
            100.0f,
            false,
            true,
            false,
            {},
            "service_bay",
            "inner_spur_service"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%water_0001]")) {
        AddObject({
            "[%water_0001]",
            "Inner Spur Water Reclaimer",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            64.0f,
            -5.6f,
            0.0f,
            3.0f,
            2.0f,
            3.1f,
            100.0f,
            false,
            true,
            false,
            {},
            "water_reclaimer",
            "inner_spur_water"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%services_0001]")) {
        AddObject({
            "[%services_0001]",
            "Shelter 17 Lanline Service Hub",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            66.8f,
            -6.8f,
            0.0f,
            2.6f,
            1.8f,
            2.8f,
            100.0f,
            false,
            true,
            false,
            {},
            "lanline_service_hub",
            "shelter17_services"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%fey_0001]")) {
        AddObject({
            "[%fey_0001]",
            "Shelter 17 Fey Ring Gate",
            InteractionType::Terminal,
            ObjectCategory::Landmark,
            69.4f,
            -5.6f,
            0.0f,
            3.0f,
            2.0f,
            3.2f,
            100.0f,
            false,
            true,
            false,
            {},
            "fey_ring",
            "intercity_ring"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%med_0001]")) {
        AddObject({
            "[%med_0001]",
            "Field Medical Relay",
            InteractionType::Terminal,
            ObjectCategory::Terminal,
            63.5f,
            -8.7f,
            0.0f,
            1.8f,
            1.4f,
            2.1f,
            100.0f,
            false,
            true,
            false,
            {},
            "medical_support",
            "field_medical"
        });
    }
    if (IsStarterScenarioWorld() && !HasObject("[%tankservice_0001]")) {
        AddObject({
            "[%tankservice_0001]",
            "BT-72 Tank Service Anchor",
            InteractionType::Workshop,
            ObjectCategory::Hangar,
            61.7f,
            -9.5f,
            0.0f,
            3.4f,
            2.2f,
            3.0f,
            100.0f,
            false,
            true,
            false,
            {},
            "tank_service",
            "bt72_service"
        });
    }
}

bool World::HasObject(const std::string& registryId) const {
    return std::any_of(objects.begin(), objects.end(), [&](const MapObject& object) {
        return object.registryId == registryId;
    });
}

const MapObject* World::FindObjectByRegistryId(const std::string& registryId) const {
    for (const auto& object : objects) {
        if (object.registryId == registryId) {
            return &object;
        }
    }
    return nullptr;
}

MapObject* World::FindObjectByRegistryId(const std::string& registryId) {
    for (auto& object : objects) {
        if (object.registryId == registryId) {
            return &object;
        }
    }
    return nullptr;
}

const MapObject* World::FindObjectByScriptTag(const std::string& scriptTag) const {
    const std::string_view normalizedTag = NormalizeGameplayDescriptorTag(scriptTag);
    for (const auto& object : objects) {
        if (NormalizeGameplayDescriptorTag(object.scriptTag) == normalizedTag) {
            return &object;
        }
    }
    return nullptr;
}

MapObject* World::FindObjectByScriptTag(const std::string& scriptTag) {
    const std::string_view normalizedTag = NormalizeGameplayDescriptorTag(scriptTag);
    for (auto& object : objects) {
        if (NormalizeGameplayDescriptorTag(object.scriptTag) == normalizedTag) {
            return &object;
        }
    }
    return nullptr;
}

const MapObject* World::FindObjectByLinkTarget(const std::string& linkTarget) const {
    for (const auto& object : objects) {
        if (object.linkTarget == linkTarget) {
            return &object;
        }
    }
    return nullptr;
}

MapObject* World::FindObjectByLinkTarget(const std::string& linkTarget) {
    for (auto& object : objects) {
        if (object.linkTarget == linkTarget) {
            return &object;
        }
    }
    return nullptr;
}

std::vector<WorldObjectReference> World::BuildObjectReferences() const {
    std::vector<WorldObjectReference> references;
    std::unordered_map<std::string, int> objectIndexById;
    objectIndexById.reserve(objects.size());
    for (int objectIndex = 0; objectIndex < static_cast<int>(objects.size()); ++objectIndex) {
        objectIndexById[objects[static_cast<std::size_t>(objectIndex)].registryId] = objectIndex;
    }

    references.reserve(objects.size());
    for (int objectIndex = 0; objectIndex < static_cast<int>(objects.size()); ++objectIndex) {
        const auto& object = objects[static_cast<std::size_t>(objectIndex)];
        if (object.linkTarget.empty()) {
            continue;
        }

        const auto targetIt = objectIndexById.find(object.linkTarget);
        const bool resolved = targetIt != objectIndexById.end();
        if (!resolved && !LooksLikeRegistryStyleReference(object.linkTarget)) {
            continue;
        }

        WorldObjectReference reference;
        reference.field = WorldObjectReferenceField::LinkTarget;
        reference.sourceObjectIndex = objectIndex;
        reference.sourceObjectId = object.registryId;
        reference.sourceDisplayName = object.displayName;
        reference.sourceScriptTag = object.scriptTag;
        reference.targetObjectId = object.linkTarget;
        reference.viaValue = object.linkTarget;
        reference.resolved = resolved;
        if (resolved) {
            reference.targetObjectIndex = targetIt->second;
        }
        references.push_back(std::move(reference));
    }

    return references;
}

std::vector<WorldObjectReference> World::FindIncomingObjectReferences(const std::string& registryId) const {
    std::vector<WorldObjectReference> references;
    for (const auto& reference : BuildObjectReferences()) {
        if (!reference.resolved || reference.targetObjectId != registryId) {
            continue;
        }
        references.push_back(reference);
    }
    return references;
}

std::vector<WorldObjectReference> World::FindOutgoingObjectReferences(const std::string& registryId) const {
    std::vector<WorldObjectReference> references;
    for (const auto& reference : BuildObjectReferences()) {
        if (reference.sourceObjectId != registryId) {
            continue;
        }
        references.push_back(reference);
    }
    return references;
}

std::vector<std::string> World::CollectEditorLayerNames() const {
    std::vector<std::string> layerNames;
    for (std::string_view knownLayer : kKnownEditorLayers) {
        if (CountObjectsInEditorLayer(knownLayer) > 0) {
            layerNames.emplace_back(knownLayer);
        }
    }

    std::vector<std::string> customLayerNames;
    for (const auto& object : objects) {
        const std::string normalizedLayer = NormalizeEditorLayerName(object.editorLayer);
        const std::string resolvedLayer = normalizedLayer.empty()
            ? DefaultEditorLayerName(object)
            : normalizedLayer;
        if (std::find(kKnownEditorLayers.begin(), kKnownEditorLayers.end(), resolvedLayer) != kKnownEditorLayers.end()) {
            continue;
        }
        AppendUniqueLayer(customLayerNames, resolvedLayer);
    }

    std::sort(customLayerNames.begin(), customLayerNames.end());
    for (std::string& customLayer : customLayerNames) {
        layerNames.push_back(std::move(customLayer));
    }
    return layerNames;
}

int World::CountObjectsInEditorLayer(std::string_view layerName) const {
    const std::string normalizedQueryLayer = NormalizeEditorLayerName(layerName);
    if (normalizedQueryLayer.empty()) {
        return 0;
    }

    int count = 0;
    for (const auto& object : objects) {
        const std::string normalizedLayer = NormalizeEditorLayerName(object.editorLayer);
        const std::string resolvedLayer = normalizedLayer.empty()
            ? DefaultEditorLayerName(object)
            : normalizedLayer;
        if (resolvedLayer == normalizedQueryLayer) {
            ++count;
        }
    }
    return count;
}

bool World::HasIncomingObjectReferences(const std::string& registryId) const {
    for (const auto& reference : BuildObjectReferences()) {
        if (reference.resolved && reference.targetObjectId == registryId) {
            return true;
        }
    }
    return false;
}

bool World::HasScriptTag(const std::string& scriptTag) const {
    return FindObjectByScriptTag(scriptTag) != nullptr;
}

bool World::HasLinkTarget(const std::string& linkTarget) const {
    return FindObjectByLinkTarget(linkTarget) != nullptr;
}

bool World::IsStarterScenarioWorld() const {
    return HasObject("[%cryo_0001]") &&
        HasObject("[%pip_0001]") &&
        HasObject("[%archive_0001]") &&
        HasObject("[#tr_hull_0001]");
}

const MapObject* World::FindNearestInteractive(float x, float y, float radius) const {
    const MapObject* nearest = nullptr;
    float bestDistanceSq = radius * radius;

    for (const auto& object : objects) {
        if (object.interaction == InteractionType::Static) {
            continue;
        }

        const float dx = object.x - x;
        const float dy = object.y - y;
        const float distanceSq = (dx * dx) + (dy * dy);
        if (distanceSq <= bestDistanceSq) {
            nearest = &object;
            bestDistanceSq = distanceSq;
        }
    }

    return nearest;
}

void World::RemoveObject(const std::string& registryId) {
    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
            [&](const MapObject& obj) { return obj.registryId == registryId; }),
        objects.end());
}

}  // namespace bunker
