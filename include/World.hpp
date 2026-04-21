#pragma once

#include <string_view>
#include <string>
#include <vector>

#include "MapObject.hpp"

namespace bunker {

struct WorldMetadata {
    std::string name = "Prototype Wasteland";
    std::string biome = "Bunker Outskirts";
    std::string objective = "Wake from cryostasis, recover the Pip-Pad, and restore the tank link.";
    float playerSpawnX = -12.0f;
    float playerSpawnY = -8.0f;
};

enum class WorldObjectReferenceField {
    LinkTarget
};

struct WorldObjectReference {
    WorldObjectReferenceField field = WorldObjectReferenceField::LinkTarget;
    int sourceObjectIndex = -1;
    int targetObjectIndex = -1;
    std::string sourceObjectId;
    std::string sourceDisplayName;
    std::string sourceScriptTag;
    std::string targetObjectId;
    std::string viaValue;
    bool resolved = false;
};

const char* WorldObjectReferenceFieldLabel(WorldObjectReferenceField field);
std::string NormalizeEditorLayerName(std::string_view layerName);
std::string DefaultEditorLayerName(const MapObject& object);

class World {
public:
    WorldMetadata metadata;
    std::vector<MapObject> objects;

    void Clear();
    void RemoveObject(const std::string& registryId);
    void AddObject(const MapObject& obj);
    bool Load(const std::string& path);
    bool Save(const std::string& path) const;
    void GeneratePrototypeZone();
    void EnsureStarterInfrastructure();
    bool HasObject(const std::string& registryId) const;
    const MapObject* FindObjectByRegistryId(const std::string& registryId) const;
    MapObject* FindObjectByRegistryId(const std::string& registryId);
    const MapObject* FindObjectByScriptTag(const std::string& scriptTag) const;
    MapObject* FindObjectByScriptTag(const std::string& scriptTag);
    const MapObject* FindObjectByLinkTarget(const std::string& linkTarget) const;
    MapObject* FindObjectByLinkTarget(const std::string& linkTarget);
    std::vector<WorldObjectReference> BuildObjectReferences() const;
    std::vector<WorldObjectReference> FindIncomingObjectReferences(const std::string& registryId) const;
    std::vector<WorldObjectReference> FindOutgoingObjectReferences(const std::string& registryId) const;
    std::vector<std::string> CollectEditorLayerNames() const;
    int CountObjectsInEditorLayer(std::string_view layerName) const;
    bool HasIncomingObjectReferences(const std::string& registryId) const;
    bool HasScriptTag(const std::string& scriptTag) const;
    bool HasLinkTarget(const std::string& linkTarget) const;
    bool IsStarterScenarioWorld() const;
    const MapObject* FindNearestInteractive(float x, float y, float radius) const;
};

}  // namespace bunker
