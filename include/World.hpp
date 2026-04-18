#pragma once

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
    bool IsStarterScenarioWorld() const;
    const MapObject* FindNearestInteractive(float x, float y, float radius) const;
};

}  // namespace bunker
