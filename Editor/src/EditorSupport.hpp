#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "imgui.h"

#include "../../include/PrefabLibrary.hpp"
#include "../../include/SessionProfiles.hpp"
#include "../../include/World.hpp"
#include "../../include/WorldExport.hpp"
#include "../../include/WorldValidation.hpp"

namespace editor_support {

using SavedPrefab = bunker::PrefabRecord;

enum class PreviewDragMode {
    None,
    MoveSelection,
    ResizeWidth,
    ResizeDepth,
    ResizeBoth,
    MoveSpawn
};

struct PreviewInteraction {
    bool clicked = false;
    bool clickedObject = false;
    int clickedObjectIndex = -1;
    bool doubleClickedObject = false;
    bool objectUnderMouse = false;
    int objectUnderMouseIndex = -1;
    bool rightClickedObject = false;
    int rightClickedObjectIndex = -1;
    bool droppedObjectWindowItem = false;
    int droppedObjectWindowSourceType = 0;
    int droppedObjectWindowSourceIndex = -1;
    bool dropHasWorldPosition = false;
    bool draggingSelectedObject = false;
    bool draggingSelectedWidth = false;
    bool draggingSelectedDepth = false;
    bool draggingSpawn = false;
    float worldX = 0.0f;
    float worldY = 0.0f;
    float suggestedWidth = 0.0f;
    float suggestedDepth = 0.0f;
};

struct PreviewOverlayLink {
    int sourceObjectIndex = -1;
    int targetObjectIndex = -1;
    std::string label;
    bool resolved = false;
};

struct PreviewViewportState {
    float zoom = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    PreviewDragMode activeDragMode = PreviewDragMode::None;
    bool hasFocusRequest = false;
    float focusWorldX = 0.0f;
    float focusWorldY = 0.0f;
    float focusZoom = 1.4f;
    int semanticOverlayRootIndex = -1;
    std::vector<int> semanticObjectIndices{};
    std::vector<PreviewOverlayLink> semanticLinks{};
    std::string semanticOverlayLabel;
};

struct PreviewRenderOptions {
    bool showInteractionHelpers = true;
    bool showObjectLabels = false;
    bool showGrid = true;
    bool showBoundsOverlay = true;
    bool showReferenceLinks = true;
    bool showInteractionRadius = true;
    bool showServiceRadius = true;
    float gridStep = 1.0f;
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

struct AnchorCascadeResult {
    int createdCount = 0;
    int lastCreatedObjectIndex = -1;
    std::vector<int> createdObjectIndices{};
    std::vector<std::string> createdScriptTags{};
};

struct EditorLayerViewState {
    std::string name;
    bool visible = true;
    bool locked = false;
};

const char* ToLabel(bunker::InteractionType interaction);
const char* ToLabel(bunker::ObjectCategory category);
int ToIndex(bunker::InteractionType interaction);
int ToIndex(bunker::ObjectCategory category);
bunker::ObjectCategory CategoryFromIndex(int index);
bool LoadOrCreateEditorWorld(bunker::World& world, std::string& statusText);
bool CreateNewEditorWorld(bunker::World& world, std::string& statusText);
bool IsNativeWorldFilePath(const std::filesystem::path& path);
std::vector<std::filesystem::path> ListNativeWorldFiles();
bool TryLoadEditorWorldAtPath(
    const std::filesystem::path& requestedPath,
    bunker::World& world,
    std::string& statusText,
    std::filesystem::path* resolvedPath = nullptr);
bool TrySaveEditorWorldAtPath(
    const bunker::World& world,
    const std::filesystem::path& requestedPath,
    std::string& statusText,
    std::filesystem::path* resolvedPath = nullptr);
bool SetActiveWorldInProfile(const std::string& worldFileName, std::string& statusText);
void CopyStringToBuffer(const std::string& value, char* buffer, std::size_t size);
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
    float& worldSpawnY);
void SyncSelectedObjectBindings(const bunker::World& editorWorld,
    int selectedObjectIndex,
    char* selectedDisplayNameEdit,
    std::size_t selectedDisplayNameEditSize,
    char* selectedRegistryEdit,
    std::size_t selectedRegistryEditSize,
    char* selectedScriptTagEdit,
    std::size_t selectedScriptTagEditSize,
    char* selectedLinkTargetEdit,
    std::size_t selectedLinkTargetEditSize,
    char* selectedLayerEdit,
    std::size_t selectedLayerEditSize);
void SyncEditorLayerViewStates(const bunker::World& world, std::vector<EditorLayerViewState>& layerStates);
EditorLayerViewState* FindEditorLayerViewState(std::vector<EditorLayerViewState>& layerStates, std::string_view layerName);
const EditorLayerViewState* FindEditorLayerViewState(const std::vector<EditorLayerViewState>& layerStates, std::string_view layerName);
bool IsObjectVisibleInEditorLayerView(const bunker::MapObject& object, const std::vector<EditorLayerViewState>& layerStates);
bool IsObjectLockedInEditorLayerView(const bunker::MapObject& object, const std::vector<EditorLayerViewState>& layerStates);
std::string BuildSpecializedRuntimeNotes(const bunker::MapObject& object);
int FindObjectIndexByRegistryId(const bunker::World& world, const std::string& registryId);
int FindObjectIndexByScriptTag(const bunker::World& world, std::string_view scriptTag);
const ObjectPreset& SelectedPreset(const std::array<ObjectPreset, 6>& presets, int index);
std::string DefaultRegistryIdForPreset(int presetIndex, std::size_t objectCount);
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
    char* loot3);
void ApplyTerminalDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize);
void ApplyTransitionDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyWorkshopDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize);
void ApplyTowerDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyPowerPylonDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyDroneStationDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyRailDepotDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyOrbitalUplinkDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyRailFortressDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyRecoveryFabricatorDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyIndustrialGateDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyIndustrialSurveyDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyIndustrialOutpostDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyAssemblyCellDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyFoundryLineDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyReactorYardDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyCapacitorBankDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyRelaySubstationDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyServiceBayDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyWaterReclaimerDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyServiceHubDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyFeyRingDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyMedicalSupportDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyTankServiceDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyRemoteLinkDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplyEchoDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
void ApplySpecialistDescriptorPreset(bunker::MapObject& object, char* scriptTagBuffer, std::size_t scriptTagSize, char* linkTargetBuffer, std::size_t linkTargetSize);
bool ContainsCaseInsensitive(const std::string& haystack, const std::string& needle);
bool IsBlank(const char* text);
std::string TrimCopy(const char* text);
std::vector<std::string> RequiredSemanticDependencyTagsForScript(std::string_view scriptTag);
std::string NormalizeExportWorldName(const char* exportWorldFileInput);
std::string MakeDuplicateRegistryId(const bunker::World& world, const std::string& sourceRegistryId);
bool AlignObjectToDescriptorDefaults(bunker::MapObject& object, bool overwriteLinkTarget = true);
bool IsAutoGeneratedSemanticAnchor(const bunker::MapObject& object);
bool IsPinnedSemanticAnchor(const bunker::MapObject& object);
bool PinSemanticAnchorPlacement(bunker::MapObject& object, bool pinned);
bool AdoptSemanticAnchorAsAuthored(bunker::MapObject& object, bool pinPlacement = true);
int AdoptAllAutoCreatedSemanticAnchors(bunker::World& world, std::string& statusText);
int AdoptSemanticDependencyChainAsAuthored(bunker::World& world, int rootObjectIndex, bool includeRoot, std::string& statusText);
int AutoLayoutSemanticDependencyChain(bunker::World& world,
    int rootObjectIndex,
    std::string& statusText,
    bool preserveManualPlacement);
int AutoLayoutSemanticDependencyChain(bunker::World& world, int rootObjectIndex, std::string& statusText);
bool CanAutoFixValidationIssue(const bunker::ValidationIssue& issue);
bool AutoFixValidationIssue(bunker::World& world, const bunker::ValidationIssue& issue, std::string& statusText);
int AutoFixSafeValidationIssues(bunker::World& world, std::string& statusText);
bool CanCreateMissingDependencyAnchor(const bunker::ValidationIssue& issue);
bool CreateMissingDependencyAnchorForIssue(bunker::World& world, const bunker::ValidationIssue& issue, int& createdObjectIndex, std::string& statusText);
AnchorCascadeResult CreateMissingDependencyAnchorsCascadeDetailed(bunker::World& world, std::string& statusText);
int CreateMissingDependencyAnchorsCascade(bunker::World& world, std::string& statusText);
bool HasOtherObjectWithRegistryId(const bunker::World& world, const std::string& registryId, int selectedIndex);
bool LoadPrefabLibrary(std::vector<SavedPrefab>& prefabs);
std::string BuildEditorValidationStatus(const bunker::World& world);
bool TryExportValidatedWorld(const bunker::World& world,
    const std::filesystem::path& path,
    std::string& statusText,
    bunker::ExportValidationPolicy policy = bunker::ExportValidationPolicy::AllowWarnings,
    bunker::WorldExportResult* exportResult = nullptr);
bool SavePrefabLibrary(const std::vector<SavedPrefab>& prefabs);
void RequestPreviewFocus(PreviewViewportState& viewportState, float worldX, float worldY, float zoom = 1.4f);
void ClearPreviewSemanticOverlay(PreviewViewportState& viewportState);
void ShowPreviewSemanticDependencies(const bunker::World& world, int rootObjectIndex, PreviewViewportState& viewportState);
ImU32 ColorForCategory(bunker::ObjectCategory category);
const char* InteractionMarker(bunker::InteractionType interaction);
ImU32 InteractionMarkerColor(bunker::InteractionType interaction);
PreviewInteraction DrawWorldPreview(const bunker::World& world,
    int selectedObjectIndex,
    bool previewAsPlayer,
    PreviewViewportState& viewportState,
    const std::vector<EditorLayerViewState>& layerStates,
    const PreviewRenderOptions& options);

}  // namespace editor_support
