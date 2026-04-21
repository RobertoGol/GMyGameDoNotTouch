# Roadmap `Bunker Protocol`

## Статус Выполнения

- `Этап 1. Стабилизация основы` — завершен
- `Этап 2. Каркас архитектуры` — в работе
- `Этап 3. Вертикальный срез старта` — начат
- `Этап 4. Базовый боевой и RPG-слой` — в работе
- `Этап 7. Launcher v1` — завершен

Последняя подтвержденная сборка:

- `2026-04-12`
- `build_stage1_msvc`
- успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- повторно подтверждена сборка `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после восстановления `src/main.cpp` и разделения persistence по мирам снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления object search/filter и безопасного дублирования в `BunkerEditor` снова успешно собраны `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего prefab/library workflow в `BunkerEditor` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после превращения world preview в рабочую зону выбора и постановки объектов снова успешно собран `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления viewport-navigation в стиле `Creation Kit` снова успешно собран `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления drag/move выбранного объекта прямо через preview снова успешно собран `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления interaction-helper overlay и world markers в preview снова успешно собран `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления специализированных draft-кнопок для gameplay-сущностей снова успешно собран `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после расширения формата мира до `BWL2` и добавления semantic authoring-полей снова успешно собраны `BunkerGame` и `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления descriptor-presets поверх `scriptTag/linkTarget` снова успешно собран `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления `tower_sync` / `remote_link` descriptor-потока снова успешно собраны `BunkerGame` и `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего `cassette player / music bonus` слоя снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления мягкой `wireless tank charging` у workshop/tower снова успешно собран `BunkerGame`
- `2026-04-13`
- `build_verify_ninja`
- после перевода tank charging на отдельную позицию танка и синхронизации parked tank anchor снова успешно собран `BunkerGame`
- `2026-04-13`
- `build_verify_ninja`
- после перевода workshop service на реальную проверку parked tank proximity снова успешно собран `BunkerGame`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего tank energy siphon от tower/workshop снова успешно собран `BunkerGame`
- `2026-04-13`
- `build_verify_ninja`
- после добавления stress awakening и раннего `Second Wind` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего `Forward Camp` как field checkpoint/recovery point снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего `SoulSync / Sync-Link` слоя для BT-72 снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего `food/toxin SPECIAL` слоя через field ration снова успешно собран `BunkerGame`
- `2026-04-19`
- `build_verify_ninja`
- после доведения `Lanline Services` persistence/world glue снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-19`
- `build_verify_ninja_fresh`
- после launcher hardening (`PrepareSelectedCharacter`, controlled refresh worlds/snapshots, WinAPI launch соседних exe) снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-19`
- `build_verify_ninja_fresh`
- после cleanup root build-артефактов (`CMakeCache.txt`) и подтверждения схемы `main + fallback` сборка осталась рабочей без новых rebuild-ошибок
- `2026-04-21`
- `build_verify_ninja_fresh`
- после shared `undo/redo` stack и viewport authoring hardening снова успешно собраны `BunkerSmokeChecks`, `BunkerEditor`, `BunkerGame`, `BunkerLauncher`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего `Pip-Pad AR / Echo Trace` authored-behavior снова успешно собраны `BunkerGame` и `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления ранних `Cryo Specialists` и specialist-driven workshop boost снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего `Data Reconstruction` в Pip-Pad снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления ранней `field modification` для utility-slot BT-72 снова успешно собран `BunkerGame`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего `Scavenger Team` loop снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего `Power Grid` слоя для tower/workshop/camp снова успешно собран `BunkerGame`
- `2026-04-13`
- `build_verify_ninja`
- после добавления ранних `Weather Anomalies` снова успешно собран `BunkerGame`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего per-world `Ether Erosion` состояния снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-13`
- `build_verify_ninja`
- после добавления раннего `Field Service` для BT-72 снова успешно собран `BunkerGame`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `SoulLine` аварийного катапультирования снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Muscle Memory` awakening снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления ранних `Awakened Recipes` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего назначения `Cryo Specialists` снова успешно собран `BunkerGame`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Shelter Doctrine` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Infrastructure Decay` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Autopilot Caravan` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Power Pylon` / восстановления ЛЭП снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления ранних `Drone Stations` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления ранней `Trade Network` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Route Contamination` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Thermal Mode` снова успешно собран `BunkerGame`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Tow Coupler / trailer utility module` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Rail Freight Link` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Orbital Uplink` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Rail Fortress` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления раннего `Recovery Fabricator` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после расширения post-debrief objective-flow до `recovery backbone loop` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`
- `2026-04-14`
- `build_verify_ninja_fresh`
- после добавления `Shelter Recovery Index` снова успешно собраны `BunkerGame`, `BunkerLauncher`, `BunkerEditor`

## 0. Базовые правила

- Не терять уже заложенные идеи, системы и атмосферу.
- `Launcher`, `Game`, `Editor` остаются отдельными приложениями.
- `Launcher` является обязательной точкой входа для игры.
- `Editor` не является runtime-зависимостью для игрока и может быть убран из пользовательской сборки без поломки игры.
- Если функция пока не может быть завершена, допустим временный задел, но он должен быть помечен и потом заменен реальной реализацией.
- Все новые вопросы к пользователю писать в `Use_this_One.md`.
- Все новые идеи пользователя из `trash.md` учитывать как backlog.
- Если идея из `trash.md` уже реально внедрена в проект, ее нужно:
- задокументировать в рабочих файлах состояния
- удалить из `trash.md`, чтобы backlog отражал только еще не внедренные идеи

