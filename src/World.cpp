#include "../include/World.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>

namespace bunker {

namespace {

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

}  // namespace

void World::Clear() {
    objects.clear();
}

void World::AddObject(const MapObject& obj) {
    objects.push_back(obj);
}

bool World::Load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    char header[4]{};
    file.read(header, 4);
    const std::string format(header, 4);
    const bool hasExtendedObjectData = (format == "BWL2");
    if (format != "BWLD" && format != "BWL2") {
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

        objects.push_back(object);
    }

    return true;
}

bool World::Save(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write("BWL2", 4);
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
    metadata.objective = "Wake from cryostasis, recover the Pip-Pad, and restore the tank link.";
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

const MapObject* World::FindObjectByScriptTag(const std::string& scriptTag) const {
    for (const auto& object : objects) {
        if (object.scriptTag == scriptTag) {
            return &object;
        }
    }
    return nullptr;
}

MapObject* World::FindObjectByScriptTag(const std::string& scriptTag) {
    for (auto& object : objects) {
        if (object.scriptTag == scriptTag) {
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
