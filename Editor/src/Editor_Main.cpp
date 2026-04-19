#include <GLFW/glfw3.h>

#include <array>
#include <cmath>
#include <cstring>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../../include/AppPaths.hpp"
#include "../../include/RegistryId.hpp"
#include "../../include/SessionProfiles.hpp"
#include "../../include/World.hpp"

namespace {

struct ImportedConcept {
    std::string sourceLabel;
    std::string targetType;
    std::string completionMode;
};

struct SavedPrefab {
    std::string label;
    bunker::MapObject object;
};

struct PreviewInteraction {
    bool clicked = false;
    bool clickedObject = false;
    int clickedObjectIndex = -1;
    bool doubleClickedObject = false;
    bool draggingSelectedObject = false;
    float worldX = 0.0f;
    float worldY = 0.0f;
};

struct PreviewViewportState {
    float zoom = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};

struct ObjectPreset {
    const char* label;
    bunker::InteractionType interaction;
    bunker::ObjectCategory category;
    float width;
    float depth;
    float height;
    float health;
    bool blocksMovement;
    bool manualLoot;
};

const char* ToLabel(bunker::InteractionType interaction) {
    switch (interaction) {
        case bunker::InteractionType::Static: return "Static";
        case bunker::InteractionType::Container: return "Container";
        case bunker::InteractionType::Resource: return "Resource";
        case bunker::InteractionType::Terminal: return "Terminal";
        case bunker::InteractionType::Transition: return "Transition";
        case bunker::InteractionType::VehicleAnchor: return "Vehicle Anchor";
        case bunker::InteractionType::Workshop: return "Workshop";
        case bunker::InteractionType::Hostile: return "Hostile";
    }
    return "Static";
}

const char* ToLabel(bunker::ObjectCategory category) {
    switch (category) {
        case bunker::ObjectCategory::Structure: return "Structure";
        case bunker::ObjectCategory::ResourceNode: return "Resource Node";
        case bunker::ObjectCategory::Terminal: return "Terminal";
        case bunker::ObjectCategory::Vehicle: return "Vehicle";
        case bunker::ObjectCategory::Landmark: return "Landmark";
        case bunker::ObjectCategory::Container: return "Container";
        case bunker::ObjectCategory::Hangar: return "Hangar";
        case bunker::ObjectCategory::Hostile: return "Hostile";
    }
    return "Structure";
}

int ToIndex(bunker::InteractionType interaction) {
    return static_cast<int>(interaction);
}

int ToIndex(bunker::ObjectCategory category) {
    switch (category) {
        case bunker::ObjectCategory::Structure: return 0;
        case bunker::ObjectCategory::ResourceNode: return 1;
        case bunker::ObjectCategory::Terminal: return 2;
        case bunker::ObjectCategory::Vehicle: return 3;
        case bunker::ObjectCategory::Landmark: return 4;
        case bunker::ObjectCategory::Container: return 5;
        case bunker::ObjectCategory::Hangar: return 6;
        case bunker::ObjectCategory::Hostile: return 7;
    }
    return 0;
}

bunker::ObjectCategory CategoryFromIndex(int index) {
    switch (index) {
        case 0: return bunker::ObjectCategory::Structure;
        case 1: return bunker::ObjectCategory::ResourceNode;
        case 2: return bunker::ObjectCategory::Terminal;
        case 3: return bunker::ObjectCategory::Vehicle;
        case 4: return bunker::ObjectCategory::Landmark;
        case 5: return bunker::ObjectCategory::Container;
        case 6: return bunker::ObjectCategory::Hangar;
        case 7: return bunker::ObjectCategory::Hostile;
        default: return bunker::ObjectCategory::Structure;
    }
}

bool LoadOrCreateEditorWorld(bunker::World& world, std::string& statusText) {
    bunker::SessionProfile sessionProfile;
    const auto profilePath = bunker::DefaultSessionProfilePath();
    if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
        sessionProfile = bunker::MakeDefaultSessionProfile();
    }
    bunker::NormalizeSessionProfile(sessionProfile);

    const auto path = bunker::ResolveWorldPath(sessionProfile.selectedWorld);
    if (world.Load(path.string())) {
        statusText = "Loaded runtime world from " + path.string();
        return true;
    }

    world.GeneratePrototypeZone();
    world.Save(path.string());
    statusText = "Runtime world was missing. Generated a fresh prototype workspace at " + path.string();
    return false;
}

bool SetActiveWorldInProfile(const std::string& worldFileName, std::string& statusText) {
    bunker::SessionProfile sessionProfile;
    const auto profilePath = bunker::DefaultSessionProfilePath();
    if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
        sessionProfile = bunker::MakeDefaultSessionProfile();
    }
    bunker::NormalizeSessionProfile(sessionProfile);
    sessionProfile.selectedWorld = bunker::NormalizeWorldReference(worldFileName);
    if (bunker::SaveSessionProfile(sessionProfile, profilePath)) {
        statusText = "Active world changed to " + sessionProfile.selectedWorld;
        return true;
    }
    statusText = "Failed to update active world in session profile.";
    return false;
}

void CopyStringToBuffer(const std::string& value, char* buffer, std::size_t size) {
    if (buffer == nullptr || size == 0) {
        return;
    }

    strncpy_s(buffer, size, value.c_str(), _TRUNCATE);
}

void SyncEditorWorldBindings(const bunker::World& editorWorld,
                             int& selectedObjectIndex,
                             char* selectedRegistryEdit,
                             std::size_t selectedRegistryEditSize,
                             char* selectedScriptTagEdit,
                             std::size_t selectedScriptTagEditSize,
                             char* selectedLinkTargetEdit,
                             std::size_t selectedLinkTargetEditSize,
                             char* worldNameInput,
                             std::size_t worldNameInputSize,
                             char* worldBiomeInput,
                             std::size_t worldBiomeInputSize,
                             char* worldObjectiveInput,
                             std::size_t worldObjectiveInputSize,
                             float& worldSpawnX,
                             float& worldSpawnY) {
    selectedObjectIndex = -1;
    CopyStringToBuffer("", selectedRegistryEdit, selectedRegistryEditSize);
    CopyStringToBuffer("", selectedScriptTagEdit, selectedScriptTagEditSize);
    CopyStringToBuffer("", selectedLinkTargetEdit, selectedLinkTargetEditSize);
    CopyStringToBuffer(editorWorld.metadata.name, worldNameInput, worldNameInputSize);
    CopyStringToBuffer(editorWorld.metadata.biome, worldBiomeInput, worldBiomeInputSize);
    CopyStringToBuffer(editorWorld.metadata.objective, worldObjectiveInput, worldObjectiveInputSize);
    worldSpawnX = editorWorld.metadata.playerSpawnX;
    worldSpawnY = editorWorld.metadata.playerSpawnY;
}

const ObjectPreset& SelectedPreset(const std::array<ObjectPreset, 6>& presets, int index) {
    const int clampedIndex = (index < 0) ? 0 : (index >= static_cast<int>(presets.size()) ? static_cast<int>(presets.size()) - 1 : index);
    return presets[static_cast<std::size_t>(clampedIndex)];
}

std::string DefaultRegistryIdForPreset(int presetIndex, std::size_t objectCount) {
    switch (presetIndex) {
        case 0: return "[%structure_" + std::to_string(objectCount + 1) + "]";
        case 1: return "[%terminal_" + std::to_string(objectCount + 1) + "]";
        case 2: return "[%container_" + std::to_string(objectCount + 1) + "]";
        case 3: return "[%transition_" + std::to_string(objectCount + 1) + "]";
        case 4: return "[#tr_custom_" + std::to_string(objectCount + 1) + "]";
        default: return "[%resource_" + std::to_string(objectCount + 1) + "]";
    }
}

void PrepareSpecializedDraft(const char* draftName,
                             const char* registryId,
                             float& placeX,
                             float& placeY,
                             char* objectNameInput,
                             std::size_t objectNameSize,
                             char* registryInput,
                             std::size_t registrySize,
    char* loot0,
    char* loot1,
    char* loot2,
    char* loot3) {
    CopyStringToBuffer(draftName, objectNameInput, objectNameSize);
    CopyStringToBuffer(registryId, registryInput, registrySize);
    placeX = 0.0f;
    placeY = 0.0f;
    loot0[0] = '\0';
    loot1[0] = '\0';
    loot2[0] = '\0';
    loot3[0] = '\0';
}

void ApplyTerminalDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = (object.registryId.find("archive") == std::string::npos) ? "terminal_sync" : "archive_sync";
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
}

