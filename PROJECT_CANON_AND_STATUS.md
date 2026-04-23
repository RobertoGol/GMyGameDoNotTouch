# PROJECT_CANON_AND_STATUS.md

## 0. Роль файла

Этот файл хранит только:
- канон проекта;
- архитектурные границы;
- жесткие правила мира;
- роли `BT-72`, `Camp / AIMP`, мастерских и редактора;
- проверенный текущий статус;
- что из старых материалов и веток остается валидным;
- общий вектор до завершения базовой игры.

Он не хранит активный todo-список и не является backlog-файлом.

## 1. Источники правды

Приоритет истины:
1. текущий код;
2. smoke-checks и тесты;
3. этот файл;
4. `Next.md`;
5. `trash.md`.

`Next_split/*` и `Next_compact.md` считать только legacy-выжимками и справочным слоем. Они не должны спорить с кодом и этими тремя основными файлами.

## 2. Форма проекта

### Название
`Bunker Protocol`

### Приложения
- `BunkerLauncher`
- `BunkerGame`
- `BunkerEditor`
- `BunkerSmokeChecks`

### Базовая форма
- `solo + LAN first`
- `launcher` обязателен как пользовательская точка входа
- `editor` остается отдельным production tool
- `runtime` остается отдельным игровым приложением

### Основные pillars
- `BT-72`
- `authored world`
- `bunker-to-surface progression`
- `recovery`
- `industry`
- `logistics`
- `persistence`
- `service / support loops`
- `Pip-Pad` как основной системный интерфейс

### Базовая цель
Довести проект до первой цельной базовой игры, где уже есть:
- пробуждение и bunker-start;
- ранний доступ через карты/допуски, а не только через `Pip-Pad`;
- получение `Pip-Pad` и ранний archive/data trail;
- восстановление `BT-72`;
- `sync / link`;
- выход через ангар и surface approach;
- первый бой, первый сервис и первый recovery payoff;
- честный переход в `recovery / industry / logistics` backbone.

## 3. Жесткий канон мира

### Authored world
Мир и карту собирает разработчик в `BunkerEditor`.

Игрок:
- не редактирует карту как редактором;
- не рисует карту;
- не перестраивает authored геометрию по всей карте;
- меняет мир только через игровые механики и world states.

### Camp / AIMP
`Camp / AIMP` - отдельная игровая система:
- переносимая;
- разворачиваемая;
- позволяет свободное строительство только внутри своего радиуса;
- не заменяет editor;
- не равна мастерской;
- не равна `BT-72`.

### Мастерские
Мастерские - это authored world nodes:
- заранее расставленные разработчиком;
- находимые и зачищаемые;
- захватываемые и восстанавливаемые;
- используемые в `recovery / industry / service` loop.

Мастерская - не `Camp / AIMP` и не `BT-72`.

### BT-72
`BT-72` - центральная напарник-платформа:
- не `Camp / AIMP`;
- не мастерская;
- не просто "готовый танк";
- боевая и инженерная платформа;
- часть раннего маршрута;
- часть progression;
- часть service/modification loop;
- часть recovery backbone.

### State of Decay 2 rule
`State of Decay 2` использовать только как пример того, как authored мир живет через игровые механики:
- вылазка;
- риск;
- добыча;
- возврат;
- изменение состояния базы и мира.

Не использовать как шаблон player-side world editing.

### Что игрок может и не может

Игрок может:
- зачищать объекты;
- открывать маршруты;
- восстанавливать узлы;
- использовать мастерские;
- разворачивать `Camp / AIMP`;
- строить внутри радиуса `Camp / AIMP`;
- обслуживать и модифицировать `BT-72`.

Игрок не может:
- редактировать всю карту;
- двигать authored здания как в editor;
- превращать игру в sandbox editor mode.

## 4. Архитектура и границы систем

### Роль launcher
`BunkerLauncher` отвечает за:
- вход в проект;
- выбор мира и профиля;
- shell-layer для `Lanline`, `Lanline Services`, `Fey Ring`;
- запуск runtime;
- системные уведомления и route summary.

### Роль runtime
`BunkerGame` отвечает за:
- actual gameplay;
- progression;
- `BT-72`;
- combat / RPG;
- recovery / industry / logistics;
- world-state transitions;
- authored-world interaction.

### Роль editor
`BunkerEditor` отвечает за:
- authored world creation;
- object placement;
- semantic authoring;
- prefab/library workflow;
- validation;
- export;
- developer-facing content pipeline.

Editor не является player-facing world editor.

### Lanline / service boundary
`Lanline`, `Lanline Services` и `Fey Ring` - это не полноценный MMO/backend слой.

Это:
- launcher/runtime/session shell;
- profile/world/service consistency;
- support/service UI layer;
- solo + LAN first glue.

### Экономика

За игровые деньги:
- ресурсы;
- repair kits;
- medical;
- service items;
- recovery-support items.

За реальные деньги:
- только cosmetics / symbolic support.

Запрещено:
- оружие за реальные деньги;
- готовые танки;
- боевые преимущества;
- pay-to-win.

### Reactive tech stack boundary
Legacy reactive tech stack не является отдельной игрой внутри игры.

