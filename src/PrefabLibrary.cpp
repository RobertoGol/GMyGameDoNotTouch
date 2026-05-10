#include "../include/PrefabLibrary.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

#include "../include/AppPaths.hpp"

namespace bunker {

namespace {

std::string TrimCopy(std::string_view value) {
    std::size_t first = 0;
    while (first < value.size()) {
        const char ch = value[first];
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') {
            break;
        }
        ++first;
    }
    if (first >= value.size()) {
        return {};
    }

    std::size_t last = value.size();
    while (last > first) {
        const char ch = value[last - 1];
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') {
            break;
        }
        --last;
    }
    return std::string(value.substr(first, last - first));
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

float NormalizeDegrees(float value) {
    value = std::fmod(value, 360.0f);
    if (value < 0.0f) {
        value += 360.0f;
    }
    return value;
}

void NormalizeLootEntry(LootEntry& entry) {
    entry.minCount = std::max(1, entry.minCount);
    entry.maxCount = std::max(entry.minCount, entry.maxCount);
    if (!std::isfinite(entry.weight) || entry.weight < 0.0f) {
        entry.weight = 1.0f;
    }
}

void TrimTrailingEmptyLootEntries(std::vector<LootEntry>& entries) {
    while (!entries.empty() && entries.back().itemId.empty()) {
        entries.pop_back();
    }
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
    prefab.object.rotationX = NormalizeDegrees(prefab.object.rotationX);
    prefab.object.rotationY = NormalizeDegrees(prefab.object.rotationY);
    prefab.object.rotationZ = NormalizeDegrees(prefab.object.rotationZ);
    if (prefab.object.editorLayer.empty()) {
        prefab.object.editorLayer = DefaultEditorLayerName(prefab.object);
    }

    if (prefab.object.lootMode != LootMode::ManualList && prefab.object.lootMode != LootMode::RandomTable) {
        prefab.object.lootMode = LootMode::ManualList;
    }
    if (prefab.object.lootEntries.empty()) {
        for (const auto& lootId : prefab.object.manualLootIds) {
            if (!lootId.empty()) {
                prefab.object.lootEntries.push_back({lootId, 1, 1, 1.0f});
            }
        }
    }
    for (auto& entry : prefab.object.lootEntries) {
        NormalizeLootEntry(entry);
    }
    TrimTrailingEmptyLootEntries(prefab.object.lootEntries);
    prefab.object.manualLoot = prefab.object.manualLoot || !prefab.object.lootEntries.empty();
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

    bool prefabLinesHaveZ = true;
    bool prefabLinesHaveRotation = false;
    bool prefabLinesHaveScalableLoot = false;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        if (line.starts_with('#')) {
            if (line.find("BUNKER_PREFABS_V") != std::string::npos) {
                prefabLinesHaveZ =
                    line.find("BUNKER_PREFABS_V4") != std::string::npos ||
                    line.find("BUNKER_PREFABS_V5") != std::string::npos ||
                    line.find("BUNKER_PREFABS_V6") != std::string::npos;
                prefabLinesHaveRotation =
                    line.find("BUNKER_PREFABS_V5") != std::string::npos ||
                    line.find("BUNKER_PREFABS_V6") != std::string::npos;
                prefabLinesHaveScalableLoot =
                    line.find("BUNKER_PREFABS_V6") != std::string::npos;
            }
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
                >> prefab.object.y)) {
            return false;
        }
        if (prefabLinesHaveRotation) {
            if (!(lineStream >> prefab.object.z
                    >> prefab.object.rotationX
                    >> prefab.object.rotationY
                    >> prefab.object.rotationZ
                    >> prefab.object.width
                    >> prefab.object.depth
                    >> prefab.object.height
                    >> prefab.object.health
                    >> prefab.object.blocksMovement
                    >> prefab.object.discovered
                    >> prefab.object.manualLoot)) {
                return false;
            }
        } else if (prefabLinesHaveZ) {
            if (!(lineStream >> prefab.object.z
                    >> prefab.object.width
                    >> prefab.object.depth
                    >> prefab.object.height
                    >> prefab.object.health
                    >> prefab.object.blocksMovement
                    >> prefab.object.discovered
                    >> prefab.object.manualLoot)) {
                return false;
            }
            prefab.object.rotationX = 0.0f;
            prefab.object.rotationY = 0.0f;
            prefab.object.rotationZ = 0.0f;
        } else {
            prefab.object.z = 0.0f;
            prefab.object.rotationX = 0.0f;
            prefab.object.rotationY = 0.0f;
            prefab.object.rotationZ = 0.0f;
            if (!(lineStream >> prefab.object.width
                    >> prefab.object.depth
                    >> prefab.object.height
                    >> prefab.object.health
                    >> prefab.object.blocksMovement
                    >> prefab.object.discovered
                    >> prefab.object.manualLoot)) {
                return false;
            }
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
        std::uint32_t lootMode = 0;
        std::uint32_t lootEntryCount = 0;
        if (prefabLinesHaveScalableLoot) {
            if (lineStream >> lootMode >> lootEntryCount) {
                prefab.object.lootMode = static_cast<LootMode>(lootMode);
                prefab.object.lootEntries.clear();
                prefab.object.lootEntries.reserve(lootEntryCount);
                for (std::uint32_t lootIndex = 0; lootIndex < lootEntryCount; ++lootIndex) {
                    LootEntry entry;
                    if (!(lineStream >> std::quoted(entry.itemId)
                            >> entry.minCount
                            >> entry.maxCount
                            >> entry.weight)) {
                        return false;
                    }
                    prefab.object.lootEntries.push_back(std::move(entry));
                }
            } else {
                lineStream.clear();
            }
        }
        if (prefab.object.lootEntries.empty()) {
            for (const auto& lootId : prefab.object.manualLootIds) {
                if (!lootId.empty()) {
                    prefab.object.lootEntries.push_back({lootId, 1, 1, 1.0f});
                }
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

    file << "# BUNKER_PREFABS_V6\n";
    for (auto prefab : prefabs) {
        NormalizePrefabRecord(prefab);
        prefab.object.manualLootIds = {};
        for (std::size_t lootIndex = 0;
             lootIndex < prefab.object.manualLootIds.size() && lootIndex < prefab.object.lootEntries.size();
             ++lootIndex) {
            prefab.object.manualLootIds[lootIndex] = prefab.object.lootEntries[lootIndex].itemId;
        }
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
             << prefab.object.rotationX << ' '
             << prefab.object.rotationY << ' '
             << prefab.object.rotationZ << ' '
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
        file << ' ' << static_cast<std::uint32_t>(prefab.object.lootMode)
             << ' ' << static_cast<std::uint32_t>(prefab.object.lootEntries.size());
        for (const auto& entry : prefab.object.lootEntries) {
            file << ' ' << std::quoted(entry.itemId)
                 << ' ' << entry.minCount
                 << ' ' << entry.maxCount
                 << ' ' << entry.weight;
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
