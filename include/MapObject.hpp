#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace bunker {

enum class InteractionType {
    Static,
    Container,
    Resource,
    Terminal,
    Transition,
    VehicleAnchor,
    Workshop,
    Hostile
};

enum class ObjectCategory {
    Structure,
    ResourceNode,
    Resource = ResourceNode,
    Terminal,
    Vehicle,
    Landmark,
    Container,
    Hangar,
    Hostile
};

enum class LootMode : std::uint32_t {
    ManualList = 0,
    RandomTable = 1
};

struct LootEntry {
    std::string itemId;
    int minCount = 1;
    int maxCount = 1;
    float weight = 1.0f;

    bool operator==(const LootEntry&) const = default;
};

struct MapObject {
    std::string registryId;
    std::string displayName;
    InteractionType interaction = InteractionType::Static;
    ObjectCategory category = ObjectCategory::Structure;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    float width = 1.0f;
    float depth = 1.0f;
    float height = 1.0f;
    float health = 100.0f;

    bool blocksMovement = true;
    bool discovered = false;
    bool manualLoot = false;
    std::array<std::string, 4> manualLootIds{};
    std::string scriptTag;
    std::string linkTarget;
    std::string prefabSourceId;
    bool semanticAutoCreated = false;
    bool semanticLayoutPinned = false;
    std::string editorLayer;
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationZ = 0.0f;
    LootMode lootMode = LootMode::ManualList;
    std::vector<LootEntry> lootEntries;
};

}  // namespace bunker
