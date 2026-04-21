#include "../include/PrefabLibrary.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

#include "../include/AppPaths.hpp"

namespace bunker {

bool LoadPrefabLibrary(std::vector<PrefabRecord>& prefabs) {
    prefabs.clear();

    std::ifstream file(EditorPrefabLibraryPath());
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line.starts_with('#')) {
            continue;
        }

        std::istringstream lineStream(line);
        PrefabRecord prefab;
        if (!(lineStream >> std::quoted(prefab.label)
                >> std::quoted(prefab.object.registryId)
                >> std::quoted(prefab.object.displayName)
                >> std::quoted(prefab.object.scriptTag)
                >> std::quoted(prefab.object.linkTarget))) {
            return false;
        }

        int interaction = 0;
        int category = 0;
        if (!(lineStream >> interaction
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
                >> prefab.object.manualLoot)) {
            return false;
        }

        prefab.object.interaction = static_cast<InteractionType>(interaction);
        prefab.object.category = static_cast<ObjectCategory>(category);

        bool semanticAutoCreated = false;
        bool semanticLayoutPinned = false;
        if (lineStream >> semanticAutoCreated) {
            if (!(lineStream >> semanticLayoutPinned)) {
                return false;
            }
            prefab.object.semanticAutoCreated = semanticAutoCreated;
            prefab.object.semanticLayoutPinned = semanticLayoutPinned;
        } else {
            lineStream.clear();
        }

        for (auto& lootId : prefab.object.manualLootIds) {
            if (!(lineStream >> std::quoted(lootId))) {
                return false;
            }
        }

        prefabs.push_back(prefab);
    }

    return true;
}

bool SavePrefabLibrary(const std::vector<PrefabRecord>& prefabs) {
    std::ofstream file(EditorPrefabLibraryPath());
    if (!file.is_open()) {
        return false;
    }

    file << "# BUNKER_PREFABS_V2\n";
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
             << prefab.object.manualLoot << ' '
             << prefab.object.semanticAutoCreated << ' '
             << prefab.object.semanticLayoutPinned;
        for (const auto& lootId : prefab.object.manualLootIds) {
            file << ' ' << std::quoted(lootId);
        }
        file << '\n';
    }

    return static_cast<bool>(file);
}

}  // namespace bunker
