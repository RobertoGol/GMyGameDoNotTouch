#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "World.hpp"

namespace bunker {

enum class WorldEditorUndoRecordKind {
    AddObject,
    RemoveObject,
    UpdateObject,
    UpdateWorldMetadata,
    BatchWorldEdit
};

struct WorldEditorUndoRecord {
    WorldEditorUndoRecordKind kind = WorldEditorUndoRecordKind::UpdateObject;
    std::string label;
    int objectIndex = -1;
    std::optional<MapObject> beforeObject{};
    std::optional<MapObject> afterObject{};
    WorldMetadata beforeMetadata{};
    WorldMetadata afterMetadata{};
    std::optional<World> beforeWorld{};
    std::optional<World> afterWorld{};
    std::string beforeSelectionRegistryId;
    std::string afterSelectionRegistryId;
};

struct WorldEditorUndoOutcome {
    bool changed = false;
    std::string focusRegistryId;
    std::string statusText;
};

class WorldEditorUndoStack {
public:
    void Clear();

    bool CanUndo() const;
    bool CanRedo() const;
    bool IsDirty() const;
    int UndoCount() const;
    int RedoCount() const;

    const WorldEditorUndoRecord* PeekUndo() const;
    const WorldEditorUndoRecord* PeekRedo() const;

    void MarkSaved();

    void PushObjectAdded(std::string_view label,
        const MapObject& object,
        int objectIndex,
        std::string afterSelectionRegistryId = {});
    void PushObjectRemoved(std::string_view label,
        const MapObject& object,
        int objectIndex,
        std::string beforeSelectionRegistryId = {},
        std::string afterSelectionRegistryId = {});
    void PushObjectUpdated(std::string_view label,
        const MapObject& beforeObject,
        const MapObject& afterObject,
        int objectIndex);
    void PushWorldMetadataUpdated(std::string_view label,
        const WorldMetadata& beforeMetadata,
        const WorldMetadata& afterMetadata);
    void PushBatchWorldEdit(std::string_view label,
        const World& beforeWorld,
        const World& afterWorld,
        std::string beforeSelectionRegistryId = {},
        std::string afterSelectionRegistryId = {});

    WorldEditorUndoOutcome Undo(World& world);
    WorldEditorUndoOutcome Redo(World& world);

private:
    std::vector<WorldEditorUndoRecord> records_{};
    std::size_t nextRecordIndex_ = 0;
    std::size_t savedRecordIndex_ = 0;

    void PushRecord(WorldEditorUndoRecord record);
    static bool TryCoalesce(WorldEditorUndoRecord& existing, const WorldEditorUndoRecord& incoming);
};

}  // namespace bunker
