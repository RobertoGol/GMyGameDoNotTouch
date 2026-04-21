# Editor/src/Editor_Main.cpp

Актуальный статус по editor-файлу:

Уже закрыто:

- service/fey/industrial presets на канонических `scriptTag`
- validation panel с search/filter/focus/fix
- export history filters / compare presets / jump actions
- `Weak References / XREF` блок и delete warning
- unified inspector + layer manager
- `undo/redo`
- viewport authoring: grid-step snap, bounds gizmos, spawn drag, XREF/link overlays, interaction/service radius overlays
- `prefab/library strengthening`: stable prefab metadata/id/source/completion, `prefabSourceId` handoff в `BWL5`, usage/broken-ref visibility, focus/update/apply flows
- `import assistant hardening`: typed draft prefab generation, safe add/update в library, draft seeding, richer concept manifest
- core export discipline / world-format handoff: prefab-aware validation report и stricter shared world/export contract теперь живут в `World` / `WorldExport`, а не в ad-hoc editor glue

Следующее сюда по очереди:

- object/palette UX cleanup без переписывания editor целиком
- future multi-object prefabs / overrides только после runtime/service пакета