## 1. Стабилизация основы

Статус: `ЗАВЕРШЕН`

Отметка завершения:

- сборка `BunkerLauncher`, `BunkerGame`, `BunkerEditor` восстановлена
- добавлен локальный `external/glfw`
- `BunkerGame` переписан в рабочий runtime-каркас
- общие модели данных выровнены
- проект собран в `build_stage1_msvc`

## 2. Каркас архитектуры

Статус: `В РАБОТЕ`

Текущий чекпойнт:

- добавлены сюжетные флаги прогресса в профиль
- сохранение и загрузка расширены под ранний story flow
- мир переведен с пустоши на стартовый бункерный сценарий
- значимая часть gameplay/runtime-логики вынесена из `main.cpp` в отдельный модуль `GameRuntime`
- стартовый story/quest route вынесен в отдельный модуль `StoryRoute`
- scripted world feedback для стартового маршрута вынесен в отдельный модуль `WorldEvents`
- добавлен `LaunchSession` gate: `BunkerGame` ожидает штатный запуск через `BunkerLauncher`
- ранний XP/leveling вынесен в отдельный модуль `Progression`
- ранний `skill awakening` и пассивные навыки вынесены в отдельный модуль `SkillSystem`
- `SPECIAL` начал напрямую влиять на бой, входящий урон и энергозатраты

Результат этапа:

- архитектура выдерживает рост проекта
- progression вертикального среза больше не зашит целиком в одном runtime-файле

## 3. Вертикальный срез старта

Статус: `В РАБОТЕ`

Текущий чекпойнт:

- стартовая бункерная локация уже внедрена в runtime
- objective динамически зависит от сюжетного прогресса
- ранний mission log и сюжетные интеракции работают в каркасе игры
- hostile-встречи стали частью стартового маршрута
- soft fail-state и respawn работают внутри вертикального среза
- добавлено полевое восстановление через `cryo_medkit`
- стартовый маршрут завершается `relay sync` снаружи и `debrief`-возвратом внутрь Shelter 17
- в стартовом маршруте появились scripted zone-events для locker/archive/garage/exterior/debrief

Последний завершенный подэтап:

- `2026-04-12`
- завершен первый замкнутый контур вертикального среза:
- крио -> Pip-Pad -> архив -> laska -> танк -> ковш -> bulkhead -> debris -> ghoul -> relay -> debrief

## 4. Базовый боевой и RPG-слой

Статус: `В РАБОТЕ`

Текущий чекпойнт:

- ранний XP/leveling работает через `Progression`
- опыт за бой и ключевые этапы маршрута проходит через единый progression-слой
- ранний `skill awakening` работает через `SkillSystem`
- пассивные навыки можно включать и выключать в `Pip-Pad`
- низкий `MP` уже дает fatigue и штраф к движению
- `SPECIAL` уже влияет на входящий урон, урон в бою и энергозатраты танка
- через `field ration` появился ранний временный `food/toxin` слой: еда может на время сдвигать `SPECIAL`
- управление начинает приводиться к более простому стандарту в духе `Fallout 4 / 76`
- добавлен простой hit warning, чтобы бой ощущался лучше без усложнения управления
- добавлен ранний `stress awakening`: на критическом HP может пробудиться и сработать пассивный `Second Wind`
- добавлен ранний `Muscle Memory` awakening: пешая переноска тяжелого хлама теперь может открыть пассивку, которая через `Strength` уменьшает отдачу и рывок BT-72 при выстреле
- добавлен ранний слой `Awakened Recipes`: частое использование `Field Service` теперь может открыть `Repair Patch Recipe`, а в `Pip-Pad` появилась простая полевая сборка `repair_patch` из уже существующего salvage
- добавлено раннее `управление специалистами`: rescued engineer теперь может быть назначен в `workshop` или `scavenger support` прямо из `Pip-Pad`, и это уже меняет бонусы ремонта и scavenger loop
- добавлены ранние `специализации бункера`: `Shelter Doctrine` теперь выбирается в `Pip-Pad` и уже влияет на workshop repair, camp recovery и scavenger output
- добавлен ранний `Infrastructure Decay`: при offline-grid здания и сервисная инфраструктура мира постепенно деградируют, что уже давит на workshop repair, camp recovery и scavenger эффективность, а при восстановленной сети это состояние постепенно стабилизируется
- добавлен ранний `Autopilot Caravan`: между shelter и forward camp теперь можно включать логистический маршрут, который периодически возвращает bulk salvage и уже зависит от doctrine, engineer support и состояния инфраструктуры
- добавлено раннее `восстановление ЛЭП`: в мире появились `power pylon` узлы, которые можно восстанавливать как authored-объекты, а их восстановление уже усиливает grid, camp recovery, workshop repair, scavenger и caravan-loop
- добавлены ранние `Drone Stations`: в мире и редакторе появились authored-станции дронов, которые после активации по сети периодически приносят salvage и ether traces поверх существующей logistics-системы
- добавлена ранняя `Trade Network`: connected camps теперь могут включать торговую сеть, генерировать `trade_voucher` и обменивать их в `Pip-Pad` на базовые припасы
- добавлен ранний `Route Contamination`: если outer route долго остается под ether-давлением и без стабильной сети, на нем могут снова появляться баррикады и новые гнезда даже после прежней расчистки
- добавлен ранний `Thermal Mode`: у BT-72 появился температурный режим, который накапливается от движения и погоды, охлаждается в покое и у сетевых точек, и уже влияет на mobility и power core при перегреве
- добавлен ранний `Tow Coupler / trailer utility module`: utility-slot BT-72 теперь циклически переключается между `Bucket Rig`, `Ram Shield` и `Tow Coupler`, а `Tow Coupler` уже усиливает scavenger/caravan/drone/trade loops ценой mobility и дополнительного thermal load
- добавлен ранний `Rail Freight Link`: в мире появился authored `rail_depot`, в редакторе появился `Rail Depot` preset, а сама железнодорожная ветка уже дает тяжелые поставки salvage поверх grid/logistics backbone
- добавлен ранний `Orbital Uplink`: в мире появился authored `orbital_uplink`, в редакторе появился `Orbital Uplink` preset, а low-orbit scan теперь работает как следующий индустриальный слой поверх rail/grid/logistics backbone
- добавлен ранний `Rail Fortress`: в мире появился authored `rail_fortress_hub`, в редакторе появился `Rail Fortress` preset, а бронепоезд теперь работает как armored patrol / heavy recovery слой поверх rail freight и orbital uplink
- добавлен ранний `Recovery Fabricator`: в мире появился authored `recovery_fabricator`, в редакторе появился `Fabricator` preset, а refinery-loop теперь связывает salvage, grid и logistics в регулярное производство field supplies
- расширен post-debrief `objective-flow`: после стартового vertical slice игра теперь ведет в rail/uplink/fortress/fabricator recovery backbone вместо тупого обрыва на фразе про expansion
- добавлен `Shelter Recovery Index`: `Pip-Pad` теперь сводит power, logistics, route control и industrial nodes в единый прогресс восстановления Shelter 17

