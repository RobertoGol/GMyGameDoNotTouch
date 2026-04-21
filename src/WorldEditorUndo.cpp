#include "../include/WorldEditorUndo.hpp"

#include <algorithm>
#include <utility>

namespace bunker {

namespace {

std::string DescribeUndoAction(std::string_view prefix, const WorldEditorUndoRecord& record) {
    if (!record.label.empty()) {
        return std::string(prefix) + ": " + record.label;
    }
    return std::string(prefix) + ".";
}

bool RestoreObjectAt(World& world, const MapObject& object, int preferredIndex) {
    if (preferredIndex >= 0 && preferredIndex <= static_cast<int>(world.objects.size())) {
        world.objects.insert(world.objects.begin() + preferredIndex, object);
        return true;
    }

    world.AddObject(object);
    return true;
}

int ResolveObjectIndex(const World& world,
    const std::optional<MapObject>& primaryObject,
    const std::optional<MapObject>& secondaryObject,
    int preferredIndex) {
    if (primaryObject.has_value()) {
        if (const MapObject* object = world.FindObjectByRegistryId(primaryObject->registryId)) {
            return static_cast<int>(object - world.objects.data());
        }
    }
    if (secondaryObject.has_value()) {
        if (const MapObject* object = world.FindObjectByRegistryId(secondaryObject->registryId)) {
            return static_cast<int>(object - world.objects.data());
        }
    }
    if (preferredIndex >= 0 && preferredIndex < static_cast<int>(world.objects.size())) {
        return preferredIndex;
    }
    return -1;
}

bool ReplaceObjectAt(World& world,
    const std::optional<MapObject>& primaryObject,
    const std::optional<MapObject>& secondaryObject,
    int preferredIndex,
    const MapObject& replacement) {
    const int objectIndex = ResolveObjectIndex(world, primaryObject, secondaryObject, preferredIndex);
    if (objectIndex < 0 || objectIndex >= static_cast<int>(world.objects.size())) {
        return false;
    }
    world.objects[static_cast<std::size_t>(objectIndex)] = replacement;
    return true;
}

bool RemoveObjectAt(World& world,
    const std::optional<MapObject>& primaryObject,
    const std::optional<MapObject>& secondaryObject,
    int preferredIndex) {
    const int objectIndex = ResolveObjectIndex(world, primaryObject, secondaryObject, preferredIndex);
    if (objectIndex < 0 || objectIndex >= static_cast<int>(world.objects.size())) {
        return false;
    }
    world.objects.erase(world.objects.begin() + objectIndex);
    return true;
}

}  // namespace

void WorldEditorUndoStack::Clear() {
    records_.clear();
    nextRecordIndex_ = 0;
    savedRecordIndex_ = 0;
}

bool WorldEditorUndoStack::CanUndo() const {
    return nextRecordIndex_ > 0;
}

bool WorldEditorUndoStack::CanRedo() const {
    return nextRecordIndex_ < records_.size();
}

bool WorldEditorUndoStack::IsDirty() const {
    return nextRecordIndex_ != savedRecordIndex_;
}

int WorldEditorUndoStack::UndoCount() const {
    return static_cast<int>(nextRecordIndex_);
}

int WorldEditorUndoStack::RedoCount() const {
    return static_cast<int>(records_.size() - nextRecordIndex_);
}

const WorldEditorUndoRecord* WorldEditorUndoStack::PeekUndo() const {
    if (!CanUndo()) {
        return nullptr;
    }
    return &records_[nextRecordIndex_ - 1];
}

const WorldEditorUndoRecord* WorldEditorUndoStack::PeekRedo() const {
    if (!CanRedo()) {
        return nullptr;
    }
    return &records_[nextRecordIndex_];
}

void WorldEditorUndoStack::MarkSaved() {
    savedRecordIndex_ = nextRecordIndex_;
}

void WorldEditorUndoStack::PushObjectAdded(std::string_view label,
    const MapObject& object,
    int objectIndex,
    std::string afterSelectionRegistryId) {
    WorldEditorUndoRecord record;
    record.kind = WorldEditorUndoRecordKind::AddObject;
    record.label = std::string(label);
    record.objectIndex = objectIndex;
    record.afterObject = object;
    record.afterSelectionRegistryId = std::move(afterSelectionRegistryId);
    PushRecord(std::move(record));
}

void WorldEditorUndoStack::PushObjectRemoved(std::string_view label,
    const MapObject& object,
    int objectIndex,
    std::string beforeSelectionRegistryId,
    std::string afterSelectionRegistryId) {
    WorldEditorUndoRecord record;
    record.kind = WorldEditorUndoRecordKind::RemoveObject;
    record.label = std::string(label);
    record.objectIndex = objectIndex;
    record.beforeObject = object;
    record.beforeSelectionRegistryId = std::move(beforeSelectionRegistryId);
    record.afterSelectionRegistryId = std::move(afterSelectionRegistryId);
    PushRecord(std::move(record));
}

void WorldEditorUndoStack::PushObjectUpdated(std::string_view label,
    const MapObject& beforeObject,
    const MapObject& afterObject,
    int objectIndex) {
    WorldEditorUndoRecord record;
    record.kind = WorldEditorUndoRecordKind::UpdateObject;
    record.label = std::string(label);
    record.objectIndex = objectIndex;
    record.beforeObject = beforeObject;
    record.afterObject = afterObject;
    record.beforeSelectionRegistryId = beforeObject.registryId;
    record.afterSelectionRegistryId = afterObject.registryId;
    PushRecord(std::move(record));
}

void WorldEditorUndoStack::PushWorldMetadataUpdated(std::string_view label,
    const WorldMetadata& beforeMetadata,
    const WorldMetadata& afterMetadata) {
    WorldEditorUndoRecord record;
    record.kind = WorldEditorUndoRecordKind::UpdateWorldMetadata;
    record.label = std::string(label);
    record.beforeMetadata = beforeMetadata;
    record.afterMetadata = afterMetadata;
    PushRecord(std::move(record));
}

void WorldEditorUndoStack::PushBatchWorldEdit(std::string_view label,
    const World& beforeWorld,
    const World& afterWorld,
    std::string beforeSelectionRegistryId,
    std::string afterSelectionRegistryId) {
    WorldEditorUndoRecord record;
    record.kind = WorldEditorUndoRecordKind::BatchWorldEdit;
    record.label = std::string(label);
    record.beforeWorld = beforeWorld;
    record.afterWorld = afterWorld;
    record.beforeSelectionRegistryId = std::move(beforeSelectionRegistryId);
    record.afterSelectionRegistryId = std::move(afterSelectionRegistryId);
    PushRecord(std::move(record));
}

WorldEditorUndoOutcome WorldEditorUndoStack::Undo(World& world) {
    WorldEditorUndoOutcome outcome;
    if (!CanUndo()) {
        outcome.statusText = "Nothing to undo.";
        return outcome;
    }

    const WorldEditorUndoRecord& record = records_[nextRecordIndex_ - 1];
    bool applied = false;
    switch (record.kind) {
    case WorldEditorUndoRecordKind::AddObject:
        applied = RemoveObjectAt(world, record.afterObject, record.beforeObject, record.objectIndex);
        outcome.focusRegistryId = record.beforeSelectionRegistryId;
        break;
    case WorldEditorUndoRecordKind::RemoveObject:
        if (record.beforeObject.has_value()) {
            applied = RestoreObjectAt(world, *record.beforeObject, record.objectIndex);
        }
        outcome.focusRegistryId = record.beforeSelectionRegistryId.empty()
            ? (record.beforeObject.has_value() ? record.beforeObject->registryId : std::string{})
            : record.beforeSelectionRegistryId;
        break;
    case WorldEditorUndoRecordKind::UpdateObject:
        if (record.beforeObject.has_value()) {
            applied = ReplaceObjectAt(world, record.afterObject, record.beforeObject, record.objectIndex, *record.beforeObject);
        }
        outcome.focusRegistryId = record.beforeSelectionRegistryId;
        break;
    case WorldEditorUndoRecordKind::UpdateWorldMetadata:
        world.metadata = record.beforeMetadata;
        applied = true;
        break;
    case WorldEditorUndoRecordKind::BatchWorldEdit:
        if (record.beforeWorld.has_value()) {
            world = *record.beforeWorld;
            applied = true;
        }
        outcome.focusRegistryId = record.beforeSelectionRegistryId;
        break;
    }

    if (!applied) {
        outcome.statusText = "Undo failed: unable to locate edited object state.";
        return outcome;
    }

    --nextRecordIndex_;
    outcome.changed = true;
    outcome.statusText = DescribeUndoAction("Undo", record);
    return outcome;
}

WorldEditorUndoOutcome WorldEditorUndoStack::Redo(World& world) {
    WorldEditorUndoOutcome outcome;
    if (!CanRedo()) {
        outcome.statusText = "Nothing to redo.";
        return outcome;
    }

    const WorldEditorUndoRecord& record = records_[nextRecordIndex_];
    bool applied = false;
    switch (record.kind) {
    case WorldEditorUndoRecordKind::AddObject:
        if (record.afterObject.has_value()) {
            applied = RestoreObjectAt(world, *record.afterObject, record.objectIndex);
        }
        outcome.focusRegistryId = record.afterSelectionRegistryId.empty()
            ? (record.afterObject.has_value() ? record.afterObject->registryId : std::string{})
            : record.afterSelectionRegistryId;
        break;
    case WorldEditorUndoRecordKind::RemoveObject:
        applied = RemoveObjectAt(world, record.beforeObject, record.afterObject, record.objectIndex);
        outcome.focusRegistryId = record.afterSelectionRegistryId;
        break;
    case WorldEditorUndoRecordKind::UpdateObject:
        if (record.afterObject.has_value()) {
            applied = ReplaceObjectAt(world, record.beforeObject, record.afterObject, record.objectIndex, *record.afterObject);
        }
        outcome.focusRegistryId = record.afterSelectionRegistryId;
        break;
    case WorldEditorUndoRecordKind::UpdateWorldMetadata:
        world.metadata = record.afterMetadata;
        applied = true;
        break;
    case WorldEditorUndoRecordKind::BatchWorldEdit:
        if (record.afterWorld.has_value()) {
            world = *record.afterWorld;
            applied = true;
        }
        outcome.focusRegistryId = record.afterSelectionRegistryId;
        break;
    }

    if (!applied) {
        outcome.statusText = "Redo failed: unable to restore edited object state.";
        return outcome;
    }

    ++nextRecordIndex_;
    outcome.changed = true;
    outcome.statusText = DescribeUndoAction("Redo", record);
    return outcome;
}

void WorldEditorUndoStack::PushRecord(WorldEditorUndoRecord record) {
    if (nextRecordIndex_ < records_.size()) {
        records_.erase(records_.begin() + static_cast<std::ptrdiff_t>(nextRecordIndex_), records_.end());
        if (savedRecordIndex_ > nextRecordIndex_) {
            savedRecordIndex_ = nextRecordIndex_;
        }
    }

    if (!records_.empty() && nextRecordIndex_ > 0 &&
        TryCoalesce(records_[nextRecordIndex_ - 1], record)) {
        return;
    }

    records_.push_back(std::move(record));
    nextRecordIndex_ = records_.size();
}

bool WorldEditorUndoStack::TryCoalesce(WorldEditorUndoRecord& existing, const WorldEditorUndoRecord& incoming) {
    if (existing.kind != incoming.kind) {
        return false;
    }

    if (existing.kind == WorldEditorUndoRecordKind::UpdateObject &&
        existing.beforeObject.has_value() &&
        existing.afterObject.has_value() &&
        incoming.beforeObject.has_value() &&
        incoming.afterObject.has_value()) {
        const std::string& existingTailRegistryId = existing.afterObject->registryId;
        const std::string& incomingHeadRegistryId = incoming.beforeObject->registryId;
        const std::string& existingHeadRegistryId = existing.beforeObject->registryId;
        if (existing.objectIndex == incoming.objectIndex &&
            (existingTailRegistryId == incomingHeadRegistryId || existingHeadRegistryId == incomingHeadRegistryId)) {
            existing.afterObject = incoming.afterObject;
            existing.label = incoming.label;
            existing.afterSelectionRegistryId = incoming.afterSelectionRegistryId;
            return true;
        }
    }

    if (existing.kind == WorldEditorUndoRecordKind::UpdateWorldMetadata) {
        existing.afterMetadata = incoming.afterMetadata;
        existing.label = incoming.label;
        return true;
    }

    if (existing.kind == WorldEditorUndoRecordKind::BatchWorldEdit &&
        existing.afterWorld.has_value() &&
        incoming.beforeWorld.has_value() &&
        existing.label == incoming.label) {
        existing.afterWorld = incoming.afterWorld;
        existing.afterSelectionRegistryId = incoming.afterSelectionRegistryId;
        return true;
    }

    return false;
}

}  // namespace bunker
