#include "../include/WorldSemanticAuthoring.hpp"

#include <algorithm>
#include <deque>
#include <functional>
#include <iomanip>
#include <map>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "../include/GameplayDescriptorRegistry.hpp"
#include "../include/WorldValidation.hpp"

namespace bunker {

namespace {

std::string MakeAutoAnchorRegistryId(const World& world, std::string_view scriptTag) {
    for (int index = 1; index <= 9999; ++index) {
        std::ostringstream candidate;
        candidate << "[%" << scriptTag << "_auto_" << std::setw(4) << std::setfill('0') << index << "]";
        if (!world.HasObject(candidate.str())) {
            return candidate.str();
        }
    }
    return "[%semantic_anchor_overflow]";
}

void SeedGenericDescriptorShape(const GameplayDescriptorSpec& spec, MapObject& object) {
    object.health = 100.0f;
    object.discovered = true;
    object.manualLoot = false;
    object.manualLootIds = {};
    object.lootMode = LootMode::ManualList;
    object.lootEntries.clear();
    object.blocksMovement = false;

    if (spec.preferredCategory == ObjectCategory::Landmark ||
        spec.preferredInteraction == InteractionType::Transition) {
        object.width = 2.8f;
        object.depth = 1.8f;
        object.height = 2.2f;
    } else if (spec.preferredCategory == ObjectCategory::Hangar ||
               spec.preferredInteraction == InteractionType::Workshop) {
        object.width = 3.4f;
        object.depth = 2.6f;
        object.height = 2.8f;
    } else {
        object.width = 1.6f;
        object.depth = 1.4f;
        object.height = 2.0f;
    }

    if (spec.scriptTag == "tower_sync") {
        object.width = 1.8f;
        object.depth = 1.6f;
        object.height = 3.2f;
    } else if (spec.scriptTag == "relay_substation") {
        object.width = 3.1f;
        object.depth = 2.6f;
        object.height = 3.1f;
    } else if (spec.scriptTag == "service_bay") {
        object.width = 3.3f;
        object.depth = 2.8f;
        object.height = 3.0f;
    } else if (spec.scriptTag == "recovery_fabricator") {
        object.width = 3.0f;
        object.depth = 2.4f;
        object.height = 2.8f;
    }
}

bool BuildSemanticAnchorPrototype(std::string_view scriptTag, MapObject& object) {
    World templateWorld;
    templateWorld.GeneratePrototypeZone();
    templateWorld.EnsureStarterInfrastructure();
    if (const auto* templateObject = templateWorld.FindObjectByScriptTag(std::string(scriptTag)); templateObject != nullptr) {
        object = *templateObject;
        object.registryId.clear();
        object.x = 0.0f;
        object.y = 0.0f;
        object.z = 0.0f;
        object.discovered = true;
        return true;
    }

    const auto* spec = FindGameplayDescriptor(scriptTag);
    if (spec == nullptr) {
        return false;
    }

    object = {};
    object.scriptTag = std::string(spec->scriptTag);
    object.displayName = std::string(spec->label);
    AlignObjectToDescriptorDefaults(object, true);
    SeedGenericDescriptorShape(*spec, object);
    return true;
}

void PositionAnchorNearSource(MapObject& object, const MapObject* source, int ordinal) {
    if (source == nullptr) {
        object.x = 2.5f + static_cast<float>(ordinal % 3) * 2.0f;
        object.y = static_cast<float>((ordinal % 2 == 0) ? 0.0f : 2.0f);
        return;
    }

    const float lateralOffset = std::max(2.5f, source->width * 0.75f + 2.0f);
    const float verticalOffset = 1.5f + static_cast<float>(ordinal % 3) * 0.8f;
    object.x = source->x + lateralOffset;
    object.y = source->y + ((ordinal % 2 == 0) ? verticalOffset : -verticalOffset);
}

int FindObjectIndexByRegistryId(const World& world, const std::string& registryId) {
    for (int index = 0; index < static_cast<int>(world.objects.size()); ++index) {
        if (world.objects[static_cast<std::size_t>(index)].registryId == registryId) {
            return index;
        }
    }
    return -1;
}

int FindObjectIndexByScriptTag(const World& world, std::string_view scriptTag) {
    const std::string normalizedTag = std::string(NormalizeGameplayDescriptorTag(scriptTag));
    for (int index = 0; index < static_cast<int>(world.objects.size()); ++index) {
        if (world.objects[static_cast<std::size_t>(index)].scriptTag == normalizedTag) {
            return index;
        }
    }
    return -1;
}

}  // namespace

std::vector<std::string_view> RequiredSemanticDependencyTags(std::string_view scriptTag) {
    scriptTag = NormalizeGameplayDescriptorTag(scriptTag);

    if (scriptTag == "lanline_service_hub" || scriptTag == "medical_support") {
        return {"tower_sync"};
    }
    if (scriptTag == "tank_service") {
        return {"service_bay"};
    }
    if (scriptTag == "relay_substation") {
        return {"capacitor_bank", "reactor_yard", "industrial_outpost"};
    }
    if (scriptTag == "service_bay") {
        return {"relay_substation", "foundry_line", "industrial_outpost"};
    }
    if (scriptTag == "water_reclaimer") {
        return {"service_bay", "relay_substation", "recovery_fabricator"};
    }

    return {};
}

std::vector<SemanticDependencyEdge> BuildSemanticDependencyGraph(const World& world, int rootObjectIndex) {
    std::vector<SemanticDependencyEdge> edges;
    if (rootObjectIndex < 0 || rootObjectIndex >= static_cast<int>(world.objects.size())) {
        return edges;
    }

    std::unordered_set<int> visitedObjectIndices;
    std::function<void(int)> visit = [&](int objectIndex) {
        if (objectIndex < 0 || objectIndex >= static_cast<int>(world.objects.size())) {
            return;
        }
        if (!visitedObjectIndices.insert(objectIndex).second) {
            return;
        }

        const auto& object = world.objects[static_cast<std::size_t>(objectIndex)];
        for (const std::string_view dependencyTag : RequiredSemanticDependencyTags(object.scriptTag)) {
            const int dependencyObjectIndex = FindObjectIndexByScriptTag(world, dependencyTag);
            edges.push_back({
                objectIndex,
                dependencyObjectIndex,
                object.scriptTag,
                std::string(dependencyTag),
                dependencyObjectIndex >= 0
            });
            if (dependencyObjectIndex >= 0) {
                visit(dependencyObjectIndex);
            }
        }
    };

    visit(rootObjectIndex);
    return edges;
}

std::vector<int> CollectSemanticDependencyObjectIndices(const World& world, int rootObjectIndex, bool includeRoot) {
    std::vector<int> objectIndices;
    std::unordered_set<int> seenObjectIndices;
    auto tryPushObjectIndex = [&](int objectIndex) {
        if (objectIndex < 0 || objectIndex >= static_cast<int>(world.objects.size())) {
            return;
        }
        if (seenObjectIndices.insert(objectIndex).second) {
            objectIndices.push_back(objectIndex);
        }
    };

    if (includeRoot) {
        tryPushObjectIndex(rootObjectIndex);
    }

    for (const auto& edge : BuildSemanticDependencyGraph(world, rootObjectIndex)) {
        tryPushObjectIndex(edge.sourceObjectIndex);
        tryPushObjectIndex(edge.dependencyObjectIndex);
    }

    return objectIndices;
}

bool IsAutoGeneratedSemanticAnchor(const MapObject& object) {
    return object.semanticAutoCreated;
}

bool IsPinnedSemanticAnchor(const MapObject& object) {
    return object.semanticLayoutPinned;
}

bool PinSemanticAnchorPlacement(MapObject& object, bool pinned) {
    if (object.semanticLayoutPinned == pinned) {
        return false;
    }
    object.semanticLayoutPinned = pinned;
    return true;
}

bool AdoptSemanticAnchorAsAuthored(MapObject& object, bool pinPlacement) {
    bool changed = false;
    if (object.semanticAutoCreated) {
        object.semanticAutoCreated = false;
        changed = true;
    }
    if (object.semanticLayoutPinned != pinPlacement) {
        object.semanticLayoutPinned = pinPlacement;
        changed = true;
    }
    return changed;
}

int AdoptAllAutoCreatedSemanticAnchors(World& world, std::string& statusText) {
    int adoptedAnchors = 0;
    for (auto& object : world.objects) {
        if (!IsAutoGeneratedSemanticAnchor(object)) {
            continue;
        }
        if (AdoptSemanticAnchorAsAuthored(object, true)) {
            ++adoptedAnchors;
        }
    }

    statusText = adoptedAnchors > 0
        ? "Adopted " + std::to_string(adoptedAnchors) + " auto-created semantic anchors across the world."
        : "No auto-created semantic anchors to adopt.";
    return adoptedAnchors;
}

int AdoptSemanticDependencyChainAsAuthored(World& world, int rootObjectIndex, bool includeRoot, std::string& statusText) {
    if (rootObjectIndex < 0 || rootObjectIndex >= static_cast<int>(world.objects.size())) {
        statusText = "Semantic adopt skipped: invalid root object.";
        return 0;
    }

    const auto dependencyObjectIndices = CollectSemanticDependencyObjectIndices(world, rootObjectIndex, includeRoot);
    int adoptedAnchors = 0;
    for (const int objectIndex : dependencyObjectIndices) {
        if (objectIndex == rootObjectIndex && !includeRoot) {
            continue;
        }
        if (objectIndex < 0 || objectIndex >= static_cast<int>(world.objects.size())) {
            continue;
        }

        auto& object = world.objects[static_cast<std::size_t>(objectIndex)];
        if (!IsAutoGeneratedSemanticAnchor(object)) {
            continue;
        }
        if (AdoptSemanticAnchorAsAuthored(object, true)) {
            ++adoptedAnchors;
        }
    }

    const auto& rootObject = world.objects[static_cast<std::size_t>(rootObjectIndex)];
    statusText = adoptedAnchors > 0
        ? "Adopted " + std::to_string(adoptedAnchors) + " semantic anchors as authored for " + rootObject.registryId + "."
        : "No auto-created semantic anchors to adopt for " + rootObject.registryId + ".";
    return adoptedAnchors;
}

int AutoLayoutSemanticDependencyChain(World& world,
    int rootObjectIndex,
    std::string& statusText,
    const SemanticLayoutOptions& options) {
    if (rootObjectIndex < 0 || rootObjectIndex >= static_cast<int>(world.objects.size())) {
        statusText = "Semantic layout skipped: invalid root object.";
        return 0;
    }

    const auto graph = BuildSemanticDependencyGraph(world, rootObjectIndex);
    if (graph.empty()) {
        statusText = "Semantic layout skipped: selected object has no dependency chain.";
        return 0;
    }

    const auto& rootObject = world.objects[static_cast<std::size_t>(rootObjectIndex)];
    std::map<int, std::vector<int>> objectsByDepth;
    std::unordered_set<int> seenObjects;
    std::unordered_map<int, int> depthByObject{};
    std::unordered_map<int, int> encounterOrder{};
    std::unordered_map<int, std::vector<int>> resolvedDependenciesByObject{};

    encounterOrder[rootObjectIndex] = 0;
    int nextEncounterOrder = 1;
    for (const auto& edge : graph) {
        if (!edge.dependencyPresent ||
            edge.sourceObjectIndex < 0 ||
            edge.dependencyObjectIndex < 0) {
            continue;
        }
        resolvedDependenciesByObject[edge.sourceObjectIndex].push_back(edge.dependencyObjectIndex);
        if (!encounterOrder.contains(edge.dependencyObjectIndex)) {
            encounterOrder.emplace(edge.dependencyObjectIndex, nextEncounterOrder++);
        }
    }

    depthByObject[rootObjectIndex] = 0;
    std::deque<int> pendingObjectIndices{};
    pendingObjectIndices.push_back(rootObjectIndex);
    while (!pendingObjectIndices.empty()) {
        const int objectIndex = pendingObjectIndices.front();
        pendingObjectIndices.pop_front();
        const int objectDepth = depthByObject[objectIndex];
        const auto adjacencyIt = resolvedDependenciesByObject.find(objectIndex);
        if (adjacencyIt == resolvedDependenciesByObject.end()) {
            continue;
        }

        for (const int dependencyObjectIndex : adjacencyIt->second) {
            const int dependencyDepth = objectDepth + 1;
            auto [existingDepthIt, inserted] = depthByObject.emplace(dependencyObjectIndex, dependencyDepth);
            if (inserted || dependencyDepth < existingDepthIt->second) {
                existingDepthIt->second = dependencyDepth;
                pendingObjectIndices.push_back(dependencyObjectIndex);
            }
        }
    }

    float widestObject = rootObject.width;
    float deepestFootprint = rootObject.depth;
    for (const auto& [objectIndex, depth] : depthByObject) {
        if (objectIndex < 0 || objectIndex >= static_cast<int>(world.objects.size()) || depth == 0) {
            continue;
        }
        const auto& object = world.objects[static_cast<std::size_t>(objectIndex)];
        widestObject = std::max(widestObject, object.width);
        deepestFootprint = std::max(deepestFootprint, object.depth);
        if (seenObjects.insert(objectIndex).second) {
            objectsByDepth[depth].push_back(objectIndex);
        }
    }

    int movedObjects = 0;
    int preservedManualAnchors = 0;
    const float depthSpacing = std::max(7.5f, widestObject + 4.5f);
    const float laneSpacing = std::max(4.5f, deepestFootprint + 2.5f);
    for (auto& [depth, objectIndices] : objectsByDepth) {
        std::sort(objectIndices.begin(), objectIndices.end(), [&](int lhs, int rhs) {
            const int lhsOrder = encounterOrder.contains(lhs) ? encounterOrder[lhs] : lhs;
            const int rhsOrder = encounterOrder.contains(rhs) ? encounterOrder[rhs] : rhs;
            return lhsOrder < rhsOrder;
        });

        const float layerCenter = (static_cast<float>(objectIndices.size()) - 1.0f) * 0.5f;
        for (int laneIndex = 0; laneIndex < static_cast<int>(objectIndices.size()); ++laneIndex) {
            const int objectIndex = objectIndices[static_cast<std::size_t>(laneIndex)];
            auto& object = world.objects[static_cast<std::size_t>(objectIndex)];
            if (options.preserveManualPlacement &&
                (IsPinnedSemanticAnchor(object) || !IsAutoGeneratedSemanticAnchor(object))) {
                ++preservedManualAnchors;
                continue;
            }
            const float targetX = rootObject.x + depthSpacing * static_cast<float>(depth);
            const float targetY = rootObject.y + (static_cast<float>(laneIndex) - layerCenter) * laneSpacing;
            if (object.x != targetX || object.y != targetY) {
                object.x = targetX;
                object.y = targetY;
                ++movedObjects;
            }
        }
    }

    if (movedObjects > 0 && preservedManualAnchors > 0) {
        statusText = "Reflowed semantic chain for " + rootObject.registryId +
            " (" + std::to_string(movedObjects) + " auto anchors, preserved " +
            std::to_string(preservedManualAnchors) + " authored/pinned anchors).";
    } else if (movedObjects > 0) {
        statusText = "Reflowed semantic chain for " + rootObject.registryId +
            " (" + std::to_string(movedObjects) + " anchors).";
    } else if (preservedManualAnchors > 0) {
        statusText = "Semantic chain preserved " + std::to_string(preservedManualAnchors) +
            " authored/pinned anchors for " + rootObject.registryId + ".";
    } else {
        statusText = "Semantic chain already matches layout for " + rootObject.registryId + ".";
    }
    return movedObjects;
}

int AutoLayoutSemanticDependencyChain(World& world, int rootObjectIndex, std::string& statusText) {
    return AutoLayoutSemanticDependencyChain(world, rootObjectIndex, statusText, {});
}

bool AlignObjectToDescriptorDefaults(MapObject& object, bool overwriteLinkTarget) {
    bool changed = false;
    const std::string normalizedTag = std::string(NormalizeGameplayDescriptorTag(object.scriptTag));
    if (object.scriptTag != normalizedTag) {
        object.scriptTag = normalizedTag;
        changed = true;
    }

    const auto* spec = FindGameplayDescriptor(object.scriptTag);
    if (spec == nullptr) {
        return changed;
    }

    if (object.interaction != spec->preferredInteraction) {
        object.interaction = spec->preferredInteraction;
        changed = true;
    }
    if (object.category != spec->preferredCategory) {
        object.category = spec->preferredCategory;
        changed = true;
    }
    if (object.displayName.empty()) {
        object.displayName = std::string(spec->label);
        changed = true;
    }

    const char* defaultLinkTarget = DefaultGameplayDescriptorLinkTarget(spec->scriptTag);
    if (overwriteLinkTarget && defaultLinkTarget != nullptr && object.linkTarget != defaultLinkTarget) {
        object.linkTarget = defaultLinkTarget;
        changed = true;
    }

    return changed;
}

bool CanAutoFixValidationIssue(const ValidationIssue& issue) {
    return issue.code == "legacy_script_tag_alias" ||
        issue.code == "interaction_mismatch" ||
        issue.code == "category_mismatch" ||
        issue.code == "missing_required_link_target" ||
        issue.code == "missing_canonical_link_target" ||
        issue.code == "descriptor_link_target_mismatch" ||
        issue.code == "transition_without_link_target" ||
        issue.code == "missing_display_name";
}

bool AutoFixValidationIssue(World& world, const ValidationIssue& issue, std::string& statusText) {
    if (!CanAutoFixValidationIssue(issue) || issue.objectId.empty()) {
        return false;
    }

    const int objectIndex = FindObjectIndexByRegistryId(world, issue.objectId);
    if (objectIndex < 0) {
        return false;
    }

    auto& object = world.objects[static_cast<std::size_t>(objectIndex)];
    if (!AlignObjectToDescriptorDefaults(object, true)) {
        return false;
    }

    statusText = "Applied safe validation fix to " + object.registryId + ".";
    return true;
}

int AutoFixSafeValidationIssues(World& world, std::string& statusText) {
    const auto issues = ValidateWorldForRuntime(world);
    int appliedFixes = 0;
    for (const auto& issue : issues) {
        std::string localStatus;
        if (AutoFixValidationIssue(world, issue, localStatus)) {
            ++appliedFixes;
            statusText = localStatus;
        }
    }

    if (appliedFixes == 0) {
        statusText = "No safe validation fixes available.";
    } else {
        statusText = "Applied " + std::to_string(appliedFixes) + " safe validation fixes.";
    }
    return appliedFixes;
}

bool CanCreateMissingDependencyAnchor(const ValidationIssue& issue) {
    return issue.code == "missing_authored_dependency" && !issue.relatedValue.empty();
}

bool CreateMissingDependencyAnchorForIssue(World& world, const ValidationIssue& issue, int& createdObjectIndex, std::string& statusText) {
    createdObjectIndex = -1;
    if (!CanCreateMissingDependencyAnchor(issue)) {
        return false;
    }
    if (world.FindObjectByScriptTag(issue.relatedValue) != nullptr) {
        statusText = "Missing dependency already exists in world: " + issue.relatedValue;
        return false;
    }

    MapObject createdAnchor;
    if (!BuildSemanticAnchorPrototype(issue.relatedValue, createdAnchor)) {
        statusText = "Failed to build anchor prototype for " + issue.relatedValue + ".";
        return false;
    }

    createdAnchor.registryId = MakeAutoAnchorRegistryId(world, issue.relatedValue);
    createdAnchor.semanticAutoCreated = true;
    createdAnchor.semanticLayoutPinned = false;
    const int sourceObjectIndex = issue.objectId.empty() ? -1 : FindObjectIndexByRegistryId(world, issue.objectId);
    const MapObject* sourceObject = sourceObjectIndex >= 0
        ? &world.objects[static_cast<std::size_t>(sourceObjectIndex)]
        : nullptr;
    PositionAnchorNearSource(createdAnchor, sourceObject, static_cast<int>(world.objects.size()));
    world.AddObject(createdAnchor);
    createdObjectIndex = static_cast<int>(world.objects.size()) - 1;
    statusText = "Created missing anchor '" + issue.relatedValue + "' for " +
        (issue.objectId.empty() ? std::string("validation flow.") : issue.objectId + ".");
    return true;
}

SemanticAnchorBatchResult CreateMissingDependencyAnchorsCascade(World& world, std::string& statusText) {
    SemanticAnchorBatchResult result;
    for (int pass = 0; pass < 12; ++pass) {
        const auto issues = ValidateWorldForRuntime(world);
        bool createdInPass = false;
        for (const auto& issue : issues) {
            int createdObjectIndex = -1;
            std::string localStatus;
            if (CreateMissingDependencyAnchorForIssue(world, issue, createdObjectIndex, localStatus)) {
                createdInPass = true;
                ++result.createdCount;
                result.lastCreatedObjectIndex = createdObjectIndex;
                result.createdObjectIndices.push_back(createdObjectIndex);
                if (createdObjectIndex >= 0 && createdObjectIndex < static_cast<int>(world.objects.size())) {
                    result.createdScriptTags.push_back(world.objects[static_cast<std::size_t>(createdObjectIndex)].scriptTag);
                }
            }
        }
        if (!createdInPass) {
            break;
        }
    }

    statusText = (result.createdCount > 0)
        ? "Created " + std::to_string(result.createdCount) + " missing dependency anchors in cascade."
        : "No missing dependency anchors were created.";
    return result;
}

}  // namespace bunker