## 5. Танк и ранняя техника

Статус: `В РАБОТЕ`

Текущий чекпойнт:

- вход / выход в танк уже работает
- `cockpit` уже есть как режим камеры
- ковш уже влияет на расчистку маршрута
- энергия танка уже тратится на расчистку
- `Pilot Sync` уже снижает расход энергии как ранний пассивный бонус
- у танка уже есть ранний `Tank HUD`
- повреждения hull/bucket/sensors уже видны игроку и начинают влиять на мобильность
- добавлен ранний `workshop repair loop`
- мастерская уже может чинить и частично восполнять ресурсы танка через ремонтные материалы
- `R` закреплен как reload, `G` используется для специальной атаки
- `E` закреплен как обычное use/interact-действие
- `F` закреплен как контекстный enter/open для дверей, переходов и танка
- вращение освобождено от `E` и перенесено на стрелки влево/вправо
- у танка появилась легкая инерция движения без ухода в тяжелую симуляцию
- у танкового выстрела появилась небольшая отдача с коротким откатом и визуальным camera kick
- мастерская теперь восстанавливает не только hull/bucket/sensors, но и turret/cockpit/power core в умеренном объеме
- у BT-72 появился ранний `Tow Coupler Mk.I`: это третий utility-режим вместо отдельной перегруженной кнопочной схемы, он усиливает логистику и снабжение, но делает машину тяжелее и горячее в движении
- появился ранний `Rail Freight Link`: железнодорожная логистика пока не про огромный бронепоезд, а про первый рабочий industrial spur с heavy salvage loop, который зависит от power grid, pylons и общей logistics-сети
- появился ранний `Orbital Uplink`: запуск спутника пока не про полную глобальную карту, а про первый рабочий orbital scan слой, который требует rail/grid backbone и дает route intelligence, ether suppression и supply traces
- появилась ранняя `Rail Fortress` версия: железнодорожная магистраль пока не про весь межрегиональный транспорт, а про первый бронепоезд как мобильный heavy logistics hub и armored patrol вдоль восстановленного spur
- появился ранний `Recovery Fabricator`: это первый явный production-node, который уже превращает добычу из rail/trade/orbital backbone в полезные расходники и recovery stock для Shelter 17
- добавлены `Shelter Recovery Milestones`: recovery index теперь дает persistent чекпойнты на `25/50/75%`, одноразовые награды и закрепляет recovery-loop как цельную прогрессию базы
- добавлено раннее `Camp Fortification`: field checkpoint теперь можно укреплять за salvage, а уровень укрепления уже влияет на contamination control, recovery stability и общий Shelter Recovery Index
- добавлен persistent `Industrial Gate`: после поднятия recovery backbone у Shelter 17 появился следующий authored progression-gate в deeper industrial zone, уже включенный в objective-flow, world state и editor presets
- добавлен `Industrial Survey Beacon`: после открытия industrial gate появился первый persistent recon/intel loop deeper industrial zone с authored beacon, survey cycles и интеграцией в recovery index
- добавлен `Inner Spur Outpost`: после survey этапа появился первый persistent foothold deeper industrial zone с supply runs, world-state и отдельным authored outpost hub
- добавлен `Assembly Cell`: после outpost этапа появился первый локальный промышленный узел deeper industrial zone с assembly cycles, authored node и интеграцией в recovery progression
- добавлен `Foundry Line`: после assembly cell появился первый heavy-production узел inner spur с foundry cycles, heavy plate output и ранним влиянием на восстановление корпуса/инфраструктуры
- добавлен `Reactor Yard`: после foundry появился heavy-energy узел inner spur с reactor cycles, power-cell output и ранним влиянием на energy backbone deeper industrial zone
- добавлен `Capacitor Bank`: после reactor yard появился буферный heavy-grid узел inner spur, который уже стабилизирует energy backbone, охлаждает BT-72 и поднимает устойчивость deeper industrial zone
- добавлен ранний `Field Service` / мобильный верстак BT-72: в `Pip-Pad` танк может получить небольшой полевой ремонт и подпитку за расходники, но без замены роли полноценной мастерской
- добавлен ранний `SoulLine`: при потере корпуса BT-72 теперь аварийно выбрасывает оператора наружу вместо немедленного тупого вайпа, а сам танк остается отключенным до ремонта

