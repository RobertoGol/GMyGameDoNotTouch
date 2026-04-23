# Next

## 0. Роль файла

Этот файл хранит только активную незавершенную работу, которая ведет проект к завершению базовой игры.

Если пункт уже живет в коде и smoke-checks, он должен быть вынесен из этого файла в `PROJECT_CANON_AND_STATUS.md`.

## 1. Текущий активный проход

Главная цель текущего прохода:
довести уже существующий первый маршрут до showable start vertical slice, а не открывать новую параллельную ветку.

Рабочий порядок:
1. `start route / first playable route polish`
2. `BT-72 / combat / RPG depth`
3. `recovery / industry / logistics` payoff и handoff
4. `runtime / launcher / profile / service glue` как поддерживающий слой качества

Границы этого прохода:
- не делать карту и authored layout;
- не уходить в новые editor-feature ветки;
- не делать player-side world editor;
- не уходить в DLC/expansion work.

## 2. First Playable Route - System Sequence

Это системная последовательность, а не layout:

интро  
-> cryo wake  
-> bunker passage  
-> ранний доступ через карту/допуск  
-> `Pip-Pad`  
-> archive / paper trail  
-> hangar approach  
-> `BT-72` hull + core + service notes  
-> staged restoration  
-> `sync / link`  
-> hangar tutorial slice  
-> bunker exit / gate / lift  
-> first surface arrival  
-> heavy clearance  
-> first combat  
-> first service / rest  
-> first recovery node  
-> debrief  
-> industrial follow-up

## 3. Active Gameplay / System Priorities

### Start route / vertical slice polish
- дожать presentation/feedback для `hangar -> bunker exit -> surface arrival`, чтобы payoff читался как цельный vertical slice;
- улучшить ощущение первого маршрута как showable slice, а не только checklist-а;
- держать authored-world работу только на уровне системных support hooks.

### BT-72 / combat / RPG depth
- углубить pilot/gunner loop поверх уже работающего second-seat flow;
- усилить различие между пешим и `BT-72` боем;
- улучшить feel боя: impact, muzzle/shock, damage readability, combat pacing;
- дать `SPECIAL`/skills/service choices более ранний реальный вес в первом срезе.

### Recovery / industry / logistics
- превратить first recovery node из одиночного payoff в честный handoff к mid-game backbone;
- связать relay/service/water/industrial nodes с видимыми runtime consequences;
- держать мастерские authored world nodes, а не заменять их строительством игрока;
- сделать recovery/service/logistics продолжением первого маршрута, а не отдельным меню.

### Runtime / launcher / profile / service glue
- держать route checkpoint, objective preview, profile flags, world state и `Lanline` state в одном состоянии истины;
- улучшить launcher-side route/recovery cues без превращения launcher в браузер или backend shell;
- дожать save/load и smoke coverage на handoff `route -> recovery backbone`;
- не возвращать в активный проход базовый service-shell, который уже закрыт.

## 4. Reactive Tech Stack - Quality Layer

Этот слой внедряется только поверх активной игры.

### Обязательно
- honest AI perception;
- trigger discipline;
- corridor / hangar pathing;
- reactive interactables;
- shatterable glass;
- projectile / muzzle / shock feedback;
- animation readability;
- limited foliage interaction;
- limited rain / water / reflection feedback.

### Ограниченно
- mechanical damage;
- momentum damage for clearance/combat;
- limited debris;
- limited light destruction;
- limited mirrors/reflections.

### Не превращать в отдельную ветку
- no full destruction sandbox;
- no full fluid sim;
- no crowds-first work;
- no always-on friendly fire;
- no "tech demo first, game later".

## 5. Следующий рабочий пакет

1. усиление `recovery / industry / logistics` readability после debrief, только как честное продолжение первого маршрута
2. только точечный follow-up по `BT-72 / combat / RPG depth`, если он исправляет реальный vertical-slice blocker, а не открывает новую ветку
