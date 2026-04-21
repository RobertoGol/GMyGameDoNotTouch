#include "../include/PrefabLibrary.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "../include/AppPaths.hpp"

namespace bunker {

namespace {

std::string TrimCopy(std::string_view value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return std::string(value.substr(first, last - first + 1));
}

std::string ToLowerCopy(std::string_view value) {
    std::string lower(value);
    for (char& ch : lower) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return lower;
}

std::string CollapseUnderscores(std::string value) {
    value.erase(
        std::unique(value.begin(), value.end(), [](char lhs, char rhs) {
            return lhs == '_' && rhs == '_';
        }),
        value.end());
    while (!value.empty() && value.front() == '_') {
        value.erase(value.begin());
    }
    while (!value.empty() && value.back() == '_') {
        value.pop_back();
    }
    return value;
}

std::string SanitizePrefabIdStem(std::string_view value) {
    std::string stem;
    stem.reserve(value.size());
    for (const char ch : value) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            stem.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        } else {
            stem.push_back('_');
        }
    }
    stem = CollapseUnderscores(std::move(stem));
    if (stem.empty()) {
        stem = "captured_prefab";
    }
    return stem;
}

std::string NormalizeTargetType(std::string_view value) {
    const std::string lower = ToLowerCopy(TrimCopy(value));
    if (lower == "prop") {
        return "Prop";
    }
    if (lower == "item") {
        return "Item";
    }
    if (lower == "structure") {
        return "Structure";
    }
    if (lower == "environment") {
        return "Environment";
    }
    if (lower == "scene module" || lower == "scene_module" || lower == "scenemodule") {
        return "Scene Module";
    }
    return {};
}

bool ReadOptionalQuotedString(std::istringstream& lineStream, std::string& value) {
    std::streampos originalPos = lineStream.tellg();
    std::string parsedValue;
    if (lineStream >> std::quoted(parsedValue)) {
        value = std::move(parsedValue);
        return true;
    }

    lineStream.clear();
    if (originalPos != std::streampos(-1)) {
        lineStream.seekg(originalPos);
    }
    return false;
}

}  // namespace

std::string NormalizePrefabRecordId(std::string_view prefabId) {
    std::string trimmed = TrimCopy(prefabId);
    if (trimmed.empty()) {
        return {};
    }

    std::string normalized = SanitizePrefabIdStem(trimmed);
    if (!normalized.starts_with("prefab_")) {
        normalized = "prefab_" + normalized;
    }
    return normalized;
}

std::string DefaultPrefabTargetType(const MapObject& object) {
    switch (object.category) {
    case ObjectCategory::Container:
        return "Item";
    case ObjectCategory::Structure:
    case ObjectCategory::Hangar:
        return "Structure";
    case ObjectCategory::Landmark:
        return object.interaction == InteractionType::Transition ? "Scene Module" : "Environment";
    case ObjectCategory::ResourceNode:
        return "Environment";
    case ObjectCategory::Terminal:
    case ObjectCategory::Vehicle:
    case ObjectCategory::Hostile:
    default:
        return "Prop";
    }
}

void NormalizePrefabRecord(PrefabRecord& prefab) {
    prefab.label = TrimCopy(prefab.label);
    if (prefab.label.empty()) {
        prefab.label = prefab.object.displayName.empty() ? "Captured Prefab" : prefab.object.displayName;
    }

    prefab.id = NormalizePrefabRecordId(prefab.id.empty() ? prefab.label : prefab.id);
    prefab.targetType = NormalizeTargetType(prefab.targetType);
    if (prefab.targetType.empty()) {
        prefab.targetType = DefaultPrefabTargetType(prefab.object);
    }

    prefab.sourceLabel = TrimCopy(prefab.sourceLabel);
    prefab.completionMode = TrimCopy(prefab.completionMode);
    if (prefab.completionMode.empty()) {
        prefab.completionMode = "Captured";
    }

    prefab.object.editorLayer = NormalizeEditorLayerName(prefab.object.editorLayer);
    if (prefab.object.editorLayer.empty()) {
        prefab.object.editorLayer = DefaultEditorLayerName(prefab.object);
    }
}