## 6. Persistence и мир

Статус: `В РАБОТЕ`

Текущий чекпойнт:

- `StaticEraser` уже сохраняет удаленные статические объекты
- профиль, story-прогресс, tapes и пассивные навыки уже сохраняются
- мир читает готовые `*.bwld` без зависимости на редактор
- повреждения и состояние танка теперь тоже сохраняются в профиле
- repair/workshop loop опирается на это состояние, а не на временную runtime-заглушку
- возле мастерской добавлен отдельный `Workshop Supply Cache` как ранний источник ремонтных и боевых расходников
- `BunkerGame` и `BunkerEditor` теперь работают с выбранным миром из профиля, а не только с одним жестко заданным `start_zone.bwld`
- удаленные объекты через `StaticEraser` теперь тоже сохраняются раздельно по выбранным мирам
- в метаданные мира добавлен `player spawn`, чтобы authored worlds были реально запускаемыми
- кастомные карты больше не должны насильно откатываться в стартовый бункер через `EnsureStarterInfrastructure`
- starter-specific `objective`, `radio` и scripted zone-events теперь применяются только к bunker-slice, а не ко всем authored worlds подряд
- в мире появился ранний `Forward Camp` loop: field checkpoint сохраняется в профиле, влияет на запуск и respawn и поддерживает recovery-ритм вылазки
- в authored worlds появился ранний `Pip-Pad AR / Echo Trace` слой через `scriptTag`, чтобы мир мог вести игрока к скрытым точкам не только хардкодом
- в runtime появился ранний `Cryo Specialist` слой: специалистов можно будить как authored-сущности мира, а rescued engineer уже усиливает workshop repair loop
- появился ранний `Scavenger Team` loop: лагерь, зачищенный маршрут и awakened engineer теперь дают пассивный возврат salvage в camp inventory
- появился ранний per-world `Ether Erosion` слой: `Ether Fog` теперь может постепенно заращивать маршрут кристаллическим давлением, а `tower sync`, `Forward Camp` и `Scavenger Teams` умеют это состояние частично подавлять и очищать

## Боевой Контур

Текущий чекпойнт:

- у пешего режима и танка теперь есть базовая и специальная атака
- урон и затраты уже различаются между пешим режимом и техникой
- hostile-цели теперь могут повреждать не только игрока, но и сам танк
- добавлено визуальное предупреждение о недавнем попадании по игроку или танку

## 7. Launcher v1

Статус: `ПОЧТИ ЗАВЕРШЕН`

Текущий чекпойнт:

