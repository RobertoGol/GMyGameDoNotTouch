# ROADMAP

## 2026-04-23

- подтвержден отдельный checkpoint `surface arrival` между `bunker exit` и `heavy clearance`;
- runtime и launcher читают один и тот же vertical-slice route summary: checkpoint, прогресс, `next payoff`;
- `surface arrival` теперь живет в profile persistence, scripted world event и smoke-checks;
- первый combat cue, first service handoff и debrief -> industrial handoff теперь реально отдаются через runtime events и проверяются smoke-checks;
- `StoryRoute` больше не откатывается на ранние route-step objective/checkpoint, если поздний story-state уже достигнут и один промежуточный flag отстал;
- hostile runtime получил честный layered awareness, blocked-line trigger discipline и blocker-aware sidestep/pathing без wallhack и без магической навигации;
- `BT-72` heavy-shot feedback теперь читабелен в runtime: muzzle flash, shock-wave cue и отдельный seat-role feedback, все покрыто smoke-checks;
- Debug build для `BunkerGame`, `BunkerLauncher`, `BunkerEditor` и `BunkerSmokeChecks` снова проходит под MSVC;
- активный следующий фронт: `recovery-node -> mid-game` handoff, затем lightweight random-event layer поверх route/recovery state.
