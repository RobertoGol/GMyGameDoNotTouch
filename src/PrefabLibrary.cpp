#include "../include/PrefabLibrary.hpp"

#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "../include/AppPaths.hpp"
#include "../include/World.hpp"

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

        while (lineStream && std::isspace(static_cast<unsigned char>(lineStream.peek()))) {
            lineStream.get();
        }
        if (lineStream.peek() == '"') {
            if (!(lineStream >> std::quoted(prefab.object.editorLayer))) {
                return false;
            }
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
        prefab.object.editorLayer = NormalizeEditorLayerName(prefab.object.editorLayer);
        if (prefab.object.editorLayer.empty()) {
            prefab.object.editorLayer = DefaultEditorLayerName(prefab.object);
        }

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

    file << "# BUNKER_PREFABS_V3\n";
    for (const auto& prefab : prefabs) {
        const std::string normalizedLayer = NormalizeEditorLayerName(prefab.object.editorLayer);
        file << std::quoted(prefab.label) << ' '
             << std::quoted(prefab.object.registryId) << ' '
             << std::quoted(prefab.object.displayName) << ' '
             << std::quoted(prefab.object.scriptTag) << ' '
             << std::quoted(prefab.object.linkTarget) << ' '
             << std::quoted(normalizedLayer.empty() ? DefaultEditorLayerName(prefab.object) : normalizedLayer) << ' '
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