- `Launcher` остается обязательной точкой входа
- лаунчер готовит профиль, режим сессии и выбранный мир перед запуском
- добавлены выбор мира, LAN-поля и более ясный startup summary
- добавлен `LaunchSession` gate для штатного запуска `BunkerGame`
- `Editor` остается отдельным инструментом и не нужен игроку для запуска игры
- `Lanline - optime` зафиксирован как LAN-first session format в UX лаунчера
- в лаунчере появился ранний session roster / online roster, чтобы игроки не теряли друг друга в рамках LAN-формата
- добавлен ранний `LanlineSession` state между лаунчером и игрой
- session roster теперь виден не только в лаунчере, но и внутри `BunkerGame` через runtime session-state
- `Lanline - optime` теперь ведет ранний session log / presence log между лаунчером и игрой
- `LanlineSession` теперь имеет `sessionId` и `updatedAt`, чтобы LAN-сессия была идентифицируемой и наблюдаемой как состояние, а не как набор строк
- `LanlineSession` snapshots теперь пишутся в отдельный каталог, а launcher показывает список известных LAN-сессий как ранний browser/discovery слой
- launcher теперь умеет применять выбранный `Lanline - optime` snapshot к текущим полям host/world/mode, а не только пассивно его показывать
- browser/discovery для `Lanline - optime` вынесен в общий session-модуль и теперь виден и в launcher, и внутри runtime
- `launcher stability pass` tightened: выбор мира больше не держится на временных `c_str()`, списки миров и `Lanline` snapshots теперь обновляются через controlled refresh, индексы клампятся безопасно, `PrepareSelectedCharacter(...)` больше не падает на битом профиле, а запуск `BunkerGame.exe` / `BunkerEditor.exe` идет через WinAPI из директории самого `BunkerLauncher`
- `Lanline - optime` diagnostics tightened: launcher и runtime теперь показывают live reachability probe, ping-like timing, world match, snapshot freshness и visible session presence поверх уже существующего snapshot/session-state слоя
- добавлен ранний shell-слой `Lanline Services`: общий UI/state для launcher и runtime с relay friends/chat/voice settings, support request catalog без combat advantages и ранним `Fey Ring Network` schedule board
- `Lanline Services` доведен до рабочего glue-слоя: появился `LanlineServicesSave` с launcher/runtime save-load, unlock переведен на `tower_sync` через `WorldFieldState`, в мир добавлены authored anchors `lanline_service_hub / fey_ring / medical_support / tank_service`, а `BunkerEditor` получил новые presets и validation warnings под этот сервисный контур
- session lifecycle для `Lanline - optime` стал явнее: launcher/runtime теперь пишут `lifecycleStage`, `activeActor`, `pendingPeer` и `connectedPeer`, так что host/client flow уже читается как ранний handshake, а не как безликий roster dump
- launcher-side handshake control tightened: выбранный `Lanline` snapshot теперь можно не только применить, но и host-side принять pending peer или сбросить peer link прямо из `BunkerLauncher`
- client join-target flow tightened: выбранный host snapshot теперь может быть явно залочен как `Join Selected Lanline Session`, launcher seed'ит join request в target session, а runtime больше не теряет host-side peer link при входе клиента
- `Lanline` discovery/browser стал читабельнее: launcher теперь показывает joinability (`joinable/pending/linked/non-host`), объясняет почему snapshot можно или нельзя использовать как join-target, и тем самым превращает список session snapshots в ранний LAN join browser
- launcher client-flow tightened further: при входе в `LAN Client` режим launcher теперь автоматически фокусирует первый joinable host snapshot, если такой уже есть, и не заставляет игрока вручную искать подходящий join-target
- launcher browser теперь показывает ранний lobby-capacity слой для `Lanline`: `occupied/open slots` считаются из текущего roster, так что host snapshot читается уже как lobby с вместимостью, а не просто как endpoint + lifecycle
- host lobby seats для `Lanline` стали поведенческими: pending/accepted peer теперь занимают конкретные `LAN slot` записи в roster, а не живут только в `pendingPeer/connectedPeer`, так что browser и runtime уже читают lobby как места, а не как абстрактный peer string
- launcher/runtime lobby UX tightened further: `Lanline` seats теперь явно показывают slot-state (`Open / Pending / Accepted / Active`), а runtime notifications ловят смену роли внутри lobby, так что host/client flow уже читается как seat-state machine, а не просто как список игроков
- slot-state machine tightened further: `Lanline` host lobby теперь проходит через `Open -> Reserved Client -> Pending Client -> Client`, где выбор join-target может заранее резервировать место в launcher, а реальный запуск клиента продвигает seat дальше по lifecycle
- pre-match layer added on top of `Lanline` lobby seats: launcher умеет переключать `ready/not ready` для host/client seats и armed state для host session, а runtime `NET` tab теперь показывает ready-состояние и ловит ready-transitions как отдельные session notifications
- `Lanline Services` support catalog tightened to dual-currency rules: operational requests (`materials / utility / tank service / medical`) остаются на `Recovery Scrip`, а `skins / cosmetics` вынесены в отдельный `Symbolic Support` поток без боевого преимущества и с жесткой catalog validation
- `Lanline Services` support UI split completed: launcher/runtime теперь показывают отдельные витрины `Supplies / Tank Service / Medical / Cosmetics` вместо одной смешанной support-вкладки, так что operational и cosmetic flows больше не конфликтуют в одном списке
- `Fey Ring Network` schedule board tightened further: launcher/runtime теперь разделяют `inter-city` и `inter-server` окна, явно показывают load/stability/next-cycle и держат locked-state messaging по unlock-tier вместо плоского списка маршрутов
- relay chat moved beyond local echo: `Transmit` в `Lanline Services` теперь пишет в `LanlineSessionState` и snapshot mirror, так что active session chat реально переносится между launcher/runtime через общий session-state слой, хотя voice transport все еще остается shell-only
- voice layer moved beyond pure settings shell: launcher/runtime теперь могут relay'ить voice activity / push-to-talk state / peak level через `LanlineSessionState`, хотя raw audio transport все еще остается future work

Что еще осталось:

- cleanup build-артефактов и сборочной дисциплины репозитория

## 8. Editor v1

Статус: `В РАБОТЕ`

Долгосрочная цель:

- `BunkerEditor` должен вырасти в полноценный toolset уровня `Creation Kit` для этого проекта, а не остаться ограниченным редактором карты
- текущий `Editor v1` это только первый production-useful слой на пути к этому

Текущий чекпойнт:

