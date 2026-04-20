#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include "imgui.h"

#include "../../include/SessionProfiles.hpp"
#include "../../include/World.hpp"

namespace editor_support {

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

const char* ToLabel(bunker::InteractionType interaction);
const char* ToLabel(bunker::ObjectCategory category);
int ToIndex(bunker::InteractionType interaction);
int ToIndex(bunker::ObjectCategory category);
bunker::ObjectCategory CategoryFromIndex(int index);
bool LoadOrCreateEditorWorld(bunker::World& world, std::string& statusText);
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
std::string NormalizeExportWorldName(const char* exportWorldFileInput);
std::string MakeDuplicateRegistryId(const bunker::World& world, const std::string& sourceRegistryId);
bool HasOtherObjectWithRegistryId(const bunker::World& world, const std::string& registryId, int selectedIndex);
bool LoadPrefabLibrary(std::vector<SavedPrefab>& prefabs);
std::string BuildEditorValidationStatus(const bunker::World& world);
bool TryExportValidatedWorld(const bunker::World& world, const std::filesystem::path& path, std::string& statusText);
bool SavePrefabLibrary(const std::vector<SavedPrefab>& prefabs);
ImU32 ColorForCategory(bunker::ObjectCategory category);
const char* InteractionMarker(bunker::InteractionType interaction);
ImU32 InteractionMarkerColor(bunker::InteractionType interaction);
PreviewInteraction DrawWorldPreview(const bunker::World& world,
    int selectedObjectIndex,
    bool previewAsPlayer,
    PreviewViewportState& viewportState,
    bool showInteractionHelpers,
    bool showObjectLabels);

}  // namespace editor_support
