# src/GameRuntime.cpp

Уже закрыто:

- proximity/service hooks живут в runtime: `lanline_service_hub`, `tank_service`, `medical_support`, `fey_ring`
- `Lanline Services` существуют не только в launcher-shell: `Pip-Pad` показывает service state, умеет claim delivered orders и синхронизирует их обратно в profile/world glue
- first playable route получил реальный runtime glue: `cryo/core/garage/echo/hull/bucket/workshop/relay/debrief` теперь ведут staged `BT-72 restore -> sync -> clearance module -> first combat -> first service -> recovery node`
- ранний bunker-start теперь реально проходит через `bunker_access_card`: core clue выдает карту доступа, `Pip-Pad` и archive больше не открываются без этого шага, а objective text и event copy держат route context
- `BT-72` two-seat/gunner flow живет в runtime, а не в заметках: есть seat swap, gunner-only fire mode, seat-aware restrictions на utility/workbench/clearance, HUD/title labels и mirror в `LanlineSessionState`

Следующее:

- vertical slice polish вокруг bunker exit / surface arrival и deeper BT-72 / combat reactivity, а не новый service-shell
