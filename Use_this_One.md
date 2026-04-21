# Рабочий Буфер Для Следующих Вопросов И Уточнений

Пиши сюда новые вопросы, ответы, уточнения и правки по проекту.
это ты сюда вопросы пишешь а не я

# Current Checkpoint

Export/history path в editor закрыт:
- export audit history
- validation snapshot / shipping baseline
- historical checkpoint compare
- object-aware regressions / improvements
- history query/filter/preset layer
- editor UI для baseline diff, quick compare presets, jump actions и filtered audit list
- smoke на quick filters / presets / blocked / baseline / no-match fallback

Registry/XREF base тоже уже дожат до usable уровня:
- shared weak-ref/XREF queries по `registryId`-style `linkTarget`
- inspector block с incoming/outgoing references
- jump-to-reference
- delete warning для referenced objects
- smoke на resolved/unresolved reference graph

Validation/warnings panel тоже стал плотнее:
- issue search
- severity filter
- selected-object-only filter
- focus по отфильтрованному issue list

Unified property inspector и layer manager тоже теперь закрыты:
- `MapObject` / world / prefab roundtrip теперь держат `editorLayer` с backward-compatible infer для legacy `BWL3`
- object library умеет `layer filter`, compact layer badges и search по layer name
- в editor есть layer manager с `show/hide`, `lock/unlock`, `filter`, `select first`
- preview учитывает layer visibility/lock state и не даёт случайно drag/select по locked/hidden слоям
- выбранный объект собран в более цельный inspector с секциями `Identity / Gameplay`, `Semantic / Validation`, `XREF`, `Descriptor Presets`, `Transform / Runtime`, `Loot`, `Actions`
- inspector умеет править `display name` и `layer`, плюс показывает specialized runtime notes для `lanline_service_hub / fey_ring / tank_service / medical_support / tower_sync / rail`
- smoke-check теперь фиксирует `editorLayer` roundtrip для world/prefab и legacy `BWL3` layer inference

Следующий слой без открытия новых больших систем:
- undo/redo