int FindPrefabRecordIndexById(const std::vector<PrefabRecord>& prefabs, std::string_view prefabId) {
    const std::string normalizedPrefabId = NormalizePrefabRecordId(prefabId);
    if (normalizedPrefabId.empty()) {
        return -1;
    }

    for (int index = 0; index < static_cast<int>(prefabs.size()); ++index) {
        if (NormalizePrefabRecordId(prefabs[static_cast<std::size_t>(index)].id) == normalizedPrefabId) {
            return index;
        }
    }
    return -1;
}

const PrefabRecord* FindPrefabRecordById(const std::vector<PrefabRecord>& prefabs, std::string_view prefabId) {
    const int index = FindPrefabRecordIndexById(prefabs, prefabId);
    if (index < 0) {
        return nullptr;
    }
    return &prefabs[static_cast<std::size_t>(index)];
}

int CountPrefabUsageInWorld(const World& world, std::string_view prefabId) {
    return static_cast<int>(CollectPrefabUsageObjectIndices(world, prefabId).size());
}

int CountPrefabDerivedObjects(const World& world) {
    int count = 0;
    for (const auto& object : world.objects) {
        if (!object.prefabSourceId.empty()) {
            ++count;
        }
    }
    return count;
}

std::vector<int> CollectPrefabUsageObjectIndices(const World& world, std::string_view prefabId) {
    std::vector<int> indices;
    const std::string normalizedPrefabId = NormalizePrefabRecordId(prefabId);
    if (normalizedPrefabId.empty()) {
        return indices;
    }

    for (int index = 0; index < static_cast<int>(world.objects.size()); ++index) {
        if (NormalizePrefabRecordId(world.objects[static_cast<std::size_t>(index)].prefabSourceId) == normalizedPrefabId) {
            indices.push_back(index);
        }
    }
    return indices;
}

std::vector<int> CollectBrokenPrefabReferenceObjectIndices(
    const World& world,
    const std::vector<PrefabRecord>& prefabs) {
    std::vector<int> indices;
    for (int index = 0; index < static_cast<int>(world.objects.size()); ++index) {
        const auto& object = world.objects[static_cast<std::size_t>(index)];
        if (object.prefabSourceId.empty()) {
            continue;
        }
        if (FindPrefabRecordById(prefabs, object.prefabSourceId) == nullptr) {
            indices.push_back(index);
        }
    }
    return indices;
}

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

        std::streampos extraDataPos = lineStream.tellg();
        if (ReadOptionalQuotedString(lineStream, prefab.id)) {
            if (!(lineStream >> std::quoted(prefab.targetType)
                    >> std::quoted(prefab.sourceLabel)
                    >> std::quoted(prefab.completionMode))) {
                return false;
            }
        } else {
            lineStream.clear();
            if (extraDataPos != std::streampos(-1)) {
                lineStream.seekg(extraDataPos);
            }
        }

        NormalizePrefabRecord(prefab);
        prefabs.push_back(std::move(prefab));
    }

    return true;
}

bool SavePrefabLibrary(const std::vector<PrefabRecord>& prefabs) {
    std::ofstream file(EditorPrefabLibraryPath());
    if (!file.is_open()) {
        return false;
    }

    file << "# BUNKER_PREFABS_V4\n";
    for (auto prefab : prefabs) {
        NormalizePrefabRecord(prefab);
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
        file << ' ' << std::quoted(prefab.id)
             << ' ' << std::quoted(prefab.targetType)
             << ' ' << std::quoted(prefab.sourceLabel)
             << ' ' << std::quoted(prefab.completionMode)
             << '\n';
    }

    return static_cast<bool>(file);
}

}  // namespace bunker