- редактор умеет загружать текущий runtime-мир
- редактор загружает и экспортирует именно выбранный мир из профиля/лаунчера
- редактор показывает базовую библиотеку объектов мира
- можно добавлять новые объекты по preset-моделям
- есть защита от дублирующихся `Registry ID`
- можно удалять выбранные объекты
- можно редактировать метаданные мира прямо в редакторе
- можно редактировать позицию, размеры, здоровье и loot выбранного объекта
- можно менять `interaction` и `category` у выбранного объекта
- runtime-мир можно экспортировать обратно в `*.bwld`
- мир можно экспортировать и в новый отдельный `*.bwld` файл
- редактор может назначать новый мир активным для следующего запуска через профиль
- редактор умеет задавать spawn игрока для конкретного мира
- редактор теперь дает popup-валидацию при конфликте `Registry ID`
- выбранный объект можно быстро назначить точкой spawn для мира
- `preview mode` больше не пустой: в редакторе появился живой 2D world preview с объектами, выделением и spawn marker
- в библиотеке объектов появились быстрый поиск и category-filter для authored worlds с большим количеством сущностей
- выбранный объект теперь можно безопасно дублировать с автоматическим новым `Registry ID` и смещением копии
- появился ранний prefab/library workflow: выбранный объект можно сохранить как prefab, хранить в библиотеке и быстро повторно размещать в мире
- `preview mode` начал работать как authoring viewport: клик по объекту в preview выбирает его, а клик по пустому месту задает точку постановки для draft/prefab
- preview получил базовую viewport-навигацию в духе `Creation Kit`: `MMB pan`, `wheel zoom`, `double-click focus`, `Reset View`
- выбранный объект теперь можно двигать прямо через preview, без ручного редактирования координат в полях
- preview теперь показывает interaction-helper markers и явный `SPAWN` marker, чтобы authoring был ближе к semantic viewport, а не только к цветным блокам
- в editor authoring появились быстрые draft-потоки для `terminal`, `transition`, `workshop` и `hostile`, чтобы быстрее собирать игровые маршруты и encounter-точки
- у объектов мира появились легкие semantic authoring-поля `scriptTag` и `linkTarget`, чтобы editor мог задавать trigger/terminal/transition-смысл без хардкода только по `registryId`
- поверх `scriptTag/linkTarget` в editor уже появились быстрые descriptor-presets для terminal / transition / workshop authoring
- идеи из `trash.md` уже начали переходить в runtime: появился ранний слой `radio tower / remote link` через `tower_sync` и `remote_link`
- кассетный плеер как часть `Pip-Pad` начал работать системно: музыкальная кассета теперь дает мягкий бонус к движению без усложнения управления
- рядом с `workshop` и `tower/grid`-узлами у танка теперь есть мягкая автоматическая подзарядка без полной реализации энергосети
- автоматическая подзарядка танка теперь привязана к позиции самого BT-72, а не к тому, сидит ли в нем игрок
- parked tank anchor теперь синхронизируется с профилем, чтобы зарядка и возврат к танку жили в одном мировом состоянии
- мастерская больше не чинит танк магически из любой точки мира: BT-72 должен реально стоять в зоне workshop service
- у BT-72 появился ранний `energy siphon` слой: из кокпита можно быстро добрать заряд у tower relay и workshop power coupler
- у BT-72 появился ранний `SoulSync / Sync-Link`: стиль пилота начинает сдвигать танк в `Bulwark Sync` или `Stabilizer Sync` с мягкими боевыми бонусами
- у BT-72 появилась ранняя `field modification`: utility-slot теперь можно переключать между `Bucket Rig` и `Ram Shield` через `Pip-Pad`
- редактор остается отдельным tool и не нужен игроку для запуска игры

Что еще осталось:

- библиотека объектов
- размещение объектов на карте
- интерактивности
- контейнеры и loot
- экспорт мира в runtime
- preview mode
- ранний import assistant
- проверка и контроль `Registry ID`

Дальнейшее расширение до `Creation Kit`-масштаба:

- ячейки / world partition / слои
- prefab/library workflow
- ранний prefab/library workflow уже начат, но еще не доведен до полноценной production-библиотеки
- квестовые триггеры и маркеры
- терминалы, data carriers и связанная логика
- враги, NPC и spawn placement
- playtest / preview mode
- import pipeline для контента
- контроль ссылок и валидация данных мира
- инфраструктура, дороги, техника, заводы и логистические узлы

## 9. Data Cards, записи, лор

Статус: `ЧАСТИЧНО НАЧАТ`

Текущий чекпойнт:

- tapes / archive carriers уже есть в раннем контуре
- relay/debrief уже дают data-like progression записи
- поврежденные data fragments теперь можно реконструировать прямо в `Pip-Pad DATA` за `MP`, без отдельной тяжелой мини-игры

## 10. Заводы, энергия, инфраструктура

Статус: `НЕ НАЧАТ ПО-СУЩЕСТВУ`

Текущий чекпойнт:

- появился ранний `Power Grid` слой: `tower sync` и `relay` теперь влияют на workshop service, camp recovery и scavenger loop как на одну общую инфраструктуру

## 9.1 Атмосферные аномалии

Статус: `ЧАСТИЧНО НАЧАТ`

Текущий чекпойнт:

- появились ранние погодные аномалии `Acid Rain` и `Ether Fog`
- `Acid Rain` давит на hull/power core танка или HP пешего оператора
- `Ether Fog` давит на sensors танка или MP пешего оператора и ухудшает подвижность
- состояние аномалии отражается в HUD, `Pip-Pad` и screen overlay

## 11. LAN foundation

Статус: `В РАБОТЕ`

Текущий чекпойнт:

- `Lanline - optime` уже живет не только в launcher UX: runtime читает active session-state и snapshot discovery, показывает host/join metadata, world match, snapshot freshness и player/session presence
- runtime notifications теперь ловят вход/выход игроков, смену lobby-role/ready-state, session world drift, relay chat mirror и voice presence поверх общего `LanlineSessionState`
- launcher и runtime читают один и тот же session/snapshot слой, так что `Lanline - optime` уже ведет себя как ранняя игровая session-модель, а не как launcher-only seed

## 12. После вертикального среза

Приоритетное расширение:

- combat depth
- factory/logistics
- content tools
- world expansion
- LAN growth
- heavy transport
- rail/logistics future systems

## 13. Правило развития

Каждый следующий этап должен:

- не ломать уже сделанное
- улучшать архитектуру, а не наращивать хаос
- оставлять заделы только там, где это действительно временно необходимо
- возвращаться к заглушкам после появления нужных зависимостей

## 14. Последний чекпойнт

