#pragma once

#include <string>
#include <vector>

#include "MapObject.hpp"

namespace bunker {

struct PrefabRecord {
    std::string label;
    MapObject object;
};

bool LoadPrefabLibrary(std::vector<PrefabRecord>& prefabs);
bool SavePrefabLibrary(const std::vector<PrefabRecord>& prefabs);

}  // namespace bunker
