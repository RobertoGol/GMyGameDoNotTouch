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
- Debug build для `BunkerGame`, `BunkerLauncher`, `BunkerEditor` и `BunkerSmokeChecks` снова проходит под MSVC;
- активный следующий фронт: `hangar -> bunker exit -> surface arrival` readability/presentation polish без authored layout work, затем точечный `BT-72 / combat / RPG` follow-up для первого маршрута.