- добавлен `Relay Substation`: после `Capacitor Bank` inner spur теперь не висит отдельным контуром, а уже маршрутизирует энергию обратно в backbone `Shelter 17`
- добавлен `Service Bay`: после `Relay Substation` deeper industrial zone уже умеет не только питать backbone, но и обслуживать `BT-72` через отдельный service-loop
- добавлен `Water Reclaimer`: purification-узел теперь доведен до checkpoint через runtime-cycle, согласованные условия активации, `EnsureStarterInfrastructure` и editor descriptor preset, а long-range recovery получил стабильный источник clean water
- выровнены activation-gates deeper industrial chain: `Foundry Line`, `Reactor Yard`, `Capacitor Bank`, `Relay Substation`, `Service Bay` и `Water Reclaimer` теперь не входят в ложное active-state без тех же зависимостей, которые нужны их рабочим циклам
- выровнена player-facing continuity после `Water Reclaimer`: финальный objective-text теперь явно фиксирует, что recovery backbone `Shelter 17` поднят, вместо отката к слишком ранней общей фразе про `Industrial Gate`
- усилен milestone-feedback на recovery backbone: `Checkpoint III` теперь явно подтверждает стабильный backbone `Shelter 17` и выдает `clean_water` как системно связанный reward
- уточнены player-facing тексты вокруг debrief и launcher summary: `Shelter 17` теперь показывает recovery-state не только в runtime objective, но и в session flow / debrief messaging
- mission log `Recovery Backbone` summary больше не опирается на голый recovery index threshold и теперь показывает backbone-state по реальным узлам `Relay Substation + Service Bay + Water Reclaimer`
- runtime `Lanline - optime` panel выровнен с launcher-side flow: игра теперь явно показывает, что session state seeded из `BunkerLauncher`, дает runtime/world match и показывает richer snapshot metadata
- runtime `Lanline - optime` known-session browser tightened further: `BunkerGame` теперь показывает по discovered snapshots те же joinability/slots/host reachability/ping/snapshot freshness/presence метаданные, что и launcher, вместо голого списка session ids
- editor production path tightened: `Save As` и `Set Active` теперь валидируют target world, prefab placement сразу фокусирует новый объект, prefab/concept outputs показывают реальные пути, а concept backlog получил remove/clear flow
- editor/runtime sync tightened further: workspace reload теперь синхронизирует export target с active runtime world, direct runtime export явно пишет в active world, а `Export Save-As And Set Active` закрывает типовой authoring handoff одним действием
- early `DATA / archive` flow tightened: стартовый archive terminal больше не дает повторный fake-sync прогресс, а `Pip-Pad DATA` теперь сводит archive/relay/debrief state и reconstruction progress в явный player-facing summary
- specialist/workshop/field-service chain tightened: `TankNeedsRepair` теперь учитывает весь реальный damage-set `BT-72`, field service boost привязан к `engineer -> scavenger_support`, а `Pip-Pad` честно показывает workshop log, engineer assignment и awakened `repair_patch` recipe progress
- logistics/support mission-log flow tightened: `caravan / trade / rail / orbital / fortress / fabricator` больше не включаются через UI в ложный active-state без своих реальных prerequisites, а `Pip-Pad` теперь явно показывает blocked-state и recovery support stock
- `BT-72` service/HUD continuity tightened: cockpit HUD теперь показывает `turret` и `power core` вместе с low-ammo/low-core warnings, а hangar fast-service больше не оставляет танк полностью repaired, но частично не rearmed
- inner spur mission-log continuity tightened: `industrial survey / outpost / assembly / foundry / reactor / capacitor / relay / service bay / water reclaimer` теперь в `Pip-Pad` показывают не только `active/standby`, но и `blocked-state`, когда authored activation сохранен, а runtime prerequisites уже сорваны
- recovery semantics tightened end-to-end: `ShelterRecoveryIndex`, launcher `Recovery status`, `CurrentStoryObjective` и `Recovery Route` больше не считают late recovery/inner spur узлы завершенными только по флагу `active`, если их реальная operational цепочка уже сорвана
- operational semantics moved toward shared profile layer: readiness/operational checks для `recovery / logistics / inner spur` теперь меньше дублируются между `GameRuntime`, `StoryRoute` и `Launcher`, что снижает риск нового semantic drift при дальнейшем добивании `v1`
- editor/runtime handoff tightened further: `BunkerEditor` теперь прямо показывает active runtime objective, operational recovery diagnostics и export-target drift, launcher summary тоже дает richer recovery loop snapshot, а `WorldMetadata` больше не стартует с устаревшим generic objective
- runtime/world continuity tightened further: scripted starter events, terminal sync feedback, camp recovery message и window title теперь лучше отражают реальный recovery progression `Shelter 17`, а не опираются на слишком общие ранние формулировки
- editor semantic presets moved to shared canonical defaults: `Assembly Cell -> Water Reclaimer`, `Lanline Service Hub`, `Fey Ring`, `Medical Support`, `Tank Service` и другие authored anchors теперь берут default `linkTarget` из общего gameplay-descriptor registry, а smoke-check фиксирует starter semantic chain против тех же canonical targets
- editor validation tightened from summary to actionable warnings: runtime validation теперь ловит `canonical linkTarget drift` и missing authored dependencies для recovery/service anchors, а `BunkerEditor` показывает issue list, умеет фокусировать проблемный объект и применять safe autofix для descriptor drift прямо до экспорта
- editor validation moved one step closer to authoring workflow: missing dependency issues теперь несут structured context (`scriptTag` + required anchor), а `BunkerEditor` умеет создавать недостающие semantic anchors по issue-level кнопке или каскадом для recovery/service chain прямо из validation panel
- semantic authoring logic moved onto a shared runtime/editor layer: cascade anchor creation и safe descriptor autofix теперь живут в `WorldSemanticAuthoring`, smoke-check напрямую проверяет каскадную сборку `water_reclaimer -> service/relay/recovery chain`, а validation actions в editor сразу переводят preview на созданный или исправленный узел
- editor semantic route tooling tightened further: required dependency rules теперь shared между validation и authoring graph, preview умеет рисовать semantic chain overlay с highlight/link lines, а selected-object panel показывает dependency status с focus/create flow по конкретным anchors
- semantic chain authoring tightened further: `WorldSemanticAuthoring` теперь умеет layered reflow для dependency chain, `BunkerEditor` может auto-layout после cascade/create и вручную для выбранного semantic root, а smoke-check фиксирует колонны depth-layer и lane ordering для `water_reclaimer` recovery chain
- semantic layout tightened for mixed authored/auto chains: shared reflow теперь умеет сохранять hand-authored semantic anchors и перестраивать только auto-created хвост, editor явно показывает `authored/auto` origin для dependency anchors и даёт toggle `Preserve manual semantic anchors`, а smoke-check фиксирует оба режима — safe preserve и полный chain reflow
- semantic authoring state is now persisted instead of inferred at runtime: `MapObject` хранит explicit `semanticAutoCreated / semanticLayoutPinned`, мир сохраняет это в `BWL3` с fallback-инференсом для legacy `BWL2`, editor умеет `pin/unpin`, `Adopt As Authored` и batch `Adopt Semantic Chain As Authored`, а smoke-check покрывает `BWL3` roundtrip и legacy auto-anchor migration
- semantic authoring debt is now visible across the whole toolchain: runtime validation предупреждает про `auto-created semantic anchors still present`, editor даёт world-wide `Adopt All Auto Semantic Anchors` и issue-level adopt flow, export явно пишет сколько auto semantic anchors осталось в warning-set, а prefab library сохраняет новый semantic state в backward-compatible `V2` формате
- validated authoring/export pipeline moved onto shared modules: prefab library IO вынесен из editor glue в общий `PrefabLibrary`, export теперь идёт через shared `WorldExport` с policy-aware gating и validation-report artifact рядом с `*.bwld`, а smoke-check покрывает prototype-vs-shipping export, blocked export reports и prefab semantic-state roundtrip
- export audit/view pipeline tightened further: shared `WorldExport` теперь пишет decision-aware validation reports и append-only export audit trail с policy/outcome metadata, `BunkerEditor` прямо показывает report/audit previews и last export result, а smoke-check фиксирует audit history для prototype/shipping authoring flow
- shipping export baseline became actionable instead of passive: successful `shipping-safe` export теперь пишет shared validation baseline snapshot, `BunkerEditor` показывает baseline preview и live diff против текущего authoring-state, а smoke-check фиксирует regression/improvement flow для canonical `water_reclaimer` drift
- shipping baseline drift is now object-aware: shared snapshot хранит concrete validation issue signatures (`code/objectId/scriptTag/related`), `BunkerEditor` показывает object-level regressions против последнего shipping-safe export с `Focus/Fix` flow, а smoke-check проверяет и прямой `water_reclaimer` regression, и same-count drift, где warning count не меняется, но mismatch переезжает на другой anchor
- export history compare moved beyond the latest baseline: каждый export теперь архивирует validation snapshot и пишет его в audit trail, shared `WorldExport` умеет грузить historical entries и сравнивать текущий workspace с выбранным checkpoint, а `BunkerEditor` показывает selectable audit checkpoints с object-level regressions/improvements и `Focus/Fix` flow против конкретного history entry
- export audit navigation is now production-useful instead of flat-only: shared `WorldExport` получил history filters / compare presets / latest-match resolution, `BunkerEditor` умеет быстро прыгать к `last successful shipping / prototype / blocked / baseline updated`, показывает filtered compact audit list, а smoke-check покрывает quick filter / preset / no-match сценарии
- editor reference discipline moved beyond duplicate-ID checks: shared `World` теперь строит incoming/outgoing weak-ref graph по `registryId`-style `linkTarget`, `BunkerEditor` показывает `Weak References / XREF` block с jump-to-reference и предупреждает перед удалением referenced objects, а smoke-check фиксирует resolved/unresolved reference graph
- editor validation panel hardened toward production workflow: `BunkerEditor` теперь умеет искать issues по `code / registry / scriptTag / related`, фильтровать `all/errors/warnings`, переключаться в `selected object only` режим и фокусировать объект уже из filtered issue list, чтобы validation не упиралась в один плоский поток
- editor authoring surface hardened further: `MapObject/world/prefab` теперь хранят `editorLayer` c legacy `BWL3` infer, `BunkerEditor` получил layer manager (`show/hide`, `lock/unlock`, `filter`, `select first`) и unified inspector-секции с `display name/layer` editing и specialized runtime notes, а smoke-check покрывает layer roundtrip и backward-compatible layer inference
- editor undo discipline is now shared instead of ad-hoc: `WorldEditorUndoStack` хранит add/remove/update/world-metadata/batch-edit дельты, коалесцирует повторные edits, `BunkerEditor` показывает dirty-state и next undo/redo actions, а smoke-check покрывает add/update/metadata/remove/batch roundtrip
- world preview moved closer to a real authoring viewport: `BunkerEditor` теперь поддерживает grid-step snap, bounds gizmos для `width/depth`, drag player spawn, selected-object `XREF/link` overlay и interaction/service radius overlays поверх уже существующего selection/focus/semantic preview flow
