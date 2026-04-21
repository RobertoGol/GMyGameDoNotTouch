#include "EditorSupport.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "../../include/AppPaths.hpp"
#include "../../include/AtomicPersistence.hpp"
#include "../../include/GameplayDescriptorRegistry.hpp"
#include "../../include/RegistryId.hpp"
#include "../../include/WorldExport.hpp"
#include "../../include/WorldSemanticAuthoring.hpp"
#include "../../include/WorldValidation.hpp"

namespace editor_support {

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

    const auto saveResult = bunker::SaveWorldAtomically(world, path);
    if (!saveResult.ok) {
        statusText = "Runtime world was missing. Failed to persist generated workspace: " + saveResult.message;
        return false;
    }

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
    const auto saveResult = bunker::SaveProfileAtomically(sessionProfile, profilePath);
    if (saveResult.ok) {
        statusText = "Active world changed to " + sessionProfile.selectedWorld;
        return true;
    }
    statusText = "Failed to update active world in session profile: " + saveResult.message;
    return false;
}

void CopyStringToBuffer(const std::string& value, char* buffer, std::size_t size) {
    if (buffer == nullptr || size == 0) {
        return;
    }

    strncpy_s(buffer, size, value.c_str(), _TRUNCATE);
}

void ApplyGameplayDescriptorPreset(
    bunker::MapObject& object,
    std::string_view scriptTag,
    const char* defaultLinkTarget,
    char* scriptTagBuffer,
    std::size_t scriptTagSize,
    char* linkTargetBuffer = nullptr,
    std::size_t linkTargetSize = 0) {
    object.scriptTag = std::string(bunker::NormalizeGameplayDescriptorTag(scriptTag));
    if (const auto* spec = bunker::FindGameplayDescriptor(object.scriptTag)) {
        object.interaction = spec->preferredInteraction;
        object.category = spec->preferredCategory;
    }
    const char* resolvedDefaultLinkTarget = defaultLinkTarget;
    if (resolvedDefaultLinkTarget == nullptr) {
        resolvedDefaultLinkTarget = bunker::DefaultGameplayDescriptorLinkTarget(object.scriptTag);
    }
    if (resolvedDefaultLinkTarget != nullptr && object.linkTarget.empty()) {
        object.linkTarget = resolvedDefaultLinkTarget;
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    if (linkTargetBuffer != nullptr && linkTargetSize > 0) {
        CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
    }
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

void SyncSelectedObjectBindings(const bunker::World& editorWorld,
    int selectedObjectIndex,
    char* selectedRegistryEdit,
    std::size_t selectedRegistryEditSize,
    char* selectedScriptTagEdit,
    std::size_t selectedScriptTagEditSize,
    char* selectedLinkTargetEdit,
    std::size_t selectedLinkTargetEditSize) {
    if (selectedObjectIndex < 0 || selectedObjectIndex >= static_cast<int>(editorWorld.objects.size())) {
        CopyStringToBuffer("", selectedRegistryEdit, selectedRegistryEditSize);
        CopyStringToBuffer("", selectedScriptTagEdit, selectedScriptTagEditSize);
        CopyStringToBuffer("", selectedLinkTargetEdit, selectedLinkTargetEditSize);
        return;
    }

    const auto& object = editorWorld.objects[static_cast<std::size_t>(selectedObjectIndex)];
    CopyStringToBuffer(object.registryId, selectedRegistryEdit, selectedRegistryEditSize);
    CopyStringToBuffer(object.scriptTag, selectedScriptTagEdit, selectedScriptTagEditSize);
    CopyStringToBuffer(object.linkTarget, selectedLinkTargetEdit, selectedLinkTargetEditSize);
}

int FindObjectIndexByRegistryId(const bunker::World& world, const std::string& registryId) {
    for (int index = 0; index < static_cast<int>(world.objects.size()); ++index) {
        if (world.objects[static_cast<std::size_t>(index)].registryId == registryId) {
            return index;
        }
    }
    return -1;
}

int FindObjectIndexByScriptTag(const bunker::World& world, std::string_view scriptTag) {
    const std::string normalizedTag = std::string(bunker::NormalizeGameplayDescriptorTag(scriptTag));
    for (int index = 0; index < static_cast<int>(world.objects.size()); ++index) {
        if (world.objects[static_cast<std::size_t>(index)].scriptTag == normalizedTag) {
            return index;
        }
    }
    return -1;
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
    object.scriptTag.clear();
    if (object.linkTarget.empty()) {
        object.linkTarget = "next_zone";
    }
    CopyStringToBuffer(object.scriptTag, scriptTagBuffer, scriptTagSize);
    CopyStringToBuffer(object.linkTarget, linkTargetBuffer, linkTargetSize);
}

void ApplyWorkshopDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize) {
    ApplyGameplayDescriptorPreset(object, "workshop_service", nullptr, scriptTagBuffer, scriptTagSize);
}

void ApplyTowerDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "tower_sync", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyPowerPylonDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "power_pylon", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyDroneStationDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "drone_station", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyRailDepotDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "rail_depot", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyOrbitalUplinkDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "orbital_uplink", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyRailFortressDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "rail_fortress_hub", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyRecoveryFabricatorDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "recovery_fabricator", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyIndustrialGateDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "industrial_gate", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyIndustrialSurveyDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "industrial_survey", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyIndustrialOutpostDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "industrial_outpost", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyAssemblyCellDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "assembly_cell", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyFoundryLineDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "foundry_line", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyReactorYardDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "reactor_yard", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyCapacitorBankDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "capacitor_bank", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyRelaySubstationDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "relay_substation", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyServiceBayDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "service_bay", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyWaterReclaimerDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "water_reclaimer", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyServiceHubDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "lanline_service_hub", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyFeyRingDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "fey_ring", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyMedicalSupportDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "medical_support", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyTankServiceDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "tank_service", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyRemoteLinkDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "remote_link", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplyEchoDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "echo_trace", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
}

void ApplySpecialistDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize) {
    ApplyGameplayDescriptorPreset(object, "specialist_cryo", nullptr, scriptTagBuffer, scriptTagSize, linkTargetBuffer, linkTargetSize);
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

std::vector<std::string> RequiredSemanticDependencyTagsForScript(std::string_view scriptTag) {
    std::vector<std::string> dependencyTags;
    for (const std::string_view dependencyTag : bunker::RequiredSemanticDependencyTags(scriptTag)) {
        dependencyTags.emplace_back(dependencyTag);
    }
    return dependencyTags;
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

bool AlignObjectToDescriptorDefaults(bunker::MapObject& object, bool overwriteLinkTarget) {
    return bunker::AlignObjectToDescriptorDefaults(object, overwriteLinkTarget);
}

bool IsAutoGeneratedSemanticAnchor(const bunker::MapObject& object) {
    return bunker::IsAutoGeneratedSemanticAnchor(object);
}

bool IsPinnedSemanticAnchor(const bunker::MapObject& object) {
    return bunker::IsPinnedSemanticAnchor(object);
}

bool PinSemanticAnchorPlacement(bunker::MapObject& object, bool pinned) {
    return bunker::PinSemanticAnchorPlacement(object, pinned);
}

bool AdoptSemanticAnchorAsAuthored(bunker::MapObject& object, bool pinPlacement) {
    return bunker::AdoptSemanticAnchorAsAuthored(object, pinPlacement);
}

int AdoptAllAutoCreatedSemanticAnchors(bunker::World& world, std::string& statusText) {
    return bunker::AdoptAllAutoCreatedSemanticAnchors(world, statusText);
}

int AdoptSemanticDependencyChainAsAuthored(bunker::World& world, int rootObjectIndex, bool includeRoot, std::string& statusText) {
    return bunker::AdoptSemanticDependencyChainAsAuthored(world, rootObjectIndex, includeRoot, statusText);
}

int AutoLayoutSemanticDependencyChain(bunker::World& world,
    int rootObjectIndex,
    std::string& statusText,
    bool preserveManualPlacement) {
    bunker::SemanticLayoutOptions options;
    options.preserveManualPlacement = preserveManualPlacement;
    return bunker::AutoLayoutSemanticDependencyChain(world, rootObjectIndex, statusText, options);
}

int AutoLayoutSemanticDependencyChain(bunker::World& world, int rootObjectIndex, std::string& statusText) {
    return bunker::AutoLayoutSemanticDependencyChain(world, rootObjectIndex, statusText);
}

bool CanAutoFixValidationIssue(const bunker::ValidationIssue& issue) {
    return bunker::CanAutoFixValidationIssue(issue);
}

bool AutoFixValidationIssue(bunker::World& world, const bunker::ValidationIssue& issue, std::string& statusText) {
    return bunker::AutoFixValidationIssue(world, issue, statusText);
}

int AutoFixSafeValidationIssues(bunker::World& world, std::string& statusText) {
    return bunker::AutoFixSafeValidationIssues(world, statusText);
}

bool CanCreateMissingDependencyAnchor(const bunker::ValidationIssue& issue) {
    return bunker::CanCreateMissingDependencyAnchor(issue);
}

bool CreateMissingDependencyAnchorForIssue(bunker::World& world, const bunker::ValidationIssue& issue, int& createdObjectIndex, std::string& statusText) {
    return bunker::CreateMissingDependencyAnchorForIssue(world, issue, createdObjectIndex, statusText);
}

AnchorCascadeResult CreateMissingDependencyAnchorsCascadeDetailed(bunker::World& world, std::string& statusText) {
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    return {
        batch.createdCount,
        batch.lastCreatedObjectIndex,
        batch.createdObjectIndices,
        batch.createdScriptTags
    };
}

int CreateMissingDependencyAnchorsCascade(bunker::World& world, std::string& statusText) {
    return CreateMissingDependencyAnchorsCascadeDetailed(world, statusText).createdCount;
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
    return bunker::LoadPrefabLibrary(prefabs);
}

std::string BuildEditorValidationStatus(const bunker::World& world) {
    const auto issues = bunker::ValidateWorldForRuntime(world);
    return bunker::BuildValidationSummary(issues);
}

bool TryExportValidatedWorld(const bunker::World& world,
    const std::filesystem::path& path,
    std::string& statusText,
    bunker::ExportValidationPolicy policy,
    bunker::WorldExportResult* exportResult) {
    const auto detailedResult = bunker::ExportWorldWithValidation(world, path, policy);
    if (exportResult != nullptr) {
        *exportResult = detailedResult;
    }
    statusText = detailedResult.message;
    return detailedResult.ok;
}

bool SavePrefabLibrary(const std::vector<SavedPrefab>& prefabs) {
    return bunker::SavePrefabLibrary(prefabs);
}

void RequestPreviewFocus(PreviewViewportState& viewportState, float worldX, float worldY, float zoom) {
    viewportState.hasFocusRequest = true;
    viewportState.focusWorldX = worldX;
    viewportState.focusWorldY = worldY;
    viewportState.focusZoom = std::clamp(zoom, 0.25f, 3.0f);
}

void ClearPreviewSemanticOverlay(PreviewViewportState& viewportState) {
    viewportState.semanticOverlayRootIndex = -1;
    viewportState.semanticObjectIndices.clear();
    viewportState.semanticLinks.clear();
    viewportState.semanticOverlayLabel.clear();
}

void ShowPreviewSemanticDependencies(const bunker::World& world, int rootObjectIndex, PreviewViewportState& viewportState) {
    ClearPreviewSemanticOverlay(viewportState);
    if (rootObjectIndex < 0 || rootObjectIndex >= static_cast<int>(world.objects.size())) {
        return;
    }

    const auto dependencyGraph = bunker::BuildSemanticDependencyGraph(world, rootObjectIndex);
    if (dependencyGraph.empty()) {
        return;
    }

    viewportState.semanticOverlayRootIndex = rootObjectIndex;
    viewportState.semanticObjectIndices = bunker::CollectSemanticDependencyObjectIndices(world, rootObjectIndex, true);
    viewportState.semanticLinks.reserve(dependencyGraph.size());
    for (const auto& edge : dependencyGraph) {
        viewportState.semanticLinks.push_back({
            edge.sourceObjectIndex,
            edge.dependencyObjectIndex,
            edge.dependencyScriptTag,
            edge.dependencyPresent
        });
    }

    const auto& rootObject = world.objects[static_cast<std::size_t>(rootObjectIndex)];
    viewportState.semanticOverlayLabel = rootObject.displayName;
    viewportState.semanticOverlayLabel += " semantic chain";
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

    if (viewportState.hasFocusRequest) {
        viewportState.zoom = std::clamp(viewportState.focusZoom, 0.25f, 3.0f);
        const float focusScale = std::max(0.25f, baseScale * viewportState.zoom);
        viewportState.offsetX = (size.x * 0.5f) - 6.0f - ((viewportState.focusWorldX - minX) * focusScale);
        viewportState.offsetY = (size.y * 0.5f) - 6.0f - ((viewportState.focusWorldY - minY) * focusScale);
        viewportState.hasFocusRequest = false;
    }
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

    if (!viewportState.semanticOverlayLabel.empty()) {
        drawList->AddText(
            ImVec2(origin.x + 10.0f, origin.y + 8.0f),
            IM_COL32(132, 220, 240, 255),
            viewportState.semanticOverlayLabel.c_str());
    }

    auto isSemanticOverlayObject = [&](int objectIndex) {
        return std::find(
            viewportState.semanticObjectIndices.begin(),
            viewportState.semanticObjectIndices.end(),
            objectIndex) != viewportState.semanticObjectIndices.end();
    };

    for (int edgeIndex = 0; edgeIndex < static_cast<int>(viewportState.semanticLinks.size()); ++edgeIndex) {
        const auto& edge = viewportState.semanticLinks[static_cast<std::size_t>(edgeIndex)];
        if (edge.sourceObjectIndex < 0 || edge.sourceObjectIndex >= static_cast<int>(world.objects.size())) {
            continue;
        }

        const auto& sourceObject = world.objects[static_cast<std::size_t>(edge.sourceObjectIndex)];
        const ImVec2 sourcePoint = toCanvas(sourceObject.x, sourceObject.y);
        if (edge.resolved &&
            edge.targetObjectIndex >= 0 &&
            edge.targetObjectIndex < static_cast<int>(world.objects.size())) {
            const auto& targetObject = world.objects[static_cast<std::size_t>(edge.targetObjectIndex)];
            const ImVec2 targetPoint = toCanvas(targetObject.x, targetObject.y);
            drawList->AddLine(sourcePoint, targetPoint, IM_COL32(74, 190, 220, 210), 2.0f);
            const ImVec2 labelPoint((sourcePoint.x + targetPoint.x) * 0.5f + 4.0f, (sourcePoint.y + targetPoint.y) * 0.5f - 12.0f);
            drawList->AddText(labelPoint, IM_COL32(140, 220, 235, 230), edge.label.c_str());
        } else {
            const float xOffset = 28.0f + static_cast<float>(edgeIndex % 3) * 14.0f;
            const float yOffset = -26.0f + static_cast<float>(edgeIndex % 2) * 18.0f;
            const ImVec2 unresolvedPoint(sourcePoint.x + xOffset, sourcePoint.y + yOffset);
            drawList->AddLine(sourcePoint, unresolvedPoint, IM_COL32(240, 174, 78, 220), 2.0f);
            drawList->AddCircleFilled(unresolvedPoint, 4.0f, IM_COL32(240, 174, 78, 255), 12);
            drawList->AddText(ImVec2(unresolvedPoint.x + 6.0f, unresolvedPoint.y - 10.0f), IM_COL32(250, 206, 122, 240), edge.label.c_str());
        }
    }

    for (int index = 0; index < static_cast<int>(world.objects.size()); ++index) {
        const auto& object = world.objects[static_cast<std::size_t>(index)];
        const ImVec2 min = toCanvas(object.x - object.width * 0.5f, object.y - object.depth * 0.5f);
        const ImVec2 max = toCanvas(object.x + object.width * 0.5f, object.y + object.depth * 0.5f);
        const bool semanticOverlayObject = isSemanticOverlayObject(index);
        const bool semanticOverlayRoot = index == viewportState.semanticOverlayRootIndex;
        drawList->AddRectFilled(min, max, ColorForCategory(object.category), 2.0f);
        if (semanticOverlayObject) {
            drawList->AddRect(
                ImVec2(min.x - 3.0f, min.y - 3.0f),
                ImVec2(max.x + 3.0f, max.y + 3.0f),
                semanticOverlayRoot ? IM_COL32(84, 230, 255, 255) : IM_COL32(92, 196, 214, 235),
                3.0f,
                0,
                semanticOverlayRoot ? 3.0f : 2.0f);
        }
        drawList->AddRect(min, max,
            index == selectedObjectIndex ? IM_COL32(255, 255, 180, 255) : IM_COL32(25, 25, 25, 220),
            2.0f,
            0,
            index == selectedObjectIndex ? 2.0f : 1.0f);

        if (showInteractionHelpers) {
            const ImVec2 center = toCanvas(object.x, object.y);
            const float halfH = std::max(4.0f, object.depth * 0.5f * scale);
            const ImU32 markerColor = InteractionMarkerColor(object.interaction);
            drawList->AddCircleFilled(center, 9.0f, markerColor, 16);
            drawList->AddText(ImVec2(center.x - 3.0f, center.y - halfH - 16.0f), IM_COL32(20, 20, 20, 255), InteractionMarker(object.interaction));
        }

        if (showObjectLabels) {
            drawList->AddText(ImVec2(min.x, max.y + 2.0f), IM_COL32(220, 220, 220, 255), object.displayName.c_str());
        }
    }

    const ImVec2 spawnPoint = toCanvas(world.metadata.playerSpawnX, world.metadata.playerSpawnY);
    drawList->AddCircleFilled(spawnPoint, 6.0f, IM_COL32(120, 210, 255, 255), 16);
    drawList->AddText(ImVec2(spawnPoint.x + 8.0f, spawnPoint.y - 10.0f), IM_COL32(180, 220, 255, 255), "SPAWN");

    const ImVec2 mousePos = ImGui::GetIO().MousePos;
    const bool hovered = ImGui::IsWindowHovered();
    if (hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
        const ImVec2 drag = ImGui::GetIO().MouseDelta;
        viewportState.offsetX += drag.x;
        viewportState.offsetY += drag.y;
    }
    if (hovered) {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            viewportState.zoom = std::clamp(viewportState.zoom + wheel * 0.1f, 0.25f, 3.0f);
        }
    }

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        interactionResult.clicked = true;
        const ImVec2 worldPoint = toWorld(mousePos);
        interactionResult.worldX = worldPoint.x;
        interactionResult.worldY = worldPoint.y;

        for (int index = static_cast<int>(world.objects.size()) - 1; index >= 0; --index) {
            const auto& object = world.objects[static_cast<std::size_t>(index)];
            const bool withinX = worldPoint.x >= object.x - object.width * 0.5f && worldPoint.x <= object.x + object.width * 0.5f;
            const bool withinY = worldPoint.y >= object.y - object.depth * 0.5f && worldPoint.y <= object.y + object.depth * 0.5f;
            if (withinX && withinY) {
                interactionResult.clickedObject = true;
                interactionResult.clickedObjectIndex = index;
                interactionResult.doubleClickedObject = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
                break;
            }
        }
    }

    interactionResult.draggingSelectedObject =
        hovered && selectedObjectIndex >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left);

    ImGui::EndChild();
    return interactionResult;
}

}  // namespace editor_support
