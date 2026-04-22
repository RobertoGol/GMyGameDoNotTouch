# src/GameRuntime.cpp

Уже закрыто:

- proximity/service hooks живут в runtime: `lanline_service_hub`, `tank_service`, `medical_support`, `fey_ring`
- `Lanline Services` существуют не только в launcher-shell: `Pip-Pad` показывает service state, умеет claim delivered orders и синхронизирует их обратно в profile/world glue
- first playable route получил реальный runtime glue: `cryo/core/garage/echo/hull/bucket/workshop/relay/debrief` теперь ведут staged `BT-72 restore -> sync -> clearance module -> first combat -> first service -> recovery node`

Следующее:

- vertical slice polish вокруг BT-72 / recovery / service payoff, а не новый service-shell