Это только quality-layer поверх:
- start route;
- `BT-72`;
- first combat;
- first service/rest;
- recovery payoff.

## 5. Что из старых материалов остается валидным

Брать из старых веток и заметок только то, что совместимо с текущим кодом и каноном:

### `Project_M`
Остается валидным как источник для:
- runtime behavior;
- LAN contour;
- техники;
- persistence;
- world logic;
- BT-72 / tank thinking;
- recovery/service flow.

### `Aegis-9300`
Остается валидным как:
- launcher mood;
- terminal feel;
- dry system UI reference.

### `Новая папка`
Остается валидной как ранний источник для:
- account / character / world layering;
- authored world thinking;
- world/resource layering;
- общей UX-логики мира.

### `void-project`
Допустим только как tonal reference. Не как архитектурная база.

### Из старых текстов сохраняется по смыслу
- `Pip-Pad` + archive/data trail;
- launcher-first entry flow;
- authored recovery backbone;
- `BT-72` как центральная механическая ось;
- service/logistics/recovery как ядро mid-game.

### Что не считать базовым обязательством
Старые ответвления про:
- музыку как системный слой;
- assistant / Nerv direction;
- consciousness / resonance;
- расширенный streaming/region prep;
- большой asset-pipeline R&D

считать valid later-ideas, но не blockers базовой игры.

## 6. Проверенный текущий статус

Ниже - не wish-list, а то, что уже подтверждается текущим кодом и smoke-checks.

### Уже сильно продвинуто и реально живет в проекте
- split `Launcher / Game / Editor / SmokeChecks`;
- launcher gate через launch ticket;
- atomic save flow для мира и профиля;
- editor spine: validation, export history, compare presets, XREF/weak refs, layers, unified inspector, undo/redo, viewport authoring;
- prefab/library `v1`;
- import assistant;
- world-format tightening и export discipline;
- `Lanline Services` save/load и profile/runtime/launcher sync;
- launcher announcement widget с локальным persisted read-state;
- `tank_service` как реальный `BT-72` payoff;
- relay-credit progression helpers;
- post-debrief recovery handoff summary в одном состоянии истины для story/runtime/launcher;
- lightweight route-event layer поверх route/recovery state с persistence, cooldown и smoke coverage;
- early `BT-72 / RPG` weight pass: `SPECIAL`, passive skills, crew coordination и service doctrine теперь реально влияют на first combat / gunner loop / first service;
- limited modular damage для mechanical hostiles теперь реально живет в first combat: роботам можно сбивать sensors / weapon / mobility, а runtime prompt, tank HUD и Pip-Pad читают этот combat state одинаково и smoke-checks его покрывают;
- launcher selected-world preview, recovery handoff, route-event summary и services unlock теперь читают один и тот же world-scoped state без рассинхрона между UI-выбором мира и summary;
- first-route presentation pass для `hangar -> bunker exit -> surface arrival` теперь собран в единый route-beat слой: beat/cue/payoff читаются одинаково в runtime, Pip-Pad, launcher и key event copy, плюс покрыты smoke-checks;
- shared world semantic/descriptor/validation contracts.

### Первый маршрут уже поддержан системно
По коду и smoke-checks уже есть route-layer для:
- cryo wake;
- emergency melee pickup;
- bunker access card;
- `Pip-Pad` acquisition;
- archive corridor / first vermin gate;
- `BT-72` hull/core/service notes;
- staged `BT-72` restoration;
- `sync / link`;
- clearance blueprint/materials/install;
- outer bulkhead unlock;
- heavy debris clearance;
- first tank combat;
- first service/rest step;
- first recovery node;
- debrief;
- handoff в industrial follow-up.

### Что больше не должно висеть как активный todo
Следующие слои уже нельзя держать в `Next.md` как "делать сейчас":
- editor hardening прошлого прохода;
- `Lanline Services` persistence/glue базового уровня;
- launcher announcement widget;
- first-route persistence/objective layer;
- access-card gated bunker start;
- `BT-72` second-seat / trusted gunner flow;
- `tank_service` service-kit basics;
- relay-credit helper layer.

### Главный текущий незакрытый фронт
Основная незавершенная работа уже не в toolchain, а в самой игре:
1. start vertical slice polish;
2. `BT-72 / combat / RPG depth`;
3. `recovery / industry / logistics` как плотный mid-game backbone;
4. runtime / launcher / profile / service glue hardening только там, где это поддерживает базовую игру.

## 7. Вектор до завершения базовой игры

Правильный порядок:
1. дожать первый маршрут до showable vertical slice;
2. углубить `BT-72 / combat / RPG`;
3. сделать первый recovery payoff началом устойчивого mid-game loop;
4. держать launcher/runtime/profile/world/service state в синке;
5. использовать reactive tech stack только как слой качества поверх уже работающей игры.

Неправильный порядок:
- уходить в новый большой editor-R&D;
- строить DLC/expansion ветки раньше базы;
- превращать `Lanline` в отдельный интернет-продукт;
- разворачивать player-side world editor.

## 8. Роли трех файлов

- `PROJECT_CANON_AND_STATUS.md` - канон и статус.
- `Next.md` - только активная незавершенная работа к базовой игре.
- `trash.md` - backlog / later / не-блокирующие идеи.
