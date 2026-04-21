#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "World.hpp"

namespace bunker {

struct PrefabRecord {
    std::string id;
    std::string label;
    std::string targetType;
    std::string sourceLabel;
    std::string completionMode;
    MapObject object;
};

std::string NormalizePrefabRecordId(std::string_view prefabId);
std::string DefaultPrefabTargetType(const MapObject& object);
void NormalizePrefabRecord(PrefabRecord& prefab);
int FindPrefabRecordIndexById(const std::vector<PrefabRecord>& prefabs, std::string_view prefabId);
const PrefabRecord* FindPrefabRecordById(const std::vector<PrefabRecord>& prefabs, std::string_view prefabId);
int CountPrefabUsageInWorld(const World& world, std::string_view prefabId);
int CountPrefabDerivedObjects(const World& world);
std::vector<int> CollectPrefabUsageObjectIndices(const World& world, std::string_view prefabId);
std::vector<int> CollectBrokenPrefabReferenceObjectIndices(
    const World& world,
    const std::vector<PrefabRecord>& prefabs);
bool LoadPrefabLibrary(std::vector<PrefabRecord>& prefabs);
bool SavePrefabLibrary(const std::vector<PrefabRecord>& prefabs);

}  // namespace bunker