void ApplyTransitionDescriptorPreset(bunker::MapObject& object,
                                     char* scriptTagBuffer,
                                     std::size_t scriptTagSize,
                                     char* linkTargetBuffer,
                                     std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Transition;
    object.category = bunker::ObjectCategory::Landmark;
    object.scriptTag = "Travel marker / route handoff";
    if (object.linkTarget.empty()) {
        object.linkTarget = "next_zone";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyWorkshopDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize) {
    object.interaction = bunker::InteractionType::Workshop;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "workshop_service";
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
}

void ApplyTowerDescriptorPreset(bunker::MapObject& object,
                                char* scriptTagBuffer,
                                std::size_t scriptTagSize,
                                char* linkTargetBuffer,
                                std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "tower_sync";
    if (object.linkTarget.empty()) {
        object.linkTarget = "regional_grid";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyPowerPylonDescriptorPreset(bunker::MapObject& object,
                                     char* scriptTagBuffer,
                                     std::size_t scriptTagSize,
                                     char* linkTargetBuffer,
                                     std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "power_pylon";
    if (object.linkTarget.empty()) {
        object.linkTarget = "regional_grid";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyDroneStationDescriptorPreset(bunker::MapObject& object,
                                       char* scriptTagBuffer,
                                       std::size_t scriptTagSize,
                                       char* linkTargetBuffer,
                                       std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "drone_station";
    if (object.linkTarget.empty()) {
        object.linkTarget = "salvage_sweep";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyRailDepotDescriptorPreset(bunker::MapObject& object,
                                    char* scriptTagBuffer,
                                    std::size_t scriptTagSize,
                                    char* linkTargetBuffer,
                                    std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "rail_depot";
    if (object.linkTarget.empty()) {
        object.linkTarget = "industrial_spur";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyOrbitalUplinkDescriptorPreset(bunker::MapObject& object,
                                        char* scriptTagBuffer,
                                        std::size_t scriptTagSize,
                                        char* linkTargetBuffer,
                                        std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "orbital_uplink";
    if (object.linkTarget.empty()) {
        object.linkTarget = "low_orbit_scan";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyRailFortressDescriptorPreset(bunker::MapObject& object,
                                       char* scriptTagBuffer,
                                       std::size_t scriptTagSize,
                                       char* linkTargetBuffer,
                                       std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "rail_fortress_hub";
    if (object.linkTarget.empty()) {
        object.linkTarget = "magistral_anchor";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyRecoveryFabricatorDescriptorPreset(bunker::MapObject& object,
                                             char* scriptTagBuffer,
                                             std::size_t scriptTagSize,
                                             char* linkTargetBuffer,
                                             std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "recovery_fabricator";
    if (object.linkTarget.empty()) {
        object.linkTarget = "industrial_refinery";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyIndustrialGateDescriptorPreset(bunker::MapObject& object,
                                         char* scriptTagBuffer,
                                         std::size_t scriptTagSize,
                                         char* linkTargetBuffer,
                                         std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "industrial_gate";
    if (object.linkTarget.empty()) {
        object.linkTarget = "inner_spur";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyIndustrialSurveyDescriptorPreset(bunker::MapObject& object,
                                           char* scriptTagBuffer,
                                           std::size_t scriptTagSize,
                                           char* linkTargetBuffer,
                                           std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "industrial_survey";
    if (object.linkTarget.empty()) {
        object.linkTarget = "inner_spur_survey";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyIndustrialOutpostDescriptorPreset(bunker::MapObject& object,
                                            char* scriptTagBuffer,
                                            std::size_t scriptTagSize,
                                            char* linkTargetBuffer,
                                            std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "industrial_outpost";
    if (object.linkTarget.empty()) {
        object.linkTarget = "inner_spur_outpost";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyAssemblyCellDescriptorPreset(bunker::MapObject& object,
                                       char* scriptTagBuffer,
                                       std::size_t scriptTagSize,
                                       char* linkTargetBuffer,
                                       std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "assembly_cell";
    if (object.linkTarget.empty()) {
        object.linkTarget = "inner_spur_assembly";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyFoundryLineDescriptorPreset(bunker::MapObject& object,
                                      char* scriptTagBuffer,
                                      std::size_t scriptTagSize,
                                      char* linkTargetBuffer,
                                      std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "foundry_line";
    if (object.linkTarget.empty()) {
        object.linkTarget = "inner_spur_foundry";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyReactorYardDescriptorPreset(bunker::MapObject& object,
                                      char* scriptTagBuffer,
                                      std::size_t scriptTagSize,
                                      char* linkTargetBuffer,
                                      std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "reactor_yard";
    if (object.linkTarget.empty()) {
        object.linkTarget = "inner_spur_reactor";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyCapacitorBankDescriptorPreset(bunker::MapObject& object,
                                        char* scriptTagBuffer,
                                        std::size_t scriptTagSize,
                                        char* linkTargetBuffer,
                                        std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "capacitor_bank";
    if (object.linkTarget.empty()) {
        object.linkTarget = "inner_spur_capacitor";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyRelaySubstationDescriptorPreset(bunker::MapObject& object,
                                          char* scriptTagBuffer,
                                          std::size_t scriptTagSize,
                                          char* linkTargetBuffer,
                                          std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "relay_substation";
    if (object.linkTarget.empty()) {
        object.linkTarget = "shelter17_backbone";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyServiceBayDescriptorPreset(bunker::MapObject& object,
                                     char* scriptTagBuffer,
                                     std::size_t scriptTagSize,
                                     char* linkTargetBuffer,
                                     std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "service_bay";
    if (object.linkTarget.empty()) {
        object.linkTarget = "inner_spur_service";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyWaterReclaimerDescriptorPreset(bunker::MapObject& object,
                                         char* scriptTagBuffer,
                                         std::size_t scriptTagSize,
                                         char* linkTargetBuffer,
                                         std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "water_reclaimer";
    if (object.linkTarget.empty()) {
        object.linkTarget = "inner_spur_water";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyServiceHubDescriptorPreset(bunker::MapObject& object,
                                     char* scriptTagBuffer,
                                     std::size_t scriptTagSize,
                                     char* linkTargetBuffer,
                                     std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "lanline_service_hub";
    if (object.linkTarget.empty()) {
        object.linkTarget = "shelter17_services";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyFeyRingDescriptorPreset(bunker::MapObject& object,
                                  char* scriptTagBuffer,
                                  std::size_t scriptTagSize,
                                  char* linkTargetBuffer,
                                  std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Landmark;
    object.scriptTag = "fey_ring";
    if (object.linkTarget.empty()) {
        object.linkTarget = "intercity_ring";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyMedicalSupportDescriptorPreset(bunker::MapObject& object,
                                         char* scriptTagBuffer,
                                         std::size_t scriptTagSize,
                                         char* linkTargetBuffer,
                                         std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "medical_support";
    if (object.linkTarget.empty()) {
        object.linkTarget = "field_medical";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyTankServiceDescriptorPreset(bunker::MapObject& object,
                                      char* scriptTagBuffer,
                                      std::size_t scriptTagSize,
                                      char* linkTargetBuffer,
                                      std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Workshop;
    object.category = bunker::ObjectCategory::Hangar;
    object.scriptTag = "tank_service";
    if (object.linkTarget.empty()) {
        object.linkTarget = "bt72_service";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyRemoteLinkDescriptorPreset(bunker::MapObject& object,
                                     char* scriptTagBuffer,
                                     std::size_t scriptTagSize,
                                     char* linkTargetBuffer,
                                     std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "remote_link";
    if (object.linkTarget.empty()) {
        object.linkTarget = "gate_control";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyEchoDescriptorPreset(bunker::MapObject& object,
                               char* scriptTagBuffer,
                               std::size_t scriptTagSize,
                               char* linkTargetBuffer,
                               std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "echo_trace";
    if (object.linkTarget.empty()) {
        object.linkTarget = "hidden_cache";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplySpecialistDescriptorPreset(bunker::MapObject& object,
                                     char* scriptTagBuffer,
                                     std::size_t scriptTagSize,
                                     char* linkTargetBuffer,
                                     std::size_t linkTargetSize) {
    object.interaction = bunker::InteractionType::Terminal;
    object.category = bunker::ObjectCategory::Terminal;
    object.scriptTag = "specialist_cryo";
    if (object.linkTarget.empty()) {
        object.linkTarget = "engineer";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

std::string ToLowerCopy(std::string value) {
    for (char& ch : value) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

bool ContainsCaseInsensitive(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) {
        return true;
    }
    return ToLowerCopy(haystack).find(ToLowerCopy(needle)) != std::string::npos;
}

bool IsBlank(const char* text) {
    if (text == nullptr) {
        return true;
    }
    while (*text != '\0') {
        if (!std::isspace(static_cast<unsigned char>(*text))) {
            return false;
        }
        ++text;
    }
    return true;
}

std::string TrimCopy(const char* text) {
    if (text == nullptr) {
        return {};
    }
    std::string value(text);
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string NormalizeExportWorldName(const char* exportWorldFileInput) {
    std::string exportName = TrimCopy(exportWorldFileInput);
    if (exportName.empty()) {
        return {};
    }
    if (!exportName.ends_with(".bwld")) {
        exportName += ".bwld";
    }
    return bunker::NormalizeWorldReference(exportName);
}

std::string MakeDuplicateRegistryId(const bunker::World& world, const std::string& sourceRegistryId) {
    std::string stem = sourceRegistryId;
    if (!stem.empty() && stem.back() == ']') {
        stem.pop_back();
    }

    for (int copyIndex = 1; copyIndex < 10000; ++copyIndex) {
        std::string candidate = stem + "_copy_" + std::to_string(copyIndex);
        if (!sourceRegistryId.empty() && sourceRegistryId.back() == ']') {
            candidate += "]";
        }
        if (!world.HasObject(candidate)) {
            return candidate;
        }
    }

    return stem + "_copy_overflow";
}

bool HasOtherObjectWithRegistryId(const bunker::World& world, const std::string& registryId, int selectedIndex) {
    for (int index = 0; index < static_cast<int>(world.objects.size()); ++index) {
        if (index == selectedIndex) {
            continue;
        }
        if (world.objects[static_cast<std::size_t>(index)].registryId == registryId) {
            return true;
        }
    }
    return false;
}

bool LoadPrefabLibrary(std::vector<SavedPrefab>& prefabs) {
    prefabs.clear();

    std::ifstream file(bunker::EditorPrefabLibraryPath());
    if (!file.is_open()) {
        return false;
    }

    SavedPrefab prefab;
    while (file >> std::quoted(prefab.label)
                >> std::quoted(prefab.object.registryId)
                >> std::quoted(prefab.object.displayName)
                >> std::quoted(prefab.object.scriptTag)
                >> std::quoted(prefab.object.linkTarget)) {
        int interaction = 0;
        int category = 0;
        file >> interaction
             >> category
             >> prefab.object.x
             >> prefab.object.y
             >> prefab.object.z
             >> prefab.object.width
             >> prefab.object.depth
             >> prefab.object.height
             >> prefab.object.health
             >> prefab.object.blocksMovement
             >> prefab.object.discovered
             >> prefab.object.manualLoot;
        if (!file) {
            break;
        }

        prefab.object.interaction = static_cast<bunker::InteractionType>(interaction);
        prefab.object.category = static_cast<bunker::ObjectCategory>(category);
        for (auto& lootId : prefab.object.manualLootIds) {
            if (!(file >> std::quoted(lootId))) {
                return false;
            }
        }
        prefabs.push_back(prefab);
    }

    return true;
}

bool SavePrefabLibrary(const std::vector<SavedPrefab>& prefabs) {
    std::ofstream file(bunker::EditorPrefabLibraryPath());
    if (!file.is_open()) {
        return false;
    }

    for (const auto& prefab : prefabs) {
        file << std::quoted(prefab.label) << ' '
             << std::quoted(prefab.object.registryId) << ' '
             << std::quoted(prefab.object.displayName) << ' '
             << std::quoted(prefab.object.scriptTag) << ' '
             << std::quoted(prefab.object.linkTarget) << ' '
             << static_cast<int>(prefab.object.interaction) << ' '
             << static_cast<int>(prefab.object.category) << ' '
             << prefab.object.x << ' '
             << prefab.object.y << ' '
             << prefab.object.z << ' '
             << prefab.object.width << ' '
             << prefab.object.depth << ' '
             << prefab.object.height << ' '
             << prefab.object.health << ' '
             << prefab.object.blocksMovement << ' '
             << prefab.object.discovered << ' '
             << prefab.object.manualLoot;
        for (const auto& lootId : prefab.object.manualLootIds) {
            file << ' ' << std::quoted(lootId);
        }
        file << '\n';
    }

    return static_cast<bool>(file);
}

ImU32 ColorForCategory(bunker::ObjectCategory category) {
    switch (category) {
        case bunker::ObjectCategory::Structure: return IM_COL32(110, 120, 140, 255);
        case bunker::ObjectCategory::ResourceNode: return IM_COL32(180, 140, 60, 255);
        case bunker::ObjectCategory::Terminal: return IM_COL32(40, 170, 210, 255);
        case bunker::ObjectCategory::Vehicle: return IM_COL32(170, 170, 80, 255);
        case bunker::ObjectCategory::Landmark: return IM_COL32(80, 180, 110, 255);
        case bunker::ObjectCategory::Container: return IM_COL32(170, 80, 60, 255);
        case bunker::ObjectCategory::Hangar: return IM_COL32(150, 120, 70, 255);
        case bunker::ObjectCategory::Hostile: return IM_COL32(200, 60, 60, 255);
    }
    return IM_COL32(150, 150, 150, 255);
}

const char* InteractionMarker(bunker::InteractionType interaction) {
    switch (interaction) {
        case bunker::InteractionType::Static: return "S";
        case bunker::InteractionType::Container: return "C";
        case bunker::InteractionType::Resource: return "R";
        case bunker::InteractionType::Terminal: return "T";
        case bunker::InteractionType::Transition: return "X";
        case bunker::InteractionType::VehicleAnchor: return "V";
        case bunker::InteractionType::Workshop: return "W";
        case bunker::InteractionType::Hostile: return "H";
    }
    return "?";
}

ImU32 InteractionMarkerColor(bunker::InteractionType interaction) {
    switch (interaction) {
        case bunker::InteractionType::Static: return IM_COL32(190, 190, 190, 210);
        case bunker::InteractionType::Container: return IM_COL32(214, 120, 90, 230);
        case bunker::InteractionType::Resource: return IM_COL32(212, 178, 90, 230);
        case bunker::InteractionType::Terminal: return IM_COL32(80, 210, 230, 230);
        case bunker::InteractionType::Transition: return IM_COL32(110, 220, 140, 230);
        case bunker::InteractionType::VehicleAnchor: return IM_COL32(220, 210, 90, 230);
        case bunker::InteractionType::Workshop: return IM_COL32(255, 170, 70, 230);
        case bunker::InteractionType::Hostile: return IM_COL32(230, 80, 80, 230);
    }
    return IM_COL32(220, 220, 220, 210);
}

PreviewInteraction DrawWorldPreview(const bunker::World& world,
                                    int selectedObjectIndex,
                                    bool previewAsPlayer,
                                    PreviewViewportState& viewportState,
                                    bool showInteractionHelpers,
                                    bool showObjectLabels) {
    PreviewInteraction interactionResult;
    ImGui::Text("World Preview");
    ImGui::TextDisabled(previewAsPlayer ? "Preview: player readability" : "Preview: editor overview");

    const ImVec2 canvasSize(0.0f, 220.0f);
    ImGui::BeginChild("WorldPreviewCanvas", canvasSize, true);
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 size = ImGui::GetContentRegionAvail();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(origin, ImVec2(origin.x + size.x, origin.y + size.y), IM_COL32(18, 22, 28, 255));
    drawList->AddRect(origin, ImVec2(origin.x + size.x, origin.y + size.y), IM_COL32(60, 80, 96, 255));

    float minX = world.metadata.playerSpawnX;
    float maxX = world.metadata.playerSpawnX;
    float minY = world.metadata.playerSpawnY;
    float maxY = world.metadata.playerSpawnY;
    for (const auto& object : world.objects) {
        minX = std::min(minX, object.x - object.width);
        maxX = std::max(maxX, object.x + object.width);
        minY = std::min(minY, object.y - object.depth);
        maxY = std::max(maxY, object.y + object.depth);
    }

    const float worldWidth = std::max(1.0f, maxX - minX);
    const float worldHeight = std::max(1.0f, maxY - minY);
    const float scaleX = (size.x - 12.0f) / worldWidth;
    const float scaleY = (size.y - 12.0f) / worldHeight;
    const float baseScale = std::max(1.0f, std::min(scaleX, scaleY));
    const float scale = std::max(0.25f, baseScale * viewportState.zoom);

    auto toCanvas = [&](float worldX, float worldY) {
        const float px = origin.x + 6.0f + viewportState.offsetX + ((worldX - minX) * scale);
        const float py = origin.y + 6.0f + viewportState.offsetY + ((worldY - minY) * scale);
        return ImVec2(px, py);
    };

    auto toWorld = [&](const ImVec2& canvasPoint) {
        const float worldX = minX + ((canvasPoint.x - (origin.x + 6.0f) - viewportState.offsetX) / scale);
        const float worldY = minY + ((canvasPoint.y - (origin.y + 6.0f) - viewportState.offsetY) / scale);
        return ImVec2(worldX, worldY);
    };

    if (ImGui::IsWindowHovered()) {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            viewportState.zoom = std::clamp(viewportState.zoom + (wheel * 0.1f), 0.5f, 3.0f);
        }
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
            const ImVec2 drag = ImGui::GetIO().MouseDelta;
            viewportState.offsetX += drag.x;
            viewportState.offsetY += drag.y;
        }
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        const ImVec2 mousePos = ImGui::GetIO().MousePos;
        if (mousePos.x >= origin.x && mousePos.x <= origin.x + size.x &&
            mousePos.y >= origin.y && mousePos.y <= origin.y + size.y) {
            interactionResult.clicked = true;
            const ImVec2 worldPoint = toWorld(mousePos);
            interactionResult.worldX = worldPoint.x;
            interactionResult.worldY = worldPoint.y;
            for (int index = static_cast<int>(world.objects.size()) - 1; index >= 0; --index) {
                const auto& object = world.objects[static_cast<std::size_t>(index)];
                const float minObjectX = object.x - (object.width * 0.5f);
                const float maxObjectX = object.x + (object.width * 0.5f);
                const float minObjectY = object.y - (object.depth * 0.5f);
                const float maxObjectY = object.y + (object.depth * 0.5f);
                if (worldPoint.x >= minObjectX && worldPoint.x <= maxObjectX &&
                    worldPoint.y >= minObjectY && worldPoint.y <= maxObjectY) {
                    interactionResult.clickedObject = true;
                    interactionResult.clickedObjectIndex = index;
                    interactionResult.doubleClickedObject = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
                    break;
                }
            }
        }
    }

    if (ImGui::IsWindowHovered() &&
        selectedObjectIndex >= 0 &&
        selectedObjectIndex < static_cast<int>(world.objects.size()) &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        const ImVec2 mousePos = ImGui::GetIO().MousePos;
        if (mousePos.x >= origin.x && mousePos.x <= origin.x + size.x &&
            mousePos.y >= origin.y && mousePos.y <= origin.y + size.y) {
            const auto& selectedObject = world.objects[static_cast<std::size_t>(selectedObjectIndex)];
            const ImVec2 selectedCenter = toCanvas(selectedObject.x, selectedObject.y);
            const float pickRadiusX = std::max(8.0f, selectedObject.width * 0.5f * scale);
            const float pickRadiusY = std::max(8.0f, selectedObject.depth * 0.5f * scale);
            if (std::abs(mousePos.x - selectedCenter.x) <= pickRadiusX &&
                std::abs(mousePos.y - selectedCenter.y) <= pickRadiusY) {
                const ImVec2 worldPoint = toWorld(mousePos);
                interactionResult.draggingSelectedObject = true;
                interactionResult.worldX = worldPoint.x;
                interactionResult.worldY = worldPoint.y;
            }
        }
    }

    if (!world.objects.empty()) {
        for (int index = 0; index < static_cast<int>(world.objects.size()); ++index) {
            const auto& object = world.objects[static_cast<std::size_t>(index)];
            const ImVec2 center = toCanvas(object.x, object.y);
            const float halfW = std::max(2.0f, object.width * 0.5f * scale);
            const float halfH = std::max(2.0f, object.depth * 0.5f * scale);
            const ImVec2 min(center.x - halfW, center.y - halfH);
            const ImVec2 max(center.x + halfW, center.y + halfH);
            drawList->AddRectFilled(min, max, ColorForCategory(object.category), 2.0f);
            if (selectedObjectIndex == index) {
                drawList->AddRect(min, max, IM_COL32(255, 240, 110, 255), 2.0f, 0, 2.0f);
            }
            if (showInteractionHelpers) {
                const ImU32 markerColor = InteractionMarkerColor(object.interaction);
                drawList->AddCircleFilled(ImVec2(center.x, center.y - halfH - 10.0f), 8.0f, markerColor);
                drawList->AddText(ImVec2(center.x - 3.0f, center.y - halfH - 16.0f), IM_COL32(20, 20, 20, 255), InteractionMarker(object.interaction));
            }
            if (showObjectLabels || selectedObjectIndex == index) {
                drawList->AddText(ImVec2(center.x + halfW + 4.0f, center.y - 6.0f), IM_COL32(220, 220, 220, 220), object.displayName.c_str());
                if (selectedObjectIndex == index && !object.scriptTag.empty()) {
                    drawList->AddText(ImVec2(center.x + halfW + 4.0f, center.y + 8.0f), IM_COL32(150, 210, 180, 220), object.scriptTag.c_str());
                }
                if (selectedObjectIndex == index && !object.linkTarget.empty()) {
                    const std::string routeLabel = "-> " + object.linkTarget;
                    drawList->AddText(ImVec2(center.x + halfW + 4.0f, center.y + 22.0f), IM_COL32(140, 190, 230, 220), routeLabel.c_str());
                }
            }
        }

        const ImVec2 spawnPoint = toCanvas(world.metadata.playerSpawnX, world.metadata.playerSpawnY);
        drawList->AddCircleFilled(spawnPoint, 5.0f, IM_COL32(120, 255, 170, 255));
        drawList->AddCircle(spawnPoint, 9.0f, IM_COL32(120, 255, 170, 180), 0, 2.0f);
        drawList->AddText(ImVec2(spawnPoint.x + 8.0f, spawnPoint.y - 8.0f), IM_COL32(120, 255, 170, 220), "SPAWN");
    } else {
        drawList->AddText(ImVec2(origin.x + 12.0f, origin.y + 12.0f), IM_COL32(180, 180, 180, 255), "No objects in world preview.");
    }

    ImGui::Dummy(size);
    ImGui::EndChild();
    return interactionResult;
}

}  // namespace

int main() {
    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1420, 900, "BunkerEditor", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    bunker::EnsureProjectDirectories();

    char conceptInput[256] = "reference_tank_side.png";
    int targetTypeIndex = 0;
    int completionIndex = 1;
    bool showImportAssistant = true;
    bool previewAsPlayer = true;
    bool snapToGrid = true;
    bool aiPathPreview = true;
    bool showInteractionHelpers = true;
    bool showObjectLabels = false;
    PreviewViewportState previewViewport;
    std::vector<ImportedConcept> importedConcepts;
    std::vector<SavedPrefab> savedPrefabs;
    std::string statusText = "Editor ready. Prepare assets or export a runtime prototype.";

    bunker::World editorWorld;
    LoadOrCreateEditorWorld(editorWorld, statusText);
    LoadPrefabLibrary(savedPrefabs);

    const char* targetTypes[] = {
        "Placeable Object",
        "Usable Item",
        "Vehicle Module",
        "Location Blockout",
        "Terminal/Interactive",
    };

    const char* completionModes[] = {
        "Mirror missing side",
        "Logical completion",
        "Keep as partial shell",
    };

    const std::array<ObjectPreset, 6> objectPresets = {{
        {"Structure", bunker::InteractionType::Static, bunker::ObjectCategory::Structure, 2.4f, 2.0f, 2.4f, 100.0f, true, false},
        {"Terminal", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal, 1.6f, 1.4f, 2.0f, 100.0f, false, false},
        {"Container", bunker::InteractionType::Container, bunker::ObjectCategory::Container, 1.4f, 1.2f, 1.2f, 70.0f, false, true},
        {"Transition", bunker::InteractionType::Transition, bunker::ObjectCategory::Landmark, 2.8f, 1.8f, 2.2f, 100.0f, false, false},
        {"Vehicle Anchor", bunker::InteractionType::VehicleAnchor, bunker::ObjectCategory::Vehicle, 3.2f, 2.0f, 2.0f, 180.0f, true, false},
        {"Resource Node", bunker::InteractionType::Resource, bunker::ObjectCategory::ResourceNode, 2.0f, 1.8f, 1.2f, 60.0f, true, true},
    }};
    const char* objectPresetLabels[] = {
        "Structure",
        "Terminal",
        "Container",
        "Transition",
        "Vehicle Anchor",
        "Resource Node",
    };

    int presetIndex = 0;
    int selectedObjectIndex = -1;
    int selectedPrefabIndex = -1;
    int selectedImportedConceptIndex = -1;
    int objectCategoryFilter = -1;
    bool useDraftInteractionOverride = false;
    bool useDraftCategoryOverride = false;
    bunker::InteractionType draftInteractionOverride = bunker::InteractionType::Static;
    bunker::ObjectCategory draftCategoryOverride = bunker::ObjectCategory::Structure;
    float placeX = 0.0f;
    float placeY = 0.0f;
    char registryInput[128] = "[%structure_1]";
    char objectNameInput[128] = "New Structure";
    char loot0[64] = "";
    char loot1[64] = "";
    char loot2[64] = "";
    char loot3[64] = "";
    char worldNameInput[128] = "";
    char worldBiomeInput[128] = "";
    char worldObjectiveInput[256] = "";
    char selectedRegistryEdit[128] = "";
    char selectedScriptTagEdit[128] = "";
    char selectedLinkTargetEdit[128] = "";
    float worldSpawnX = editorWorld.metadata.playerSpawnX;
    float worldSpawnY = editorWorld.metadata.playerSpawnY;
    char exportWorldFileInput[128] = "";
    char objectSearchInput[128] = "";
    char prefabLabelInput[128] = "New Prefab";
    CopyStringToBuffer(editorWorld.metadata.name, worldNameInput, IM_ARRAYSIZE(worldNameInput));
    CopyStringToBuffer(editorWorld.metadata.biome, worldBiomeInput, IM_ARRAYSIZE(worldBiomeInput));
    CopyStringToBuffer(editorWorld.metadata.objective, worldObjectiveInput, IM_ARRAYSIZE(worldObjectiveInput));
    CopyStringToBuffer("", selectedRegistryEdit, IM_ARRAYSIZE(selectedRegistryEdit));
    CopyStringToBuffer("", selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit));
    CopyStringToBuffer("", selectedLinkTargetEdit, IM_ARRAYSIZE(selectedLinkTargetEdit));
    {
        bunker::SessionProfile initialProfile;
        const auto profilePath = bunker::DefaultSessionProfilePath();
        if (!bunker::LoadSessionProfile(profilePath, initialProfile)) {
            initialProfile = bunker::MakeDefaultSessionProfile();
        }
        bunker::NormalizeSessionProfile(initialProfile);
        CopyStringToBuffer(initialProfile.selectedWorld, exportWorldFileInput, IM_ARRAYSIZE(exportWorldFileInput));
    }
    const char* interactionLabels[] = {"Static", "Container", "Resource", "Terminal", "Transition", "Vehicle Anchor", "Workshop", "Hostile"};
    const char* categoryLabels[] = {"Structure", "Resource Node", "Terminal", "Vehicle", "Landmark", "Container", "Hangar", "Hostile"};
    const char* objectFilterLabels[] = {"All Categories", "Structure", "Resource Node", "Terminal", "Vehicle", "Landmark", "Container", "Hangar", "Hostile"};

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glViewport(0, 0, 1420, 900);
        glClearColor(0.07f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::SetNextWindowPos(ImVec2(16.0f, 16.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(320.0f, 868.0f), ImGuiCond_Always);
        ImGui::Begin("Asset Palette", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Creation Kit Style Toolset");
        ImGui::Separator();
        ImGui::BulletText("World cells and landmark placement");
        ImGui::BulletText("Interactive objects and containers");
        ImGui::BulletText("Vehicle hulls, modules, and tank wrecks");
        ImGui::BulletText("Item authoring and loot setup");
        ImGui::BulletText("Scene export for BunkerGame runtime");
        ImGui::Separator();
        ImGui::Checkbox("Snap to grid", &snapToGrid);
        ImGui::Checkbox("AI path preview", &aiPathPreview);
        ImGui::Checkbox("Preview as player", &previewAsPlayer);
        ImGui::Checkbox("Show interaction helpers", &showInteractionHelpers);
        ImGui::Checkbox("Show object labels", &showObjectLabels);
        ImGui::Checkbox("Import assistant window", &showImportAssistant);
        ImGui::Separator();
        ImGui::Text("Object Authoring");
        ImGui::Combo("Preset", &presetIndex, objectPresetLabels, IM_ARRAYSIZE(objectPresetLabels));
        ImGui::TextDisabled("Quick gameplay drafts");
        if (ImGui::Button("Terminal Draft", ImVec2(150.0f, 0.0f))) {
            presetIndex = 1;
            PrepareSpecializedDraft("Operations Terminal", "[%terminal_ops_0001]", placeX, placeY,
                                    objectNameInput, IM_ARRAYSIZE(objectNameInput),
                                    registryInput, IM_ARRAYSIZE(registryInput),
                                    loot0, loot1, loot2, loot3);
            useDraftInteractionOverride = true;
            useDraftCategoryOverride = true;
            draftInteractionOverride = bunker::InteractionType::Terminal;
            draftCategoryOverride = bunker::ObjectCategory::Terminal;
            statusText = "Prepared terminal draft.";
        }
        ImGui::SameLine();
        if (ImGui::Button("Transition Draft", ImVec2(150.0f, 0.0f))) {
            presetIndex = 3;
            PrepareSpecializedDraft("Bulkhead Transition", "[%transition_gate_0001]", placeX, placeY,
                                    objectNameInput, IM_ARRAYSIZE(objectNameInput),
                                    registryInput, IM_ARRAYSIZE(registryInput),
                                    loot0, loot1, loot2, loot3);
            useDraftInteractionOverride = true;
            useDraftCategoryOverride = true;
            draftInteractionOverride = bunker::InteractionType::Transition;
            draftCategoryOverride = bunker::ObjectCategory::Landmark;
            statusText = "Prepared transition draft.";
        }
        if (ImGui::Button("Workshop Draft", ImVec2(150.0f, 0.0f))) {
            presetIndex = 1;
            PrepareSpecializedDraft("Field Workshop", "[%workshop_0001]", placeX, placeY,
                                    objectNameInput, IM_ARRAYSIZE(objectNameInput),
                                    registryInput, IM_ARRAYSIZE(registryInput),
                                    loot0, loot1, loot2, loot3);
            useDraftInteractionOverride = true;
            useDraftCategoryOverride = true;
            draftInteractionOverride = bunker::InteractionType::Workshop;
            draftCategoryOverride = bunker::ObjectCategory::Terminal;
            statusText = "Prepared workshop draft.";
        }
        ImGui::SameLine();
        if (ImGui::Button("Hostile Draft", ImVec2(150.0f, 0.0f))) {
            presetIndex = 0;
            PrepareSpecializedDraft("Hostile Contact", "[%hostile_0001]", placeX, placeY,
                                    objectNameInput, IM_ARRAYSIZE(objectNameInput),
                                    registryInput, IM_ARRAYSIZE(registryInput),
                                    loot0, loot1, loot2, loot3);
            useDraftInteractionOverride = true;
            useDraftCategoryOverride = true;
            draftInteractionOverride = bunker::InteractionType::Hostile;
            draftCategoryOverride = bunker::ObjectCategory::Hostile;
            statusText = "Prepared hostile draft.";
        }
        ImGui::InputText("Registry ID", registryInput, IM_ARRAYSIZE(registryInput));
        ImGui::InputText("Display Name", objectNameInput, IM_ARRAYSIZE(objectNameInput));
        ImGui::InputFloat("Place X", &placeX, 0.5f, 2.0f, "%.1f");
        ImGui::InputFloat("Place Y", &placeY, 0.5f, 2.0f, "%.1f");
        if (SelectedPreset(objectPresets, presetIndex).manualLoot) {
            ImGui::InputText("Loot 1", loot0, IM_ARRAYSIZE(loot0));
            ImGui::InputText("Loot 2", loot1, IM_ARRAYSIZE(loot1));
            ImGui::InputText("Loot 3", loot2, IM_ARRAYSIZE(loot2));
            ImGui::InputText("Loot 4", loot3, IM_ARRAYSIZE(loot3));
        }
        if (ImGui::Button("Reset Draft", ImVec2(-1.0f, 32.0f))) {
            CopyStringToBuffer(DefaultRegistryIdForPreset(presetIndex, editorWorld.objects.size()), registryInput, IM_ARRAYSIZE(registryInput));
            CopyStringToBuffer(std::string(objectPresetLabels[presetIndex]) + " Draft", objectNameInput, IM_ARRAYSIZE(objectNameInput));
            placeX = 0.0f;
            placeY = 0.0f;
            useDraftInteractionOverride = false;
            useDraftCategoryOverride = false;
            loot0[0] = '\0';
            loot1[0] = '\0';
            loot2[0] = '\0';
            loot3[0] = '\0';
            statusText = "Draft object fields reset.";
        }
        if (ImGui::Button("Add Object To World", ImVec2(-1.0f, 36.0f))) {
            if (IsBlank(registryInput)) {
                statusText = "Registry ID cannot be empty for a world object.";
            } else if (!bunker::RegistryId::IsValid(registryInput)) {
                statusText = std::string("Registry ID format is invalid: ") + registryInput;
            } else if (IsBlank(objectNameInput)) {
                statusText = "Display name cannot be empty for a world object.";
            } else if (editorWorld.HasObject(registryInput)) {
                statusText = std::string("Duplicate Registry ID blocked: ") + registryInput;
                ImGui::OpenPopup("Duplicate Registry ID");
            } else {
                const auto& preset = SelectedPreset(objectPresets, presetIndex);
                bunker::MapObject object;
                object.registryId = registryInput;
                object.displayName = objectNameInput;
                object.interaction = preset.interaction;
                object.category = preset.category;
                if (useDraftInteractionOverride) {
                    object.interaction = draftInteractionOverride;
                }
                if (useDraftCategoryOverride) {
                    object.category = draftCategoryOverride;
                }
                object.x = snapToGrid ? std::round(placeX) : placeX;
                object.y = snapToGrid ? std::round(placeY) : placeY;
                object.width = preset.width;
                object.depth = preset.depth;
                object.height = preset.height;
                object.health = preset.health;
                object.blocksMovement = preset.blocksMovement;
                object.discovered = true;
                object.manualLoot = preset.manualLoot;
                object.manualLootIds = {loot0, loot1, loot2, loot3};
                editorWorld.AddObject(object);
                selectedObjectIndex = static_cast<int>(editorWorld.objects.size()) - 1;
                CopyStringToBuffer(editorWorld.objects.back().registryId, selectedRegistryEdit, IM_ARRAYSIZE(selectedRegistryEdit));
                CopyStringToBuffer(editorWorld.objects.back().scriptTag, selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit));
                CopyStringToBuffer(editorWorld.objects.back().linkTarget, selectedLinkTargetEdit, IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Added object to editor world and focused selection: " + object.displayName;
                CopyStringToBuffer(DefaultRegistryIdForPreset(presetIndex, editorWorld.objects.size()), registryInput, IM_ARRAYSIZE(registryInput));
                useDraftInteractionOverride = false;
                useDraftCategoryOverride = false;
            }
        }
        ImGui::Separator();
        ImGui::Text("Prefab Library");
        ImGui::InputText("Prefab Label", prefabLabelInput, IM_ARRAYSIZE(prefabLabelInput));
        if (ImGui::Button("Place Selected Prefab", ImVec2(-1.0f, 32.0f))) {
            if (selectedPrefabIndex >= 0 && selectedPrefabIndex < static_cast<int>(savedPrefabs.size())) {
                bunker::MapObject placed = savedPrefabs[static_cast<std::size_t>(selectedPrefabIndex)].object;
                placed.registryId = MakeDuplicateRegistryId(editorWorld, placed.registryId);
                placed.x = snapToGrid ? std::round(placeX) : placeX;
                placed.y = snapToGrid ? std::round(placeY) : placeY;
                editorWorld.AddObject(placed);
                selectedObjectIndex = static_cast<int>(editorWorld.objects.size()) - 1;
                CopyStringToBuffer(editorWorld.objects.back().registryId, selectedRegistryEdit, IM_ARRAYSIZE(selectedRegistryEdit));
                CopyStringToBuffer(editorWorld.objects.back().displayName, objectNameInput, IM_ARRAYSIZE(objectNameInput));
                CopyStringToBuffer(editorWorld.objects.back().scriptTag, selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit));
                CopyStringToBuffer(editorWorld.objects.back().linkTarget, selectedLinkTargetEdit, IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Placed prefab into world and focused selection: " + savedPrefabs[static_cast<std::size_t>(selectedPrefabIndex)].label;
            } else {
                statusText = "No prefab selected for placement.";
            }
        }
        if (ImGui::Button("Save Prefab Library", ImVec2(-1.0f, 32.0f))) {
            statusText = SavePrefabLibrary(savedPrefabs)
                ? "Prefab library saved to " + bunker::EditorPrefabLibraryPath().string()
                : "Failed to save prefab library.";
        }
        if (ImGui::BeginPopupModal("Duplicate Registry ID", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextWrapped("Another object already uses this Registry ID. Choose a different stable ID before adding the object.");
            if (ImGui::Button("Close", ImVec2(180.0f, 0.0f))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(352.0f, 16.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(720.0f, 868.0f), ImGuiCond_Always);
        ImGui::Begin("World Authoring", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Current workspace");
        ImGui::BulletText("Map: %s", editorWorld.metadata.name.c_str());
        ImGui::BulletText("Biome: %s", editorWorld.metadata.biome.c_str());
        ImGui::BulletText("Objects in workspace: %d", static_cast<int>(editorWorld.objects.size()));
        ImGui::BulletText("Preview mode: %s", previewAsPlayer ? "Player eye / tank checks enabled" : "Free editor camera");
        ImGui::Separator();
        ImGui::Text("World Metadata");
        if (ImGui::InputText("World Name", worldNameInput, IM_ARRAYSIZE(worldNameInput))) {
            editorWorld.metadata.name = worldNameInput;
        }
        if (ImGui::InputText("World Biome", worldBiomeInput, IM_ARRAYSIZE(worldBiomeInput))) {
            editorWorld.metadata.biome = worldBiomeInput;
        }
        if (ImGui::InputText("World Objective", worldObjectiveInput, IM_ARRAYSIZE(worldObjectiveInput))) {
            editorWorld.metadata.objective = worldObjectiveInput;
        }
        if (ImGui::InputFloat("Player Spawn X", &worldSpawnX, 0.5f, 2.0f, "%.1f")) {
            editorWorld.metadata.playerSpawnX = snapToGrid ? std::round(worldSpawnX) : worldSpawnX;
            worldSpawnX = editorWorld.metadata.playerSpawnX;
        }
        if (ImGui::InputFloat("Player Spawn Y", &worldSpawnY, 0.5f, 2.0f, "%.1f")) {
            editorWorld.metadata.playerSpawnY = snapToGrid ? std::round(worldSpawnY) : worldSpawnY;
            worldSpawnY = editorWorld.metadata.playerSpawnY;
        }
        ImGui::Separator();
        ImGui::TextWrapped("This editor is intended to grow into the full content pipeline: maps, objects, items, terminals, loot, interactions, and export.");
        ImGui::Separator();
        ImGui::Text("World Object Library");
        ImGui::InputTextWithHint("Search", "Find by name or Registry ID", objectSearchInput, IM_ARRAYSIZE(objectSearchInput));
        ImGui::Combo("Category Filter", &objectCategoryFilter, objectFilterLabels, IM_ARRAYSIZE(objectFilterLabels));
        ImGui::BeginChild("WorldObjects", ImVec2(0.0f, 290.0f), true);
        int visibleObjectCount = 0;
        for (int index = 0; index < static_cast<int>(editorWorld.objects.size()); ++index) {
            const auto& object = editorWorld.objects[static_cast<std::size_t>(index)];
            if (objectCategoryFilter >= 0 && ToIndex(object.category) != objectCategoryFilter) {
                continue;
            }
            if (!ContainsCaseInsensitive(object.displayName, objectSearchInput) &&
                !ContainsCaseInsensitive(object.registryId, objectSearchInput)) {
                continue;
            }
            ++visibleObjectCount;
            const bool selected = (selectedObjectIndex == index);
            if (ImGui::Selectable((object.displayName + "##" + object.registryId).c_str(), selected)) {
                selectedObjectIndex = index;
                CopyStringToBuffer(object.registryId, selectedRegistryEdit, IM_ARRAYSIZE(selectedRegistryEdit));
                CopyStringToBuffer(object.scriptTag, selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit));
                CopyStringToBuffer(object.linkTarget, selectedLinkTargetEdit, IM_ARRAYSIZE(selectedLinkTargetEdit));
            }
        }
        if (visibleObjectCount == 0) {
            ImGui::TextDisabled("No objects match the current filter.");
        }
        ImGui::EndChild();
        if (selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(editorWorld.objects.size())) {
            auto& selectedObject = editorWorld.objects[static_cast<std::size_t>(selectedObjectIndex)];
            ImGui::TextWrapped("Selected: %s", selectedObject.displayName.c_str());
            ImGui::TextWrapped("Registry: %s", selectedObject.registryId.c_str());
            ImGui::Text("Pos %.1f %.1f | HP %.0f", selectedObject.x, selectedObject.y, selectedObject.health);
            ImGui::InputText("Edit Registry ID", selectedRegistryEdit, IM_ARRAYSIZE(selectedRegistryEdit));
            if (ImGui::Button("Apply Registry ID", ImVec2(-1.0f, 28.0f))) {
                if (IsBlank(selectedRegistryEdit)) {
                    statusText = "Registry ID cannot be empty.";
                } else if (!bunker::RegistryId::IsValid(selectedRegistryEdit)) {
                    statusText = std::string("Registry ID format is invalid: ") + selectedRegistryEdit;
                } else if (HasOtherObjectWithRegistryId(editorWorld, selectedRegistryEdit, selectedObjectIndex)) {
                    statusText = std::string("Duplicate Registry ID blocked: ") + selectedRegistryEdit;
                    ImGui::OpenPopup("Duplicate Registry ID");
                } else {
                    selectedObject.registryId = selectedRegistryEdit;
                    statusText = "Registry ID updated for selected object.";
                }
            }
            if (ImGui::InputText("Script Tag", selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit))) {
                selectedObject.scriptTag = selectedScriptTagEdit;
            }
            if (ImGui::InputText("Link Target", selectedLinkTargetEdit, IM_ARRAYSIZE(selectedLinkTargetEdit))) {
                selectedObject.linkTarget = selectedLinkTargetEdit;
            }
            ImGui::TextDisabled("Descriptor Presets");
            if (ImGui::Button("Archive Terminal", ImVec2(150.0f, 0.0f))) {
                selectedObject.registryId = selectedObject.registryId.empty() ? "[%archive_terminal]" : selectedObject.registryId;
                ApplyTerminalDescriptorPreset(selectedObject, selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit));
                selectedObject.scriptTag = "archive_sync";
                CopyStringToBuffer(selectedObject.registryId, selectedRegistryEdit, IM_ARRAYSIZE(selectedRegistryEdit));
                CopyStringToBuffer(selectedObject.scriptTag, selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit));
                statusText = "Applied archive terminal descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Transition Route", ImVec2(150.0f, 0.0f))) {
                ApplyTransitionDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied transition route descriptor preset.";
            }
            if (ImGui::Button("Workshop Service", ImVec2(150.0f, 0.0f))) {
                ApplyWorkshopDescriptorPreset(selectedObject, selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit));
                statusText = "Applied workshop service descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Generic Terminal", ImVec2(150.0f, 0.0f))) {
                ApplyTerminalDescriptorPreset(selectedObject, selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit));
                statusText = "Applied generic terminal descriptor preset.";
            }
            if (ImGui::Button("Tower Link", ImVec2(150.0f, 0.0f))) {
                ApplyTowerDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied radio tower descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Power Pylon", ImVec2(150.0f, 0.0f))) {
                ApplyPowerPylonDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied power pylon descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Drone Station", ImVec2(150.0f, 0.0f))) {
                ApplyDroneStationDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied drone station descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Rail Depot", ImVec2(150.0f, 0.0f))) {
                ApplyRailDepotDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied rail depot descriptor preset.";
            }
            if (ImGui::Button("Orbital Uplink", ImVec2(150.0f, 0.0f))) {
                ApplyOrbitalUplinkDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied orbital uplink descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Rail Fortress", ImVec2(150.0f, 0.0f))) {
                ApplyRailFortressDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied rail fortress descriptor preset.";
            }
            if (ImGui::Button("Fabricator", ImVec2(150.0f, 0.0f))) {
                ApplyRecoveryFabricatorDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied recovery fabricator descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Industrial Gate", ImVec2(150.0f, 0.0f))) {
                ApplyIndustrialGateDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied industrial gate descriptor preset.";
            }
            if (ImGui::Button("Survey Beacon", ImVec2(150.0f, 0.0f))) {
                ApplyIndustrialSurveyDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied industrial survey descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Inner Spur Outpost", ImVec2(150.0f, 0.0f))) {
                ApplyIndustrialOutpostDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied industrial outpost descriptor preset.";
            }
            if (ImGui::Button("Assembly Cell", ImVec2(150.0f, 0.0f))) {
                ApplyAssemblyCellDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied assembly cell descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Foundry Line", ImVec2(150.0f, 0.0f))) {
                ApplyFoundryLineDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied foundry line descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Reactor Yard", ImVec2(150.0f, 0.0f))) {
                ApplyReactorYardDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied reactor yard descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Capacitor Bank", ImVec2(150.0f, 0.0f))) {
                ApplyCapacitorBankDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied capacitor bank descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Relay Substation", ImVec2(150.0f, 0.0f))) {
                ApplyRelaySubstationDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied relay substation descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Service Bay", ImVec2(150.0f, 0.0f))) {
                ApplyServiceBayDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied service bay descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Water Reclaimer", ImVec2(150.0f, 0.0f))) {
                ApplyWaterReclaimerDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied water reclaimer descriptor preset.";
            }
            if (ImGui::Button("Remote Link", ImVec2(150.0f, 0.0f))) {
                ApplyRemoteLinkDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied remote link descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Service Hub", ImVec2(150.0f, 0.0f))) {
                ApplyServiceHubDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied Lanline service hub descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Fey Ring", ImVec2(150.0f, 0.0f))) {
                ApplyFeyRingDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied Fey Ring descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Medical Support", ImVec2(150.0f, 0.0f))) {
                ApplyMedicalSupportDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied medical support descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Tank Service", ImVec2(150.0f, 0.0f))) {
                ApplyTankServiceDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied tank service descriptor preset.";
            }
            if (ImGui::Button("Echo Trace", ImVec2(150.0f, 0.0f))) {
                ApplyEchoDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied Pip-Pad AR echo descriptor preset.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Cryo Specialist", ImVec2(150.0f, 0.0f))) {
                ApplySpecialistDescriptorPreset(
                    selectedObject,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit),
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Applied cryo specialist descriptor preset.";
            }
            int interactionIndex = ToIndex(selectedObject.interaction);
            if (ImGui::Combo("Interaction", &interactionIndex, interactionLabels, IM_ARRAYSIZE(interactionLabels))) {
                selectedObject.interaction = static_cast<bunker::InteractionType>(interactionIndex);
            }
            int categoryIndex = ToIndex(selectedObject.category);
            if (ImGui::Combo("Category", &categoryIndex, categoryLabels, IM_ARRAYSIZE(categoryLabels))) {
                selectedObject.category = CategoryFromIndex(categoryIndex);
            }
            if (ImGui::InputFloat("Edit X", &selectedObject.x, 0.5f, 2.0f, "%.1f") && snapToGrid) {
                selectedObject.x = std::round(selectedObject.x);
            }
            if (selectedObject.scriptTag == "fey_ring" && selectedObject.linkTarget.empty()) {
                ImGui::TextColored(ImVec4(0.92f, 0.65f, 0.24f, 1.0f), "Warning: fey_ring needs a linkTarget.");
            }
            if (selectedObject.scriptTag == "lanline_service_hub" && !editorWorld.HasScriptTag("tower_sync")) {
                ImGui::TextColored(ImVec4(0.92f, 0.65f, 0.24f, 1.0f), "Warning: lanline_service_hub is authored without tower_sync in this world.");
            }
            if (selectedObject.scriptTag == "tank_service" &&
                !editorWorld.HasScriptTag("service_bay") &&
                selectedObject.category != bunker::ObjectCategory::Hangar) {
                ImGui::TextColored(ImVec4(0.92f, 0.65f, 0.24f, 1.0f), "Warning: tank_service should live with service_bay coverage or a hangar-category anchor.");
            }
            if (ImGui::InputFloat("Edit Y", &selectedObject.y, 0.5f, 2.0f, "%.1f") && snapToGrid) {
                selectedObject.y = std::round(selectedObject.y);
            }
            ImGui::InputFloat("Width", &selectedObject.width, 0.1f, 0.5f, "%.1f");
            ImGui::InputFloat("Depth", &selectedObject.depth, 0.1f, 0.5f, "%.1f");
            ImGui::InputFloat("Height", &selectedObject.height, 0.1f, 0.5f, "%.1f");
            ImGui::InputFloat("Health", &selectedObject.health, 5.0f, 20.0f, "%.0f");
            ImGui::Checkbox("Blocks Movement", &selectedObject.blocksMovement);
            ImGui::Checkbox("Discovered", &selectedObject.discovered);
            ImGui::Checkbox("Manual Loot", &selectedObject.manualLoot);
            if (selectedObject.manualLoot) {
                char editLoot0[64] = "";
                char editLoot1[64] = "";
                char editLoot2[64] = "";
                char editLoot3[64] = "";
                CopyStringToBuffer(selectedObject.manualLootIds[0], editLoot0, IM_ARRAYSIZE(editLoot0));
                CopyStringToBuffer(selectedObject.manualLootIds[1], editLoot1, IM_ARRAYSIZE(editLoot1));
                CopyStringToBuffer(selectedObject.manualLootIds[2], editLoot2, IM_ARRAYSIZE(editLoot2));
                CopyStringToBuffer(selectedObject.manualLootIds[3], editLoot3, IM_ARRAYSIZE(editLoot3));
                if (ImGui::InputText("Edit Loot 1", editLoot0, IM_ARRAYSIZE(editLoot0))) {
                    selectedObject.manualLootIds[0] = editLoot0;
                }
                if (ImGui::InputText("Edit Loot 2", editLoot1, IM_ARRAYSIZE(editLoot1))) {
                    selectedObject.manualLootIds[1] = editLoot1;
                }
                if (ImGui::InputText("Edit Loot 3", editLoot2, IM_ARRAYSIZE(editLoot2))) {
                    selectedObject.manualLootIds[2] = editLoot2;
                }
                if (ImGui::InputText("Edit Loot 4", editLoot3, IM_ARRAYSIZE(editLoot3))) {
                    selectedObject.manualLootIds[3] = editLoot3;
                }
            }
            if (ImGui::Button("Use Selected Object As Player Spawn", ImVec2(-1.0f, 28.0f))) {
                editorWorld.metadata.playerSpawnX = snapToGrid ? std::round(selectedObject.x) : selectedObject.x;
                editorWorld.metadata.playerSpawnY = snapToGrid ? std::round(selectedObject.y) : selectedObject.y;
                worldSpawnX = editorWorld.metadata.playerSpawnX;
                worldSpawnY = editorWorld.metadata.playerSpawnY;
                statusText = "Player spawn moved to selected object.";
            }
            if (ImGui::Button("Duplicate Selected Object", ImVec2(-1.0f, 28.0f))) {
                bunker::MapObject duplicate = selectedObject;
                duplicate.registryId = MakeDuplicateRegistryId(editorWorld, selectedObject.registryId);
                duplicate.displayName += " Copy";
                duplicate.x += snapToGrid ? 1.0f : 0.75f;
                duplicate.y += snapToGrid ? 1.0f : 0.75f;
                if (snapToGrid) {
                    duplicate.x = std::round(duplicate.x);
                    duplicate.y = std::round(duplicate.y);
                }
                editorWorld.AddObject(duplicate);
                selectedObjectIndex = static_cast<int>(editorWorld.objects.size()) - 1;
                CopyStringToBuffer(editorWorld.objects.back().registryId, selectedRegistryEdit, IM_ARRAYSIZE(selectedRegistryEdit));
                CopyStringToBuffer(editorWorld.objects.back().scriptTag, selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit));
                CopyStringToBuffer(editorWorld.objects.back().linkTarget, selectedLinkTargetEdit, IM_ARRAYSIZE(selectedLinkTargetEdit));
                statusText = "Duplicated selected object with a new Registry ID.";
            }
            if (ImGui::Button("Capture Selected As Prefab", ImVec2(-1.0f, 28.0f))) {
                SavedPrefab prefab;
                prefab.label = (prefabLabelInput[0] != '\0') ? prefabLabelInput : selectedObject.displayName;
                prefab.object = selectedObject;
                savedPrefabs.push_back(prefab);
                selectedPrefabIndex = static_cast<int>(savedPrefabs.size()) - 1;
                statusText = "Captured selected object into prefab library.";
            }
            if (ImGui::Button("Delete Selected Object", ImVec2(-1.0f, 32.0f))) {
                statusText = "Deleted object: " + selectedObject.displayName;
                editorWorld.RemoveObject(selectedObject.registryId);
                selectedObjectIndex = -1;
                CopyStringToBuffer("", selectedRegistryEdit, IM_ARRAYSIZE(selectedRegistryEdit));
                CopyStringToBuffer("", selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit));
                CopyStringToBuffer("", selectedLinkTargetEdit, IM_ARRAYSIZE(selectedLinkTargetEdit));
            }
        } else {
            ImGui::TextDisabled("No object selected.");
        }
        ImGui::Separator();
        ImGui::Text("Prefab Backlog");
        if (savedPrefabs.empty()) {
            ImGui::TextDisabled("No prefabs captured yet.");
        } else {
            ImGui::BeginChild("PrefabLibraryList", ImVec2(0.0f, 100.0f), true);
            for (int index = 0; index < static_cast<int>(savedPrefabs.size()); ++index) {
                const auto& prefab = savedPrefabs[static_cast<std::size_t>(index)];
                const bool selected = (selectedPrefabIndex == index);
                if (ImGui::Selectable((prefab.label + "##prefab_" + std::to_string(index)).c_str(), selected)) {
                    selectedPrefabIndex = index;
                    CopyStringToBuffer(prefab.label, prefabLabelInput, IM_ARRAYSIZE(prefabLabelInput));
                }
            }
            ImGui::EndChild();
            if (selectedPrefabIndex >= 0 && selectedPrefabIndex < static_cast<int>(savedPrefabs.size())) {
                const auto& prefab = savedPrefabs[static_cast<std::size_t>(selectedPrefabIndex)];
                ImGui::TextWrapped("Prefab: %s", prefab.label.c_str());
                ImGui::TextWrapped("Seed Object: %s", prefab.object.displayName.c_str());
                ImGui::TextDisabled("Interaction: %s | Category: %s", ToLabel(prefab.object.interaction), ToLabel(prefab.object.category));
                if (ImGui::Button("Remove Selected Prefab", ImVec2(-1.0f, 28.0f))) {
                    savedPrefabs.erase(savedPrefabs.begin() + selectedPrefabIndex);
                    selectedPrefabIndex = -1;
                    statusText = "Removed prefab from library.";
                }
            }
        }
        ImGui::Separator();
        ImGui::Text("Imported concept backlog");
        if (importedConcepts.empty()) {
            ImGui::TextDisabled("No concepts prepared yet.");
        } else {
            ImGui::BeginChild("ImportedConcepts", ImVec2(0.0f, 92.0f), true);
            for (int index = 0; index < static_cast<int>(importedConcepts.size()); ++index) {
                const auto& imported = importedConcepts[static_cast<std::size_t>(index)];
                const bool selected = (selectedImportedConceptIndex == index);
                if (ImGui::Selectable((imported.sourceLabel + "##concept_" + std::to_string(index)).c_str(), selected)) {
                    selectedImportedConceptIndex = index;
                }
            }
            ImGui::EndChild();
            if (selectedImportedConceptIndex >= 0 && selectedImportedConceptIndex < static_cast<int>(importedConcepts.size())) {
                const auto& imported = importedConcepts[static_cast<std::size_t>(selectedImportedConceptIndex)];
                ImGui::TextWrapped("Concept: %s", imported.sourceLabel.c_str());
                ImGui::TextWrapped("Target: %s", imported.targetType.c_str());
                ImGui::TextWrapped("Completion: %s", imported.completionMode.c_str());
                if (ImGui::Button("Remove Selected Concept", ImVec2(-1.0f, 28.0f))) {
                    importedConcepts.erase(importedConcepts.begin() + selectedImportedConceptIndex);
                    selectedImportedConceptIndex = -1;
                    statusText = "Removed concept draft from backlog.";
                }
            }
            if (ImGui::Button("Clear Concept Backlog", ImVec2(-1.0f, 28.0f))) {
                importedConcepts.clear();
                selectedImportedConceptIndex = -1;
                statusText = "Cleared imported concept backlog.";
            }
        }
        ImGui::Separator();
        ImGui::Text("Preview checklist");
        ImGui::BulletText("1st person readability");
        ImGui::BulletText("3rd person scale");
        ImGui::BulletText("Cockpit/tank clearance");
        ImGui::BulletText("Collision sanity");
        ImGui::BulletText("Terminal reach and interact distance");
        if (showInteractionHelpers) {
            ImGui::Separator();
            ImGui::Text("Marker Legend");
            ImGui::BulletText("T = Terminal");
            ImGui::BulletText("X = Transition");
            ImGui::BulletText("W = Workshop");
            ImGui::BulletText("H = Hostile");
            ImGui::BulletText("C = Container");
            ImGui::BulletText("V = Vehicle Anchor");
        }
        ImGui::Separator();
        const PreviewInteraction previewInteraction = DrawWorldPreview(
            editorWorld,
            selectedObjectIndex,
            previewAsPlayer,
            previewViewport,
            showInteractionHelpers,
            showObjectLabels);
        if (previewInteraction.draggingSelectedObject &&
            selectedObjectIndex >= 0 &&
            selectedObjectIndex < static_cast<int>(editorWorld.objects.size())) {
            auto& draggedObject = editorWorld.objects[static_cast<std::size_t>(selectedObjectIndex)];
            draggedObject.x = snapToGrid ? std::round(previewInteraction.worldX) : previewInteraction.worldX;
            draggedObject.y = snapToGrid ? std::round(previewInteraction.worldY) : previewInteraction.worldY;
            placeX = draggedObject.x;
            placeY = draggedObject.y;
            statusText = "Moved selected object through world preview.";
        }
        if (previewInteraction.clicked) {
            placeX = snapToGrid ? std::round(previewInteraction.worldX) : previewInteraction.worldX;
            placeY = snapToGrid ? std::round(previewInteraction.worldY) : previewInteraction.worldY;
            if (previewInteraction.clickedObject &&
                previewInteraction.clickedObjectIndex >= 0 &&
                previewInteraction.clickedObjectIndex < static_cast<int>(editorWorld.objects.size())) {
                selectedObjectIndex = previewInteraction.clickedObjectIndex;
                CopyStringToBuffer(
                    editorWorld.objects[static_cast<std::size_t>(selectedObjectIndex)].registryId,
                    selectedRegistryEdit,
                    IM_ARRAYSIZE(selectedRegistryEdit));
                CopyStringToBuffer(
                    editorWorld.objects[static_cast<std::size_t>(selectedObjectIndex)].scriptTag,
                    selectedScriptTagEdit,
                    IM_ARRAYSIZE(selectedScriptTagEdit));
                CopyStringToBuffer(
                    editorWorld.objects[static_cast<std::size_t>(selectedObjectIndex)].linkTarget,
                    selectedLinkTargetEdit,
                    IM_ARRAYSIZE(selectedLinkTargetEdit));
                if (previewInteraction.doubleClickedObject) {
                    previewViewport.offsetX = 0.0f;
                    previewViewport.offsetY = 0.0f;
                    previewViewport.zoom = 1.4f;
                }
                statusText = "Selected object from world preview.";
            } else {
                statusText = "Preview cursor updated for object/prefab placement.";
            }
        }
        if (ImGui::Button("Reset View", ImVec2(120.0f, 0.0f))) {
            previewViewport = {};
            statusText = "Preview camera reset.";
        }
        ImGui::SameLine();
        ImGui::TextDisabled("LMB select/drag/place | MMB pan | Wheel zoom | Double-click focus");
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(1088.0f, 16.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(316.0f, 420.0f), ImGuiCond_Always);
        ImGui::Begin("Export / Runtime", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Export targets");
        ImGui::BulletText("World -> .bwld");
        ImGui::BulletText("Concept items -> palette records");
        ImGui::BulletText("Interactive terminals -> runtime descriptors");
        bunker::SessionProfile activeProfile;
        const auto activeProfilePath = bunker::DefaultSessionProfilePath();
        if (!bunker::LoadSessionProfile(activeProfilePath, activeProfile)) {
            activeProfile = bunker::MakeDefaultSessionProfile();
        }
        bunker::NormalizeSessionProfile(activeProfile);
        const auto* runtimeWorldState = bunker::FindWorldFieldState(activeProfile, activeProfile.selectedWorld);
        const bool runtimeBackboneStable = runtimeWorldState != nullptr &&
            bunker::IsStableRecoveryBackbone(activeProfile, *runtimeWorldState);
        const bool runtimeTradeOperational = runtimeWorldState != nullptr &&
            bunker::IsTradeNetworkOperational(activeProfile, *runtimeWorldState);
        const bool runtimeRailOperational = runtimeWorldState != nullptr &&
            bunker::IsRailFreightOperational(activeProfile, *runtimeWorldState);
        const bool runtimeOrbitalOperational = runtimeWorldState != nullptr &&
            bunker::IsOrbitalUplinkOperational(activeProfile, *runtimeWorldState);
        const bool runtimeFabricatorOperational = runtimeWorldState != nullptr &&
            bunker::IsRecoveryFabricatorOperational(activeProfile, *runtimeWorldState);
        const bool runtimeWaterOperational = runtimeWorldState != nullptr &&
            bunker::IsWaterReclaimerOperational(*runtimeWorldState);
        const bool runtimeWorldMatchesWorkspace = activeProfile.selectedWorld == NormalizeExportWorldName(exportWorldFileInput);
        const auto runtimeWorldPath = bunker::ResolveWorldPath(activeProfile.selectedWorld);
        bunker::World runtimeWorldPreview;
        const bool runtimeWorldLoaded = runtimeWorldPreview.Load(runtimeWorldPath.string());
        const std::string runtimeObjective = runtimeWorldLoaded
            ? runtimeWorldPreview.metadata.objective
            : "Runtime world objective unavailable until a valid .bwld is present.";
        ImGui::Separator();
        ImGui::TextWrapped("Active runtime world: %s", activeProfile.selectedWorld.c_str());
        ImGui::TextWrapped("Runtime world path: %s", runtimeWorldPath.string().c_str());
        ImGui::TextWrapped("Runtime objective: %s", runtimeObjective.c_str());
        ImGui::TextWrapped("Workspace objective target: %s", editorWorld.metadata.objective.c_str());
        ImGui::TextWrapped("Workspace export target: %s", NormalizeExportWorldName(exportWorldFileInput).c_str());
        ImGui::BulletText("Runtime/world handoff: %s", runtimeWorldMatchesWorkspace ? "aligned" : "drifted");
        ImGui::BulletText("Runtime file exists: %s", std::filesystem::exists(runtimeWorldPath) ? "yes" : "no");
        ImGui::BulletText("Recovery status: %s",
            runtimeBackboneStable ? "stable backbone" :
            (activeProfile.story.returnedToBase ? "recovery buildout active" : "starter route"));
        ImGui::BulletText("Trade loop: %s", runtimeTradeOperational ? "operational" : "not operational");
        ImGui::BulletText("Rail loop: %s", runtimeRailOperational ? "operational" : "not operational");
        ImGui::BulletText("Orbital support: %s", runtimeOrbitalOperational ? "operational" : "not operational");
        ImGui::BulletText("Fabricator: %s", runtimeFabricatorOperational ? "operational" : "not operational");
        ImGui::BulletText("Water reclaimer: %s", runtimeWaterOperational ? "operational" : "not operational");
        ImGui::TextWrapped("Prefab library: %s", bunker::EditorPrefabLibraryPath().string().c_str());
        ImGui::TextWrapped("Concept manifest: %s", bunker::EditorConceptManifestPath().string().c_str());
        ImGui::Separator();
        if (ImGui::Button("Adopt Runtime Objective Into Workspace", ImVec2(-1.0f, 28.0f))) {
            editorWorld.metadata.objective = runtimeObjective;
            CopyStringToBuffer(editorWorld.metadata.objective, worldObjectiveInput, IM_ARRAYSIZE(worldObjectiveInput));
            statusText = "Copied active runtime objective into workspace metadata.";
        }
        if (ImGui::Button("Point Save-As To Active Runtime World", ImVec2(-1.0f, 28.0f))) {
            CopyStringToBuffer(activeProfile.selectedWorld, exportWorldFileInput, IM_ARRAYSIZE(exportWorldFileInput));
            statusText = "Aligned Save-As target with active runtime world.";
        }
        ImGui::Separator();
        if (ImGui::Button("Prepare prototype export", ImVec2(-1.0f, 36.0f))) {
            importedConcepts.push_back({"prototype_export_note", "Runtime Package", "Logical completion"});
            statusText = "Prepared prototype export note in concept backlog.";
        }
        if (ImGui::Button("Reload runtime world", ImVec2(-1.0f, 36.0f))) {
            LoadOrCreateEditorWorld(editorWorld, statusText);
            SyncEditorWorldBindings(
                editorWorld,
                selectedObjectIndex,
                selectedRegistryEdit, IM_ARRAYSIZE(selectedRegistryEdit),
                selectedScriptTagEdit, IM_ARRAYSIZE(selectedScriptTagEdit),
                selectedLinkTargetEdit, IM_ARRAYSIZE(selectedLinkTargetEdit),
                worldNameInput, IM_ARRAYSIZE(worldNameInput),
                worldBiomeInput, IM_ARRAYSIZE(worldBiomeInput),
                worldObjectiveInput, IM_ARRAYSIZE(worldObjectiveInput),
                worldSpawnX, worldSpawnY);
            CopyStringToBuffer(activeProfile.selectedWorld, exportWorldFileInput, IM_ARRAYSIZE(exportWorldFileInput));
        }
        if (ImGui::Button("Export start world for BunkerGame", ImVec2(-1.0f, 36.0f))) {
            bunker::SessionProfile sessionProfile;
            const auto profilePath = bunker::DefaultSessionProfilePath();
            if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
                sessionProfile = bunker::MakeDefaultSessionProfile();
            }
            bunker::NormalizeSessionProfile(sessionProfile);
            const auto path = bunker::ResolveWorldPath(sessionProfile.selectedWorld);
            if (editorWorld.Save(path.string())) {
                CopyStringToBuffer(sessionProfile.selectedWorld, exportWorldFileInput, IM_ARRAYSIZE(exportWorldFileInput));
                statusText = "Exported workspace into active runtime world: " + path.string();
            } else {
                statusText = "Failed to export runtime world to " + path.string();
            }
        }
        ImGui::InputText("Save As", exportWorldFileInput, IM_ARRAYSIZE(exportWorldFileInput));
        if (ImGui::Button("Export As New World File", ImVec2(-1.0f, 36.0f))) {
            const std::string exportName = NormalizeExportWorldName(exportWorldFileInput);
            if (exportName.empty()) {
                statusText = "Save As world name cannot be empty.";
            } else {
                const auto path = bunker::ResolveWorldPath(exportName);
                if (editorWorld.Save(path.string())) {
                    statusText = "Exported new world file to " + path.string();
                    CopyStringToBuffer(exportName, exportWorldFileInput, IM_ARRAYSIZE(exportWorldFileInput));
                } else {
                    statusText = "Failed to export new world file to " + path.string();
                }
            }
        }
        if (ImGui::Button("Set Save-As World As Active", ImVec2(-1.0f, 36.0f))) {
            const std::string exportName = NormalizeExportWorldName(exportWorldFileInput);
            if (exportName.empty()) {
                statusText = "Set Active needs a Save As world name first.";
            } else {
                const auto path = bunker::ResolveWorldPath(exportName);
                if (!std::filesystem::exists(path)) {
                    statusText = "Cannot set active world before export exists: " + path.string();
                } else {
                    if (SetActiveWorldInProfile(exportName, statusText)) {
                        CopyStringToBuffer(exportName, exportWorldFileInput, IM_ARRAYSIZE(exportWorldFileInput));
                    }
                }
            }
        }
        if (ImGui::Button("Export Save-As And Set Active", ImVec2(-1.0f, 36.0f))) {
            const std::string exportName = NormalizeExportWorldName(exportWorldFileInput);
            if (exportName.empty()) {
                statusText = "Export+Set Active needs a Save As world name first.";
            } else {
                const auto path = bunker::ResolveWorldPath(exportName);
                if (!editorWorld.Save(path.string())) {
                    statusText = "Failed to export and activate world: " + path.string();
                } else if (!SetActiveWorldInProfile(exportName, statusText)) {
                    statusText = "Exported world but failed to activate it: " + path.string();
                } else {
                    CopyStringToBuffer(exportName, exportWorldFileInput, IM_ARRAYSIZE(exportWorldFileInput));
                    statusText = "Exported world and activated it for runtime: " + path.string();
                }
            }
        }
        if (ImGui::Button("Write concept manifest", ImVec2(-1.0f, 36.0f))) {
            if (importedConcepts.empty()) {
                statusText = "Concept manifest skipped: backlog is empty.";
            } else {
                std::ofstream manifest(bunker::EditorConceptManifestPath());
                if (manifest.is_open()) {
                    for (const auto& imported : importedConcepts) {
                        manifest << imported.sourceLabel << " | " << imported.targetType << " | " << imported.completionMode << '\n';
                    }
                    statusText = "Wrote concept manifest to " + bunker::EditorConceptManifestPath().string();
                } else {
                    statusText = "Failed to write concept manifest.";
                }
            }
        }
        ImGui::TextWrapped("Game must run without the editor. This tool only prepares content packages for the runtime client.");
        ImGui::Separator();
        ImGui::TextWrapped("%s", statusText.c_str());
        ImGui::End();

        if (showImportAssistant) {
            ImGui::SetNextWindowPos(ImVec2(1088.0f, 454.0f), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(316.0f, 430.0f), ImGuiCond_Always);
            ImGui::Begin("Import Assistant", &showImportAssistant, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            ImGui::TextWrapped("Drop or describe a concept image here. The assistant turns it into a placeable object, usable item, module, or location blockout.");
            ImGui::InputText("Source", conceptInput, IM_ARRAYSIZE(conceptInput));
            ImGui::Combo("Target", &targetTypeIndex, targetTypes, IM_ARRAYSIZE(targetTypes));
            ImGui::Combo("Completion", &completionIndex, completionModes, IM_ARRAYSIZE(completionModes));
            ImGui::Separator();
            ImGui::TextWrapped("Completion rules:");
            ImGui::BulletText("Mirror unseen side when symmetry is likely");
            ImGui::BulletText("Use logical completion when structure is asymmetric");
            ImGui::BulletText("Keep partial shell when concept should stay open");
            if (ImGui::Button("Convert concept to draft asset", ImVec2(-1.0f, 36.0f))) {
                if (IsBlank(conceptInput)) {
                    statusText = "Import assistant needs a source label or concept note before conversion.";
                } else {
                    importedConcepts.push_back({conceptInput, targetTypes[targetTypeIndex], completionModes[completionIndex]});
                    statusText = std::string("Prepared concept draft: ") + conceptInput;
                    conceptInput[0] = '\0';
                }
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
