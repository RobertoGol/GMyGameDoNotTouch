# ROADMAP

## 2026-04-23

- подтвержден отдельный checkpoint `surface arrival` между `bunker exit` и `heavy clearance`;
- runtime и launcher читают один и тот же vertical-slice route summary: checkpoint, прогресс, `next payoff`;
- `surface arrival` теперь живет в profile persistence, scripted world event и smoke-checks;
- первый combat cue, first service handoff и debrief -> industrial handoff теперь реально отдаются через runtime events и проверяются smoke-checks;
- `StoryRoute` больше не откатывается на ранние route-step objective/checkpoint, если поздний story-state уже достигнут и один промежуточный flag отстал;
- hostile runtime получил честный layered awareness, blocked-line trigger discipline и blocker-aware sidestep/pathing без wallhack и без магической навигации;
- `BT-72` heavy-shot feedback теперь читабелен в runtime: muzzle flash, shock-wave cue и отдельный seat-role feedback, все покрыто smoke-checks;
- post-debrief `recovery -> mid-game` handoff теперь читается одинаково в story/runtime/launcher/Pip-Pad;
- lightweight route-event layer теперь живет в world/profile persistence: weighted incidents, cooldown, resolve/fail state, rewards и smoke-check coverage без ломания authored route;
- ранний `BT-72 / RPG` weight pass закрыт: `SPECIAL`, passive skills, crew-support и doctrine/service choices теперь реально меняют first combat, gunner burst и first service;
- launcher selected-world summary hardening закрыт: story objective, recovery handoff, route-event layer и Lanline Services preview больше не расходятся с выбранным в UI миром; world-scoped preview путь покрыт smoke-checks;
- `hangar -> bunker exit -> surface arrival` presentation/readability pass закрыт: route-beat layer теперь дает единые beat/cue/payoff в runtime, Pip-Pad, launcher и ключевых route events без authored layout work;
- `BT-72 / combat` modular-damage pass закрыт для первого маршрута: robot contacts теперь теряют sensors / weapon / mobility от тяжелых BT-72 ударов, threat/readability cues живут в prompt/HUD/Pip-Pad и smoke-checks это проверяют;
- точечный `BT-72` seat-policy follow-up закрыт: runtime toggle второго места теперь уважает `pilot_only / trusted_only / open_crew` и больше не расходится с profile/launcher/Pip-Pad state;
- random-event hardening закрыт для базового слоя: route-event lifecycle теперь читает offered/active/success/failed/expired/cooldown состояния, остается locked до завершения onboarding и debrief handoff, а редкий `merchant_window` живет как один merchant incident через Pip-Pad/service glue без постоянного рынка на карте;
- post-debrief `recovery / industry / logistics` readability pass закрыт: shared backbone stage/status/payoff теперь читается одинаково в runtime, Pip-Pad и launcher, а debrief и route-event copy подтягивают ту же industrial-backbone стадию;
- `Lanline Services` consistency hardening закрыт: support shell теперь зеркалит industrial-backbone stage/status/payoff, route-event summary и merchant-window presence из того же world-scoped unlock snapshot;
- Debug build для `BunkerGame`, `BunkerLauncher`, `BunkerEditor` и `BunkerSmokeChecks` снова проходит под MSVC;
- активный следующий фронт: только точечный `BT-72 / combat / RPG` follow-up, если всплывет реальный vertical-slice blocker; остальное — только smoke/build-driven hardening без открытия новой ветки.
