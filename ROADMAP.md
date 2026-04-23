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
- Debug build для `BunkerGame`, `BunkerLauncher`, `BunkerEditor` и `BunkerSmokeChecks` снова проходит под MSVC;
- активный следующий фронт: усиление `recovery / industry / logistics` readability после debrief, затем только точечный `BT-72 / combat / RPG` follow-up если всплывет реальный vertical-slice blocker.
