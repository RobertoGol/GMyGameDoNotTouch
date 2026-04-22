# Start Route Canon — Bunker Exit / Hangar / Surface Arrival

Важно:
- это НЕ карта;
- это НЕ layout;
- это НЕ level design;
- это системный канон первого играбельного маршрута;
- геометрию и расстановку разработчик делает сам в редакторе.

## Общая последовательность

Интро
-> пробуждение в многоместной криозоне
-> ранний жилой/служебный проход по убежищу
-> первые вредители / радтараканы
-> первая дубинка / ранний melee
-> ранний доступ через карты/допуски
-> предвосхищение Pip-Pad через бумажные чертежи, старые версии и техдокументы
-> получение Pip-Pad
-> углубление в инженерный слой комплекса
-> выход в ангар
-> обнаружение корпуса BT-72 и отдельно ядра
-> поиск голозаписей / схем / сервисных подсказок / материалов
-> staged restoration BT-72
-> sync/link
-> учебный отрезок внутри ангара
-> открытие bunker exit / hangar gate / lift route
-> выезд на BT-72 на поверхность
-> первый surface arrival
-> тяжелая расчистка
-> первый бой
-> первый сервис / передышка
-> tower / recovery payoff
-> возврат / дебриф / следующий хук

## Что считать каноном старта

### 1. Убежище
- убежище не одноместное;
- есть многоместная криозона;
- есть жилой, служебный и инженерный слои;
- часть секций читается как жилые/рабочие;
- часть секций уже аварийные, заброшенные или зараженные.

### 2. Ранний доступ
До Pip-Pad часть продвижения идет через:
- карты доступа;
- жетоны/служебный допуск;
- локальные ключи/override;
- простые терминальные или механические допуски.

Pip-Pad не должен быть единственным ранним методом доступа.

### 3. Pip-Pad
Pip-Pad должен быть заранее предвосхищен средой:
- бумажные чертежи;
- старые версии;
- техзаметки;
- следы разработки;
- рабочие столы и документация.

Pip-Pad не должен ощущаться как магический предмет без контекста.

### 4. BT-72
BT-72 в начале не выдается готовым.
Игрок находит:
- корпус;
- отдельно ядро;
- записи;
- схемы;
- материалы;
- сервисные подсказки.

Ранний loop BT-72:
- поиск
- понимание
- сбор материалов
- staged restoration
- activation
- sync/link

### 5. Ангар
Ангар — это отдельный большой переходный слой между внутренним убежищем и внешним миром.

В ангаре должны читаться:
- мостки;
- тяжелый промышленный свет;
- сервисные площадки;
- BT-72 workflow;
- door/gate/lift mechanics;
- ощущение масштаба машины и выхода.

### 6. Выход из убежища
Игрок не выходит на поверхность пешком сразу.
Правильный порядок:
- внутренний bunker-start;
- восстановление BT-72;
- sync/link;
- внутренний танковый tutorial slice;
- открытие выхода;
- выезд на BT-72 наружу.

### 7. Логика выхода
Выход из убежища должен ощущаться как важный инженерный узел, а не как просто дверь.

Допустимые формы:
- вертикальный lift exit;
- горизонтальный hangar gate;
- гибрид: внутренняя тяжелая дверь + короткий transition shaft + surface hatch/ring.

### 8. Surface arrival
Первое место выхода на поверхность должно быть:
- сильным визуальным payoff;
- переходом из подземного мира в surface world;
- стартом настоящего surface loop.

Сначала допустим более базовый surface ring / hatch / pad.
Позже эта точка может стать более укрепленным узлом.

### 9. Первый surface loop
После выхода на поверхность игрок должен получить:
- первое большое пространство;
- первый тяжелый obstacle / clearance task;
- первый authored combat encounter;
- первую service/rest point;
- первый recovery payoff.

Surface arrival не должен быть просто красивой заставкой.

### 10. BT-72 two-seat support
BT-72 должен поддерживать двухместность:
- основное место пилота;
- второе место для другого игрока;
- второе место открывается только если владелец / основной пилот разрешил доступ;
- вторая позиция — отдельная боевая роль;
- для второй позиции допустим отдельный weapon role: minigun / machine gun / support gunner seat.

Это особенно важно для solo + LAN first.

## Что реализовывать сейчас

Нужно реализовывать не карту, а только системную поддержку:

- cryo start state
- multi-occupancy shelter context
- early melee + vermin support
- access card / early clearance logic before Pip-Pad
- Pip-Pad acquisition flow
- blueprint / holo-record discovery flow
- staged BT-72 restoration logic
- core installation / activation
- sync/link flow
- internal hangar tutorial slice
- bunker exit unlock logic
- hangar gate / lift transition support
- first surface arrival support
- first heavy clearance support
- first combat encounter support
- first service/rest flow
- tower / recovery payoff hooks
- debrief / next hook flow
- BT-72 second seat permission + gunner role support

## Что НЕ делать сейчас

- карту;
- authored layout;
- конкретную геометрию помещений;
- surface city layout;
- расстановку интерьеров;
- укрепления и декор вручную;
- literal copying of existing famous bunker/vault designs.

## Визуальный принцип
Использовать референсы как mood / structure / logic,
но не копировать буквально:
- двери,
- знаки,
- форму,
- пропуска,
- конкретные логотипы,
- конкретные узнаваемые элементы известных игр.

Нужен свой bunker standard, свой access layer, свой hangar exit, свой visual language.


# Next

## Живой статус

Этот верхний блок теперь главный.
Старый длинный текст ниже оставлен как архив контекста, чтобы ничего не терялось.
### Следующий рабочий пакет

- `start vertical slice polish`
- затем `BT-72 / combat / RPG depth`
- затем `recovery / industry / logistics` как более плотный mid-game backbone

### Закрыто в этом проходе

- канонизирован общий `scriptTag` registry: editor/runtime/validation теперь понимают одинаковые теги и legacy-aliases
- `WorldValidation` больше не считает каждый семантический `linkTarget` битой ссылкой; жёстко проверяются только ссылки, похожие на `registryId`
- editor presets переведены на канонические теги для `tower_sync`, `workshop_service`, `rail_depot`, `rail_fortress_hub`, `industrial_gate`, `fey_ring`, `tank_service`
- runtime interaction и terminal sync теперь понимают alias-теги без ручного ремонта старых world-файлов
- smoke-check покрывает alias normalization и validation contract
- `World::Load` теперь канонизирует legacy `scriptTag`, а `World::FindObjectByScriptTag` / `HasScriptTag` работают через normalized lookup
- editor warnings по `fey_ring` / `lanline_service_hub` / `tank_service` тоже учитывают legacy-aliases, а не только новые теги
- `GameplayDescriptorRegistry` вынесен из header в [src/GameplayDescriptorRegistry.cpp], `WorldValidation` вынесен в [src/WorldValidation.cpp]
- editor export теперь проходит через validation gate: blocking ошибки режут export, warnings не режут и попадают в статус
- `WorldExport` расширен query/filter/preset слоем: есть `WorldExportHistoryFilter`, `WorldExportComparePreset`, `WorldExportHistoryQuery`, `WorldExportHistorySelection`
- shared export history helper-layer теперь умеет `MatchesHistoryQuery`, `FilterWorldExportHistoryEntries`, `FindLatestMatchingHistoryEntry`, `ResolveComparePresetTarget` и compact badge/summary labels
- `BunkerEditor` получил filter row, compare preset combo, jump actions к `last successful shipping / prototype / blocked / baseline updated` и compact filtered audit list
- smoke-check теперь покрывает quick history filters, preset resolution, baseline-updated selection, blocked checkpoint selection и no-match fallback
- shared `World` теперь умеет `FindObjectByRegistryId`, `BuildObjectReferences`, `FindIncomingObjectReferences`, `FindOutgoingObjectReferences`, `HasIncomingObjectReferences` для `registryId`-style weak refs через `linkTarget`
- `BunkerEditor` теперь показывает `Weak References / XREF` блок у выбранного объекта, умеет jump-to-reference и предупреждает перед удалением объекта, на который еще ссылаются другие authored nodes
- smoke-check теперь покрывает shared world reference graph: resolved incoming/outgoing refs, unresolved weak ref и incoming-reference flag
- `BunkerEditor` validation panel теперь умеет issue search, severity filter, selected-object-only filter и фокус по уже отфильтрованному списку, чтобы warnings не тонули в общей ленте
- `MapObject` / `World` / `PrefabLibrary` теперь держат `editorLayer`: `BWL4` пишет его явно, legacy `BWL3` world-файлы получают inferred layer по object semantics, а smoke-check фиксирует roundtrip и backward-compatible layer inference
- `BunkerEditor` получил production-useful `layer manager`: `show/hide`, `lock/unlock`, `filter by layer`, `select first`, compact layer badges в object list и preview discipline для hidden/locked layers
- выбранный объект теперь собран в более цельный inspector: `display name` и `layer` редактируются рядом с identity/gameplay полями, секции `Semantic / Validation`, `XREF`, `Descriptor Presets`, `Transform / Runtime`, `Loot`, `Actions` больше не размазаны одной простыней, а runtime notes подсказывают service/fey/tank context
- shared `WorldEditorUndoStack` теперь ведет add/remove/update/world-metadata/batch-edit историю, коалесцирует повторные edits и покрыт smoke-check на add/update/metadata/remove/batch redo-flow
- `BunkerEditor` теперь держит живой `undo/redo` поверх authoring session: dirty-state синхронизирован с `load/export`, а быстрые semantic/layout/edit actions не остаются одноразовыми без отката
- world preview дожат до рабочего authoring viewport: есть grid-step snap, bounds gizmos для `width/depth`, drag player spawn, selected-object `XREF/link` overlay и interaction/service radius overlays поверх уже существующего selection/focus/semantic-chain flow
- `prefab/library` дожат до production-useful `v1`: shared `PrefabRecord` теперь хранит stable `id / targetType / sourceLabel / completionMode`, authored world держит `prefabSourceId` в `BWL5`, editor показывает usage/broken-ref visibility, умеет `Focus Source / Focus First Usage / Update Prefab From Selected / Apply Prefab To Draft`, а smoke-check фиксирует prefab metadata, usage tracking и export-report summary
- `import assistant` больше не просто backlog-список: есть typed classification `Prop / Item / Structure / Environment / Scene Module`, safe prefab-draft generation, прямое add/update в library, seed current draft fields и richer concept manifest вместо голой текстовой заметки
- export discipline tightened без переписывания history-path: `WorldValidation` теперь ловит missing world metadata / empty object table / invalid spawn, validation report пишет `BWL5`, layers, prefab-derived counts, broken prefab refs и library-load status, а smoke-check фиксирует prefab-aware export report
- shared `Lanline Services` contract теперь прозрачно связывает launcher/runtime/profile: unlock-state хранит explicit `tower / relay / service / water / fey` flags поверх tier, launcher/runtime overlay-ят service state поверх `SessionProfile`, а smoke-check покрывает unlock resolution, services save roundtrip и profile snapshot sync
- `Lanline Services` больше не заканчиваются на panel-state: support orders проходят `Queued -> Routed -> Delivered -> Claimed`, runtime `Pip-Pad` умеет claim delivered parcels в профиль/инвентарь, а `SessionProfile/WorldFieldState` теперь получают mirror по pending orders, cosmetics и relay-credit spend
- launcher main screen теперь держит compact non-modal announcement widget в левом верхнем углу: local build notice показывается по `build/announcement` drift, хранит read-state в `SessionProfile`, не требует интернета и не блокирует основной UI
- authored `tank_service` дожат до реального BT-72 payoff: runtime выбирает лучший доступный `service kit` по текущему повреждению, `track/servo/engine/lens` наборы дают разные subsystem effects и не тратятся впустую на полностью исправный танк, а smoke-check фиксирует весь `tank service kit` flow
- первый playable route перестал быть только текстом: в `SessionProfile` появился отдельный `FirstPlayableRouteProgress`, `StoryRoute` теперь держит checkpoint/objective/BT-72 restore helper-ы для runtime+launcher, а `GameRuntime` ведет маршрут через `cryo -> clues -> Pip-Pad -> archive -> BT-72 restore -> sync -> clearance module -> first combat -> first service -> recovery node -> debrief`

### Правило на следующие проходы

Сначала закрываем пункты из этого верхнего блока.
Старые заметки ниже не удаляются, а служат архивом и пояснением.

Границу `Lanline` не выносим в новый отдельный md: рабочий канон держим в коде, `ROADMAP.md` и верхнем блоке этого файла.

Старое содержимое `Next.md` разложено по отдельным файлам, чтобы этот файл больше не был гигантской свалкой текста.

Смотри:

- общий индекс: `Next_split/README.md`
- общий план: `Next_split/00_MASTER_PLAN.md`
- launcher: `Next_split/Launcher/src/Launcher_Main.cpp.md`
- services:
  - `Next_split/include/SessionProfiles.hpp.md`
  - `Next_split/include/LanlineServices.hpp.md`
  - `Next_split/src/LanlineServices.cpp.md`
- runtime/world:
  - `Next_split/include/GameRuntime.hpp.md`
  - `Next_split/src/GameRuntime.cpp.md`
  - `Next_split/include/World.hpp.md`
  - `Next_split/src/World.cpp.md`
- story/progression:
  - `Next_split/include/Progression.hpp.md`
  - `Next_split/src/Progression.cpp.md`
  - `Next_split/include/StoryRoute.hpp.md`
  - `Next_split/src/StoryRoute.cpp.md`
- tank/editor:
  - `Next_split/include/HangarSystem.hpp.md`
  - `Next_split/Editor/src/Editor_Main.cpp.md`
- docs:
  - `Next_split/ROADMAP.tasks.md`
  - `Next_split/Use_this_One.tasks.md`
  - `Next_split/trash.tasks.md`

Если понадобится, следующим проходом можно уже не раскладывать заметки, а прямо пройтись по этим файлам и добить код по одному блоку за раз.

## Архив старых заметок

Ниже оставлен сырой исторический контекст. Он больше не является главным планом и не должен спорить с верхним блоком этого файла.

разберу в двух слоях — что уже есть сейчас по факту, и что именно нужно, чтобы проект выглядел как показываемый/почти релизный. Параллельно вытащу сильные места архитектуры и дам готовые вставляемые куски кода там, где вижу самые дорогие пробелы.

Я уже подтвердил базовую картину: это не просто набросок, а собранный CMake-проект с тремя отдельными приложениями (BunkerGame, BunkerLauncher, BunkerEditor) и своей дорожной картой. Дальше добираю сам код модулей и документацию, чтобы оценка “сколько сделано / сколько осталось” была не по названиям файлов, а по реальной логике и связанности систем.

Добираю последние опорные места: сейчас свожу архитектуру, прогрессию, запуск, редактор и сохранения в одну карту, чтобы дать не “впечатление”, а нормальный техразбор с приоритетами и вставляемыми решениями.

Thought for 14m 11s

Я реально прошёл не только README/ROADMAP, а именно структуру и ключевые исходники: CMakeLists.txt, src/main.cpp, src/GameRuntime.cpp, src/World.cpp, src/StoryRoute.cpp, src/SkillSystem.cpp, Launcher/src/Launcher_Main.cpp, Editor/src/Editor_Main.cpp, include/SessionProfiles.hpp, src/LaunchSession.cpp, src/LanlineServices.cpp. По факту это уже не “заготовка”, а большой прототипный набор из трёх приложений: игра, лаунчер и редактор, собранных одним CMake-проектом на C++20/OpenGL/GLFW/ImGui.

Главный вывод

Сейчас проект выглядит как сильный mid-stage prototype / systems-heavy vertical slice framework, а не как ранний скелет. Основа уже есть: отдельный launcher, отдельный editor, контроль запуска через launch ticket, загрузка/создание мира, сохранения, story-route, skill awakening, инфраструктурная прогрессия, авторинг через editor и слой Lanline services. Но до состояния “приятно показывать всем” и тем более до “релизно” ещё остаётся заметная работа по уплотнению архитектуры, валидации данных, полировке UX и убиранию монолитов.

Сколько сделано / сколько осталось

Это уже моя инженерная оценка, не математика по чекбоксам:

Как текущее ядро проекта: сделано примерно 68–75%.
Почему так: базовая архитектура, редактор, launcher-flow, world persistence и большая часть стартового gameplay-loop уже на месте.
До состояния “можно уверенно показывать как вертикальный срез / демо”: осталось примерно 20–30%.
То есть не нужно “делать игру с нуля”; нужно дожать целостность: стартовый сценарий, стабильность сейвов, UX лаунчера, editor-validation, чистка самых тяжёлых if-chain/monolith участков и подготовка демо-сборки.
До реально релизного состояния: осталось примерно 55–65%.
Потому что релиз — это уже не только systems/content, а ещё устойчивые миграции сохранений, тесты, CI, packaging, crash-safe persistence, data validation, контентная балансировка, UX-полировка, поддержка ошибок пользователя и ясная продуктовая граница, что такое Lanline: атмосферный shell или реальный multiplayer-service layer.
Что уже сделано хорошо
1) Архитектурно правильно разделены роли приложений

У тебя не одна “всё-в-одном” программа, а три отдельные: BunkerGame, BunkerLauncher, BunkerEditor. Это очень правильный уровень мышления для проекта такого типа: игрок идёт через launcher, контент редактируется editor’ом, runtime остаётся runtime’ом. Уже сам CMake это подтверждает.

2) Launcher — не декоративный, а реально встроен в продуктовый поток

main.cpp не просто пишет “лучше запускать из launcher”, а реально требует launch ticket и при ошибке показывает окно с отказом запуска. В LaunchSession.cpp ticket выдаётся, одноразово потребляется, удаляется после чтения и истекает через 60 секунд. Это очень хороший уровень продуманности для прототипа.

3) World/persistence уже не игрушечные

Игра грузит выбранный мир, а если мира нет — генерирует стартовую зону, сохраняет её и затем принудительно достраивает starter infrastructure. В World.cpp уже есть формат мира с версиями BWLD и BWL2, а roadmap отдельно фиксирует расширение до BWL2 и semantic authoring fields. Это значит, что ты уже думаешь не “про одну карту руками”, а про живущий формат данных.

4) Editor уже реально полезный, а не заглушка

По roadmap там уже добавлены search/filter, безопасное дублирование, prefab/library workflow, рабочий preview для выбора и постановки объектов, navigation в стиле Creation Kit, drag/move через preview, interaction overlay, специализированные gameplay-draft кнопки и descriptor-presets поверх scriptTag/linkTarget. Это очень серьёзный шаг: editor уже стал частью production loop, а не просто окном “поставить кубик”.

5) Прогрессия собрана вокруг систем, а не только around UI

Есть выделенный StoryRoute.cpp, а skill-awakening в SkillSystem.cpp завязан на реальные действия игрока: archive sync, foot kills, tank actions, stress survival, heavy carry drills, field service use. Это правильно: механики растут из поведения игрока, а не только из кнопки “получить перк”.

6) Lanline layer уже тематически встроен в игру

Это не просто фиктивная вкладка. Там есть friends/chat/voice/support flow, сохранение/восстановление состояния, relay messages, voice presence и явно прописано, что raw audio transport — future work. То есть слой уже честно обозначен как системный shell с persistence/state mirror, а не как законченная сеть.

Что сейчас тормозит проект
1) Монолиты слишком большие

Самый большой structural risk: несколько файлов стали “центрами мира”.
GameRuntime.cpp — 3835 lines / 181 KB, Editor_Main.cpp — 2041 lines, Launcher_Main.cpp — 1249 lines, SessionProfiles.hpp — 1267 lines. Это уже не просто “много кода”; это сигнал, что рост функционала начинает бить по поддерживаемости, локальному reasoning, тестируемости и скорости добавления контента.

2) Слишком многое держится на string-driven логике

Судя по runtime-структуре и editor-presets, важная часть поведения строится вокруг scriptTag и linkTarget. Для прототипа это быстро и мощно, но дальше это начинает быть хрупким: опечатка в строке = сломанная логика, битая связка, тихая ошибка авторинга. Сам подход хороший для authoring, но ему уже нужен слой реестров, валидаторов и нормальных dispatch tables.

3) Persistence уже есть, но ещё не “crash-safe”

main.cpp сохраняет мир и профиль на выходе, что хорошо, но сейчас следующий шаг — атомарные сохранения, versioned migrations и более жёсткая защита от partially-written state. Для проекта с несколькими видами данных это уже критично.

4) Lanline пока продуктово не до конца определён

Сейчас это сильный атмосферный и системный слой, но ещё не полноценный multiplayer/network runtime. И это нормально — просто это надо закрепить решением. Либо это остаётся “immersive asynchronous/session shell”, либо потом нужен отдельный сетевой контракт и другой объём работ.

5) По видимой структуре репозитория я не увидел явного слоя автотестов/CI

Для прототипа это нормально, но для showable/release-пути именно здесь потом будут самые дорогие баги: broken world refs, regression в save format, невалидные prefab data, случайно сломанный launcher-flow.

Что делать дальше: сначала до “можно показывать”, потом до “релизно”
Этап A — быстро довести до сильного демо
Заморозить scope демо.
Не пытаться сейчас полировать весь мир. Выбери один гарантированный маршрут: bunker wake-up → Pip-Pad → tank link → first tower sync → первый recovery loop → один яркий service/Lanline payoff. Это уже достаточно для демо.
Сделать editor-validation перед сохранением мира.
Нужны проверки: duplicate registryId, битые linkTarget, suspicious scriptTag, объекты без имени, битые transition-style anchors.
Перевести 2–3 самых жирных string-if цепочки в таблицы.
Не весь runtime сразу. Сначала:
DescribeTerminalSync, затем interaction-dispatch для authored terminals, затем editor descriptor mapping.
Сделать crash-safer save flow.
Хотя бы atomic temp-write для world/profile.
Подготовить “golden demo world + reset profile”.
Чтобы можно было в 1 клик вернуться в корректное демо-состояние.
Подчистить launcher UX.
Чётко показывать: selected world, current objective summary, selected character, network/session status, кнопка launch, кнопка open editor.
Этап B — довести до инженерно зрелого проекта
Разбить GameRuntime.cpp на подсистемы.
Минимальный распил:
RuntimeInteraction.cpp
RuntimeRecovery.cpp
RuntimeTank.cpp
RuntimeLanline.cpp
RuntimeUI.cpp
RuntimeStorySync.cpp
Разгрузить SessionProfiles.hpp.
Сейчас там, похоже, сосредоточены model + normalization + save/load + operational graph. Я бы разделил:
SessionProfileTypes.hpp
SessionProfileNormalize.cpp
SessionProfilePersistence.cpp
WorldFieldStateRules.cpp
Сделать migration/versioning для profile save.
Сейчас уже есть живой evolving проект. Значит, старые сейвы будут ломаться без нормальной версии и миграций.
Собрать world authoring contract.
То есть задокументировать:
какие scriptTag допустимы,
какие требуют linkTarget,
какие ждут конкретный InteractionType,
какие дают прогрессию,
какие editor presets им соответствуют.
Добавить automated checks.
Хотя бы:
загрузить/сохранить мир и сравнить семантику,
валидация всех world files,
smoke-test launcher ticket flow,
save/load profile roundtrip.
Уточнить статус Lanline.
Либо это fiction-driven services layer, либо начинаешь проектировать реальные transport/session rules отдельно.
Самые сильные части проекта

Сильнее всего у тебя сейчас выглядят:

Product thinking: launcher/game/editor разделены правильно.
Data evolution thinking: мир уже versioned (BWLD → BWL2).
Authoring direction: editor уже заточен под gameplay authoring, а не только под геометрию.
Systemic progression: story, skills, tank/recovery/service loops растут вместе.
Launch discipline: launcher ticket — это зрелая мысль.
Готовые куски кода, которые я бы вставил прямо сейчас

Этот блок уже внедрён в проект и очищен из `Next.md`, чтобы не хранить здесь устаревшие дубли кода:

- table-driven `DescribeTerminalSync` / `TryGetTerminalSyncText`
- `WorldValidation`
- `AtomicPersistence`
    const fs::path backupPath = finalPath.string() + ".bak";

    fs::remove(tempPath, ec);

    if (!writer(tempPath)) {
        fs::remove(tempPath, ec);
        return {false, "Failed to write temp file: " + tempPath.string()};
    }

    if (fs::exists(backupPath, ec)) {
        fs::remove(backupPath, ec);
    }

    if (fs::exists(finalPath, ec)) {
        fs::rename(finalPath, backupPath, ec);
        if (ec) {
            fs::remove(tempPath, ec);
            return {false, "Failed to rotate old file: " + finalPath.string()};
        }
    }

    fs::rename(tempPath, finalPath, ec);
    if (ec) {
        std::error_code restoreEc;
        if (fs::exists(backupPath, restoreEc)) {
            fs::rename(backupPath, finalPath, restoreEc);
        }
        fs::remove(tempPath, restoreEc);
        return {false, "Failed to promote temp file: " + finalPath.string()};
    }

    if (fs::exists(backupPath, ec)) {
        fs::remove(backupPath, ec);
    }

    return {true, {}};
}

inline SaveStatus SaveWorldAtomically(const World& world, const fs::path& worldPath) {
    return AtomicWriteFile(worldPath, [&](const fs::path& tempPath) {
        return world.Save(tempPath.string());
    });
}

inline SaveStatus SaveProfileAtomically(const SessionProfile& profile, const fs::path& profilePath) {
    return AtomicWriteFile(profilePath, [&](const fs::path& tempPath) {
        return SaveSessionProfile(profile, tempPath);
    });
}

} // namespace bunker

Замена в main.cpp вместо прямого save на выходе:

const auto worldSave = bunker::SaveWorldAtomically(world, worldPath);
const auto profileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);

if (!worldSave.ok) {
    std::fprintf(stderr, "World save failed: %s\n", worldSave.message.c_str());
}
if (!profileSave.ok) {
    std::fprintf(stderr, "Profile save failed: %s\n", profileSave.message.c_str());
}

staticEraser.Save(sessionProfile.selectedWorld);

Если сжать всё в одну фразу: у тебя уже не “идея игры”, а реально сложившийся systems-prototype с правильным разделением launcher/game/editor; главный следующий выигрыш даст не добавление ещё 20 фич, а стабилизация данных, распил монолитов и жёсткая валидация authoring-пайплайна.

Могу следующим сообщением сделать ещё более жёсткий разбор по конкретным файлам: что именно вынести из GameRuntime.cpp, SessionProfiles.hpp, Editor_Main.cpp, Launcher_Main.cpp и в каком порядке лучше рефакторить.

зайду уже в конкретные исходники по слоям и соберу именно карту рефакторинга: что выносить из GameRuntime.cpp, SessionProfiles.hpp, Editor_Main.cpp, Launcher_Main.cpp, в каком порядке и с каким эффектом. В конце отдельно перечислю, какие именно “20 фич” я имел в виду, чтобы это не осталось абстракцией.

Уже видно узкое место точнее: roadmap богатый, но самый большой риск не в “нехватке идей”, а в том, что несколько файлов стали контейнерами слишком многих обязанностей сразу. Сейчас доберу конкретику из этих файлов и сведу это в порядок рефакторинга по шагам, не общими словами.

Thought for 7m 19s

Сделал уже именно то, что ты просил: не “общие впечатления”, а разложил по конкретным файлам, что из них выносить, в каком порядке и зачем.

Короткий вывод

Сейчас проект уже выглядит как сильный systems-prototype, а не как сырой набросок: Launcher v1 у тебя почти закрыт, Editor v1 уже production-useful, LAN foundation в работе, Data Cards/лор начаты частично, а большой слой “заводы/энергия/инфраструктура” сам roadmap честно помечает как по сути ещё не начатый до полноценного состояния. При этом самые тяжёлые места — это уже не отсутствие идей, а монолиты: GameRuntime.cpp 4114 строк, SessionProfiles.hpp 1267 строк, Editor_Main.cpp 2041 строка, Launcher_Main.cpp 1249 строк.

Что я бы рефакторил по файлам
1) src/GameRuntime.cpp — главный кандидат на распил

Сейчас это уже слишком большой “центр вселенной”. Внутри у тебя сидят сразу: tank utility/helpers, inventory/economy helpers, objective/recovery logic, grid/logistics/industrial updates, Lanline diagnostics, PIP-PAD UI и куски interaction-логики. Особенно видно, что файл уже содержит длинные цепочки по scriptTag, пачку Update... функций для recovery/industry и огромный UI-блок DrawPipPad(...).

Что в нём хорошо:

логика recovery-loop уже реально системная, а не декоративная;
progression увязана с инфраструктурой;
Story/PIP-PAD/NET/SERV ощущаются как части одного мира, а не случайные вкладки.
Это подтверждается и roadmap: rail freight, orbital uplink, Rail Fortress, Recovery Fabricator, industrial gate/survey/outpost и дальнейшие inner-spur узлы уже встроены в текущий контур.

Что в нём плохо:

слишком много обязанностей в одном translation unit;
string-driven dispatch по scriptTag уже становится дорогим в поддержке;
UI и gameplay update loops перемешаны сильнее, чем нужно.

Как я бы делил в первую очередь:

RuntimeInteraction.cpp
DescribeTerminalSync
contextual interaction helpers
terminal/service anchor logic
всё, что завязано на scriptTag и linkTarget
RuntimeEconomy.cpp
AddInventoryItem
HasInventoryItem
ConsumeInventoryItem
CurrentInventoryWeight
TryConsumeFieldRation
RuntimeRecovery.cpp
UpdateScavengerTeams
UpdateCaravanRoute
UpdateDroneStations
UpdateTradeNetwork
UpdateRailFreight
UpdateOrbitalUplink
UpdateRailFortress
UpdateRecoveryFabricator
UpdateRecoveryMilestones
RuntimeIndustry.cpp
UpdateIndustrialSurvey
UpdateIndustrialOutpost
UpdateAssemblyCell
UpdateFoundryLine
UpdateReactorYard
UpdateCapacitorBank
UpdateRelaySubstation
UpdateServiceBay
UpdateWaterReclaimer
RuntimePipPad.cpp
вместо одного DrawPipPad сделать:
DrawPipPadStatTab
DrawPipPadInventoryTab
DrawPipPadDataTab
DrawPipPadMapTab
DrawPipPadQuestTab
DrawPipPadNetTab
DrawPipPadServicesTab
RuntimeLanlineUi.cpp
slot-state helpers
diagnostics cache
runtime NET tab render

Приоритет: сначала вытащить DrawPipPad, потом recovery/industry updates, потом interaction dispatch.
Это даст самый большой выигрыш по читаемости без слома геймплея.

2) include/SessionProfiles.hpp — второй по важности файл

Сейчас это уже не просто header со структурами. Там сидят:

сами модели (SessionProfile, WorldFieldState, CharacterProfile, tank и т.д.),
world-state lookup/merge,
normalize logic,
operational helper functions,
SaveSessionProfile,
LoadSessionProfile.

Это место уже стало слишком “умным” для header-файла.

Что в нём хорошо:

WorldFieldState у тебя уже реально богатый и отражает живущий recovery-state;
NormalizeSessionProfile — правильная мысль;
per-world state и selected-world flow сделаны концептуально верно.

Что в нём рискованно:

save/load живут inline в header;
свой key=value формат без явной версии и без безопасных parse wrappers;
любое изменение профиля раздувает recompilation и повышает риск тихих ошибок.

Как делить:

SessionProfileTypes.hpp
только struct/enum
SessionProfileQuery.hpp
FindWorldFieldState
HasAwakenedSpecialistRole
HasAssignedSpecialistRole
operational boolean helpers
SessionProfileNormalize.cpp
NormalizeWorldFieldState
NormalizeSessionProfile
merge helpers
SessionProfilePersistence.cpp
SaveSessionProfile
LoadSessionProfile
safe parse helpers
version header
позже: SessionProfileMigrations.cpp
когда начнёшь реально ломать старые сейвы

Самое важное здесь: не оставлять persistence внутри header и добавить versioned save-format.

3) Editor/src/Editor_Main.cpp — очень сильный, но уже перегруженный

Тут хорошая новость: editor у тебя уже не игрушка. По roadmap он умеет runtime world handoff, spawn setup, preview mode, search/filter, duplicate with new registry id, prefab/library workflow и authoring presets.

По коду это тоже видно:

есть DrawWorldPreview(...),
есть prefab load/save,
есть descriptor preset helpers,
есть export/runtime panel,
есть import assistant,
есть warnings по fey_ring, lanline_service_hub, tank_service.
То есть логика уже полезная. Но почти всё это живёт внутри одного main().

Что я бы выносил:

EditorPresetDrafts.cpp
PrepareSpecializedDraft
preset creation
default registry id generation
EditorDescriptorPresets.cpp
все Apply...DescriptorPreset(...)
это отдельный authoring-layer
EditorPrefabLibrary.cpp
LoadPrefabLibrary
SavePrefabLibrary
capture/place/remove prefab
EditorPreview.cpp
DrawWorldPreview
viewport state
selection/drag/focus logic
EditorRuntimeExport.cpp
runtime alignment panel
export/save-as/set-active flow
import assistant manifest
EditorPanels.cpp
DrawAssetPalettePanel
DrawWorldAuthoringPanel
DrawExportPanel
DrawImportAssistantPanel

Что editor’у сейчас не хватает сильнее всего:

полноценной world validation перед export,
единого descriptor registry,
undo/redo,
проверки битых ссылок,
отчёта “что именно сломано в authored world”.
4) Launcher/src/Launcher_Main.cpp — уже близко к хорошему виду

По roadmap launcher почти завершён: обязательная точка входа, выбор мира, session setup, startup summary, launch gate, LAN-first flow.

По коду сильные стороны:

Play BunkerGame и Open BunkerEditor встроены правильно;
launch идёт через ticket;
joinability/ready-seat/match-start logic уже собрана;
session browser уже не пустой список, а реальный lobby/status view.

Что плохо:

много lobby helper-логики дублируется между launcher/runtime;
main() всё ещё держит auth mock, refresh, LAN session browser, launch orchestration и services panel одновременно.

Как делить:

LauncherLanlineLobby.cpp
slot-state helpers
joinability
ready logic
accept/clear/reserve peer logic
LauncherLaunchFlow.cpp
prepare profile
issue ticket
save roster state
launch sibling executable
LauncherPanels.cpp
DrawAccessConsole
DrawOperationsDeck
session summary / known sessions / selected diagnostics
LanlineLobbyLogic.hpp
общий shared helper для launcher + runtime + main.cpp
5) src/main.cpp, src/LaunchSession.cpp, src/StoryRoute.cpp, src/SkillSystem.cpp

Тут разный приоритет.

LaunchSession.cpp я бы почти не трогал: он компактный и по делу. Сейчас там уже есть issue/consume ticket, удаление ticket после чтения и истечение по времени через 60 секунд. Это как раз удачная и завершённая часть.

StoryRoute.cpp я бы сохранил как отдельный канон прогрессии. Он небольшой и очень полезный: route уже явно ведёт от cryo/Pip-Pad/archive/tank/relay до rail/orbital/fabricator/industrial chain и water reclaimer. Его не надо обратно растворять в runtime.

SkillSystem.cpp тоже не монстр и мыслится правильно: awakening растёт из archive sync, foot kills, tank actions, stress survival, heavy carry, field service. Это хороший файл; позже его можно сделать data-driven, но не сейчас.

main.cpp же стоит подтянуть:

заменить прямые save-вызовы на atomic save;
вынести bootstrap/loading world/profile в GameBootstrap.cpp;
оставить в main.cpp только gate + init + run + shutdown.
В каком порядке это делать

Вот порядок, который я бы реально выбрал:

Вынести общий LanlineLobbyLogic.hpp и удалить дубли из launcher/runtime/main.
Вынести DrawPipPad по вкладкам.
Вынести recovery/industry update functions из GameRuntime.cpp.
Сделать GameplayDescriptorRegistry.hpp.
Сделать WorldValidation.hpp и включить её в editor export.
Перенести persistence из SessionProfiles.hpp в .cpp.
Добавить versioned profile save/load.
Разрезать Editor_Main.cpp на panels + preview + presets + export.
Разрезать Launcher_Main.cpp на panels + lanline + launch flow.
Добавить 3 smoke-checks: world roundtrip, profile roundtrip, launch ticket flow.
Готовые куски кода, которые реально стоит вставить
1) Общая логика Lanline lobby, чтобы убрать дубли
#pragma once

#include <algorithm>
#include <string>
#include "LanlineSession.hpp"

namespace bunker {

inline bool IsLanlineAwaitingSlot(const LanlinePlayerEntry& entry) {
    return entry.role == "Awaiting";
}

inline bool IsLanlinePendingSlot(const LanlinePlayerEntry& entry) {
    return entry.role == "Pending Client";
}

inline bool IsLanlineReservedSlot(const LanlinePlayerEntry& entry) {
    return entry.role == "Reserved Client";
}

inline bool IsLanlineAcceptedSlot(const LanlinePlayerEntry& entry) {
    return entry.role == "Client";
}

inline bool IsLanlineReadyEligibleSlot(const LanlinePlayerEntry& entry) {
    return entry.role == "Host" || entry.role == "Client" || entry.role == "Local Operator";
}

inline const char* LanlineSlotStateLabel(const LanlinePlayerEntry& entry) {
    if (IsLanlineAwaitingSlot(entry)) return "Open";
    if (IsLanlinePendingSlot(entry)) return "Pending";
    if (IsLanlineReservedSlot(entry)) return "Reserved";
    if (IsLanlineAcceptedSlot(entry)) return "Accepted";
    return entry.online ? "Active" : "Reserved";
}

inline const char* LanlineReadyLabel(const LanlinePlayerEntry& entry) {
    if (!IsLanlineReadyEligibleSlot(entry)) return "-";
    return entry.ready ? "Ready" : "Not Ready";
}

inline int FindFirstAwaitingSlotIndex(const LanlineSessionState& state) {
    for (int i = 0; i < static_cast<int>(state.players.size()); ++i) {
        if (IsLanlineAwaitingSlot(state.players[static_cast<std::size_t>(i)])) {
            return i;
        }
    }
    return -1;
}

inline int MaxLanlineSessionSlots(const LanlineSessionState& session) {
    if (session.mode != "LAN Host") return 0;
    return std::max(1, static_cast<int>(session.players.size()));
}

inline int OccupiedLanlineSessionSlots(const LanlineSessionState& session) {
    int occupied = 0;
    for (const auto& player : session.players) {
        if (session.mode == "LAN Host" && IsLanlineAwaitingSlot(player)) continue;
        if (player.role != "Awaiting") ++occupied;
    }
    return occupied;
}

inline int AvailableLanlineSessionSlots(const LanlineSessionState& session) {
    return std::max(0, MaxLanlineSessionSlots(session) - OccupiedLanlineSessionSlots(session));
}

inline int PendingLanlineSessionSlots(const LanlineSessionState& session) {
    int count = 0;
    for (const auto& player : session.players) {
        if (IsLanlinePendingSlot(player)) ++count;
    }
    return count;
}

inline int ReservedLanlineSessionSlots(const LanlineSessionState& session) {
    int count = 0;
    for (const auto& player : session.players) {
        if (IsLanlineReservedSlot(player)) ++count;
    }
    return count;
}

inline int AcceptedLanlineSessionSlots(const LanlineSessionState& session) {
    int count = 0;
    for (const auto& player : session.players) {
        if (IsLanlineAcceptedSlot(player)) ++count;
    }
    return count;
}

inline int ReadyLanlineSessionSlots(const LanlineSessionState& session) {
    int count = 0;
    for (const auto& player : session.players) {
        if (IsLanlineReadyEligibleSlot(player) && player.ready) ++count;
    }
    return count;
}

inline bool IsJoinableLanlineSession(const LanlineSessionState& session) {
    if (session.mode != "LAN Host") return false;
    if (!session.connectedPeer.empty()) return false;
    return session.lifecycleStage == "HostLobbyOpen" ||
           session.lifecycleStage == "HostJoinPending" ||
           session.lifecycleStage == "HostRuntimeActive";
}

inline bool IsLanlineMatchStartReady(const LanlineSessionState& session) {
    if (session.mode != "LAN Host") return false;
    if (ReservedLanlineSessionSlots(session) > 0 || PendingLanlineSessionSlots(session) > 0) return false;

    int readyEligible = 0;
    for (const auto& player : session.players) {
        if (!IsLanlineReadyEligibleSlot(player)) continue;
        ++readyEligible;
        if (!player.ready) return false;
    }
    return readyEligible >= 2;
}

} // namespace bunker
2) Единый registry для scriptTag, чтобы editor и runtime говорили на одном языке
#pragma once

#include <array>
#include <optional>
#include <string_view>
#include "MapObject.hpp"

namespace bunker {

struct GameplayDescriptorSpec {
    std::string_view scriptTag;
    std::string_view label;
    InteractionType preferredInteraction;
    ObjectCategory preferredCategory;
    bool requiresLinkTarget = false;
};

inline const GameplayDescriptorSpec* FindGameplayDescriptor(std::string_view scriptTag) {
    static constexpr std::array<GameplayDescriptorSpec, 12> kSpecs{{
        {"tower_sync", "Tower Sync", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"power_pylon", "Power Pylon", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"drone_station", "Drone Station", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"rail_freight", "Rail Depot", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"orbital_uplink", "Orbital Uplink", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"rail_fortress", "Rail Fortress", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"recovery_fabricator", "Recovery Fabricator", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"industrial_gate", "Industrial Gate", InteractionType::Transition, ObjectCategory::Landmark, false},
        {"fey_ring", "Fey Ring", InteractionType::Transition, ObjectCategory::Landmark, true},
        {"lanline_service_hub", "Lanline Service Hub", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"tank_service", "Tank Service", InteractionType::Workshop, ObjectCategory::Hangar, false},
        {"remote_link", "Remote Link", InteractionType::Terminal, ObjectCategory::Terminal, true},
    }};

    for (const auto& spec : kSpecs) {
        if (spec.scriptTag == scriptTag) return &spec;
    }
    return nullptr;
}

} // namespace bunker
3) Валидация authored world перед export
#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "GameplayDescriptorRegistry.hpp"
#include "World.hpp"

namespace bunker {

enum class ValidationSeverity {
    Warning,
    Error
};

struct ValidationIssue {
    ValidationSeverity severity;
    std::string objectId;
    std::string message;
};

inline std::vector<ValidationIssue> ValidateWorldForRuntime(const World& world) {
    std::vector<ValidationIssue> issues;
    std::unordered_set<std::string> ids;

    for (const auto& obj : world.objects) {
        if (obj.registryId.empty()) {
            issues.push_back({ValidationSeverity::Error, "", "Object has empty registryId."});
        } else if (!ids.insert(obj.registryId).second) {
            issues.push_back({ValidationSeverity::Error, obj.registryId, "Duplicate registryId."});
        }

        if (obj.displayName.empty()) {
            issues.push_back({ValidationSeverity::Warning, obj.registryId, "Object has empty displayName."});
        }

        if (const auto* spec = FindGameplayDescriptor(obj.scriptTag)) {
            if (obj.interaction != spec->preferredInteraction) {
                issues.push_back({
                    ValidationSeverity::Warning,
                    obj.registryId,
                    "Interaction does not match preferred interaction for scriptTag " + std::string(spec->scriptTag)
                });
            }

            if (obj.category != spec->preferredCategory) {
                issues.push_back({
                    ValidationSeverity::Warning,
                    obj.registryId,
                    "Category does not match preferred category for scriptTag " + std::string(spec->scriptTag)
                });
            }

            if (spec->requiresLinkTarget && obj.linkTarget.empty()) {
                issues.push_back({
                    ValidationSeverity::Error,
                    obj.registryId,
                    "scriptTag requires linkTarget: " + std::string(spec->scriptTag)
                });
            }
        }

        if (!obj.linkTarget.empty() && !world.HasObject(obj.linkTarget)) {
            issues.push_back({
                ValidationSeverity::Error,
                obj.registryId,
                "Broken linkTarget: " + obj.linkTarget
            });
        }
    }

    return issues;
}

inline int CountErrors(const std::vector<ValidationIssue>& issues) {
    int errors = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Error) ++errors;
    }
    return errors;
}

} // namespace bunker

Минимальная интеграция в editor export:

const auto issues = bunker::ValidateWorldForRuntime(editorWorld);
if (bunker::CountErrors(issues) > 0) {
    statusText = "Export blocked: authored world has validation errors.";
} else {
    // editorWorld.Save(...)
}
Что я бы НЕ трогал первым

Не трогал бы первым:

StoryRoute.cpp
SkillSystem.cpp
LaunchSession.cpp

Они не идеальны, но у них хорошая цена/польза: они уже маленькие, осмысленные и не главные источники боли. Главное сейчас — это монолиты и data-validation.

Что я имел в виду под “не добавлять ещё 20 фич”

Вот именно какие 20 фич я бы сейчас не добавлял, пока не сделаны распил, валидация и safe persistence:

полноценный крафтовый UI с очередями производства
новые типы транспорта кроме текущего контура
отдельная мини-игра для реконструкции data fragments
расширенный cosmetics marketplace
полноценный voice transport вместо текущего presence-shell
PvP-логика
procedural world partition generation
система NPC-диалогов с ветвлениями
фракции и репутация
отдельная deep inventory grid-система
модульная броня для всех машин
хардкорная economy simulation по всем ресурсам
advanced pathfinding simulation для convoy/drone/rail
погодные аномалии нового типа сверх Acid Rain / Ether Fog
полноценный quest editor до завершения world validation
achievements/challenges layer
custom skins pipeline
matchmaking/online account layer поверх LAN-first контура
replay/photo mode
полноценный mod SDK

Под такие ограничения тебе сейчас нужен не “идеальный большой рефакторинг”, а план с максимальной отдачей на 1 запрос, 1 патч, 1 файл. Иначе лимиты Codex/Windsurf/ChatGPT сгорят раньше, чем код станет заметно лучше.

Что это значит на практике

Тебе сейчас нельзя:

скармливать ИИ весь репозиторий целиком,
просить “полностью перепиши архитектуру”,
запускать длинные многошаговые сессии с контекстом на десятки файлов,
тратить дни на красивые, но не критичные фичи.

Тебе сейчас нужно:

брать только самые дорогие узкие места,
работать по 1 файлу за раз,
просить у ИИ только diff / patch / конкретный код,
сначала дожать “демо-стабильность”, потом уже красоту.
Лучшая стратегия под твои лимиты
Роль инструментов
ChatGPT

Используй как:

архитектора,
ревьюера,
генератор точных планов,
генератор маленьких вставляемых кусков кода,
редактор prompt'ов для Codex/Windsurf.
Codex / Windsurf

Используй как:

исполнителя точечных изменений,
генератора патчей по одному файлу,
помощника по рутинному распилу.
Локально сам

Делай:

grep / поиск по проекту,
сборку,
запуск,
фиксацию ошибок,
проверку что патч правда скомпилировался.

То есть не ИИ “ведёт проект”, а ты ведёшь проект, а ИИ закрывает узкие ручные задачи.

Что делать за твои 5 дней
Цель

Не “сделать всё”, а получить:

более чистый код,
меньше риска сломать сейвы,
стабильный запуск через launcher,
editor с базовой валидацией,
showable demo build.
День 1 — только критический фундамент

Сделай 3 вещи и больше ничего.

1. Валидация мира перед сохранением/export

Это даст огромную отдачу сразу:

ловит битые registryId,
duplicate ids,
пустые displayName,
broken linkTarget,
несовпадения scriptTag / interaction.

Это одна из самых выгодных задач в проекте.

2. Атомарные сохранения

Чтобы не убивать мир/профиль кривым выходом.
Сначала world save, потом profile save через temp file + rename.

3. Registry для scriptTag

Не весь runtime переписывать.
Только сделать единый реестр соответствий:

scriptTag
label
preferred interaction
preferred category
requiresLinkTarget

Это сразу улучшит:

editor,
validation,
будущий runtime dispatch.

Итог дня 1: проект уже станет заметно взрослее, даже без новых фич.

День 2 — распил самого больного места без войны со всем проектом

Не трогай весь GameRuntime.cpp.
Только 2 самых жирных участка:

1. Вынеси DrawPipPad(...)

Сделай отдельные функции по вкладкам:

stats
inventory
data
map
quests
net
services
2. Вынеси DescribeTerminalSync и похожие string-driven куски

Даже если не весь interaction dispatch, хотя бы самые длинные if/else.

Почему именно это:
это даст сильный выигрыш по читаемости и почти не требует ломать core-loop.

День 3 — разгрузка SessionProfiles.hpp

Не надо пока делать идеальную систему миграций.

Сделай минимум:

Вынести из header:
SaveSessionProfile
LoadSessionProfile
parse helpers
normalize helpers

Оставь в .hpp:

только типы,
несколько маленьких inline query-функций, если без них совсем неудобно.

Почему это выгодно:
профиль — это одна из осей проекта. Когда persistence сидит в giant header, всё дорожает:

правки,
чтение,
компиляция,
риск ошибок.
День 4 — demo path

Тут не рефакторинг, а продукт.

Сделай один гарантированный маршрут показа:

запуск из launcher
выбор мира
вход в игру
стартовая зона
Pip-Pad
tank/service или tower sync
один recovery payoff
один Lanline/service payoff
Параллельно
сделай “demo world”
сделай “reset profile”
убери всё, что может случайно сломать показ

То есть не “весь проект теперь идеален”, а “демо проходит без стыда”.

День 5 — только polish и сборка
Сделай:
smoke test сценарий на бумаге,
release-like сборку,
быстрый список известных багов,
короткий README “как запускать демо”.
Не делай:
новые подсистемы,
большие переписывания,
новые экраны ради красоты.
Что не делать при лимитах

Вот прям запрещённые действия под твою ситуацию:

Не проси ИИ читать весь репозиторий заново каждый раз.
Не кидай по 10 файлов в один запрос.
Не проси “перепиши архитектуру полностью”.
Не проси “сделай production-ready всё”.
Не трать лимиты на стилистику и косметику раньше валидации/сейвов/демо.
Не давай ИИ самому выбирать, что менять во всём проекте.
Не запускай длинные циклы “ещё улучши / ещё улучши / ещё улучши”.
Как экономить лимиты максимально жёстко
Правило 1: один запрос = один файл

Пример хорошего режима:

сначала сам находишь нужный участок,
копируешь только его,
просишь только точечный патч.

Не “проанализируй весь GameRuntime.cpp”, а:

вот функция DrawPipPad, разбей её на 7 функций без изменения поведения, верни только готовый код/patch

Правило 2: проси diff, а не объяснение

Самый экономный формат:

Прочитай только этот фрагмент.
Не объясняй долго.
Верни unified diff.
Не меняй имена публичных типов.
Не добавляй новых зависимостей.

Правило 3: сначала локальный поиск, потом ИИ

Ищи сам:

rg "DrawPipPad"
rg "SaveSessionProfile"
rg "scriptTag"
rg "linkTarget"
rg "Lanline"

ИИ не должен тратить токены на поиск того, что тебе rg покажет за секунду.

Правило 4: не загружай контекст повторно

Один раз сформулируй для себя краткое описание проекта, и дальше просто вставляй его в коротком виде:

C++20 game project with Game/Launcher/Editor.
Need small safe refactor, no behavior changes, minimal dependencies.

Этого обычно хватает.

Готовые экономные prompt-шаблоны
1. Для распила функции
Read only the code I pasted below.
Task: split this function into smaller static helpers without changing behavior.
Constraints:
- keep C++20
- no new libraries
- preserve public interfaces
- keep naming style close to existing code
- return only the final code or unified diff
2. Для валидации мира
Read only this header and the World/Object structures below.
Task: write a minimal WorldValidation.hpp that checks:
- empty registryId
- duplicate registryId
- broken linkTarget
- empty displayName
- scriptTag rules for requiresLinkTarget
Constraints:
- no exceptions
- C++20
- header-only is acceptable
- return paste-ready code only
3. Для atomic save
Read only the save/load related code below.
Task: add atomic save using temp file + rename.
Constraints:
- preserve existing save format
- keep std::filesystem
- no external libs
- return minimal patch only
4. Для SessionProfiles
Read only this header.
Task: move persistence and normalization code out of the header into cpp files.
Do not redesign the entire system.
Keep public structs stable.
Return:
1) proposed file split
2) exact code for the new .hpp/.cpp pieces
3) minimal include changes
Приоритет задач по отдаче на лимит

Если совсем мало запросов, делай строго в таком порядке:

WorldValidation.hpp
AtomicPersistence.hpp
GameplayDescriptorRegistry.hpp
распил DrawPipPad
вынос SaveSessionProfile/LoadSessionProfile
общий LanlineLobbyLogic.hpp

Это самые выгодные шесть задач.

Что с VPN и трафиком

Если VPN реально ограничен по времени и трафику, то избегай:

повторных полных клонов,
скачивания крупных артефактов/зависимостей,
лишних IDE/cloud sync операций,
тяжелых AI-индексаций всего репозитория.

Практично делать так:

один локальный клон,
по возможности shallow fetch,
не гонять лишние бинарники,
не открывать облачные ассистенты на весь workspace, если можно скормить только файл.
Самая правильная мысль под твою ситуацию

Тебе сейчас нужен не “максимально умный процесс”, а максимально дешёвый по лимитам процесс.

То есть цель ближайших дней:

не сделать проект идеальным,
а сделать его заметно надёжнее и показабельнее за минимум запросов.
Мой честный совет по тактике

Работай так:

Пачка 1
world validation
atomic save
Пачка 2
descriptor registry
DescribeTerminalSync
Пачка 3
DrawPipPad split
Пачка 4
SessionProfiles persistence split
Пачка 5
demo world + reset profile + smoke test

Это лучший баланс между:

пользой,
риском,
числом AI-запросов,
временем,
шансом успеть до конца этих 5 дней.

это важно — тогда я не буду опираться на старую картину. Сначала быстро пересниму текущую структуру репозитория и уже после этого дам точные шаги в формате: какой файл удалить, какой добавить, полный путь и готовый код.

Thought for 2m 52s

Не забыл. Как раз поэтому сейчас дам тебе формат “сделай руками и сразу получишь выигрыш”.

После обновления репа безопаснее идти только добавочными шагами, ничего не удаляя на первом проходе: у тебя всё ещё один большой Editor/src/Editor_Main.cpp на 2041 строку, один Launcher/src/Launcher_Main.cpp, SessionProfiles.hpp остаётся большим header’ом на 1267 строк с inline SaveSessionProfile/LoadSessionProfile, а src/main.cpp всё ещё сохраняет мир и профиль напрямую в нескольких местах. Ещё в репе появился Next_split, но он сам помечен как разбор старого Next.md, а не как новый source of truth.

Шаг 1

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл на этом шаге

Добавить: include/GameplayDescriptorRegistry.hpp
Путь нового файла: include/GameplayDescriptorRegistry.hpp

Код для include/GameplayDescriptorRegistry.hpp
#pragma once

#include <array>
#include <string_view>

#include "MapObject.hpp"

namespace bunker {

struct GameplayDescriptorSpec {
    std::string_view scriptTag;
    std::string_view label;
    InteractionType preferredInteraction;
    ObjectCategory preferredCategory;
    bool requiresLinkTarget = false;
};

inline const GameplayDescriptorSpec* FindGameplayDescriptor(std::string_view scriptTag) {
    static constexpr std::array<GameplayDescriptorSpec, 15> kSpecs{{
        {"tower_sync", "Tower Sync", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"power_pylon", "Power Pylon", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"drone_station", "Drone Station", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"rail_freight", "Rail Freight", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"orbital_uplink", "Orbital Uplink", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"rail_fortress", "Rail Fortress", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"recovery_fabricator", "Recovery Fabricator", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"industrial_gate", "Industrial Gate", InteractionType::Transition, ObjectCategory::Landmark, false},
        {"industrial_survey", "Industrial Survey", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"industrial_outpost", "Industrial Outpost", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"assembly_cell", "Assembly Cell", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"reactor_yard", "Reactor Yard", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"lanline_service_hub", "Lanline Service Hub", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"tank_service", "Tank Service", InteractionType::Workshop, ObjectCategory::Hangar, false},
        {"fey_ring", "Fey Ring", InteractionType::Transition, ObjectCategory::Landmark, true},
    }};

    for (const auto& spec : kSpecs) {
        if (spec.scriptTag == scriptTag) {
            return &spec;
        }
    }
    return nullptr;
}

inline bool ScriptTagRequiresLinkTarget(std::string_view scriptTag) {
    if (const auto* spec = FindGameplayDescriptor(scriptTag)) {
        return spec->requiresLinkTarget;
    }
    return false;
}

} // namespace bunker
Зачем это первым

Это создаёт один общий справочник для scriptTag, чтобы потом одинаково использовать его в editor, validation и runtime, а не держать смысл тегов размазанным по коду. У тебя уже есть MapObject с scriptTag и linkTarget, так что это естественное расширение текущей модели.

Следующий шаг

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл на этом шаге

Добавить: include/WorldValidation.hpp
Путь нового файла: include/WorldValidation.hpp

Код для include/WorldValidation.hpp
#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "GameplayDescriptorRegistry.hpp"
#include "World.hpp"

namespace bunker {

enum class ValidationSeverity {
    Warning,
    Error
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Warning;
    std::string code;
    std::string objectId;
    std::string message;
};

inline std::vector<ValidationIssue> ValidateWorldForRuntime(const World& world) {
    std::vector<ValidationIssue> issues;
    std::unordered_set<std::string> registryIds;

    for (const auto& obj : world.objects) {
        if (obj.registryId.empty()) {
            issues.push_back({
                ValidationSeverity::Error,
                "missing_registry_id",
                "",
                "Object has empty registryId."
            });
        } else if (!registryIds.insert(obj.registryId).second) {
            issues.push_back({
                ValidationSeverity::Error,
                "duplicate_registry_id",
                obj.registryId,
                "Duplicate registryId detected: " + obj.registryId
            });
        }

        if (obj.displayName.empty()) {
            issues.push_back({
                ValidationSeverity::Warning,
                "missing_display_name",
                obj.registryId,
                "Object has empty displayName."
            });
        }

        if (!obj.linkTarget.empty() && !world.HasObject(obj.linkTarget)) {
            issues.push_back({
                ValidationSeverity::Error,
                "broken_link_target",
                obj.registryId,
                "linkTarget does not resolve to any object: " + obj.linkTarget
            });
        }

        if (const auto* spec = FindGameplayDescriptor(obj.scriptTag)) {
            if (obj.interaction != spec->preferredInteraction) {
                issues.push_back({
                    ValidationSeverity::Warning,
                    "interaction_mismatch",
                    obj.registryId,
                    "Interaction does not match preferred interaction for scriptTag '" +
                        std::string(spec->scriptTag) + "'."
                });
            }

            if (obj.category != spec->preferredCategory) {
                issues.push_back({
                    ValidationSeverity::Warning,
                    "category_mismatch",
                    obj.registryId,
                    "Category does not match preferred category for scriptTag '" +
                        std::string(spec->scriptTag) + "'."
                });
            }

            if (spec->requiresLinkTarget && obj.linkTarget.empty()) {
                issues.push_back({
                    ValidationSeverity::Error,
                    "missing_required_link_target",
                    obj.registryId,
                    "scriptTag '" + std::string(spec->scriptTag) + "' requires a linkTarget."
                });
            }
        } else if (!obj.scriptTag.empty() && ScriptTagRequiresLinkTarget(obj.scriptTag) && obj.linkTarget.empty()) {
            issues.push_back({
                ValidationSeverity::Error,
                "missing_required_link_target",
                obj.registryId,
                "scriptTag requires a linkTarget."
            });
        }

        if (obj.interaction == InteractionType::Transition && obj.linkTarget.empty()) {
            issues.push_back({
                ValidationSeverity::Warning,
                "transition_without_link_target",
                obj.registryId,
                "Transition object has no linkTarget."
            });
        }
    }

    return issues;
}

inline int CountValidationErrors(const std::vector<ValidationIssue>& issues) {
    int count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Error) {
            ++count;
        }
    }
    return count;
}

inline int CountValidationWarnings(const std::vector<ValidationIssue>& issues) {
    int count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Warning) {
            ++count;
        }
    }
    return count;
}

inline std::string BuildValidationSummary(const std::vector<ValidationIssue>& issues) {
    const int errors = CountValidationErrors(issues);
    const int warnings = CountValidationWarnings(issues);

    if (errors == 0 && warnings == 0) {
        return "World validation passed: no issues found.";
    }
    if (errors == 0) {
        return "World validation passed with warnings: " + std::to_string(warnings) + ".";
    }
    return "World validation failed: " + std::to_string(errors) +
           " errors, " + std::to_string(warnings) + " warnings.";
}

} // namespace bunker
Зачем это вторым

У тебя World уже хранит objects, есть HasObject, HasScriptTag, FindObjectByLinkTarget и другие lookup-методы, так что validation садится поверх существующей модели очень естественно. Это самый дешёвый способ резко уменьшить шанс сломать authored world.

Следующий шаг

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл на этом шаге

Добавить: include/AtomicPersistence.hpp
Путь нового файла: include/AtomicPersistence.hpp

Код для include/AtomicPersistence.hpp
#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <system_error>

#include "SessionProfiles.hpp"
#include "World.hpp"

namespace bunker {

namespace fs = std::filesystem;

struct SaveStatus {
    bool ok = false;
    std::string message;
};

inline SaveStatus AtomicWriteFile(
    const fs::path& finalPath,
    const std::function<bool(const fs::path&)>& writer) {

    std::error_code ec;

    if (!finalPath.parent_path().empty()) {
        fs::create_directories(finalPath.parent_path(), ec);
    }

    const fs::path tempPath = finalPath.string() + ".tmp";
    const fs::path backupPath = finalPath.string() + ".bak";

    fs::remove(tempPath, ec);

    if (!writer(tempPath)) {
        fs::remove(tempPath, ec);
        return {false, "Failed to write temp file: " + tempPath.string()};
    }

    if (fs::exists(backupPath, ec)) {
        fs::remove(backupPath, ec);
    }

    if (fs::exists(finalPath, ec)) {
        fs::rename(finalPath, backupPath, ec);
        if (ec) {
            fs::remove(tempPath, ec);
            return {false, "Failed to rotate existing file: " + finalPath.string()};
        }
    }

    fs::rename(tempPath, finalPath, ec);
    if (ec) {
        std::error_code restoreEc;
        if (fs::exists(backupPath, restoreEc)) {
            fs::rename(backupPath, finalPath, restoreEc);
        }
        fs::remove(tempPath, restoreEc);
        return {false, "Failed to promote temp file: " + finalPath.string()};
    }

    if (fs::exists(backupPath, ec)) {
        fs::remove(backupPath, ec);
    }

    return {true, {}};
}

inline SaveStatus SaveWorldAtomically(const World& world, const fs::path& worldPath) {
    return AtomicWriteFile(worldPath, [&](const fs::path& tempPath) {
        return world.Save(tempPath.string());
    });
}

inline SaveStatus SaveProfileAtomically(const SessionProfile& profile, const fs::path& profilePath) {
    return AtomicWriteFile(profilePath, [&](const fs::path& tempPath) {
        return SaveSessionProfile(profile, tempPath);
    });
}

} // namespace bunker
Зачем это третьим

SessionProfiles.hpp сейчас сам сохраняет и загружает профиль inline, а src/main.cpp вызывает сохранение профиля и мира напрямую. Поэтому безопасная обёртка поверх существующего формата — это лучший ход: минимум риска, максимум пользы.

Следующий шаг — правка существующего файла

Файл: Editor/src/Editor_Main.cpp
Путь: Editor/src/Editor_Main.cpp

Сначала добавь два include сразу после этой строки:

#include "../../include/World.hpp"
Вставить в Editor/src/Editor_Main.cpp
#include "../../include/WorldValidation.hpp"
#include "../../include/AtomicPersistence.hpp"

Теперь найди внутри LoadOrCreateEditorWorld(...) этот кусок:

world.GeneratePrototypeZone();
world.Save(path.string());
statusText = "Runtime world was missing. Generated a fresh prototype workspace at " + path.string();
return false;

И замени его на это:

Заменить в Editor/src/Editor_Main.cpp
world.GeneratePrototypeZone();

const auto saveResult = bunker::SaveWorldAtomically(world, path);
if (!saveResult.ok) {
    statusText = "Runtime world was missing. Failed to persist generated workspace: " + saveResult.message;
    return false;
}

statusText = "Runtime world was missing. Generated a fresh prototype workspace at " + path.string();
return false;
Ещё один кусок для Editor/src/Editor_Main.cpp

Добавь рядом с другими helper-функциями в anonymous namespace:

std::string BuildEditorValidationStatus(const bunker::World& world) {
    const auto issues = bunker::ValidateWorldForRuntime(world);
    return bunker::BuildValidationSummary(issues);
}

Сейчас у editor уже есть загрузка runtime-world через session profile и сохранение активного мира обратно в профиль, так что именно сюда логично вешать validation и atomic persistence первым делом.

Следующий шаг — правка существующего файла

Файл: src/main.cpp
Путь: src/main.cpp

Найди блок include’ов. Сейчас там есть AppPaths.hpp, GameRuntime.hpp, LanlineSession.hpp, LaunchSession.hpp, WorldEvents.hpp.

Сразу после:

#include "../include/WorldEvents.hpp"

добавь:

Вставить в src/main.cpp
#include "../include/AtomicPersistence.hpp"

Теперь замени первый блок, где создаётся дефолтный профиль, если загрузка не удалась.

Найди:

if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
    sessionProfile = bunker::MakeDefaultSessionProfile();
    bunker::SaveSessionProfile(sessionProfile, profilePath);
}

И замени на:

Заменить в src/main.cpp
if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
    sessionProfile = bunker::MakeDefaultSessionProfile();
    const auto initialProfileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);
    if (!initialProfileSave.ok) {
        std::fprintf(stderr, "Initial profile save failed: %s\n", initialProfileSave.message.c_str());
    }
}

Теперь найди F5 save-блок. Сейчас там прямой world.Save(...) и SaveSessionProfile(...).

Заменить на:

Заменить в src/main.cpp
const bool saveNow = glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS;

if (saveNow && !gameState.savePressed) {
    const auto worldSave = bunker::SaveWorldAtomically(world, worldPath);
    const auto profileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);

    if (worldSave.ok && profileSave.ok) {
        staticEraser.Save(sessionProfile.selectedWorld);
        gameState.lastEvent = "Field save committed.";
    } else {
        std::string failure = "Field save failed:";
        if (!worldSave.ok) {
            failure += " world=" + worldSave.message;
        }
        if (!profileSave.ok) {
            if (!worldSave.ok) failure += ";";
            failure += " profile=" + profileSave.message;
        }
        gameState.lastEvent = failure;
    }
}

И в самом конце файла замени финальное сохранение перед shutdown:

Сейчас там:

world.Save(worldPath.string());
bunker::SaveSessionProfile(sessionProfile, profilePath);
staticEraser.Save(sessionProfile.selectedWorld);
Заменить в src/main.cpp
const auto finalWorldSave = bunker::SaveWorldAtomically(world, worldPath);
const auto finalProfileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);

if (!finalWorldSave.ok) {
    std::fprintf(stderr, "Final world save failed: %s\n", finalWorldSave.message.c_str());
}
if (!finalProfileSave.ok) {
    std::fprintf(stderr, "Final profile save failed: %s\n", finalProfileSave.message.c_str());
}

if (finalWorldSave.ok && finalProfileSave.ok) {
    staticEraser.Save(sessionProfile.selectedWorld);
}
Следующий шаг

Сейчас ничего не удаляй. Это намеренно: сначала делаем три новых header-файла и две точечные интеграции, чтобы получить:

единый реестр scriptTag,
validation мира,
безопасное сохранение мира и профиля,
минимум риска сломать сборку.

уже вижу главное: это не “мы всё сломали кодом”, а сначала ломается toolchain/include path, а потом clang сыпет каскадом ложных ошибок по GLFW, ImGui и даже std::string. Сейчас быстро вытащу из лога опорные места и дам тебе порядок действий без воды.

Thought for 1m 13s

Да, картина уже ясна: главная проблема сейчас не в моих добавленных header’ах, а в том, что clang/Windsurf вообще не может нормально разобрать main.cpp из-за отсутствующего GLFW/glfw3.h. После этого идут каскадные ложные ошибки по glfwInit, GLFWwindow, ImGui, glViewport, GL_COLOR_BUFFER_BIT и даже странные ошибки в std::string/type_traits. В логе это видно прямо: сначала GLFW/glfw3.h file not found, потом Too many errors emitted, а дальше уже сыпется всё остальное.

Что это значит по сути:
сейчас не надо удалять мои новые файлы. Сначала нужно починить среду сборки / индексатор, иначе ты будешь видеть фальшивые ошибки даже в нормальном коде. То, что AtomicPersistence.hpp помечен как unused-includes, — это только предупреждение, не причина падения. Отдельно у тебя ещё есть две фоновые IDE-проблемы: невалидное значение в Windsurf/User/settings.json и поломанный .clang-format. Они не главные, но тоже шумят в диагностике.

Что делать прямо сейчас
1) Ничего не удаляй из проекта

include/GameplayDescriptorRegistry.hpp, include/WorldValidation.hpp, include/AtomicPersistence.hpp пока не удаляй.
Сначала убедись, что сам проект собирается через CMake, а не только ругается индексатор.

2) Почини first blocker: GLFW/include paths

У тебя первая ошибка — отсутствие GLFW/glfw3.h. Пока она не исправлена, все ошибки ниже почти бесполезно анализировать.

Сделай так в терминале из корня проекта:

cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --config Debug
Что смотреть
Если сборка тоже падает на GLFW/glfw3.h not found, значит проблема реально в CMakeLists.txt или в зависимости GLFW.
Если сборка проходит, а краснота остаётся только в Windsurf, значит проблема в clangd / compile database / IDE, а не в коде.
3) Если сборка падает на GLFW — править надо CMake, а не main.cpp

Судя по логам, main.cpp использует GLFW/OpenGL/ImGui symbols, но индексатор их не видит.

Тут есть только 3 нормальных варианта:

Вариант A — GLFW подтягивается как submodule / add_subdirectory

Тогда в CMakeLists.txt должно быть что-то вроде:

add_subdirectory(external/glfw)
target_link_libraries(BunkerGame PRIVATE glfw)
Вариант B — GLFW ставится в систему / vcpkg / package manager

Тогда должно быть что-то вроде:

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(BunkerGame PRIVATE glfw)
Вариант C — include path руками

Это самый слабый вариант, но как временный:

target_include_directories(BunkerGame PRIVATE path/to/glfw/include)
4) Если сборка проходит, но Windsurf всё ещё красный

Тогда проблема уже не в проекте, а в том, что clangd не видит compile flags.

Сделай одно из двух:

Способ 1

После генерации CMake проверь, что появился файл:

build/compile_commands.json

Потом либо:

настрой clangd/Windsurf на эту папку build,
либо положи копию compile_commands.json в корень проекта.
Способ 2

Если используешь CMake Tools, убедись, что открыт именно CMake-kit/target, а не просто папка как обычный текстовый workspace.

5) AtomicPersistence.hpp is not used directly — это не авария

Это обычный warning от include-cleaner. Он появился на строке 15 main.cpp.

Тут логика простая:

если ты уже заменил прямые вызовы world.Save(...) / SaveSessionProfile(...) на SaveWorldAtomically(...) и SaveProfileAtomically(...), то предупреждение, скорее всего, исчезнет после нормальной индексации;
если ты ещё не заменил эти места, тогда просто временно убери include из src/main.cpp:

Файл: src/main.cpp
Что удалить:

#include "../include/AtomicPersistence.hpp"

И добавь его обратно только когда вставишь вызовы атомарного сохранения.

6) .clang-format у тебя тоже сломан

В логе прямо есть:

Incorrect type. Expected "string".
Incorrect type. Expected ".clang-format config".

Это не ломает игру, но ломает tooling и шумит. Самый быстрый путь:

Временно

Переименуй файл:

.clang-format -> .clang-format.bak

Это самый дешёвый способ убрать шум, пока не починишь конфиг.

7) Windsurf/User/settings.json тоже содержит невалидное значение

Лог показывает, что в settings.json на 49 строке стоит значение, которого нет в списке допустимых расширений.

Это не причина отсутствия GLFW, но IDE из-за этого тоже может вести себя грязно.

Самый безопасный ход:

открой settings.json,
найди строку 49,
убери только то значение, которое не входит в список разрешённых.
Что делать в правильном порядке

Иди вот так:

Сначала
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --config Debug
Потом
если build падает — чини CMakeLists.txt / GLFW dependency
если build проходит — чини clangd/Windsurf indexing
временно убери битый .clang-format
подчисти settings.json
Самый важный вывод

Сейчас не код рефакторинга главный враг, а сломанный include/tooling слой.
Пока не исчезнет первая ошибка GLFW/glfw3.h file not found, все остальные сообщения по ImGui, OpenGL, GLFWwindow и даже часть ошибок по STL надо считать вторичными.

уже видно важную вещь: сам по себе он не выглядит “фатально сломанным”, но для IDE у тебя не хватает двух вещей — экспорт compile commands и более явного пути к GLFW include. Поэтому дам тебе не теорию, а готовую замену файла целиком.

Thought for 22s

Твой CMakeLists.txt в целом нормальный, и build через CMake вполне может собраться. Но для Windsurf/clangd у тебя сейчас слабое место в двух местах:

не экспортируется compile_commands.json, поэтому индексатор может не понимать реальные include paths;
путь к GLFW рассчитывается через target glfw, что обычно нормально для сборки, но clangd в IDE часто ведёт себя лучше, когда include путь указан явно.

То есть я бы сейчас ничего не удалял из исходников, а сначала починил именно CMake.

Шаг 1

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл

Заменить целиком файл:
Путь: CMakeLists.txt

Полный код для CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolSuite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

function(configure_bunker_target target_name)
    target_include_directories(${target_name} PRIVATE ${COMMON_INCLUDE_DIRS})
    target_link_libraries(${target_name} PRIVATE glfw OpenGL::GL)

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_executable(BunkerGame
    src/GameRuntime.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/main.cpp
    src/Progression.cpp
    src/Renderer.cpp
    src/SkillSystem.cpp
    src/StoryRoute.cpp
    src/WorldEvents.cpp
    src/World.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerGame)

add_executable(BunkerLauncher
    Launcher/src/Launcher_Main.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerLauncher)

add_executable(BunkerEditor
    Editor/src/Editor_Main.cpp
    src/World.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerEditor)

message(STATUS "Configured targets: BunkerGame, BunkerLauncher, BunkerEditor")
Что делать сразу после замены

В корне проекта выполни:

cmake -S . -B build
cmake --build build --config Debug

После этого проверь, появился ли файл:

build/compile_commands.json

Если появился — это уже большой шаг, потому что clangd сможет взять реальные compile flags вместо гадания.

Следующий шаг

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл

Правка существующего файла:
Путь: src/main.cpp

Сейчас по логам у тебя AtomicPersistence.hpp подсвечивается как unused include. Это не ошибка, но если ты ещё не вставил атомарные save-вызовы, лучше временно убрать include, чтобы не плодить шум. В логе это видно как warning, а не blocker.

Найди в src/main.cpp
#include "../include/AtomicPersistence.hpp"
Если ты ещё НЕ заменил save-вызовы — временно удали эту строку

То есть:

Удалить файл: ничего
Удалить строку в файле:
Путь: src/main.cpp

#include "../include/AtomicPersistence.hpp"
Следующий шаг

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл

Переименовать файл временно:
Старый путь: .clang-format
Новый путь: .clang-format.bak

Это потому что у тебя в диагностике .clang-format сам по себе сломан и даёт YAML-ошибки. Это не ломает build, но ломает tooling и мешает понять реальное состояние проекта. Это прямо видно в логе.

Следующий шаг

Ничего не удалять из проекта, пока не проверишь вот эти два результата:

проходит ли cmake --build build --config Debug;
исчезла ли ошибка 'GLFW/glfw3.h' file not found. Она у тебя первая и главная в списке. Пока она не ушла, все ошибки ниже по glfwInit, GLFWwindow, ImGui, glViewport и GL_COLOR_BUFFER_BIT почти наверняка вторичные.
Самое важное сейчас

Текущий CMakeLists.txt я бы оценил так:

для реальной сборки он был почти достаточный;
для Windsurf/clangd он был недостаточно явный.

Поэтому первый правильный ход — не ломать код дальше, а стабилизировать CMake и индексатор.


разберу в двух слоях — что уже есть сейчас по факту, и что именно нужно, чтобы проект выглядел как показываемый/почти релизный. Параллельно вытащу сильные места архитектуры и дам готовые вставляемые куски кода там, где вижу самые дорогие пробелы.

Я уже подтвердил базовую картину: это не просто набросок, а собранный CMake-проект с тремя отдельными приложениями (BunkerGame, BunkerLauncher, BunkerEditor) и своей дорожной картой. Дальше добираю сам код модулей и документацию, чтобы оценка “сколько сделано / сколько осталось” была не по названиям файлов, а по реальной логике и связанности систем.

Добираю последние опорные места: сейчас свожу архитектуру, прогрессию, запуск, редактор и сохранения в одну карту, чтобы дать не “впечатление”, а нормальный техразбор с приоритетами и вставляемыми решениями.

Thought for 14m 11s

Я реально прошёл не только README/ROADMAP, а именно структуру и ключевые исходники: CMakeLists.txt, src/main.cpp, src/GameRuntime.cpp, src/World.cpp, src/StoryRoute.cpp, src/SkillSystem.cpp, Launcher/src/Launcher_Main.cpp, Editor/src/Editor_Main.cpp, include/SessionProfiles.hpp, src/LaunchSession.cpp, src/LanlineServices.cpp. По факту это уже не “заготовка”, а большой прототипный набор из трёх приложений: игра, лаунчер и редактор, собранных одним CMake-проектом на C++20/OpenGL/GLFW/ImGui.

Главный вывод

Сейчас проект выглядит как сильный mid-stage prototype / systems-heavy vertical slice framework, а не как ранний скелет. Основа уже есть: отдельный launcher, отдельный editor, контроль запуска через launch ticket, загрузка/создание мира, сохранения, story-route, skill awakening, инфраструктурная прогрессия, авторинг через editor и слой Lanline services. Но до состояния “приятно показывать всем” и тем более до “релизно” ещё остаётся заметная работа по уплотнению архитектуры, валидации данных, полировке UX и убиранию монолитов.

Сколько сделано / сколько осталось

Это уже моя инженерная оценка, не математика по чекбоксам:

Как текущее ядро проекта: сделано примерно 68–75%.
Почему так: базовая архитектура, редактор, launcher-flow, world persistence и большая часть стартового gameplay-loop уже на месте.
До состояния “можно уверенно показывать как вертикальный срез / демо”: осталось примерно 20–30%.
То есть не нужно “делать игру с нуля”; нужно дожать целостность: стартовый сценарий, стабильность сейвов, UX лаунчера, editor-validation, чистка самых тяжёлых if-chain/monolith участков и подготовка демо-сборки.
До реально релизного состояния: осталось примерно 55–65%.
Потому что релиз — это уже не только systems/content, а ещё устойчивые миграции сохранений, тесты, CI, packaging, crash-safe persistence, data validation, контентная балансировка, UX-полировка, поддержка ошибок пользователя и ясная продуктовая граница, что такое Lanline: атмосферный shell или реальный multiplayer-service layer.
Что уже сделано хорошо
1) Архитектурно правильно разделены роли приложений

У тебя не одна “всё-в-одном” программа, а три отдельные: BunkerGame, BunkerLauncher, BunkerEditor. Это очень правильный уровень мышления для проекта такого типа: игрок идёт через launcher, контент редактируется editor’ом, runtime остаётся runtime’ом. Уже сам CMake это подтверждает.

2) Launcher — не декоративный, а реально встроен в продуктовый поток

main.cpp не просто пишет “лучше запускать из launcher”, а реально требует launch ticket и при ошибке показывает окно с отказом запуска. В LaunchSession.cpp ticket выдаётся, одноразово потребляется, удаляется после чтения и истекает через 60 секунд. Это очень хороший уровень продуманности для прототипа.

3) World/persistence уже не игрушечные

Игра грузит выбранный мир, а если мира нет — генерирует стартовую зону, сохраняет её и затем принудительно достраивает starter infrastructure. В World.cpp уже есть формат мира с версиями BWLD и BWL2, а roadmap отдельно фиксирует расширение до BWL2 и semantic authoring fields. Это значит, что ты уже думаешь не “про одну карту руками”, а про живущий формат данных.

4) Editor уже реально полезный, а не заглушка

По roadmap там уже добавлены search/filter, безопасное дублирование, prefab/library workflow, рабочий preview для выбора и постановки объектов, navigation в стиле Creation Kit, drag/move через preview, interaction overlay, специализированные gameplay-draft кнопки и descriptor-presets поверх scriptTag/linkTarget. Это очень серьёзный шаг: editor уже стал частью production loop, а не просто окном “поставить кубик”.

5) Прогрессия собрана вокруг систем, а не только around UI

Есть выделенный StoryRoute.cpp, а skill-awakening в SkillSystem.cpp завязан на реальные действия игрока: archive sync, foot kills, tank actions, stress survival, heavy carry drills, field service use. Это правильно: механики растут из поведения игрока, а не только из кнопки “получить перк”.

6) Lanline layer уже тематически встроен в игру

Это не просто фиктивная вкладка. Там есть friends/chat/voice/support flow, сохранение/восстановление состояния, relay messages, voice presence и явно прописано, что raw audio transport — future work. То есть слой уже честно обозначен как системный shell с persistence/state mirror, а не как законченная сеть.

Что сейчас тормозит проект
1) Монолиты слишком большие

Самый большой structural risk: несколько файлов стали “центрами мира”.
GameRuntime.cpp — 3835 lines / 181 KB, Editor_Main.cpp — 2041 lines, Launcher_Main.cpp — 1249 lines, SessionProfiles.hpp — 1267 lines. Это уже не просто “много кода”; это сигнал, что рост функционала начинает бить по поддерживаемости, локальному reasoning, тестируемости и скорости добавления контента.

2) Слишком многое держится на string-driven логике

Судя по runtime-структуре и editor-presets, важная часть поведения строится вокруг scriptTag и linkTarget. Для прототипа это быстро и мощно, но дальше это начинает быть хрупким: опечатка в строке = сломанная логика, битая связка, тихая ошибка авторинга. Сам подход хороший для authoring, но ему уже нужен слой реестров, валидаторов и нормальных dispatch tables.

3) Persistence уже есть, но ещё не “crash-safe”

main.cpp сохраняет мир и профиль на выходе, что хорошо, но сейчас следующий шаг — атомарные сохранения, versioned migrations и более жёсткая защита от partially-written state. Для проекта с несколькими видами данных это уже критично.

4) Lanline пока продуктово не до конца определён

Сейчас это сильный атмосферный и системный слой, но ещё не полноценный multiplayer/network runtime. И это нормально — просто это надо закрепить решением. Либо это остаётся “immersive asynchronous/session shell”, либо потом нужен отдельный сетевой контракт и другой объём работ.

5) По видимой структуре репозитория я не увидел явного слоя автотестов/CI

Для прототипа это нормально, но для showable/release-пути именно здесь потом будут самые дорогие баги: broken world refs, regression в save format, невалидные prefab data, случайно сломанный launcher-flow.

Что делать дальше: сначала до “можно показывать”, потом до “релизно”
Этап A — быстро довести до сильного демо
Заморозить scope демо.
Не пытаться сейчас полировать весь мир. Выбери один гарантированный маршрут: bunker wake-up → Pip-Pad → tank link → first tower sync → первый recovery loop → один яркий service/Lanline payoff. Это уже достаточно для демо.
Сделать editor-validation перед сохранением мира.
Нужны проверки: duplicate registryId, битые linkTarget, suspicious scriptTag, объекты без имени, битые transition-style anchors.
Перевести 2–3 самых жирных string-if цепочки в таблицы.
Не весь runtime сразу. Сначала:
DescribeTerminalSync, затем interaction-dispatch для authored terminals, затем editor descriptor mapping.
Сделать crash-safer save flow.
Хотя бы atomic temp-write для world/profile.
Подготовить “golden demo world + reset profile”.
Чтобы можно было в 1 клик вернуться в корректное демо-состояние.
Подчистить launcher UX.
Чётко показывать: selected world, current objective summary, selected character, network/session status, кнопка launch, кнопка open editor.
Этап B — довести до инженерно зрелого проекта
Разбить GameRuntime.cpp на подсистемы.
Минимальный распил:
RuntimeInteraction.cpp
RuntimeRecovery.cpp
RuntimeTank.cpp
RuntimeLanline.cpp
RuntimeUI.cpp
RuntimeStorySync.cpp
Разгрузить SessionProfiles.hpp.
Сейчас там, похоже, сосредоточены model + normalization + save/load + operational graph. Я бы разделил:
SessionProfileTypes.hpp
SessionProfileNormalize.cpp
SessionProfilePersistence.cpp
WorldFieldStateRules.cpp
Сделать migration/versioning для profile save.
Сейчас уже есть живой evolving проект. Значит, старые сейвы будут ломаться без нормальной версии и миграций.
Собрать world authoring contract.
То есть задокументировать:
какие scriptTag допустимы,
какие требуют linkTarget,
какие ждут конкретный InteractionType,
какие дают прогрессию,
какие editor presets им соответствуют.
Добавить automated checks.
Хотя бы:
загрузить/сохранить мир и сравнить семантику,
валидация всех world files,
smoke-test launcher ticket flow,
save/load profile roundtrip.
Уточнить статус Lanline.
Либо это fiction-driven services layer, либо начинаешь проектировать реальные transport/session rules отдельно.
Самые сильные части проекта

Сильнее всего у тебя сейчас выглядят:

Product thinking: launcher/game/editor разделены правильно.
Data evolution thinking: мир уже versioned (BWLD → BWL2).
Authoring direction: editor уже заточен под gameplay authoring, а не только под геометрию.
Systemic progression: story, skills, tank/recovery/service loops растут вместе.
Launch discipline: launcher ticket — это зрелая мысль.
Готовые куски кода, которые я бы вставил прямо сейчас
1) Убрать длинный if/else из DescribeTerminalSync в GameRuntime.cpp

Это маленький, но очень правильный шаг: поведение становится легче расширять и меньше шансов сломать цепочку.

#include <array>
#include <string_view>

namespace bunker {
namespace {

const char* TryGetTerminalSyncText(std::string_view tag) {
    static constexpr std::array<std::pair<std::string_view, const char*>, 22> kSyncTexts{{
        {"tower_sync", "Tower sync complete. Regional grid reach expanded."},
        {"power_pylon", "Pylon registry mirrored. Grid restoration route updated."},
        {"drone_station", "Drone station ledger mirrored. Sweep routes registered to Pip-Pad."},
        {"rail_freight", "Rail freight depot records mirrored. Heavy spur logistics registered."},
        {"orbital_uplink", "Orbital uplink records mirrored. Long-range scan queue registered."},
        {"rail_fortress", "Rail Fortress patrol package mirrored. Spur security doctrine updated."},
        {"recovery_fabricator", "Recovery fabricator recipes mirrored. Shelter supply pipeline updated."},
        {"industrial_gate", "Industrial gate overrides mirrored. Inner spur access route logged."},
        {"industrial_survey", "Industrial survey notes mirrored. Inner spur reconnaissance queue updated."},
        {"industrial_outpost", "Inner spur outpost records mirrored. Forward recovery foothold logged."},
        {"assembly_cell", "Assembly cell notes mirrored. Local recovery production registered."},
        {"foundry_line", "Foundry line records mirrored. Heavy fabrication route registered."},
        {"reactor_yard", "Reactor yard records mirrored. Heavy energy recovery route registered."},
        {"capacitor_bank", "Capacitor bank records mirrored. Buffered grid discharge route registered."},
        {"relay_substation", "Relay substation notes mirrored. Backbone return flow updated."},
        {"service_bay", "Service bay notes mirrored. BT-72 support route updated."},
        {"water_reclaimer", "Water reclaimer notes mirrored. Frontier recovery reserves updated."},
        {"lanline_service_hub", "Lanline service hub mirrored. Shelter 17 service catalog now resolves through authored relay infrastructure."},
        {"fey_ring", "Fey Ring route mirrored. Transit windows registered to the relay map."},
        {"medical_support", "Medical support node mirrored. Field treatment requests now route through authored relay anchors."},
        {"tank_service", "Tank service anchor mirrored. BT-72 maintenance route updated."},
        {"specialist_cryo", "Cryo specialist registry mirrored. Shelter staffing ledger updated."}
    }};

    for (const auto& [key, text] : kSyncTexts) {
        if (key == tag) {
            return text;
        }
    }
    return nullptr;
}

std::string DescribeTerminalSync(const MapObject& object) {
    if (const char* text = TryGetTerminalSyncText(object.scriptTag)) {
        return text;
    }
    if (object.scriptTag == "echo_trace") {
        return "Residual echo trace mirrored to Pip-Pad.";
    }
    if (object.scriptTag == "workshop_service") {
        return "Workshop terminal mirrored. BT-72 service route and repair notes updated.";
    }
    return object.scriptTag.empty()
        ? "Terminal sync complete. Additional archive fragments copied to Pip-Pad."
        : "Terminal sync complete: " + object.scriptTag;
}

} // namespace
} // namespace bunker
2) Добавить валидацию мира перед сохранением

Файл: include/WorldValidation.hpp

#pragma once

#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "World.hpp"

namespace bunker {

enum class ValidationSeverity {
    Warning,
    Error
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Warning;
    std::string code;
    std::string objectId;
    std::string message;
};

inline bool ScriptTagRequiresLinkTarget(std::string_view scriptTag) {
    return scriptTag == "remote_link";
}

inline std::vector<ValidationIssue> ValidateWorld(const World& world) {
    std::vector<ValidationIssue> issues;
    std::unordered_set<std::string> registryIds;

    for (const auto& obj : world.objects) {
        if (obj.registryId.empty()) {
            issues.push_back({
                ValidationSeverity::Error,
                "missing_registry_id",
                "",
                "Object has empty registryId."
            });
        } else if (!registryIds.insert(obj.registryId).second) {
            issues.push_back({
                ValidationSeverity::Error,
                "duplicate_registry_id",
                obj.registryId,
                "Duplicate registryId detected: " + obj.registryId
            });
        }

        if (obj.displayName.empty()) {
            issues.push_back({
                ValidationSeverity::Warning,
                "missing_display_name",
                obj.registryId,
                "Object has empty displayName."
            });
        }

        if (ScriptTagRequiresLinkTarget(obj.scriptTag) && obj.linkTarget.empty()) {
            issues.push_back({
                ValidationSeverity::Error,
                "missing_link_target",
                obj.registryId,
                "scriptTag '" + obj.scriptTag + "' requires linkTarget."
            });
        }

        if (!obj.linkTarget.empty() && !world.HasObject(obj.linkTarget)) {
            issues.push_back({
                ValidationSeverity::Error,
                "broken_link_target",
                obj.registryId,
                "linkTarget '" + obj.linkTarget + "' does not resolve to any registryId."
            });
        }

        if (obj.interaction == InteractionType::Transition && obj.linkTarget.empty()) {
            issues.push_back({
                ValidationSeverity::Warning,
                "transition_without_target",
                obj.registryId,
                "Transition object has no linkTarget."
            });
        }
    }

    return issues;
}

inline int CountValidationErrors(const std::vector<ValidationIssue>& issues) {
    int count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Error) {
            ++count;
        }
    }
    return count;
}

} // namespace bunker

Минимальная интеграция в editor перед save:

#include "../include/WorldValidation.hpp"

// перед world.Save(...)
const auto issues = bunker::ValidateWorld(world);
const int errorCount = bunker::CountValidationErrors(issues);

if (errorCount > 0) {
    saveStatusText = "World validation failed: " + std::to_string(errorCount) + " blocking errors.";
} else if (world.Save(currentWorldPath.string())) {
    saveStatusText = "World saved successfully.";
} else {
    saveStatusText = "World save failed.";
}
3) Сделать атомарные сохранения мира и профиля

Файл: include/AtomicPersistence.hpp

#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <system_error>

#include "SessionProfiles.hpp"
#include "World.hpp"

namespace bunker {

namespace fs = std::filesystem;

struct SaveStatus {
    bool ok = false;
    std::string message;
};

inline SaveStatus AtomicWriteFile(
    const fs::path& finalPath,
    const std::function<bool(const fs::path&)>& writer) {

    std::error_code ec;
    if (!finalPath.parent_path().empty()) {
        fs::create_directories(finalPath.parent_path(), ec);
    }

    const fs::path tempPath = finalPath.string() + ".tmp";
    const fs::path backupPath = finalPath.string() + ".bak";

    fs::remove(tempPath, ec);

    if (!writer(tempPath)) {
        fs::remove(tempPath, ec);
        return {false, "Failed to write temp file: " + tempPath.string()};
    }

    if (fs::exists(backupPath, ec)) {
        fs::remove(backupPath, ec);
    }

    if (fs::exists(finalPath, ec)) {
        fs::rename(finalPath, backupPath, ec);
        if (ec) {
            fs::remove(tempPath, ec);
            return {false, "Failed to rotate old file: " + finalPath.string()};
        }
    }

    fs::rename(tempPath, finalPath, ec);
    if (ec) {
        std::error_code restoreEc;
        if (fs::exists(backupPath, restoreEc)) {
            fs::rename(backupPath, finalPath, restoreEc);
        }
        fs::remove(tempPath, restoreEc);
        return {false, "Failed to promote temp file: " + finalPath.string()};
    }

    if (fs::exists(backupPath, ec)) {
        fs::remove(backupPath, ec);
    }

    return {true, {}};
}

inline SaveStatus SaveWorldAtomically(const World& world, const fs::path& worldPath) {
    return AtomicWriteFile(worldPath, [&](const fs::path& tempPath) {
        return world.Save(tempPath.string());
    });
}

inline SaveStatus SaveProfileAtomically(const SessionProfile& profile, const fs::path& profilePath) {
    return AtomicWriteFile(profilePath, [&](const fs::path& tempPath) {
        return SaveSessionProfile(profile, tempPath);
    });
}

} // namespace bunker

Замена в main.cpp вместо прямого save на выходе:

const auto worldSave = bunker::SaveWorldAtomically(world, worldPath);
const auto profileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);

if (!worldSave.ok) {
    std::fprintf(stderr, "World save failed: %s\n", worldSave.message.c_str());
}
if (!profileSave.ok) {
    std::fprintf(stderr, "Profile save failed: %s\n", profileSave.message.c_str());
}

staticEraser.Save(sessionProfile.selectedWorld);

Если сжать всё в одну фразу: у тебя уже не “идея игры”, а реально сложившийся systems-prototype с правильным разделением launcher/game/editor; главный следующий выигрыш даст не добавление ещё 20 фич, а стабилизация данных, распил монолитов и жёсткая валидация authoring-пайплайна.

Могу следующим сообщением сделать ещё более жёсткий разбор по конкретным файлам: что именно вынести из GameRuntime.cpp, SessionProfiles.hpp, Editor_Main.cpp, Launcher_Main.cpp и в каком порядке лучше рефакторить.

ок сделай это и напиши мне в самом конце что за 20 фич

Сейчас зайду уже в конкретные исходники по слоям и соберу именно карту рефакторинга: что выносить из GameRuntime.cpp, SessionProfiles.hpp, Editor_Main.cpp, Launcher_Main.cpp, в каком порядке и с каким эффектом. В конце отдельно перечислю, какие именно “20 фич” я имел в виду, чтобы это не осталось абстракцией.

Уже видно узкое место точнее: roadmap богатый, но самый большой риск не в “нехватке идей”, а в том, что несколько файлов стали контейнерами слишком многих обязанностей сразу. Сейчас доберу конкретику из этих файлов и сведу это в порядок рефакторинга по шагам, не общими словами.

Thought for 7m 19s

Сделал уже именно то, что ты просил: не “общие впечатления”, а разложил по конкретным файлам, что из них выносить, в каком порядке и зачем.

Короткий вывод

Сейчас проект уже выглядит как сильный systems-prototype, а не как сырой набросок: Launcher v1 у тебя почти закрыт, Editor v1 уже production-useful, LAN foundation в работе, Data Cards/лор начаты частично, а большой слой “заводы/энергия/инфраструктура” сам roadmap честно помечает как по сути ещё не начатый до полноценного состояния. При этом самые тяжёлые места — это уже не отсутствие идей, а монолиты: GameRuntime.cpp 4114 строк, SessionProfiles.hpp 1267 строк, Editor_Main.cpp 2041 строка, Launcher_Main.cpp 1249 строк.

Что я бы рефакторил по файлам
1) src/GameRuntime.cpp — главный кандидат на распил

Сейчас это уже слишком большой “центр вселенной”. Внутри у тебя сидят сразу: tank utility/helpers, inventory/economy helpers, objective/recovery logic, grid/logistics/industrial updates, Lanline diagnostics, PIP-PAD UI и куски interaction-логики. Особенно видно, что файл уже содержит длинные цепочки по scriptTag, пачку Update... функций для recovery/industry и огромный UI-блок DrawPipPad(...).

Что в нём хорошо:

логика recovery-loop уже реально системная, а не декоративная;
progression увязана с инфраструктурой;
Story/PIP-PAD/NET/SERV ощущаются как части одного мира, а не случайные вкладки.
Это подтверждается и roadmap: rail freight, orbital uplink, Rail Fortress, Recovery Fabricator, industrial gate/survey/outpost и дальнейшие inner-spur узлы уже встроены в текущий контур.

Что в нём плохо:

слишком много обязанностей в одном translation unit;
string-driven dispatch по scriptTag уже становится дорогим в поддержке;
UI и gameplay update loops перемешаны сильнее, чем нужно.

Как я бы делил в первую очередь:

RuntimeInteraction.cpp
DescribeTerminalSync
contextual interaction helpers
terminal/service anchor logic
всё, что завязано на scriptTag и linkTarget
RuntimeEconomy.cpp
AddInventoryItem
HasInventoryItem
ConsumeInventoryItem
CurrentInventoryWeight
TryConsumeFieldRation
RuntimeRecovery.cpp
UpdateScavengerTeams
UpdateCaravanRoute
UpdateDroneStations
UpdateTradeNetwork
UpdateRailFreight
UpdateOrbitalUplink
UpdateRailFortress
UpdateRecoveryFabricator
UpdateRecoveryMilestones
RuntimeIndustry.cpp
UpdateIndustrialSurvey
UpdateIndustrialOutpost
UpdateAssemblyCell
UpdateFoundryLine
UpdateReactorYard
UpdateCapacitorBank
UpdateRelaySubstation
UpdateServiceBay
UpdateWaterReclaimer
RuntimePipPad.cpp
вместо одного DrawPipPad сделать:
DrawPipPadStatTab
DrawPipPadInventoryTab
DrawPipPadDataTab
DrawPipPadMapTab
DrawPipPadQuestTab
DrawPipPadNetTab
DrawPipPadServicesTab
RuntimeLanlineUi.cpp
slot-state helpers
diagnostics cache
runtime NET tab render

Приоритет: сначала вытащить DrawPipPad, потом recovery/industry updates, потом interaction dispatch.
Это даст самый большой выигрыш по читаемости без слома геймплея.

2) include/SessionProfiles.hpp — второй по важности файл

Сейчас это уже не просто header со структурами. Там сидят:

сами модели (SessionProfile, WorldFieldState, CharacterProfile, tank и т.д.),
world-state lookup/merge,
normalize logic,
operational helper functions,
SaveSessionProfile,
LoadSessionProfile.

Это место уже стало слишком “умным” для header-файла.

Что в нём хорошо:

WorldFieldState у тебя уже реально богатый и отражает живущий recovery-state;
NormalizeSessionProfile — правильная мысль;
per-world state и selected-world flow сделаны концептуально верно.

Что в нём рискованно:

save/load живут inline в header;
свой key=value формат без явной версии и без безопасных parse wrappers;
любое изменение профиля раздувает recompilation и повышает риск тихих ошибок.

Как делить:

SessionProfileTypes.hpp
только struct/enum
SessionProfileQuery.hpp
FindWorldFieldState
HasAwakenedSpecialistRole
HasAssignedSpecialistRole
operational boolean helpers
SessionProfileNormalize.cpp
NormalizeWorldFieldState
NormalizeSessionProfile
merge helpers
SessionProfilePersistence.cpp
SaveSessionProfile
LoadSessionProfile
safe parse helpers
version header
позже: SessionProfileMigrations.cpp
когда начнёшь реально ломать старые сейвы

Самое важное здесь: не оставлять persistence внутри header и добавить versioned save-format.

3) Editor/src/Editor_Main.cpp — очень сильный, но уже перегруженный

Тут хорошая новость: editor у тебя уже не игрушка. По roadmap он умеет runtime world handoff, spawn setup, preview mode, search/filter, duplicate with new registry id, prefab/library workflow и authoring presets.

По коду это тоже видно:

есть DrawWorldPreview(...),
есть prefab load/save,
есть descriptor preset helpers,
есть export/runtime panel,
есть import assistant,
есть warnings по fey_ring, lanline_service_hub, tank_service.
То есть логика уже полезная. Но почти всё это живёт внутри одного main().

Что я бы выносил:

EditorPresetDrafts.cpp
PrepareSpecializedDraft
preset creation
default registry id generation
EditorDescriptorPresets.cpp
все Apply...DescriptorPreset(...)
это отдельный authoring-layer
EditorPrefabLibrary.cpp
LoadPrefabLibrary
SavePrefabLibrary
capture/place/remove prefab
EditorPreview.cpp
DrawWorldPreview
viewport state
selection/drag/focus logic
EditorRuntimeExport.cpp
runtime alignment panel
export/save-as/set-active flow
import assistant manifest
EditorPanels.cpp
DrawAssetPalettePanel
DrawWorldAuthoringPanel
DrawExportPanel
DrawImportAssistantPanel

Что editor’у сейчас не хватает сильнее всего:

полноценной world validation перед export,
единого descriptor registry,
undo/redo,
проверки битых ссылок,
отчёта “что именно сломано в authored world”.
4) Launcher/src/Launcher_Main.cpp — уже близко к хорошему виду

По roadmap launcher почти завершён: обязательная точка входа, выбор мира, session setup, startup summary, launch gate, LAN-first flow.

По коду сильные стороны:

Play BunkerGame и Open BunkerEditor встроены правильно;
launch идёт через ticket;
joinability/ready-seat/match-start logic уже собрана;
session browser уже не пустой список, а реальный lobby/status view.

Что плохо:

много lobby helper-логики дублируется между launcher/runtime;
main() всё ещё держит auth mock, refresh, LAN session browser, launch orchestration и services panel одновременно.

Как делить:

LauncherLanlineLobby.cpp
slot-state helpers
joinability
ready logic
accept/clear/reserve peer logic
LauncherLaunchFlow.cpp
prepare profile
issue ticket
save roster state
launch sibling executable
LauncherPanels.cpp
DrawAccessConsole
DrawOperationsDeck
session summary / known sessions / selected diagnostics
LanlineLobbyLogic.hpp
общий shared helper для launcher + runtime + main.cpp
5) src/main.cpp, src/LaunchSession.cpp, src/StoryRoute.cpp, src/SkillSystem.cpp

Тут разный приоритет.

LaunchSession.cpp я бы почти не трогал: он компактный и по делу. Сейчас там уже есть issue/consume ticket, удаление ticket после чтения и истечение по времени через 60 секунд. Это как раз удачная и завершённая часть.

StoryRoute.cpp я бы сохранил как отдельный канон прогрессии. Он небольшой и очень полезный: route уже явно ведёт от cryo/Pip-Pad/archive/tank/relay до rail/orbital/fabricator/industrial chain и water reclaimer. Его не надо обратно растворять в runtime.

SkillSystem.cpp тоже не монстр и мыслится правильно: awakening растёт из archive sync, foot kills, tank actions, stress survival, heavy carry, field service. Это хороший файл; позже его можно сделать data-driven, но не сейчас.

main.cpp же стоит подтянуть:

заменить прямые save-вызовы на atomic save;
вынести bootstrap/loading world/profile в GameBootstrap.cpp;
оставить в main.cpp только gate + init + run + shutdown.
В каком порядке это делать

Вот порядок, который я бы реально выбрал:

Вынести общий LanlineLobbyLogic.hpp и удалить дубли из launcher/runtime/main.
Вынести DrawPipPad по вкладкам.
Вынести recovery/industry update functions из GameRuntime.cpp.
Сделать GameplayDescriptorRegistry.hpp.
Сделать WorldValidation.hpp и включить её в editor export.
Перенести persistence из SessionProfiles.hpp в .cpp.
Добавить versioned profile save/load.
Разрезать Editor_Main.cpp на panels + preview + presets + export.
Разрезать Launcher_Main.cpp на panels + lanline + launch flow.
Добавить 3 smoke-checks: world roundtrip, profile roundtrip, launch ticket flow.
Готовые куски кода, которые реально стоит вставить
1) Общая логика Lanline lobby, чтобы убрать дубли
#pragma once

#include <algorithm>
#include <string>
#include "LanlineSession.hpp"

namespace bunker {

inline bool IsLanlineAwaitingSlot(const LanlinePlayerEntry& entry) {
    return entry.role == "Awaiting";
}

inline bool IsLanlinePendingSlot(const LanlinePlayerEntry& entry) {
    return entry.role == "Pending Client";
}

inline bool IsLanlineReservedSlot(const LanlinePlayerEntry& entry) {
    return entry.role == "Reserved Client";
}

inline bool IsLanlineAcceptedSlot(const LanlinePlayerEntry& entry) {
    return entry.role == "Client";
}

inline bool IsLanlineReadyEligibleSlot(const LanlinePlayerEntry& entry) {
    return entry.role == "Host" || entry.role == "Client" || entry.role == "Local Operator";
}

inline const char* LanlineSlotStateLabel(const LanlinePlayerEntry& entry) {
    if (IsLanlineAwaitingSlot(entry)) return "Open";
    if (IsLanlinePendingSlot(entry)) return "Pending";
    if (IsLanlineReservedSlot(entry)) return "Reserved";
    if (IsLanlineAcceptedSlot(entry)) return "Accepted";
    return entry.online ? "Active" : "Reserved";
}

inline const char* LanlineReadyLabel(const LanlinePlayerEntry& entry) {
    if (!IsLanlineReadyEligibleSlot(entry)) return "-";
    return entry.ready ? "Ready" : "Not Ready";
}

inline int FindFirstAwaitingSlotIndex(const LanlineSessionState& state) {
    for (int i = 0; i < static_cast<int>(state.players.size()); ++i) {
        if (IsLanlineAwaitingSlot(state.players[static_cast<std::size_t>(i)])) {
            return i;
        }
    }
    return -1;
}

inline int MaxLanlineSessionSlots(const LanlineSessionState& session) {
    if (session.mode != "LAN Host") return 0;
    return std::max(1, static_cast<int>(session.players.size()));
}

inline int OccupiedLanlineSessionSlots(const LanlineSessionState& session) {
    int occupied = 0;
    for (const auto& player : session.players) {
        if (session.mode == "LAN Host" && IsLanlineAwaitingSlot(player)) continue;
        if (player.role != "Awaiting") ++occupied;
    }
    return occupied;
}

inline int AvailableLanlineSessionSlots(const LanlineSessionState& session) {
    return std::max(0, MaxLanlineSessionSlots(session) - OccupiedLanlineSessionSlots(session));
}

inline int PendingLanlineSessionSlots(const LanlineSessionState& session) {
    int count = 0;
    for (const auto& player : session.players) {
        if (IsLanlinePendingSlot(player)) ++count;
    }
    return count;
}

inline int ReservedLanlineSessionSlots(const LanlineSessionState& session) {
    int count = 0;
    for (const auto& player : session.players) {
        if (IsLanlineReservedSlot(player)) ++count;
    }
    return count;
}

inline int AcceptedLanlineSessionSlots(const LanlineSessionState& session) {
    int count = 0;
    for (const auto& player : session.players) {
        if (IsLanlineAcceptedSlot(player)) ++count;
    }
    return count;
}

inline int ReadyLanlineSessionSlots(const LanlineSessionState& session) {
    int count = 0;
    for (const auto& player : session.players) {
        if (IsLanlineReadyEligibleSlot(player) && player.ready) ++count;
    }
    return count;
}

inline bool IsJoinableLanlineSession(const LanlineSessionState& session) {
    if (session.mode != "LAN Host") return false;
    if (!session.connectedPeer.empty()) return false;
    return session.lifecycleStage == "HostLobbyOpen" ||
           session.lifecycleStage == "HostJoinPending" ||
           session.lifecycleStage == "HostRuntimeActive";
}

inline bool IsLanlineMatchStartReady(const LanlineSessionState& session) {
    if (session.mode != "LAN Host") return false;
    if (ReservedLanlineSessionSlots(session) > 0 || PendingLanlineSessionSlots(session) > 0) return false;

    int readyEligible = 0;
    for (const auto& player : session.players) {
        if (!IsLanlineReadyEligibleSlot(player)) continue;
        ++readyEligible;
        if (!player.ready) return false;
    }
    return readyEligible >= 2;
}

} // namespace bunker
2) Единый registry для scriptTag, чтобы editor и runtime говорили на одном языке
#pragma once

#include <array>
#include <optional>
#include <string_view>
#include "MapObject.hpp"

namespace bunker {

struct GameplayDescriptorSpec {
    std::string_view scriptTag;
    std::string_view label;
    InteractionType preferredInteraction;
    ObjectCategory preferredCategory;
    bool requiresLinkTarget = false;
};

inline const GameplayDescriptorSpec* FindGameplayDescriptor(std::string_view scriptTag) {
    static constexpr std::array<GameplayDescriptorSpec, 12> kSpecs{{
        {"tower_sync", "Tower Sync", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"power_pylon", "Power Pylon", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"drone_station", "Drone Station", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"rail_freight", "Rail Depot", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"orbital_uplink", "Orbital Uplink", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"rail_fortress", "Rail Fortress", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"recovery_fabricator", "Recovery Fabricator", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"industrial_gate", "Industrial Gate", InteractionType::Transition, ObjectCategory::Landmark, false},
        {"fey_ring", "Fey Ring", InteractionType::Transition, ObjectCategory::Landmark, true},
        {"lanline_service_hub", "Lanline Service Hub", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"tank_service", "Tank Service", InteractionType::Workshop, ObjectCategory::Hangar, false},
        {"remote_link", "Remote Link", InteractionType::Terminal, ObjectCategory::Terminal, true},
    }};

    for (const auto& spec : kSpecs) {
        if (spec.scriptTag == scriptTag) return &spec;
    }
    return nullptr;
}

} // namespace bunker
3) Валидация authored world перед export
#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "GameplayDescriptorRegistry.hpp"
#include "World.hpp"

namespace bunker {

enum class ValidationSeverity {
    Warning,
    Error
};

struct ValidationIssue {
    ValidationSeverity severity;
    std::string objectId;
    std::string message;
};

inline std::vector<ValidationIssue> ValidateWorldForRuntime(const World& world) {
    std::vector<ValidationIssue> issues;
    std::unordered_set<std::string> ids;

    for (const auto& obj : world.objects) {
        if (obj.registryId.empty()) {
            issues.push_back({ValidationSeverity::Error, "", "Object has empty registryId."});
        } else if (!ids.insert(obj.registryId).second) {
            issues.push_back({ValidationSeverity::Error, obj.registryId, "Duplicate registryId."});
        }

        if (obj.displayName.empty()) {
            issues.push_back({ValidationSeverity::Warning, obj.registryId, "Object has empty displayName."});
        }

        if (const auto* spec = FindGameplayDescriptor(obj.scriptTag)) {
            if (obj.interaction != spec->preferredInteraction) {
                issues.push_back({
                    ValidationSeverity::Warning,
                    obj.registryId,
                    "Interaction does not match preferred interaction for scriptTag " + std::string(spec->scriptTag)
                });
            }

            if (obj.category != spec->preferredCategory) {
                issues.push_back({
                    ValidationSeverity::Warning,
                    obj.registryId,
                    "Category does not match preferred category for scriptTag " + std::string(spec->scriptTag)
                });
            }

            if (spec->requiresLinkTarget && obj.linkTarget.empty()) {
                issues.push_back({
                    ValidationSeverity::Error,
                    obj.registryId,
                    "scriptTag requires linkTarget: " + std::string(spec->scriptTag)
                });
            }
        }

        if (!obj.linkTarget.empty() && !world.HasObject(obj.linkTarget)) {
            issues.push_back({
                ValidationSeverity::Error,
                obj.registryId,
                "Broken linkTarget: " + obj.linkTarget
            });
        }
    }

    return issues;
}

inline int CountErrors(const std::vector<ValidationIssue>& issues) {
    int errors = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Error) ++errors;
    }
    return errors;
}

} // namespace bunker

Минимальная интеграция в editor export:

const auto issues = bunker::ValidateWorldForRuntime(editorWorld);
if (bunker::CountErrors(issues) > 0) {
    statusText = "Export blocked: authored world has validation errors.";
} else {
    // editorWorld.Save(...)
}
Что я бы НЕ трогал первым

Не трогал бы первым:

StoryRoute.cpp
SkillSystem.cpp
LaunchSession.cpp

Они не идеальны, но у них хорошая цена/польза: они уже маленькие, осмысленные и не главные источники боли. Главное сейчас — это монолиты и data-validation.

Что я имел в виду под “не добавлять ещё 20 фич”

Вот именно какие 20 фич я бы сейчас не добавлял, пока не сделаны распил, валидация и safe persistence:

полноценный крафтовый UI с очередями производства
новые типы транспорта кроме текущего контура
отдельная мини-игра для реконструкции data fragments
расширенный cosmetics marketplace
полноценный voice transport вместо текущего presence-shell
PvP-логика
procedural world partition generation
система NPC-диалогов с ветвлениями
фракции и репутация
отдельная deep inventory grid-система
модульная броня для всех машин
хардкорная economy simulation по всем ресурсам
advanced pathfinding simulation для convoy/drone/rail
погодные аномалии нового типа сверх Acid Rain / Ether Fog
полноценный quest editor до завершения world validation
achievements/challenges layer
custom skins pipeline
matchmaking/online account layer поверх LAN-first контура
replay/photo mode
полноценный mod SDK

Если хочешь, следующим сообщением я сделаю совсем жёсткий production-план на 2 этапа: “за 3 дня привести код в порядок” и “за 2 недели довести до showable demo”.

Если хочешь, следующим сообщением я сделаю совсем жёсткий production-план на 2 этапа: “за 3 дня привести код в порядок” и “за 2 недели довести до showable demo”.
{но видишь ли в чем проблема ии это ChatGPT 5.4 Plus а  так же Codex+Windserf и у меня лимиты очень сильные а еще впн там на 600Гб он сейчас бесплатный но на 5 дней}
Thought for 19s

Да, это сильно меняет стратегию.

Под такие ограничения тебе сейчас нужен не “идеальный большой рефакторинг”, а план с максимальной отдачей на 1 запрос, 1 патч, 1 файл. Иначе лимиты Codex/Windsurf/ChatGPT сгорят раньше, чем код станет заметно лучше.

Что это значит на практике

Тебе сейчас нельзя:

скармливать ИИ весь репозиторий целиком,
просить “полностью перепиши архитектуру”,
запускать длинные многошаговые сессии с контекстом на десятки файлов,
тратить дни на красивые, но не критичные фичи.

Тебе сейчас нужно:

брать только самые дорогие узкие места,
работать по 1 файлу за раз,
просить у ИИ только diff / patch / конкретный код,
сначала дожать “демо-стабильность”, потом уже красоту.
Лучшая стратегия под твои лимиты
Роль инструментов
ChatGPT

Используй как:

архитектора,
ревьюера,
генератор точных планов,
генератор маленьких вставляемых кусков кода,
редактор prompt'ов для Codex/Windsurf.
Codex / Windsurf

Используй как:

исполнителя точечных изменений,
генератора патчей по одному файлу,
помощника по рутинному распилу.
Локально сам

Делай:

grep / поиск по проекту,
сборку,
запуск,
фиксацию ошибок,
проверку что патч правда скомпилировался.

То есть не ИИ “ведёт проект”, а ты ведёшь проект, а ИИ закрывает узкие ручные задачи.

Что делать за твои 5 дней
Цель

Не “сделать всё”, а получить:

более чистый код,
меньше риска сломать сейвы,
стабильный запуск через launcher,
editor с базовой валидацией,
showable demo build.
День 1 — только критический фундамент

Сделай 3 вещи и больше ничего.

1. Валидация мира перед сохранением/export

Это даст огромную отдачу сразу:

ловит битые registryId,
duplicate ids,
пустые displayName,
broken linkTarget,
несовпадения scriptTag / interaction.

Это одна из самых выгодных задач в проекте.

2. Атомарные сохранения

Чтобы не убивать мир/профиль кривым выходом.
Сначала world save, потом profile save через temp file + rename.

3. Registry для scriptTag

Не весь runtime переписывать.
Только сделать единый реестр соответствий:

scriptTag
label
preferred interaction
preferred category
requiresLinkTarget

Это сразу улучшит:

editor,
validation,
будущий runtime dispatch.

Итог дня 1: проект уже станет заметно взрослее, даже без новых фич.

День 2 — распил самого больного места без войны со всем проектом

Не трогай весь GameRuntime.cpp.
Только 2 самых жирных участка:

1. Вынеси DrawPipPad(...)

Сделай отдельные функции по вкладкам:

stats
inventory
data
map
quests
net
services
2. Вынеси DescribeTerminalSync и похожие string-driven куски

Даже если не весь interaction dispatch, хотя бы самые длинные if/else.

Почему именно это:
это даст сильный выигрыш по читаемости и почти не требует ломать core-loop.

День 3 — разгрузка SessionProfiles.hpp

Не надо пока делать идеальную систему миграций.

Сделай минимум:

Вынести из header:
SaveSessionProfile
LoadSessionProfile
parse helpers
normalize helpers

Оставь в .hpp:

только типы,
несколько маленьких inline query-функций, если без них совсем неудобно.

Почему это выгодно:
профиль — это одна из осей проекта. Когда persistence сидит в giant header, всё дорожает:

правки,
чтение,
компиляция,
риск ошибок.
День 4 — demo path

Тут не рефакторинг, а продукт.

Сделай один гарантированный маршрут показа:

запуск из launcher
выбор мира
вход в игру
стартовая зона
Pip-Pad
tank/service или tower sync
один recovery payoff
один Lanline/service payoff
Параллельно
сделай “demo world”
сделай “reset profile”
убери всё, что может случайно сломать показ

То есть не “весь проект теперь идеален”, а “демо проходит без стыда”.

День 5 — только polish и сборка
Сделай:
smoke test сценарий на бумаге,
release-like сборку,
быстрый список известных багов,
короткий README “как запускать демо”.
Не делай:
новые подсистемы,
большие переписывания,
новые экраны ради красоты.
Что не делать при лимитах

Вот прям запрещённые действия под твою ситуацию:

Не проси ИИ читать весь репозиторий заново каждый раз.
Не кидай по 10 файлов в один запрос.
Не проси “перепиши архитектуру полностью”.
Не проси “сделай production-ready всё”.
Не трать лимиты на стилистику и косметику раньше валидации/сейвов/демо.
Не давай ИИ самому выбирать, что менять во всём проекте.
Не запускай длинные циклы “ещё улучши / ещё улучши / ещё улучши”.
Как экономить лимиты максимально жёстко
Правило 1: один запрос = один файл

Пример хорошего режима:

сначала сам находишь нужный участок,
копируешь только его,
просишь только точечный патч.

Не “проанализируй весь GameRuntime.cpp”, а:

вот функция DrawPipPad, разбей её на 7 функций без изменения поведения, верни только готовый код/patch

Правило 2: проси diff, а не объяснение

Самый экономный формат:

Прочитай только этот фрагмент.
Не объясняй долго.
Верни unified diff.
Не меняй имена публичных типов.
Не добавляй новых зависимостей.

Правило 3: сначала локальный поиск, потом ИИ

Ищи сам:

rg "DrawPipPad"
rg "SaveSessionProfile"
rg "scriptTag"
rg "linkTarget"
rg "Lanline"

ИИ не должен тратить токены на поиск того, что тебе rg покажет за секунду.

Правило 4: не загружай контекст повторно

Один раз сформулируй для себя краткое описание проекта, и дальше просто вставляй его в коротком виде:

C++20 game project with Game/Launcher/Editor.
Need small safe refactor, no behavior changes, minimal dependencies.

Этого обычно хватает.

Готовые экономные prompt-шаблоны
1. Для распила функции
Read only the code I pasted below.
Task: split this function into smaller static helpers without changing behavior.
Constraints:
- keep C++20
- no new libraries
- preserve public interfaces
- keep naming style close to existing code
- return only the final code or unified diff
2. Для валидации мира
Read only this header and the World/Object structures below.
Task: write a minimal WorldValidation.hpp that checks:
- empty registryId
- duplicate registryId
- broken linkTarget
- empty displayName
- scriptTag rules for requiresLinkTarget
Constraints:
- no exceptions
- C++20
- header-only is acceptable
- return paste-ready code only
3. Для atomic save
Read only the save/load related code below.
Task: add atomic save using temp file + rename.
Constraints:
- preserve existing save format
- keep std::filesystem
- no external libs
- return minimal patch only
4. Для SessionProfiles
Read only this header.
Task: move persistence and normalization code out of the header into cpp files.
Do not redesign the entire system.
Keep public structs stable.
Return:
1) proposed file split
2) exact code for the new .hpp/.cpp pieces
3) minimal include changes
Приоритет задач по отдаче на лимит

Если совсем мало запросов, делай строго в таком порядке:

WorldValidation.hpp
AtomicPersistence.hpp
GameplayDescriptorRegistry.hpp
распил DrawPipPad
вынос SaveSessionProfile/LoadSessionProfile
общий LanlineLobbyLogic.hpp

Это самые выгодные шесть задач.

Что с VPN и трафиком

Если VPN реально ограничен по времени и трафику, то избегай:

повторных полных клонов,
скачивания крупных артефактов/зависимостей,
лишних IDE/cloud sync операций,
тяжелых AI-индексаций всего репозитория.

Практично делать так:

один локальный клон,
по возможности shallow fetch,
не гонять лишние бинарники,
не открывать облачные ассистенты на весь workspace, если можно скормить только файл.
Самая правильная мысль под твою ситуацию

Тебе сейчас нужен не “максимально умный процесс”, а максимально дешёвый по лимитам процесс.

То есть цель ближайших дней:

не сделать проект идеальным,
а сделать его заметно надёжнее и показабельнее за минимум запросов.
Мой честный совет по тактике

Работай так:

Пачка 1
world validation
atomic save
Пачка 2
descriptor registry
DescribeTerminalSync
Пачка 3
DrawPipPad split
Пачка 4
SessionProfiles persistence split
Пачка 5
demo world + reset profile + smoke test

Это лучший баланс между:

пользой,
риском,
числом AI-запросов,
временем,
шансом успеть до конца этих 5 дней.

Следующим сообщением я могу сделать тебе ультра-практичный список “ровно какие 5 запросов отправить в Codex/Windsurf по очереди”, уже в готовом виде, чтобы ты просто копировал их один за другим.

ты не забыл что я тоже поработать могу ?
скажи какой фаил удалить и какой добавить ( к обоим пропиши путь до них )
а потом код который вставить в него сразу под названием и путем к файлу

после пиши дальше следующий

Сейчас проверю точные структуры файлов, чтобы не дать тебе “красивый, но несовместимый” код. Потом дам в формате: что удалить / что добавить / полный путь / готовый код, и после каждого блока — следующий шаг.

Stopped thinking
Quick answer
извини что прервал я обновил репозиторий

Ок, это важно — тогда я не буду опираться на старую картину. Сначала быстро пересниму текущую структуру репозитория и уже после этого дам точные шаги в формате: какой файл удалить, какой добавить, полный путь и готовый код.

Thought for 2m 52s

Не забыл. Как раз поэтому сейчас дам тебе формат “сделай руками и сразу получишь выигрыш”.

После обновления репа безопаснее идти только добавочными шагами, ничего не удаляя на первом проходе: у тебя всё ещё один большой Editor/src/Editor_Main.cpp на 2041 строку, один Launcher/src/Launcher_Main.cpp, SessionProfiles.hpp остаётся большим header’ом на 1267 строк с inline SaveSessionProfile/LoadSessionProfile, а src/main.cpp всё ещё сохраняет мир и профиль напрямую в нескольких местах. Ещё в репе появился Next_split, но он сам помечен как разбор старого Next.md, а не как новый source of truth.

Шаг 1

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл на этом шаге

Добавить: include/GameplayDescriptorRegistry.hpp
Путь нового файла: include/GameplayDescriptorRegistry.hpp

Код для include/GameplayDescriptorRegistry.hpp
#pragma once

#include <array>
#include <string_view>

#include "MapObject.hpp"

namespace bunker {

struct GameplayDescriptorSpec {
    std::string_view scriptTag;
    std::string_view label;
    InteractionType preferredInteraction;
    ObjectCategory preferredCategory;
    bool requiresLinkTarget = false;
};

inline const GameplayDescriptorSpec* FindGameplayDescriptor(std::string_view scriptTag) {
    static constexpr std::array<GameplayDescriptorSpec, 15> kSpecs{{
        {"tower_sync", "Tower Sync", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"power_pylon", "Power Pylon", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"drone_station", "Drone Station", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"rail_freight", "Rail Freight", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"orbital_uplink", "Orbital Uplink", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"rail_fortress", "Rail Fortress", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"recovery_fabricator", "Recovery Fabricator", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"industrial_gate", "Industrial Gate", InteractionType::Transition, ObjectCategory::Landmark, false},
        {"industrial_survey", "Industrial Survey", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"industrial_outpost", "Industrial Outpost", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"assembly_cell", "Assembly Cell", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"reactor_yard", "Reactor Yard", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"lanline_service_hub", "Lanline Service Hub", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"tank_service", "Tank Service", InteractionType::Workshop, ObjectCategory::Hangar, false},
        {"fey_ring", "Fey Ring", InteractionType::Transition, ObjectCategory::Landmark, true},
    }};

    for (const auto& spec : kSpecs) {
        if (spec.scriptTag == scriptTag) {
            return &spec;
        }
    }
    return nullptr;
}

inline bool ScriptTagRequiresLinkTarget(std::string_view scriptTag) {
    if (const auto* spec = FindGameplayDescriptor(scriptTag)) {
        return spec->requiresLinkTarget;
    }
    return false;
}

} // namespace bunker
Зачем это первым

Это создаёт один общий справочник для scriptTag, чтобы потом одинаково использовать его в editor, validation и runtime, а не держать смысл тегов размазанным по коду. У тебя уже есть MapObject с scriptTag и linkTarget, так что это естественное расширение текущей модели.

Следующий шаг

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл на этом шаге

Добавить: include/WorldValidation.hpp
Путь нового файла: include/WorldValidation.hpp

Код для include/WorldValidation.hpp
#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "GameplayDescriptorRegistry.hpp"
#include "World.hpp"

namespace bunker {

enum class ValidationSeverity {
    Warning,
    Error
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Warning;
    std::string code;
    std::string objectId;
    std::string message;
};

inline std::vector<ValidationIssue> ValidateWorldForRuntime(const World& world) {
    std::vector<ValidationIssue> issues;
    std::unordered_set<std::string> registryIds;

    for (const auto& obj : world.objects) {
        if (obj.registryId.empty()) {
            issues.push_back({
                ValidationSeverity::Error,
                "missing_registry_id",
                "",
                "Object has empty registryId."
            });
        } else if (!registryIds.insert(obj.registryId).second) {
            issues.push_back({
                ValidationSeverity::Error,
                "duplicate_registry_id",
                obj.registryId,
                "Duplicate registryId detected: " + obj.registryId
            });
        }

        if (obj.displayName.empty()) {
            issues.push_back({
                ValidationSeverity::Warning,
                "missing_display_name",
                obj.registryId,
                "Object has empty displayName."
            });
        }

        if (!obj.linkTarget.empty() && !world.HasObject(obj.linkTarget)) {
            issues.push_back({
                ValidationSeverity::Error,
                "broken_link_target",
                obj.registryId,
                "linkTarget does not resolve to any object: " + obj.linkTarget
            });
        }

        if (const auto* spec = FindGameplayDescriptor(obj.scriptTag)) {
            if (obj.interaction != spec->preferredInteraction) {
                issues.push_back({
                    ValidationSeverity::Warning,
                    "interaction_mismatch",
                    obj.registryId,
                    "Interaction does not match preferred interaction for scriptTag '" +
                        std::string(spec->scriptTag) + "'."
                });
            }

            if (obj.category != spec->preferredCategory) {
                issues.push_back({
                    ValidationSeverity::Warning,
                    "category_mismatch",
                    obj.registryId,
                    "Category does not match preferred category for scriptTag '" +
                        std::string(spec->scriptTag) + "'."
                });
            }

            if (spec->requiresLinkTarget && obj.linkTarget.empty()) {
                issues.push_back({
                    ValidationSeverity::Error,
                    "missing_required_link_target",
                    obj.registryId,
                    "scriptTag '" + std::string(spec->scriptTag) + "' requires a linkTarget."
                });
            }
        } else if (!obj.scriptTag.empty() && ScriptTagRequiresLinkTarget(obj.scriptTag) && obj.linkTarget.empty()) {
            issues.push_back({
                ValidationSeverity::Error,
                "missing_required_link_target",
                obj.registryId,
                "scriptTag requires a linkTarget."
            });
        }

        if (obj.interaction == InteractionType::Transition && obj.linkTarget.empty()) {
            issues.push_back({
                ValidationSeverity::Warning,
                "transition_without_link_target",
                obj.registryId,
                "Transition object has no linkTarget."
            });
        }
    }

    return issues;
}

inline int CountValidationErrors(const std::vector<ValidationIssue>& issues) {
    int count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Error) {
            ++count;
        }
    }
    return count;
}

inline int CountValidationWarnings(const std::vector<ValidationIssue>& issues) {
    int count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Warning) {
            ++count;
        }
    }
    return count;
}

inline std::string BuildValidationSummary(const std::vector<ValidationIssue>& issues) {
    const int errors = CountValidationErrors(issues);
    const int warnings = CountValidationWarnings(issues);

    if (errors == 0 && warnings == 0) {
        return "World validation passed: no issues found.";
    }
    if (errors == 0) {
        return "World validation passed with warnings: " + std::to_string(warnings) + ".";
    }
    return "World validation failed: " + std::to_string(errors) +
           " errors, " + std::to_string(warnings) + " warnings.";
}

} // namespace bunker
Зачем это вторым

У тебя World уже хранит objects, есть HasObject, HasScriptTag, FindObjectByLinkTarget и другие lookup-методы, так что validation садится поверх существующей модели очень естественно. Это самый дешёвый способ резко уменьшить шанс сломать authored world.

Следующий шаг

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл на этом шаге

Добавить: include/AtomicPersistence.hpp
Путь нового файла: include/AtomicPersistence.hpp

Код для include/AtomicPersistence.hpp
#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <system_error>

#include "SessionProfiles.hpp"
#include "World.hpp"

namespace bunker {

namespace fs = std::filesystem;

struct SaveStatus {
    bool ok = false;
    std::string message;
};

inline SaveStatus AtomicWriteFile(
    const fs::path& finalPath,
    const std::function<bool(const fs::path&)>& writer) {

    std::error_code ec;

    if (!finalPath.parent_path().empty()) {
        fs::create_directories(finalPath.parent_path(), ec);
    }

    const fs::path tempPath = finalPath.string() + ".tmp";
    const fs::path backupPath = finalPath.string() + ".bak";

    fs::remove(tempPath, ec);

    if (!writer(tempPath)) {
        fs::remove(tempPath, ec);
        return {false, "Failed to write temp file: " + tempPath.string()};
    }

    if (fs::exists(backupPath, ec)) {
        fs::remove(backupPath, ec);
    }

    if (fs::exists(finalPath, ec)) {
        fs::rename(finalPath, backupPath, ec);
        if (ec) {
            fs::remove(tempPath, ec);
            return {false, "Failed to rotate existing file: " + finalPath.string()};
        }
    }

    fs::rename(tempPath, finalPath, ec);
    if (ec) {
        std::error_code restoreEc;
        if (fs::exists(backupPath, restoreEc)) {
            fs::rename(backupPath, finalPath, restoreEc);
        }
        fs::remove(tempPath, restoreEc);
        return {false, "Failed to promote temp file: " + finalPath.string()};
    }

    if (fs::exists(backupPath, ec)) {
        fs::remove(backupPath, ec);
    }

    return {true, {}};
}

inline SaveStatus SaveWorldAtomically(const World& world, const fs::path& worldPath) {
    return AtomicWriteFile(worldPath, [&](const fs::path& tempPath) {
        return world.Save(tempPath.string());
    });
}

inline SaveStatus SaveProfileAtomically(const SessionProfile& profile, const fs::path& profilePath) {
    return AtomicWriteFile(profilePath, [&](const fs::path& tempPath) {
        return SaveSessionProfile(profile, tempPath);
    });
}

} // namespace bunker
Зачем это третьим

SessionProfiles.hpp сейчас сам сохраняет и загружает профиль inline, а src/main.cpp вызывает сохранение профиля и мира напрямую. Поэтому безопасная обёртка поверх существующего формата — это лучший ход: минимум риска, максимум пользы.

Следующий шаг — правка существующего файла

Файл: Editor/src/Editor_Main.cpp
Путь: Editor/src/Editor_Main.cpp

Сначала добавь два include сразу после этой строки:

#include "../../include/World.hpp"
Вставить в Editor/src/Editor_Main.cpp
#include "../../include/WorldValidation.hpp"
#include "../../include/AtomicPersistence.hpp"

Теперь найди внутри LoadOrCreateEditorWorld(...) этот кусок:

world.GeneratePrototypeZone();
world.Save(path.string());
statusText = "Runtime world was missing. Generated a fresh prototype workspace at " + path.string();
return false;

И замени его на это:

Заменить в Editor/src/Editor_Main.cpp
world.GeneratePrototypeZone();

const auto saveResult = bunker::SaveWorldAtomically(world, path);
if (!saveResult.ok) {
    statusText = "Runtime world was missing. Failed to persist generated workspace: " + saveResult.message;
    return false;
}

statusText = "Runtime world was missing. Generated a fresh prototype workspace at " + path.string();
return false;
Ещё один кусок для Editor/src/Editor_Main.cpp

Добавь рядом с другими helper-функциями в anonymous namespace:

std::string BuildEditorValidationStatus(const bunker::World& world) {
    const auto issues = bunker::ValidateWorldForRuntime(world);
    return bunker::BuildValidationSummary(issues);
}

Сейчас у editor уже есть загрузка runtime-world через session profile и сохранение активного мира обратно в профиль, так что именно сюда логично вешать validation и atomic persistence первым делом.

Следующий шаг — правка существующего файла

Файл: src/main.cpp
Путь: src/main.cpp

Найди блок include’ов. Сейчас там есть AppPaths.hpp, GameRuntime.hpp, LanlineSession.hpp, LaunchSession.hpp, WorldEvents.hpp.

Сразу после:

#include "../include/WorldEvents.hpp"

добавь:

Вставить в src/main.cpp
#include "../include/AtomicPersistence.hpp"

Теперь замени первый блок, где создаётся дефолтный профиль, если загрузка не удалась.

Найди:

if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
    sessionProfile = bunker::MakeDefaultSessionProfile();
    bunker::SaveSessionProfile(sessionProfile, profilePath);
}

И замени на:

Заменить в src/main.cpp
if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
    sessionProfile = bunker::MakeDefaultSessionProfile();
    const auto initialProfileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);
    if (!initialProfileSave.ok) {
        std::fprintf(stderr, "Initial profile save failed: %s\n", initialProfileSave.message.c_str());
    }
}

Теперь найди F5 save-блок. Сейчас там прямой world.Save(...) и SaveSessionProfile(...).

Заменить на:

Заменить в src/main.cpp
const bool saveNow = glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS;

if (saveNow && !gameState.savePressed) {
    const auto worldSave = bunker::SaveWorldAtomically(world, worldPath);
    const auto profileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);

    if (worldSave.ok && profileSave.ok) {
        staticEraser.Save(sessionProfile.selectedWorld);
        gameState.lastEvent = "Field save committed.";
    } else {
        std::string failure = "Field save failed:";
        if (!worldSave.ok) {
            failure += " world=" + worldSave.message;
        }
        if (!profileSave.ok) {
            if (!worldSave.ok) failure += ";";
            failure += " profile=" + profileSave.message;
        }
        gameState.lastEvent = failure;
    }
}

И в самом конце файла замени финальное сохранение перед shutdown:

Сейчас там:

world.Save(worldPath.string());
bunker::SaveSessionProfile(sessionProfile, profilePath);
staticEraser.Save(sessionProfile.selectedWorld);
Заменить в src/main.cpp
const auto finalWorldSave = bunker::SaveWorldAtomically(world, worldPath);
const auto finalProfileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);

if (!finalWorldSave.ok) {
    std::fprintf(stderr, "Final world save failed: %s\n", finalWorldSave.message.c_str());
}
if (!finalProfileSave.ok) {
    std::fprintf(stderr, "Final profile save failed: %s\n", finalProfileSave.message.c_str());
}

if (finalWorldSave.ok && finalProfileSave.ok) {
    staticEraser.Save(sessionProfile.selectedWorld);
}
Следующий шаг

Сейчас ничего не удаляй. Это намеренно: сначала делаем три новых header-файла и две точечные интеграции, чтобы получить:

единый реестр scriptTag,
validation мира,
безопасное сохранение мира и профиля,
минимум риска сломать сборку.

Удалять что-то я начну рекомендовать только после этого первого компилируемого прохода, потому что текущая структура всё ещё сильно монолитная, и раннее удаление здесь опаснее, чем полезно.

Следующим сообщением я дам тебе вторую пачку в таком же формате: какие конкретно куски в Editor_Main.cpp и GameRuntime.cpp заменить, чтобы подключить validation уже в UI и убрать первый длинный string-dispatch.

Pasted text(1).txt
Document

Я уже вижу главное: это не “мы всё сломали кодом”, а сначала ломается toolchain/include path, а потом clang сыпет каскадом ложных ошибок по GLFW, ImGui и даже std::string. Сейчас быстро вытащу из лога опорные места и дам тебе порядок действий без воды.

Thought for 1m 13s

Да, картина уже ясна: главная проблема сейчас не в моих добавленных header’ах, а в том, что clang/Windsurf вообще не может нормально разобрать main.cpp из-за отсутствующего GLFW/glfw3.h. После этого идут каскадные ложные ошибки по glfwInit, GLFWwindow, ImGui, glViewport, GL_COLOR_BUFFER_BIT и даже странные ошибки в std::string/type_traits. В логе это видно прямо: сначала GLFW/glfw3.h file not found, потом Too many errors emitted, а дальше уже сыпется всё остальное.

Что это значит по сути:
сейчас не надо удалять мои новые файлы. Сначала нужно починить среду сборки / индексатор, иначе ты будешь видеть фальшивые ошибки даже в нормальном коде. То, что AtomicPersistence.hpp помечен как unused-includes, — это только предупреждение, не причина падения. Отдельно у тебя ещё есть две фоновые IDE-проблемы: невалидное значение в Windsurf/User/settings.json и поломанный .clang-format. Они не главные, но тоже шумят в диагностике.

Что делать прямо сейчас
1) Ничего не удаляй из проекта

include/GameplayDescriptorRegistry.hpp, include/WorldValidation.hpp, include/AtomicPersistence.hpp пока не удаляй.
Сначала убедись, что сам проект собирается через CMake, а не только ругается индексатор.

2) Почини first blocker: GLFW/include paths

У тебя первая ошибка — отсутствие GLFW/glfw3.h. Пока она не исправлена, все ошибки ниже почти бесполезно анализировать.

Сделай так в терминале из корня проекта:

cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --config Debug
Что смотреть
Если сборка тоже падает на GLFW/glfw3.h not found, значит проблема реально в CMakeLists.txt или в зависимости GLFW.
Если сборка проходит, а краснота остаётся только в Windsurf, значит проблема в clangd / compile database / IDE, а не в коде.
3) Если сборка падает на GLFW — править надо CMake, а не main.cpp

Судя по логам, main.cpp использует GLFW/OpenGL/ImGui symbols, но индексатор их не видит.

Тут есть только 3 нормальных варианта:

Вариант A — GLFW подтягивается как submodule / add_subdirectory

Тогда в CMakeLists.txt должно быть что-то вроде:

add_subdirectory(external/glfw)
target_link_libraries(BunkerGame PRIVATE glfw)
Вариант B — GLFW ставится в систему / vcpkg / package manager

Тогда должно быть что-то вроде:

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(BunkerGame PRIVATE glfw)
Вариант C — include path руками

Это самый слабый вариант, но как временный:

target_include_directories(BunkerGame PRIVATE path/to/glfw/include)
4) Если сборка проходит, но Windsurf всё ещё красный

Тогда проблема уже не в проекте, а в том, что clangd не видит compile flags.

Сделай одно из двух:

Способ 1

После генерации CMake проверь, что появился файл:

build/compile_commands.json

Потом либо:

настрой clangd/Windsurf на эту папку build,
либо положи копию compile_commands.json в корень проекта.
Способ 2

Если используешь CMake Tools, убедись, что открыт именно CMake-kit/target, а не просто папка как обычный текстовый workspace.

5) AtomicPersistence.hpp is not used directly — это не авария

Это обычный warning от include-cleaner. Он появился на строке 15 main.cpp.

Тут логика простая:

если ты уже заменил прямые вызовы world.Save(...) / SaveSessionProfile(...) на SaveWorldAtomically(...) и SaveProfileAtomically(...), то предупреждение, скорее всего, исчезнет после нормальной индексации;
если ты ещё не заменил эти места, тогда просто временно убери include из src/main.cpp:

Файл: src/main.cpp
Что удалить:

#include "../include/AtomicPersistence.hpp"

И добавь его обратно только когда вставишь вызовы атомарного сохранения.

6) .clang-format у тебя тоже сломан

В логе прямо есть:

Incorrect type. Expected "string".
Incorrect type. Expected ".clang-format config".

Это не ломает игру, но ломает tooling и шумит. Самый быстрый путь:

Временно

Переименуй файл:

.clang-format -> .clang-format.bak

Это самый дешёвый способ убрать шум, пока не починишь конфиг.

7) Windsurf/User/settings.json тоже содержит невалидное значение

Лог показывает, что в settings.json на 49 строке стоит значение, которого нет в списке допустимых расширений.

Это не причина отсутствия GLFW, но IDE из-за этого тоже может вести себя грязно.

Самый безопасный ход:

открой settings.json,
найди строку 49,
убери только то значение, которое не входит в список разрешённых.
Что делать в правильном порядке

Иди вот так:

Сначала
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --config Debug
Потом
если build падает — чини CMakeLists.txt / GLFW dependency
если build проходит — чини clangd/Windsurf indexing
временно убери битый .clang-format
подчисти settings.json
Самый важный вывод

Сейчас не код рефакторинга главный враг, а сломанный include/tooling слой.
Пока не исчезнет первая ошибка GLFW/glfw3.h file not found, все остальные сообщения по ImGui, OpenGL, GLFWwindow и даже часть ошибок по STL надо считать вторичными.

Пришли текущий CMakeLists.txt, и я следующим сообщением скажу ровно какой кусок в нём заменить, без теории.

cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolSuite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

function(configure_bunker_target target_name)
    target_include_directories(${target_name} PRIVATE ${COMMON_INCLUDE_DIRS})
    target_link_libraries(${target_name} PRIVATE glfw OpenGL::GL)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_executable(BunkerGame
    src/GameRuntime.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/main.cpp
    src/Progression.cpp
    src/Renderer.cpp
    src/SkillSystem.cpp
    src/StoryRoute.cpp
    src/WorldEvents.cpp
    src/World.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerGame)

add_executable(BunkerLauncher
    Launcher/src/Launcher_Main.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerLauncher)

add_executable(BunkerEditor
    Editor/src/Editor_Main.cpp
    src/World.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerEditor)

message(STATUS "Configured targets: BunkerGame, BunkerLauncher, BunkerEditor")

С CMake уже видно важную вещь: сам по себе он не выглядит “фатально сломанным”, но для IDE у тебя не хватает двух вещей — экспорт compile commands и более явного пути к GLFW include. Поэтому дам тебе не теорию, а готовую замену файла целиком.

Thought for 22s

Твой CMakeLists.txt в целом нормальный, и build через CMake вполне может собраться. Но для Windsurf/clangd у тебя сейчас слабое место в двух местах:

не экспортируется compile_commands.json, поэтому индексатор может не понимать реальные include paths;
путь к GLFW рассчитывается через target glfw, что обычно нормально для сборки, но clangd в IDE часто ведёт себя лучше, когда include путь указан явно.

То есть я бы сейчас ничего не удалял из исходников, а сначала починил именно CMake.

Шаг 1

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл

Заменить целиком файл:
Путь: CMakeLists.txt

Полный код для CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolSuite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

function(configure_bunker_target target_name)
    target_include_directories(${target_name} PRIVATE ${COMMON_INCLUDE_DIRS})
    target_link_libraries(${target_name} PRIVATE glfw OpenGL::GL)

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_executable(BunkerGame
    src/GameRuntime.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/main.cpp
    src/Progression.cpp
    src/Renderer.cpp
    src/SkillSystem.cpp
    src/StoryRoute.cpp
    src/WorldEvents.cpp
    src/World.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerGame)

add_executable(BunkerLauncher
    Launcher/src/Launcher_Main.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerLauncher)

add_executable(BunkerEditor
    Editor/src/Editor_Main.cpp
    src/World.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerEditor)

message(STATUS "Configured targets: BunkerGame, BunkerLauncher, BunkerEditor")
Что делать сразу после замены

В корне проекта выполни:

cmake -S . -B build
cmake --build build --config Debug

После этого проверь, появился ли файл:

build/compile_commands.json

Если появился — это уже большой шаг, потому что clangd сможет взять реальные compile flags вместо гадания.

Следующий шаг

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл

Правка существующего файла:
Путь: src/main.cpp

Сейчас по логам у тебя AtomicPersistence.hpp подсвечивается как unused include. Это не ошибка, но если ты ещё не вставил атомарные save-вызовы, лучше временно убрать include, чтобы не плодить шум. В логе это видно как warning, а не blocker.

Найди в src/main.cpp
#include "../include/AtomicPersistence.hpp"
Если ты ещё НЕ заменил save-вызовы — временно удали эту строку

То есть:

Удалить файл: ничего
Удалить строку в файле:
Путь: src/main.cpp

#include "../include/AtomicPersistence.hpp"
Следующий шаг

Удалить: ничего
Путь удаляемого файла: не нужно удалять файл

Переименовать файл временно:
Старый путь: .clang-format
Новый путь: .clang-format.bak

Это потому что у тебя в диагностике .clang-format сам по себе сломан и даёт YAML-ошибки. Это не ломает build, но ломает tooling и мешает понять реальное состояние проекта. Это прямо видно в логе.

Следующий шаг

Ничего не удалять из проекта, пока не проверишь вот эти два результата:

проходит ли cmake --build build --config Debug;
исчезла ли ошибка 'GLFW/glfw3.h' file not found. Она у тебя первая и главная в списке. Пока она не ушла, все ошибки ниже по glfwInit, GLFWwindow, ImGui, glViewport и GL_COLOR_BUFFER_BIT почти наверняка вторичные.
Самое важное сейчас

Текущий CMakeLists.txt я бы оценил так:

для реальной сборки он был почти достаточный;
для Windsurf/clangd он был недостаточно явный.

Поэтому первый правильный ход — не ломать код дальше, а стабилизировать CMake и индексатор.

Что делать после этого

Когда проверишь build после замены CMakeLists.txt, пришли мне:

либо новый вывод cmake --build build --config Debug,
либо напиши одной строкой:
build прошёл
или
build упал на ...

После этого я дам тебе следующий конкретный шаг в том же формате: какой файл не трогать, какой изменить, какой код вставить.

ok я обновил репозиторий буквально минуту назд

Тогда старые точечные замены я считаю условно устаревшими и сверяюсь с текущим состоянием репозитория. Сначала быстро проверю, что изменилось в CMakeLists.txt и ключевых файлах, и уже потом дам следующий шаг без риска увести тебя в старую версию.

Thought for 21s

Да, переснял текущее состояние репозитория.

Сейчас на GitHub у тебя всё ещё старая версия CMakeLists.txt: 73 строки, есть add_subdirectory(external/glfw), но нет set(CMAKE_EXPORT_COMPILE_COMMANDS ON), нет set(GLFW_INSTALL OFF CACHE BOOL "" FORCE), и в COMMON_INCLUDE_DIRS нет ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include. То есть главный следующий шаг не меняется: сначала надо дочинить CMake, а уже потом снова смотреть на красноту в Windsurf.

Делай так:

Файл: CMakeLists.txt
Действие: заменить целиком
Путь: CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolSuite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

function(configure_bunker_target target_name)
    target_include_directories(${target_name} PRIVATE ${COMMON_INCLUDE_DIRS})
    target_link_libraries(${target_name} PRIVATE glfw OpenGL::GL)

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_executable(BunkerGame
    src/GameRuntime.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/main.cpp
    src/Progression.cpp
    src/Renderer.cpp
    src/SkillSystem.cpp
    src/StoryRoute.cpp
    src/WorldEvents.cpp
    src/World.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerGame)

add_executable(BunkerLauncher
    Launcher/src/Launcher_Main.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerLauncher)

add_executable(BunkerEditor
    Editor/src/Editor_Main.cpp
    src/World.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerEditor)

message(STATUS "Configured targets: BunkerGame, BunkerLauncher, BunkerEditor")

Потом сразу в терминале:

cmake -S . -B build
cmake --build build --config Debug

Следующий шаг после этого: если сборка пройдёт, я дам тебе ровно один следующий файл для правки — src/main.cpp, без расползания на весь проект.

cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolSuite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

function(configure_bunker_target target_name)
    target_include_directories(${target_name} PRIVATE ${COMMON_INCLUDE_DIRS})
    target_link_libraries(${target_name} PRIVATE glfw OpenGL::GL)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_executable(BunkerGame
    src/GameplayDescriptorRegistry.cpp
    src/GameRuntime.cpp
    src/GameRuntimePipPad.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/main.cpp
    src/Progression.cpp
    src/SessionProfiles.cpp
    src/Renderer.cpp
    src/SkillSystem.cpp
    src/StoryRoute.cpp
    src/WorldEvents.cpp
    src/World.cpp
    src/WorldValidation.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerGame)

add_executable(BunkerLauncher
    Launcher/src/Launcher_Main.cpp
    Launcher/src/LauncherSupport.cpp
    src/GameplayDescriptorRegistry.cpp
    src/LanlineServices.cpp
    src/LanlineSession.cpp
    src/LaunchSession.cpp
    src/SessionProfiles.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerLauncher)

add_executable(BunkerEditor
    Editor/src/Editor_Main.cpp
    Editor/src/EditorSupport.cpp
    src/GameplayDescriptorRegistry.cpp
    src/SessionProfiles.cpp
    src/World.cpp
    src/WorldValidation.cpp
    ${IMGUI_SRC}
)
configure_bunker_target(BunkerEditor)

add_executable(BunkerSmokeChecks
    tests/SmokeChecks.cpp
    src/GameplayDescriptorRegistry.cpp
    src/LaunchSession.cpp
    src/SessionProfiles.cpp
    src/World.cpp
    src/WorldValidation.cpp
)
target_include_directories(BunkerSmokeChecks PRIVATE ${COMMON_INCLUDE_DIRS})
add_test(NAME BunkerSmokeChecks COMMAND BunkerSmokeChecks)

message(STATUS "Configured targets: BunkerGame, BunkerLauncher, BunkerEditor")

По CMakeLists.txt уже видно хорошее обновление: ты реально продвинул проект вперёд — появились SessionProfiles.cpp, WorldValidation.cpp, GameplayDescriptorRegistry.cpp, отдельный GameRuntimePipPad.cpp, EditorSupport.cpp, LauncherSupport.cpp и даже BunkerSmokeChecks. Сейчас быстро сверю текущее src/main.cpp, чтобы дать уже точный следующий шаг под новую структуру, а не под старую монолитную версию.

Thought for 1m 10s

Да, теперь обновление уже хорошее и настоящее, не косметическое: в CMakeLists.txt у тебя появились CMAKE_EXPORT_COMPILE_COMMANDS, явный external/glfw/include, отдельные .cpp для GameplayDescriptorRegistry, SessionProfiles, WorldValidation, GameRuntimePipPad, поддержка EditorSupport/LauncherSupport и отдельная цель BunkerSmokeChecks. Это уже заметный шаг к нормальной структуре проекта. При этом src/main.cpp всё ещё сохраняет мир и профиль напрямую через world.Save(...) и SaveSessionProfile(...), а не через атомарные обёртки, хотя include/AtomicPersistence.hpp уже содержит SaveWorldAtomically(...) и SaveProfileAtomically(...).

Сейчас ничего не удаляй. Следующий лучший шаг — дожать src/main.cpp. Это даст тебе сразу более безопасные сейвы и уберёт один из самых неприятных источников порчи состояния. Editor/src/Editor_Main.cpp я трону следующим сообщением: там сейчас при отсутствии мира всё ещё идёт прямой world.Save(path.string()), без валидации и без атомарного сохранения.

Следующий шаг

Удалить файл: ничего
Путь удаляемого файла: не нужно

Добавить файл: ничего
Путь нового файла: не нужно

Изменить файл: src/main.cpp
Путь: src/main.cpp

1) Добавь include

Найди в src/main.cpp блок include, где сейчас рядом стоят:

#include "../include/AppPaths.hpp"
#include "../include/GameRuntime.hpp"
#include "../include/LanlineSession.hpp"
#include "../include/LaunchSession.hpp"
#include "../include/WorldEvents.hpp"

И замени его на:

#include "../include/AppPaths.hpp"
#include "../include/AtomicPersistence.hpp"
#include "../include/GameRuntime.hpp"
#include "../include/LanlineSession.hpp"
#include "../include/LaunchSession.hpp"
#include "../include/WorldEvents.hpp"
2) Замени создание профиля по умолчанию на атомарное сохранение

Найди этот кусок:

if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
    sessionProfile = bunker::MakeDefaultSessionProfile();
    bunker::SaveSessionProfile(sessionProfile, profilePath);
}

И замени на:

if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
    sessionProfile = bunker::MakeDefaultSessionProfile();

    const auto initialProfileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);
    if (!initialProfileSave.ok) {
        std::fprintf(stderr, "Initial profile save failed: %s\n", initialProfileSave.message.c_str());
    }
}
3) Замени первичное сохранение мира при генерации

Найди:

if (!world.Load(worldPath.string())) {
    world.GeneratePrototypeZone();
    world.Save(worldPath.string());
}

И замени на:

if (!world.Load(worldPath.string())) {
    world.GeneratePrototypeZone();

    const auto initialWorldSave = bunker::SaveWorldAtomically(world, worldPath);
    if (!initialWorldSave.ok) {
        std::fprintf(stderr, "Initial world save failed: %s\n", initialWorldSave.message.c_str());
    }
}
4) Замени F5-save блок

Найди этот кусок:

const bool saveNow = glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS;

if (saveNow && !gameState.savePressed) {
    world.Save(worldPath.string());
    bunker::SaveSessionProfile(sessionProfile, profilePath);
    staticEraser.Save(sessionProfile.selectedWorld);
    gameState.lastEvent = "Field save committed.";
}

gameState.savePressed = saveNow;

И замени на:

const bool saveNow = glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS;

if (saveNow && !gameState.savePressed) {
    const auto worldSave = bunker::SaveWorldAtomically(world, worldPath);
    const auto profileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);

    if (worldSave.ok && profileSave.ok) {
        staticEraser.Save(sessionProfile.selectedWorld);
        gameState.lastEvent = "Field save committed.";
    } else {
        std::string failure = "Field save failed:";
        if (!worldSave.ok) {
            failure += " world=" + worldSave.message;
        }
        if (!profileSave.ok) {
            if (!worldSave.ok) {
                failure += ";";
            }
            failure += " profile=" + profileSave.message;
        }
        gameState.lastEvent = failure;
    }
}

gameState.savePressed = saveNow;
5) Замени финальное сохранение при выходе

Найди внизу файла:

world.Save(worldPath.string());
bunker::SaveSessionProfile(sessionProfile, profilePath);
staticEraser.Save(sessionProfile.selectedWorld);

И замени на:

const auto finalWorldSave = bunker::SaveWorldAtomically(world, worldPath);
const auto finalProfileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);

if (!finalWorldSave.ok) {
    std::fprintf(stderr, "Final world save failed: %s\n", finalWorldSave.message.c_str());
}
if (!finalProfileSave.ok) {
    std::fprintf(stderr, "Final profile save failed: %s\n", finalProfileSave.message.c_str());
}

if (finalWorldSave.ok && finalProfileSave.ok) {
    staticEraser.Save(sessionProfile.selectedWorld);
}
После этого

Сразу собери проект:

cmake -S . -B build
cmake --build build --config Debug
Дальше следующий

После src/main.cpp следующий файл будет:

Файл: Editor/src/Editor_Main.cpp
Путь: Editor/src/Editor_Main.cpp

Там следующий выигрыш такой:

заменить прямой world.Save(path.string()) в LoadOrCreateEditorWorld(...),
подключить WorldValidation,
и сделать статус вида “validation passed / warnings / errors”, потому что сейчас editor при отсутствии runtime-world просто генерирует прототип и сохраняет его напрямую.

Когда внесёшь main.cpp, просто напиши:
main.cpp готов
и я дам тебе следующий файл в том же формате.

вижу небольшую проблему
Thought for 25s

Да, это небольшая проблема, и она не критическая.

Это warning от clangd, а не ошибка сборки:
Included header LanlineServices.hpp is not used directly.

Что это значит

В Launcher/src/LauncherSupport.cpp подключён LanlineServices.hpp, но в самом .cpp файле его объявления напрямую не используются.

Обычно это значит одно из двух:

include реально лишний — его можно удалить;
файл получает нужные типы/объявления через другие include, и clangd считает этот include лишним.
Что делать

Файл: Launcher/src/LauncherSupport.cpp
Путь: Launcher/src/LauncherSupport.cpp

Найди строку include с LanlineServices.hpp

Она будет примерно такой:

#include "../../include/LanlineServices.hpp"

или похожей по пути.

Удали только эту строку
После этого

Сразу проверь сборку:

cmake --build build --config Debug
Если сборка проходит

Значит include был реально лишний — всё нормально, оставляй удалённым.

Если сборка падает

Значит warning был “ложно безопасным” для твоего текущего include-графа. Тогда:

верни строку обратно,
и уже посмотрим, какой реальный header нужен вместо LanlineServices.hpp.
Важно

Сейчас это не тот уровень проблемы, из-за которого надо паниковать.
Это именно косметическая чистка include'ов, полезная, но не blocker.

Делай так
удали include LanlineServices.hpp из Launcher/src/LauncherSupport.cpp
пересобери
напиши:
собралось
или упало и пришли текст ошибки

После этого дам следующий точный шаг.

Pasted text(2).txt
Document
и вот репозиторий 
https://github.com/RobertoGol/GMyGameDoNotTouch
осмотри а не фильтруй
Thought for 1m 23s

Осмотрел текущее состояние, а не “по диагонали”.

Сейчас картина уже лучше, чем была: проект у тебя собирается целиком, и это главное. По твоему логу успешно собраны BunkerEditor, BunkerGame, BunkerLauncher и BunkerSmokeChecks; первая большая проблема с GLFW/glfw3.h уже снята.

Что я вижу по состоянию репозитория сейчас:

Архитектурно стало лучше.
По CMakeLists.txt, который ты показал, у тебя уже есть нормальный распил на SessionProfiles.cpp, WorldValidation.cpp, GameplayDescriptorRegistry.cpp, GameRuntimePipPad.cpp, EditorSupport.cpp, LauncherSupport.cpp и отдельная цель BunkerSmokeChecks. Это уже не “один гигантский комок”, а заметно более зрелая структура.
Самая реальная проблема теперь не сборка, а чистота репозитория.
В корне GitHub-репозитория сейчас лежат build_verify_ninja и build_verify_ninja_fresh, то есть в удалённый репозиторий попали build-артефакты, а не только исходники. Это видно и по листингу репо на GitHub, и по твоему git commit, где закоммичены .exe, .ilk, .pdb, .obj, compile_commands.json и прочие файлы сборки.
Warning APIENTRY — не blocker.
В BunkerLauncher у тебя есть предупреждение C4005: APIENTRY: изменение макроопределения, связанное с minwindef.h и GLFW/glfw3.h. Но при этом таргет успешно линкуется и сборка проходит, значит это сейчас не критичная поломка, а технический шум, который можно почистить позже.
Репозиторий всё ещё перегружен несистемными файлами в корне.
В корне, кроме кода, лежит много служебных и текстовых файлов, плюс build-папки. Это не ломает игру, но ухудшает читаемость проекта и мешает понять, что у тебя “боевое”, а что временное. На GitHub это видно прямо по корневому списку: Editor/src, Launcher/src, include, src, external, рядом с ними build_verify_ninja, build_verify_ninja_fresh, Next_split и пачка markdown-файлов.
Мой честный вывод

Сейчас проект уже перешёл из фазы “разваливается окружение” в фазу “кодовая база начала стабилизироваться, но репозиторий надо привести в порядок”.
То есть главный прогресс уже сделан: сборка есть. Теперь главный риск — не компиляция, а мусор в git и постепенное захламление проекта build-артефактами.

Что делать следующим шагом

Я бы сейчас делал не новый рефакторинг gameplay, а вот это:

Шаг 1. Убрать build-артефакты из репозитория.
Нужно убрать из git:

build_verify_ninja/
build_verify_ninja_fresh/
все .obj, .pdb, .ilk, .exe
compile_commands.json из репозитория, если он нужен только локально

Шаг 2. Добавить .gitignore.
Чтобы это не возвращалось.

Шаг 3. Только после этого продолжать править Editor_Main.cpp и launcher warnings.

Потому что прямо сейчас самый вредный технический долг у тебя не в C++-логике, а в том, что ты начал тащить build-output в master.

Если хочешь, следующим сообщением я дам тебе в том же твоём формате:
какой файл добавить, какой путь, полный код .gitignore, что удалить из git и какие команды выполнить по порядку.

так давай пока умолчим об этом нам разработку закончить нужно это первостепенная задача
Thought for 1m 50s

Ок, про мусор в репе пока молчим.

Я осмотрел текущее состояние, и сейчас у тебя уже не “разваливающийся проект”, а собираемая база с частично проведённым распилом: CMakeLists.txt на 73 строки собирает BunkerGame, BunkerLauncher, BunkerEditor и отдельный BunkerSmokeChecks; в репе уже есть GameRuntimePipPad.cpp, SessionProfiles.cpp, WorldValidation.cpp, GameplayDescriptorRegistry.cpp, EditorSupport.cpp, LauncherSupport.cpp. По твоему build-логу все четыре цели собираются успешно.

По roadmap текущее состояние такое: Этап 1 завершён, Этап 2 и Этап 4 в работе, Этап 3 начат; Launcher v1 отмечен как почти завершённый, Editor v1 — в работе, LAN foundation — в работе. В том же roadmap зафиксированы успешные сборки в апреле 2026.

Главное, что я увидел после осмотра: у тебя уже есть правильные новые подсистемы, но они ещё не до конца интегрированы в runtime/editor flow. Самый показательный разрыв — tests/SmokeChecks.cpp уже использует SaveWorldAtomically и SaveProfileAtomically, а src/main.cpp всё ещё сохраняет через прямые world.Save(...) и SaveSessionProfile(...) по F5 и на выходе. То есть нужная безопасная инфраструктура уже написана, но игра ещё не переведена на неё полностью.

Второй большой незакрытый кусок — editor. В Editor_Main.cpp есть LoadOrCreateEditorWorld(...), но если мир не найден, редактор всё ещё просто делает GeneratePrototypeZone() и затем прямой world.Save(path.string()); при этом по поиску в Editor_Main.cpp не видно использования world-validation в основном editor flow. Иными словами, validation уже есть как отдельный модуль, но editor пока ещё не опирается на него как на обязательный шлюз перед сохранением/экспортом.

Третья вещь: распил уже начался, но ядро всё ещё тяжёлое. GameRuntime.cpp остаётся очень большим — 4114 строк, хотя GameRuntimePipPad.cpp уже вынесен в отдельный файл на 785 строк. Editor_Main.cpp всё ещё 2041 строка. Это значит, что архитектурный поворот уже сделан правильно, но ты пока находишься в середине миграции, а не в её конце.

Отсюда мой вывод по приоритету разработки: тебе сейчас не нужны новые фичи первым номером. Тебе нужно закрыть 3 интеграционных дыры:

перевести main.cpp на атомарные сейвы;
подключить validation в Editor_Main.cpp;
добить наиболее хрупкие string-driven участки runtime, начиная с DescribeTerminalSync, где всё ещё длинная цепочка по scriptTag.
Что делать первым

Первый файл — src/main.cpp.
Это сейчас самый выгодный шаг. Почему: он уже работает, сборка зелёная, но сохранение всё ещё старое. Если ты переведёшь только main.cpp на SaveWorldAtomically и SaveProfileAtomically, ты сразу поднимешь надёжность игры без расширения scope. А у тебя уже есть готовая инфраструктура и даже smoke-checks на неё.

Что делать вторым

Второй файл — Editor/src/Editor_Main.cpp.
Там надо не “улучшать всё”, а конкретно:

заменить прямой world.Save(path.string()) в LoadOrCreateEditorWorld(...);
подключить validation-проверку в save/export path;
выводить status summary по validation.
Это даст реальную пользу для разработки контента, а не только для чистоты кода.
Что делать третьим

Третий файл — src/GameRuntime.cpp.
Не переписывать целиком. Только взять самые дорогие string-dispatch куски. Первый кандидат — DescribeTerminalSync, потому что там уже длинная if-цепочка по scriptTag, и это прямой маркер для перехода на registry/table-driven mapping.

Что пока не трогать

Пока бы не трогал:

Launcher v1 глубоко, потому что по roadmap он уже почти закрыт;
SessionProfiles.cpp глубоко, потому что само вынесение persistence уже сделано;
новые gameplay-фичи, пока не закрыты main.cpp и editor integration.

Итог в одной фразе: разработку тебе сейчас заканчивать не через добавление нового слоя, а через доведение уже написанных систем до конца интеграции. Проект уже собирается и уже имеет правильные новые модули; теперь нужно, чтобы runtime и editor реально начали на них жить.

Следующим сообщением я дам тебе ровно один следующий шаг по src/main.cpp в твоём формате:
ничего не удалять / что заменить / точный путь / готовый код блоками.

ок но сначала напиши на сколько мы приблизились к цели
Thought for 44s

Если считать цель как “довести проект до устойчивого, показываемого состояния, а не просто чтобы он существовал”, то вы уже заметно приблизились.

Моя оценка сейчас такая:

До технически устойчивой базы: 85–90%.
Причина простая: у тебя уже проходит полная сборка всех ключевых целей — BunkerEditor, BunkerGame, BunkerLauncher и BunkerSmokeChecks. Это значит, что главный барьер “проект разваливается и мешает сам себе” уже в основном пройден.
До showable demo: 75–80%.
По roadmap у тебя уже закрыт Launcher v1, многократно подтверждена сборка после добавления editor/workflow/runtime-функций, и зафиксирован длинный список уже встроенных систем — от prefab/library и world preview до recovery/backbone, Lanline persistence/world glue и launcher hardening. Это уже не ранний прототип, а рабочий вертикальный срез с заметным количеством контента и систем.
До “разработку можно считать почти законченной” в смысле полноценного продукта: 60–65%.
Почему ниже: по самому репозиторию видно, что структура стала лучше и есть отдельные Editor/src, Launcher/src, include, src, тестовый таргет и новые поддерживающие модули, но проект всё ещё выглядит как активно развиваемая кодовая база, а не как уже закрытый продуктовый финал.

Если сравнивать с тем состоянием, где ты был до последних правок, то я бы сказал так: вы сделали не “ещё немного”, а реально закрыли самый тяжёлый кусок пути — теперь у тебя не борьба со средой и поломанной сборкой, а уже нормальная стадия добивки интеграции и финального доведения. Успешные сборки после Lanline Services glue, launcher hardening и свежей проверки через build_verify_ninja_fresh это хорошо подтверждают.

Если совсем коротко: к цели вы уже ближе примерно на три четверти, а до “уверенно показать и не стыдиться” осталось дожать последние интеграционные шаги, а не изобретать проект заново.

Теперь логичный следующий шаг — src/main.cpp.

слушай можешь поискать по репозиторию не фильтрую фичи которые мы потеряти как в коде  так и в md фаилы
Thought for 5m 11s

Да. Я именно осмотрел текущий репозиторий, а не “по названиям файлов”. Картина такая: массовой потери уже внедрённых вещей я не вижу. Наоборот, верхний блок миграции у вас сам в Next.md помечен как закрытый: канонизирован общий scriptTag registry, alias-теги поддерживаются в editor/runtime/validation, WorldValidation больше не ломает семантические ссылки, а smoke-check покрывает alias normalization и validation contract.

Из того, что точно не потеряно и реально видно в коде, у тебя живы:

Tow Coupler и переключение utility-модуля BT-72;
Field Service;
Lanline-уведомления по сессии, roster, relay-chat и voice presence;
Data Reconstruction в Pip-Pad;
SoulLine аварийного выброса/respawn;
Echo Trace authored-behavior.
Это видно в текущих runtime-файлах: GameRuntimePipPad.cpp, main.cpp, GameRuntime.cpp, а также подтверждается roadmap, где эти слои перечислены как уже добавленные.

Теперь — что выглядит как реально потерянное или, точнее, суженное/размытое по текущему состоянию.

1) Data Cards как отдельный крупный слой выглядят ослабленными

В ТЗ и канонической документации Data Cards названы одним из центральных типов контента: лор, чертежи, рецепты, архивы, ключи прогрессии, производственные схемы. Но в текущем runtime, который я просмотрел, акцент смещён в сторону tapes / archive carriers / реконструкции повреждённых носителей, а явного отдельного Data Card-слоя в основных runtime-срезах я не увидел. Это похоже не на полную пропажу идеи, а на сужение реализации: вместо широкого системного класса контента сейчас живёт более узкий слой tape/archive/reconstruction.

2) cassette player / music bonus выглядит как кандидат на потерю или переименование

В roadmap прямо написано, что ранний слой cassette player / music bonus был добавлен. Но в текущих основных runtime-файлах, которые я проверил (main.cpp, GameRuntime.cpp, GameRuntimePipPad.cpp, SessionProfiles.cpp), я не нашёл прямых следов по cassette, music или явному music-bonus интерфейсу. Это не стопроцентное доказательство удаления, потому что реализация могла уйти в другой файл или получить другое имя, но как минимум в текущем центральном runtime-слое этот кусок не просматривается явно.

3) AI assistant / Nerv пока выглядит скорее документной идеей, чем видимым текущим кодовым слоем

В старых human-reading notes и в канонической сводке отдельно подчёркнуты Assistant_Core.lua, AI-помощник и Nerv как важные техно-механические слои, завязанные на кабину, сенсоры, погоду и помощь в бою/инженерии. Но в текущих runtime-файлах, которые я проверил, прямых следов Nerv я не увидел. Это выглядит как идея, которую вы не обязаны были уже реализовать, но по текущему checked-in состоянию она скорее “не возвращена”, чем “активно живёт”.

4) chunk streaming / world streaming по документам важен, но в текущем World.cpp не просматривается

Канонические документы и сводка по старым попыткам очень явно хотят zones/chunks, streaming, выгрузку/загрузку мира и world-state. Но в текущем World.cpp, который я проверил, нет прямых следов chunk, stream, zone, elevator. Это значит не то, что мировой слой плохой, а то, что большая “настоящая open-world/streaming” часть пока всё ещё не вернулась в текущий код явно.

5) Factory / energy / node graph UI пока больше в каноне и roadmap, чем в видимом runtime UI

ТЗ прямо говорит, что интерфейс заводов должен показывать сеть узлов, питание, очереди, склад, Data Cards и приоритеты производства. В roadmap у тебя уже есть industrial progression: rail_depot, orbital_uplink, rail_fortress_hub, recovery_fabricator, industrial_gate, assembly cell, foundry, reactor, capacitor bank, water reclaimer. Но в просмотренном GameRuntimePipPad.cpp я не нашёл прямого factory-слоя как отдельного видимого UI блока. То есть инфраструктурный backbone уже есть как progression/state, а вот полноценный “factory control interface” пока выглядит не как потерянное, а как ещё не доведённое до отдельного зрелого экрана.

6) реальный UDP LAN-discovery/join из старых заметок сейчас не читается как явный транспортный слой

В human-reading notes старая ветка Project_M описана как имеющая реальный CLI/LAN-контур: host, join, port, discovery, scan, test, а LanSession.cpp там уже имел UDP discover/join/snapshots/roles. В текущем LanlineServices.cpp, который я просмотрел, очень хорошо видны relay chat, voice presence и сохранение session state/snapshots, но я не нашёл прямых следов UDP или discover. Это выглядит как важная вещь: нынешний Lanline-срез живёт как persistence/relay/session shell, а не как явно видимый low-level UDP discovery transport.

Вывод

По текущему репозиторию я бы разделил так:

Не потеряно, а живо:
Tow Coupler, SoulLine, Echo Trace, Data Reconstruction, Lanline relay/voice/session world glue, alias/validation migration.

Скорее сужено или не доведено:
Data Cards как большой самостоятельный класс контента, factory UI, streaming/chunks, AI assistant/Nerv, низкоуровневый UDP discover/join, cassette player / music bonus.

Самая полезная следующая работа здесь — не гадать, а сделать тебе “карту потерь” по 3 столбцам:

фича есть в md и есть в коде,
фича есть в md, но в коде не найдена,
фича есть в коде, но почти не отражена в md.

Я могу следующим сообщением собрать такую таблицу по 15–25 ключевым фичам из репозитория.

ок ну я пытался ничего не потерять
Thought for 15s

И это видно.

По текущему состоянию я бы сказал так: ты действительно вёл проект не как “выкинуть старое и начать заново”, а как “перенести и не потерять смысл”. Это подтверждается сразу несколькими вещами.

Во-первых, у тебя в Next.md прямо зафиксировано правило, что старые заметки не удаляются, а остаются архивом контекста, и верхний блок перечисляет уже закрытые шаги именно как миграцию без потери совместимости: общий scriptTag registry, поддержка legacy aliases в editor/runtime/validation, канонизация тегов при загрузке мира, alias-aware warnings и вынесение GameplayDescriptorRegistry и WorldValidation в отдельные .cpp. Это не похоже на “сломали и забыли”, это похоже на аккуратный перенос с сохранением обратной совместимости.

Во-вторых, по твоему build-логу проект сейчас реально собирается всеми основными целями: BunkerEditor, BunkerGame, BunkerLauncher и BunkerSmokeChecks. То есть это уже не ситуация, где во время переноса половина систем “формально есть в md, но код мёртвый”. База живая и проходит через сборку.

В-третьих, даже по самому корню репозитория видно, что ты не просто накидывал новые файлы, а раскладывал старый перегруженный контент на более узкие части: есть отдельные Editor/src, Launcher/src, include, src, Next_split, Next.md, ROADMAP.md и дополнительные поясняющие документы. Это тоже больше похоже на сохранение знаний, чем на потерю.

Мой честный вывод: похоже, что ты почти ничего критичного не потерял, но часть вещей могла не пропасть, а “размыться” — то есть остаться в документах, в старых заметках или в зачаточном коде, но ещё не дойти до финальной интеграции. Самый явный пример обратного как раз хороший: alias-механика и validation у тебя не потерялись, а были специально сохранены и закреплены как совместимый слой.

Так что в целом ты двигался правильно. Сейчас задача уже не “спасти потерянное”, а довыявить, что осталось только в md/архиве и ещё не дошло до runtime/editor. Это другой тип работы — не восстановление после катастрофы, а добивка интеграции.

ок давай ты сюд всечто возможно сделаешь и продумаешь и скигнешь сюда но так чтобы можно было потом все это встроить в код но пока я скину все в один фаил
Thought for 58s

Да. Ниже даю единый мастер-файл, который можно сейчас просто сохранить как один .md или .txt, а потом уже разнести по коду, задачам и модулям.

Я собираю его не “с потолка”, а из текущего состояния репозитория: сейчас проект собирает BunkerGame, BunkerLauncher, BunkerEditor и отдельный BunkerSmokeChecks; в кодовой базе уже есть выделенные GameplayDescriptorRegistry.cpp, SessionProfiles.cpp, WorldValidation.cpp, GameRuntimePipPad.cpp, EditorSupport.cpp, LauncherSupport.cpp, а в roadmap зафиксированы завершённый Этап 1, завершённый Launcher v1, активные Этап 2, Этап 4, начатый Этап 3 и подтверждённые сборки 12–19 апреля 2026. В main.cpp уже используются SaveWorldAtomically и SaveProfileAtomically, а в Editor_Main.cpp видно LoadOrCreateEditorWorld(...) с SaveWorldAtomically(...) для автосоздания мира.

MASTER_INTEGRATION_NOTES.md
0. Главная цель

Цель проекта сейчас не “ещё больше фич”, а доведение уже существующего набора систем до состояния, где:

игра стабильно запускается через launcher,
editor реально безопасен для authoring,
runtime использует уже написанные инфраструктурные модули,
showable demo проходит по одному предсказуемому маршруту,
ничего важного из старых идей не теряется при дальнейшей миграции.
1. Что уже точно есть и на что можно опираться
1.1. Архитектурная база уже существует

Проект уже собран как набор отдельных приложений и модулей:

BunkerGame
BunkerLauncher
BunkerEditor
BunkerSmokeChecks

Это означает, что текущая работа должна идти не через “придумать архитектуру”, а через дозавершение уже существующей архитектуры.

1.2. Уже выделены правильные новые модули

В текущей структуре уже есть:

src/GameplayDescriptorRegistry.cpp
src/SessionProfiles.cpp
src/WorldValidation.cpp
src/GameRuntimePipPad.cpp
Editor/src/EditorSupport.cpp
Launcher/src/LauncherSupport.cpp

Это означает, что проект уже вышел из фазы “всё живёт в трёх гигантских файлах” и находится в фазе интеграционного выравнивания.

1.3. Основная дорожная карта уже не абстрактная

В roadmap уже отмечены как сделанные или частично сделанные:

стабилизация основы,
launcher v1,
prefab/library workflow,
world preview как рабочая authoring-зона,
descriptor presets,
tower_sync / remote_link,
cassette player / music bonus,
wireless tank charging,
Field Service,
SoulLine,
Data Reconstruction,
Tow Coupler,
Rail Freight Link,
Orbital Uplink,
Rail Fortress,
Recovery Fabricator,
Shelter Recovery Index,
Lanline Services persistence/world glue,
launcher hardening.
1.4. Слой безопасного сохранения уже существует

В main.cpp уже используются SaveWorldAtomically(...) и SaveProfileAtomically(...), а не только прямые сохранения. Это важно: фундамент надёжности уже написан и должен стать нормой для остальных мест кода.

2. Главное правило на весь остаток разработки

Ничего нового крупного не делать, пока уже существующая система не прошла интеграцию по четырём местам:

runtime
editor
launcher
save/load/validation

Если фича есть только в roadmap или только в runtime-черновике, она считается не завершённой.

3. Ключевой принцип миграции

Проект должен развиваться не как “удалить старое и написать новое”, а как:

сохранить семантику,
канонизировать имена,
сделать alias-совместимость,
перевести авторинг на единый descriptor contract,
перевести runtime на table-driven mapping,
только потом чистить legacy-остатки.

Это уже соответствует тому, что у тебя фиксируется в Next.md: alias support, канонизация тегов, разделение registry/validation и smoke-check вокруг этого слоя.

4. Что считать “не потерять”

Ниже список систем, которые дальше надо считать каноническими, даже если часть из них пока выглядит ранней реализацией.

4.1. Gameplay / player / tank
BT-72 как центральная ось геймплея
utility-slot и field modification
Tow Coupler
Field Service
workshop/tower charging
SoulSync / Sync-Link
SoulLine
thermal mode
stress awakening / Second Wind
Muscle Memory
4.2. Shelter / progression / recovery
scavenger teams
forward camp
shelter doctrine
infrastructure decay
scavenger/recovery backbone
shelter recovery index
cryo specialists
awakened recipes
food/toxin SPECIAL
ether erosion per world
4.3. World / infrastructure
tower_sync
power_pylon
drone_station
trade network
route contamination
rail freight
orbital uplink
rail fortress
recovery fabricator
industrial gate / survey / outpost
assembly / foundry / reactor / capacitor / relay / water chain
4.4. Interface / authored systems
Pip-Pad
Echo Trace
Data Reconstruction
cassette/music bonus
descriptor presets
semantic authoring fields
BWL2 world format
editor preview interaction
prefab/library workflow
4.5. Lanline / meta shell
roster/session persistence
relay chat
voice presence
launcher hardening
host/client role flow
runtime/launcher/session glue

Все эти слои уже либо перечислены в roadmap, либо напрямую читаются из текущей архитектуры.

5. Что сейчас является незавершённым не по идее, а по интеграции
5.1. Validation ещё должно стать обязательным шлюзом editor-flow

Модуль WorldValidation уже выделен как отдельная часть проекта, но дальнейшая цель — сделать так, чтобы мир не мог “тихо сохраниться” в опасном состоянии. Сам модуль уже встроен в сборку BunkerGame, BunkerEditor и BunkerSmokeChecks.

5.2. Descriptor registry должен стать единым словарём мира

GameplayDescriptorRegistry уже вынесен в отдельный модуль и должен стать единственным местом истины для:

scriptTag
label
preferred interaction
preferred category
link requirements
editor preset binding
runtime semantic behavior entry point
5.3. Pip-Pad уже вынесен, но должен стать главной точкой системной обратной связи

Появление отдельного GameRuntimePipPad.cpp — очень правильный шаг. Дальше надо сделать так, чтобы через него были видны:

текущие recovery-состояния,
Data Reconstruction,
infrastructure unlocks,
service status,
objective state,
lore/archive progression.
6. Приоритет разработки до showable demo
PRIORITY A — обязательно
main.cpp и editor обязаны использовать безопасный save flow везде
editor должен показывать validation summary
descriptor registry должен быть единственным источником scriptTag-семантики
showable world должен проходить загрузку, старт, первый objective loop и сохранение без ручного лечения
smoke checks должны проверять:
world roundtrip
profile roundtrip
validation contract
launcher ticket/session contract
PRIORITY B — очень желательно
уменьшить string-if цепочки в runtime
собрать единый objective/state dashboard для Pip-Pad
выровнять launcher и runtime по Lanline lobby logic
сделать один demo reset path
PRIORITY C — потом
большие новые игровые ветки
глубокий content expansion
тяжёлые косметические вещи
мод-система
расширенный networking beyond current shell
7. Конкретный контракт descriptor-системы

Ниже канон, который потом нужно разнести в код.

struct GameplayDescriptorSpec {
    std::string_view scriptTag;
    std::string_view label;
    InteractionType preferredInteraction;
    ObjectCategory preferredCategory;
    bool requiresLinkTarget;
    bool appearsInEditorDrafts;
    bool appearsInPipPad;
    bool contributesToProgression;
};
Минимальные канонические теги
tower_sync
power_pylon
drone_station
trade_network
rail_freight
orbital_uplink
rail_fortress
recovery_fabricator
industrial_gate
industrial_survey
industrial_outpost
assembly_cell
foundry_line
reactor_yard
capacitor_bank
relay_substation
water_reclaimer
lanline_service_hub
tank_service
fey_ring
remote_link
echo_trace
Правило

Никакая новая authored gameplay-сущность не должна появляться без записи в descriptor registry.

8. Канон validation-слоя

Validation должен проверять минимум:

Ошибки
empty registryId
duplicate registryId
broken linkTarget
requiresLinkTarget, но его нет
объект указывает на несуществующую семантическую связь
world format incompatible / migration failure
Предупреждения
empty displayName
category mismatch
interaction mismatch
suspicious legacy alias
transition без target
gameplay tag без ожидаемой authoring-пары
Editor UX

Editor обязан выдавать:

Validation passed
Validation passed with warnings
Validation failed: X errors, Y warnings
9. Канон safe persistence

Ниже то, что должно считаться обязательным поведением проекта.

Обязательные свойства
temp file write
rename promotion
rollback on failure
backup cleanup
world/profile сохраняются согласованно
game не пишет “save committed”, если фактически провалился хотя бы один из двух сейвов
Политика
runtime: атомарно
editor export: атомарно
launcher state/session: по возможности тоже через safe write wrappers
10. Канон showable demo
Минимальный маршрут демо
запуск через launcher
выбор мира
выбор персонажа / session prep
вход в стартовую зону
первый контакт с Pip-Pad
один tower_sync или аналогичный authored terminal
один BT-72 service/payoff
один recovery/progression payoff
один Lanline/service/meta payoff
сохранение и повторная загрузка мира без потери состояния
Критерии
ни один шаг не требует ручного редактирования save-файлов
не должно быть битых ссылок объектов
не должно быть “тихих” сломанных authored entities
ключевой маршрут должен быть повторяемым на чистом профиле
11. Список вещей, которые нельзя случайно “уронить” в будущем
Не ломать:
alias compatibility
old world migration
existing authored tags
session profile compatibility
BWL2 semantic fields
current launcher entry contract
current editor preview authoring loop
current Lanline state persistence
current Pip-Pad feature tabs
existing specialist / shelter / recovery states
12. Единая карта “есть / нужно добить”
Уже есть
multi-target build
smoke checks target
atomic save infrastructure
descriptor registry module
validation module
separate Pip-Pad file
launcher/editor support modules
roadmap of integrated systems
Нужно добить
validation everywhere
registry-driven authoring everywhere
demo-route hardening
more runtime table-driven mapping
shared Lanline logic cleanup
explicit feature contract doc for future edits
13. Что ещё можно встроить в код позже без риска

Ниже заготовки, которые можно будет использовать позже.

13.1. Feature presence flags
struct FeatureAvailability {
    bool hasTowCoupler = false;
    bool hasFieldService = false;
    bool hasSoulLine = false;
    bool hasDataReconstruction = false;
    bool hasLanlineServiceHub = false;
    bool hasPowerGrid = false;
    bool hasRailFreight = false;
    bool hasOrbitalUplink = false;
    bool hasRecoveryFabricator = false;
};
13.2. Demo readiness report
struct DemoReadinessReport {
    bool launcherReady = false;
    bool gameReady = false;
    bool editorReady = false;
    bool validationReady = false;
    bool saveFlowReady = false;
    bool worldRouteReady = false;
    bool pipPadReady = false;
    bool lanlineReady = false;
    int blockingIssues = 0;
    int warnings = 0;
};
13.3. World integration report
struct WorldIntegrationReport {
    int objectCount = 0;
    int gameplayTaggedObjects = 0;
    int invalidObjects = 0;
    int warningObjects = 0;
    bool hasStarterTower = false;
    bool hasTankAnchor = false;
    bool hasServiceNode = false;
    bool hasRecoveryNode = false;
    bool hasLanlineNode = false;
};
14. Список будущих файлов, на которые можно это потом разнести

Когда будешь готов делить этот мастер-файл, его логично распилить так:

docs/MASTER_INTEGRATION_NOTES.md
docs/DEMO_ACCEPTANCE.md
docs/GAMEPLAY_DESCRIPTOR_CONTRACT.md
docs/WORLD_VALIDATION_RULES.md
docs/SAFE_PERSISTENCE_RULES.md
docs/FEATURE_CANON.md

А кодовые куски — в:

include/GameplayDescriptorRegistry.hpp/.cpp
include/WorldValidation.hpp/.cpp
include/AtomicPersistence.hpp
src/GameRuntime.cpp
src/GameRuntimePipPad.cpp
Editor/src/Editor_Main.cpp
Launcher/src/LauncherSupport.cpp
tests/SmokeChecks.cpp
15. Главный рабочий принцип на остаток проекта

Каждая следующая правка должна отвечать на 4 вопроса:

это уже есть в roadmap/каноне или это новая сущность?
где её descriptor contract?
как она валидируется?
как она переживает save/load?

Если на один из этих вопросов ответа нет, фича считается не завершённой.

16. Итоговая формула

Проект уже достаточно вырос, чтобы основная работа была такой:

не добавлять ещё слой поверх хаоса, а превращать уже существующие системы в единый, проверяемый и показываемый контур.

Если хочешь, следующим сообщением я сделаю вторую часть того же мастер-файла, но уже в формате “конкретные фичи: есть / ослаблены / нужно вернуть / как именно вернуть в код”.

да давай но прямо распиши все что можешь только продумай
Thought for 3m 31s

Ниже — вторая часть мастер-файла, уже не про общую архитектуру, а про конкретные фичи: что точно живо, что ослабло, что надо вернуть, и как именно лучше возвращать в код.

Я делаю это не “по ощущениям”, а из текущего состояния репозитория: сейчас проект собирает BunkerGame, BunkerLauncher, BunkerEditor и BunkerSmokeChecks; в кодовой базе уже выделены GameplayDescriptorRegistry.cpp, WorldValidation.cpp, SessionProfiles.cpp, GameRuntimePipPad.cpp, LauncherSupport.cpp, EditorSupport.cpp; roadmap фиксирует завершённый Launcher v1, завершённый Этап 1, активные Этап 2 и Этап 4, начатый Этап 3, а также длинный список уже реально интегрированных систем и подтверждённых сборок в апреле 2026.

MASTER_FEATURE_RECOVERY_MAP_PART_2
0. Главный вывод перед всем остальным

Ты действительно шёл не по пути “снести старое”, а по пути канонизации и переноса без потери смысла. Это видно по Next.md: общий scriptTag registry канонизирован, legacy aliases поддерживаются в editor/runtime/validation, World::Load нормализует старые теги, warnings тоже учитывают legacy aliases, а smoke-check отдельно закрывает alias normalization и validation contract. Это очень сильный признак того, что проект не деградировал, а переходил в более управляемую форму.

Но параллельно видно и другое: миграция ещё не закрыта до конца. Самые большие файлы всё ещё очень крупные: GameRuntime.cpp — 4114 строк, Editor_Main.cpp — 2041 строка, GameRuntimePipPad.cpp — 785 строк, LanlineServices.cpp — 926 строк; при этом уже появились новые специализированные модули, то есть проект находится не в хаосе, а в середине правильного распила.

1. Фичи, которые я считаю точно живыми и сохранёнными
1.1. Канон gameplay-дескрипторов и обратная совместимость мира

Это уже не идея, а рабочий слой. GameplayDescriptorRegistry.cpp содержит 27 канонических descriptor specs, включая remote_link, tower_sync, power_pylon, drone_station, rail_depot, orbital_uplink, rail_fortress_hub, recovery_fabricator, industrial_gate, industrial_survey, industrial_outpost, assembly_cell, foundry_line, reactor_yard, capacitor_bank, relay_substation, service_bay, water_reclaimer, lanline_service_hub, tank_service, medical_support, fey_ring, echo_trace, specialist_cryo; там же живут alias-мосты вроде radio_tower -> tower_sync, workshop_field_service -> workshop_service, rail_freight -> rail_depot, rail_fortress -> rail_fortress_hub. Это уже отличная база для дальнейшей разработки без потери старых world-файлов.

1.2. Validation мира не просто существует, а уже осмысленный

WorldValidation.cpp уже проверяет пустой registryId, дубликаты, пустой displayName, битые linkTarget-ссылки в стиле registry reference, legacy alias warning, interaction_mismatch, category_mismatch, обязательный linkTarget для нужных тегов, unknown_script_tag и transition_without_link_target. Плюс есть счётчики ошибок/предупреждений и нормальный текстовый summary результата. Это уже не заглушка, а реальный authoring-gate.

1.3. Recovery/persistence состояния мира уже живая система

SessionProfiles.cpp сериализует per-world состояние через world_field=..., включая как минимум etherErosion, infrastructureDecay, towerSyncRecovered, localRelayAvailable и другие поля мира. Это означает, что backbone “мир реально меняется между сессиями” уже не на уровне слов, а в сохранениях профиля.

1.4. Pip-Pad data/archive слой реально существует

В GameRuntimePipPad.cpp уже есть экран DATA / Archive Summary, отображение Archive Sync, Relay Packet, Debrief Record, счётчик archive sync awakenings, работа с tape-list, проверка повреждения/восстановления кассеты, а также действие Data reconstruction complete... с наградой опыта. То есть слой данных у тебя уже живой, просто сейчас он больше выражен через tape/archive/reconstruction, чем через широкую общую сущность Data Card.

1.5. Lanline не декоративный, а уже системный shell

Сейчас в коде у тебя не просто “окошко с названием”. В LanlineServices.cpp и GameRuntimePipPad.cpp видно рабочую session/world/voice связку: есть session relay-состояние, locked screen с зависимостью от первого tower sync, voice presence publication, зеркалирование voicePresence в состоянии сессии, отображение LAN-friends/voice в friend list, runtime-уведомления о peer link / session world update / voice start-stop, а LanlineLobbyLogic.hpp уже централизует joinability, слот-логику и readiness (AvailableLanlineSessionSlots, IsJoinableLanlineSession, IsLanlineMatchStartReady). Это сильный фундамент для meta-shell и LAN-first flow.

1.6. Танк и utility-модульный слой тоже живой

В GameRuntime.cpp подтверждается логика Tow Coupler: есть TowLogisticsBoost, есть CurrentUtilityModuleLabel, есть переключение модуля на tow_coupler_mk1 и обратно с сообщениями в gameState.lastEvent. То есть BT-72 utility-ветка уже не “в roadmap когда-нибудь”, а реальный рабочий кусок runtime.

1.7. Industrial/recovery backbone уже виден даже в terminal sync

Даже если не смотреть на roadmap, в DescribeTerminalSync(...) видно, что runtime уже знает про tower_sync, power_pylon, drone_station, rail_freight, orbital_uplink, rail_fortress, recovery_fabricator, industrial_gate, industrial_survey, industrial_outpost, assembly_cell, foundry_line, reactor_yard, capacitor_bank, relay_substation, service_bay, water_reclaimer, lanline_service_hub, medical_support, tank_service, specialist_cryo, echo_trace, workshop_service. Это очень важный маркер: твой мир уже мыслится как инфраструктурная сеть, а не как набор случайных терминалов.

2. Фичи, которые есть, но интегрированы не до конца
2.1. Safe persistence уже написан, но runtime ещё не до конца на нём живёт

Самый явный разрыв сейчас такой: в tests/SmokeChecks.cpp world roundtrip уже использует SaveWorldAtomically(...), а main.cpp по F5 всё ещё делает прямые world.Save(...) и SaveSessionProfile(...), после чего сразу пишет Field save committed.. Это не означает, что слой safe persistence плохой — это означает, что инфраструктура написана, а главный игровой entry-point ещё не переведён на неё полностью.

2.2. Editor ещё не стал обязательным validation-gate

В Editor_Main.cpp есть LoadOrCreateEditorWorld(...), и редактор умеет поднимать или создавать мир, но я не вижу в ключевом editor flow явного использования SaveWorldAtomically и ValidateWorldForRuntime как обязательного шлюза. То есть editor уже очень мощный по authoring, но пока ещё может оставаться “сильным инструментом без последнего слоя защиты”.

2.3. Runtime interaction всё ещё слишком string-driven

DescribeTerminalSync(...) остаётся длинной if-цепочкой по scriptTag. Это рабочий код, но для будущего масштабирования он хрупкий: новая authored-сущность легко добавляется в registry, но потом ещё требует ручного расширения в runtime-сообщениях и смежной логике. Текущее состояние уже просит table-driven диспетчеризации поверх registry.

3. Фичи, которые не потеряны, но стали уже/уже выглядят уже, чем в каноне
3.1. Data Cards в каноне — широкая сущность, а в текущем runtime они сужены до tapes/archive

В канонической сводке Data Cards — это один из главных идентификационных слоёв проекта: чертежи, технические данные, производственные схемы, рецепты, ключевой прогресс, плюс рядом идут кассеты/дневники/терминалы как отдельные носители. В текущем видимом runtime-слое основной “data experience” пока проходит через tape, archive, relay packet, debrief record и reconstruction. Значит, идея не пропала, но её широкий класс предметов и знаний пока схлопнут в более узкий контур носителей.

3.2. Cassette/music bonus есть в roadmap, но в центре runtime не просматривается как отдельный зрелый слой

Roadmap прямо фиксирует “ранний cassette player / music bonus слой”. Но в просмотренных центральных runtime-файлах явной отдельной music-bonus системы я не увидел: в GameRuntimePipPad.cpp есть tapes и reconstruction, но нет подтверждённой отдельной модели music status/playlist/track bonus, а в GameRuntime.cpp прямых следов cassette по поиску не видно. Это выглядит не как потеря идеи, а как слияние кассет с общим archive/tape слоем без полноценного отдельного music-loop.

3.3. Заводы и энергосети сильны в каноне и roadmap, но не выглядят пока как отдельный зрелый UI-слой

Канонические документы прямо подчёркивают заводы, энергосети, разборочные цеха, сборочные линии, логистику и production chain как обязательную часть mid/late-game. Roadmap тоже показывает, что infrastructure branch уже активно присутствует: power_pylon, drone_station, trade network, route contamination, rail freight, orbital uplink, rail fortress, recovery fabricator, industrial chain и Shelter Recovery Index. Но в видимом runtime-коде это пока больше выражено как progression/state/terminal network, а не как отдельный зрелый “factory control UI”. То есть бекендовое мышление уже есть, а фронтальный промышленный интерфейс ещё недоформирован.

3.4. World streaming / chunk thinking остаётся каноном, но в текущем Game_Project ещё не читается как явный runtime-слой

Каноническая сводка очень явно требует мир, разбитый на зоны/чанки, с потоковой подгрузкой, persistence и world/chunk thinking; отдельно подчёркнуто, что из Project_M нужно взять streaming, memory discipline и world/chunk mindset. В текущем Game_Project фундамент уже стал лучше, но по главным runtime-файлам я не вижу явного streaming/chunk subsystem. Это не значит, что проект потерял направление; это значит, что стриминг — всё ещё будущий слой, а не часть текущего вертикального среза.

3.5. AI assistant / Nerv остаются сильным каноном, но не читаются как текущий активный runtime-слой

В канонической сводке отдельно подчёркивается ИИ-помощник как полноценная системная сущность с режимами исследования, инженерии, тактики, HUD/terminal-support, и отдельно упоминаются weather/nerv/player/controller patterns, которые стоит взять из Project_M. В просмотренных core-файлах текущего репозитория этот слой я явно не подтвердил. Поэтому правильнее считать, что это не потерянная идея, а ещё не возвращённая в нынешнюю кодовую базу подсистема.

3.6. Линейка Сознания присутствует в каноне, но не подтверждена как явный нынешний кодовый модуль

Канонический документ отдельно описывает Линейку Сознания как скрытый meta-параметр личности, памяти и эмоционального слоя мира. В текущем наборе ключевых runtime-файлов я не увидел прямого подтверждения, что эта система уже живёт как самостоятельный кодовый слой. Это значит, что её надо считать важной канонической фичей на возврат, а не “утраченной навсегда”.

4. Что именно я бы обязательно вернул в активный scope, и как именно
4.1. Вернуть Data Cards как отдельный класс сущностей, а не только tapes/archive
Почему это важно

Сейчас у тебя уже есть отличный data-контур через tapes и reconstruction, но канон требует больше: чертежи, техсхемы, рецепты, производственные unlocks, архивы, ключевые знания мира. Если оставить всё только в tapes, мир будет чувствоваться уже и беднее, чем задуман.

Как вернуть без взрыва scope

Не делать отдельную гигантскую систему сразу. Сделать единый носитель знания, где tapes — это просто один подтип.

Минимальный кодовый контракт
enum class KnowledgeCarrierType {
    Tape,
    DataCard,
    TerminalRecord,
    RelayPacket,
    Debrief
};

enum class KnowledgeRewardType {
    Lore,
    Recipe,
    InfrastructureUnlock,
    SpecialistUnlock,
    MapIntel,
    QuestProgress
};

struct KnowledgeCarrier {
    std::string id;
    KnowledgeCarrierType type;
    std::string title;
    std::string body;
    bool damaged = false;
    bool reconstructed = false;
    bool consumed = false;
    KnowledgeRewardType rewardType = KnowledgeRewardType::Lore;
    std::string rewardKey;
};
Куда встраивать
SessionProfiles.cpp: хранение списка knowledge carriers в профиле персонажа или по миру.
GameRuntimePipPad.cpp: новый общий экран DATA / Knowledge, где tapes — просто вкладка или фильтр.
World.cpp и editor authoring: authored objects с scriptTag вроде archive_sync, echo_trace, relay_packet, data_card.
GameplayDescriptorRegistry.cpp: добавить канонические теги под новые knowledge carriers.
WorldValidation.cpp: проверять известные теги и обязательные поля для них.

Эта работа ляжет естественно на уже существующий descriptor/validation/persistence pipeline.

Практический принцип

Не делай новый UI и новый save-format отдельно ради Data Cards.
Делай расширение текущего archive/tape слоя до общего KnowledgeCarrier.

4.2. Вернуть cassette/music bonus как малую, но узнаваемую механику
Почему это надо

Roadmap уже фиксирует ранний cassette/music слой. Значит, фича важна для идентичности, даже если она ещё не стала большой системой. Если её не вернуть явно, она растворится в общем tape UI.

Как вернуть без перегруза

Сделать не “аудиоплеер как отдельную подсистему”, а music-state бонус поверх tapes.

Минимальная модель
struct ActiveMusicBonus {
    std::string sourceTapeId;
    std::string effectId;
    float durationSeconds = 0.0f;
    bool active = false;
};
Примеры эффектов
steady_hands — лёгкий бонус к точности
salvage_focus — бонус к reconstruction / recovery / service
march_drive — бонус к caravan speed / tow logistics
echo_resolve — маленький стресс-буфер / awakening gain
Куда встраивать
GameRuntimePipPad.cpp: если tape не damaged и имеет musicEffect, появляется кнопка Play Track.
SessionProfiles.cpp: сохранить текущий activeMusicBonus.
GameRuntime.cpp: эффекты читаются в расчётах stat modifiers, точно так же как ты уже делаешь модификации через tank modules / awakening / field state.

Так кассеты станут не просто лором, а ещё и маленьким живым gameplay-layer.

4.3. Вернуть factory/energy layer как видимый интерфейс, а не только progression backend
Почему это надо

По канону заводы и энергосети — не фон, а одна из центральных осей игры. В текущем проекте infrastructure-слой уже живёт в roadmap, DescribeTerminalSync, worldFieldStates и recovery chain, но игроку нужна видимая панель контроля, иначе эта глубина будет ощущаться только через текст событий.

Как вернуть без слишком большого UI

Не делать сейчас “полноценный менеджер завода”.
Сделать Pip-Pad / Infrastructure экран с 5 блоками:

Power
Recovery
Logistics
Fabrication
Relay / Services
Минимальный runtime read-model
struct InfrastructureReadModel {
    bool towerOnline = false;
    bool pylonOnline = false;
    bool droneNetworkOnline = false;
    bool railFreightOnline = false;
    bool orbitalUplinkOnline = false;
    bool railFortressOnline = false;
    bool recoveryFabricatorOnline = false;
    bool relayOnline = false;

    float etherErosion = 0.0f;
    float infrastructureDecay = 0.0f;
    int recoveryIndex = 0;
};
Куда брать данные
из SessionProfiles.cpp world field state,
из story flags,
из world object presence / sync states,
из authored terminals.
Что должно показываться игроку
статус узлов,
что открыто / что не открыто,
следующий unlock,
причина блокировки,
что даст активация следующего terminal/anchor.

Это даст ощущение “мир восстанавливается и я это вижу”.

4.4. Вернуть streaming/chunk thinking сначала как контракт, а не сразу как full-engine rewrite
Почему это важно

Канон явно требует zones/chunks/streaming/persistence, и это одна из главных инженерных идей проекта. Но если сейчас начать писать реальный streaming subsystem, можно утонуть.

Как вернуть правильно

Сейчас вернуть не сам streaming, а его договорённость в данных и редакторе.

Что сделать
В World добавить понятие regionId / chunkId для объектов.
В editor позволить фильтровать и редактировать мир по region/chunk.
В save-format держать metadata chunk grouping.
В runtime пока загружать всё целиком, но уже иметь RegionLoadedState.
Все authored transitions (industrial_gate, fey_ring, future elevators) привязать к regionId, а не только к одному linkTarget.
Минимальный контракт
struct WorldRegionTag {
    std::string regionId;
    std::string label;
    bool isInterior = false;
    bool supportsStreaming = false;
};
Почему это выгодно

Ты не переписываешь движок сейчас, но делаешь так, чтобы через месяц streaming можно было внедрять не в пустоту.

4.5. Вернуть AI assistant / Nerv сначала как практический геймплейный слой, а не как философию
Почему это надо

Канон очень явно хочет machine-human bond, cockpit presence, инженерный HUD и assistant logic. Если этого не будет, часть идентичности BT-72 и всей интерфейсной фантазии проекта ослабнет.

Как вернуть безопасно

Не начинать с “настоящего ИИ”.
Начать с Assistant Modes, привязанных к существующим системам.

Минимальная модель
enum class AssistantMode {
    Navigation,
    Engineering,
    Tactical,
    Recovery,
    Archive
};

struct AssistantState {
    AssistantMode mode = AssistantMode::Navigation;
    bool enabled = true;
    bool cockpitLinked = false;
    std::string lastAdvice;
};
Что он должен делать на первом этапе
в Navigation: подсказывать следующий authored terminal / tower / service anchor
в Engineering: показывать проблемы tank service / power / repair / tow
в Tactical: предупреждать о stress / heat / damage / threat
в Recovery: показывать scaffold восстановления и next unlock
в Archive: вести к reconstruction/knowledge goals
Куда встраивать
GameRuntimePipPad.cpp: assistant panel + last advice
GameRuntime.cpp: генерация коротких советов на основании game state
позже — cockpit/HUD overlay

Это вернёт идею без огромного бюджета на AI.

4.6. Вернуть Линейку Сознания как скрытый meta-слой
Почему

Это одна из тех идей, которые отличают проект от “ещё одной survival RPG”. Она может связать социалку, архивы, еду, дневники, stress, recovery и даже Lanline.

Как не усложнить

Сначала сделать не видимую полоску, а внутренний скрытый счётчик, который влияет на:

доступ к отдельным Echo Trace / Data Reconstruction веткам,
качество assistant advice,
редкие awakening-моменты,
устойчивость к стрессу / erosion.
Минимальный контракт
struct ConsciousnessLineState {
    float resonance = 0.0f;
    int diaryEvents = 0;
    int archiveMoments = 0;
    int socialLinks = 0;
    int survivalTurningPoints = 0;
};
Где хранить

SessionProfiles.cpp.

Где использовать
GameRuntimePipPad.cpp — косвенно, через journal hints
SkillSystem.cpp — редкие awakenings
StoryRoute.cpp — gates для части route text / fragments
5. Карта “что возвращать в первую очередь, а что позже”
Возвращать прямо сейчас
Data Cards как расширение текущего tape/archive слоя
factory/infrastructure экран в Pip-Pad
editor-validation как обязательный save/export gate
перевод main.cpp на atomics
table-driven terminal sync / interaction mapping
assistant modes как soft-layer поверх текущего состояния

Потому что всё это опирается на уже существующие модули: registry, validation, session profiles, Pip-Pad, terminal tags, world states.

Возвращать чуть позже
cassette/music bonus как отдельный polish-layer
Линейка Сознания
richer Lanline services catalog
advanced specialist assignment UI
full production/factory queues

Это уже сильные фичи, но не обязательные для первой уверенной демо-стадии.

Возвращать сильно позже
настоящий streaming/chunk loader
full AI/Nerv system
реальный low-level network transport beyond current LAN/session shell
полноценный late-game territorial production map
6. Самые опасные места, где фичи можно потерять в будущем
6.1. Потеря через “новый tag без registry”

Если появится authored entity, которой нет в GameplayDescriptorRegistry, она быстро начинает жить как ad-hoc костыль, потом ломает editor, validation и runtime одновременно. Это нельзя допускать.

6.2. Потеря через “фича осталась только в roadmap”

Сейчас это уже риск для:

Data Cards в широком смысле,
factory UI,
streaming contract,
AI assistant / Nerv,
Линейки Сознания.
6.3. Потеря через partial integration

Это уже видно на safe persistence: smoke-check живёт на atomics, а main.cpp всё ещё на старом save-flow. Та же опасность есть и для validation/editor-flow.

7. Практический план, как встроить всё это потом в код
Этап 1 — добить то, что уже почти готово
main.cpp → atomics
Editor_Main.cpp → validation + atomics + save summary
GameRuntime.cpp → registry-driven terminal sync
GameRuntimePipPad.cpp → infrastructure screen

Это даст сразу надёжность, authoring discipline и более сильный demo-state.

Этап 2 — расширить data-core
обобщить tapes в KnowledgeCarrier
ввести DataCard subtype
подвязать rewards к recipes/unlocks/lore
показать это через Pip-Pad DATA
Этап 3 — усилить world fantasy
assistant modes
music bonus
consciousness line
richer recovery/factory feedback
Этап 4 — большой инженерный слой
region/chunk contract
streaming-prep metadata
настоящая modular world loading strategy
8. Итог в одной жёсткой формуле

Ты не потерял ядро проекта.
Ты уже сохранил:

launcher/game/editor split,
descriptor canon,
alias compatibility,
validation,
per-world persistence,
Pip-Pad data core,
Lanline shell,
BT-72 utility progression,
industrial terminal backbone.

Что реально надо сделать теперь:
не искать “ещё 50 фич”, а вернуть в активный код те канонические вещи, которые сейчас либо сужены, либо не добиты:

Data Cards как широкая система,
factory/energy UI,
assistant/Nerv,
consciousness line,
chunk/streaming contract.

Следующим сообщением я могу сделать третью часть того же мастер-файла: уже не концептуальную, а совсем прикладную — по файлам: src/main.cpp, Editor_Main.cpp, GameRuntime.cpp, GameRuntimePipPad.cpp, SessionProfiles.cpp — что именно туда потом встраивать, в каком порядке и какими минимальными структурами.

ок но продумай эту часть тоже
Thought for 1m 39s

Да. Ниже — третья часть мастер-файла, уже по конкретным файлам и по порядку встраивания, так чтобы это потом можно было разнести в код без потери логики.

Я опираюсь на текущее состояние репозитория: сейчас CMakeLists.txt собирает BunkerGame, BunkerLauncher, BunkerEditor и BunkerSmokeChecks; в проект уже вынесены GameplayDescriptorRegistry.cpp, WorldValidation.cpp, SessionProfiles.cpp, GameRuntimePipPad.cpp, LauncherSupport.cpp, EditorSupport.cpp. При этом main.cpp всё ещё делает прямые world.Save(...) и SaveSessionProfile(...) при создании мира, по F5 и на выходе, а Editor_Main.cpp в LoadOrCreateEditorWorld(...) тоже сохраняет мир напрямую через world.Save(path.string()); в Editor_Main.cpp по текущему поиску не видно вызова ValidateWorldForRuntime(...). В то же время BunkerSmokeChecks уже проверяет SaveWorldAtomically(...), SaveProfileAtomically(...), alias-normalization и world validation.

MASTER_FILE_PART_3__FILE_BY_FILE_INTEGRATION_PLAN
0. Главный принцип

Сейчас проект уже не надо придумывать заново.
Его надо довести до состояния, где уже созданные модули реально используются в runtime/editor/launcher, а не просто существуют рядом.

Правильный порядок такой:

src/main.cpp
Editor/src/Editor_Main.cpp
src/GameRuntime.cpp
src/GameRuntimePipPad.cpp
src/SessionProfiles.cpp
src/GameplayDescriptorRegistry.cpp
src/WorldValidation.cpp
потом уже Launcher/src/LauncherSupport.cpp и мелкие выравнивания
1. src/main.cpp
Что этот файл делает сейчас

main.cpp у тебя уже держит весь боевой вход в игру: launch gate через ticket, загрузку/нормализацию профиля, world load/generate, starter infrastructure, story/world sync, runtime update loop, input, F5-save и финальное сохранение при выходе. При этом по текущему коду он всё ещё напрямую вызывает world.Save(worldPath.string()) и bunker::SaveSessionProfile(sessionProfile, profilePath) при первичном создании мира, на F5 и на shutdown.

Что в нём уже хорошо

Этот файл уже соединяет почти весь живой runtime-контур:

launch ticket/session state,
world generation/load,
starter infrastructure,
story/world metadata sync,
long recovery/infrastructure update chain,
UI/game loop.
То есть это не “плохой файл”, а правильный узел интеграции, просто в нём ещё остались старые save-paths.
Главная проблема

Самая дорогая проблема здесь очень конкретная:
safe persistence уже написан и даже покрыт smoke-check, но main.cpp ещё живёт на прямом save-flow. Это создаёт разрыв между тем, что проект уже умеет, и тем, как реально сохраняется игровая сессия.

Что именно сюда встраивать

В main.cpp нужно встроить 4 вещи:

1. Полный перевод на атомарные сейвы

Нужно убрать прямые вызовы:

world.Save(...)
SaveSessionProfile(...)

и заменить их на:

SaveWorldAtomically(...)
SaveProfileAtomically(...)

в трёх местах:

первичное создание мира,
F5-save,
финальное сохранение перед выходом.
Сейчас именно эти места ещё прямые.
2. Согласованное сообщение об успехе/ошибке

Сейчас по F5 выставляется "Field save committed." сразу после прямых save-вызовов. Нужно, чтобы это сообщение появлялось только если оба сейва успешны. Если нет — показывать составной failure message: world/profile.

3. Runtime readiness report

Сюда хорошо ложится маленькая сводка состояния runtime перед входом в основной loop:

мир загружен / сгенерирован,
профиль валиден,
стартовая инфраструктура обеспечена,
launch ticket применён,
static eraser подтянут.

Это не обязано быть отдельным окном. Достаточно внутренней структуры и логирования/статуса.

4. Один “источник правды” по runtime bootstrap

Сейчас в main.cpp сидит всё сразу. Его не надо переписывать сейчас целиком, но нужно выделить логические блоки:

LoadOrCreateRuntimeProfile
LoadOrCreateRuntimeWorld
InitializeRuntimePlayer
InitializeRuntimeSessionState

Даже если сначала это будут просто static функции внутри того же файла.

Минимальные структуры, которые потом сюда хорошо встанут
struct RuntimeBootstrapReport {
    bool profileLoaded = false;
    bool worldLoaded = false;
    bool worldGenerated = false;
    bool starterInfrastructureReady = false;
    bool launchTicketApplied = false;
    bool staticEraserLoaded = false;
    std::string summary;
};

struct SaveOutcome {
    bool worldSaved = false;
    bool profileSaved = false;
    bool eraserSaved = false;
    std::string message;
};
Что нельзя делать в этом файле

Не надо сейчас:

тащить сюда validation editor-уровня,
добавлять новые фичи UI,
расширять launcher logic,
делать большой рефакторинг game loop.

Задача main.cpp сейчас одна: сделать runtime надёжным и согласованным.

Критерий готовности

main.cpp считается доведённым, когда:

прямых world.Save(...) в нём больше нет,
прямых SaveSessionProfile(...) в нём больше нет,
F5 показывает успех только после двух успешных атомарных сейвов,
финальный shutdown не пишет “успех”, если что-то не сохранилось.
2. Editor/src/Editor_Main.cpp
Что этот файл делает сейчас

Editor_Main.cpp остаётся большим центром editor-flow. В нём уже есть LoadOrCreateEditorWorld(...); если runtime-world не найден, он делает GeneratePrototypeZone() и затем сразу world.Save(path.string()). По текущему поиску в этом файле не видно вызова ValidateWorldForRuntime(...).

Что здесь уже хорошо

У editor уже есть:

загрузка runtime-world через session profile,
создание workspace при отсутствии мира,
большой authoring loop,
связь с active world.
Проблема не в том, что editor слабый. Проблема в том, что последний барьер безопасности ещё не стал обязательным.
Что именно сюда встраивать
1. Атомарное создание мира в LoadOrCreateEditorWorld(...)

Сейчас в этом месте прямой world.Save(path.string()). Его надо заменить на SaveWorldAtomically(...). Это самый быстрый и дешёвый выигрыш.

2. Обязательный validation-gate перед сохранением и экспортом

Поскольку WorldValidation.cpp уже умеет:

ловить пустые и дублирующиеся registryId,
предупреждать о legacy aliases,
проверять mismatch по interaction/category,
ловить обязательный linkTarget,
ругаться на unknown script tags,
предупреждать про transitions без target,
редактор должен вызывать ValidateWorldForRuntime(world) перед save/export и выдавать summary через BuildValidationSummary(...).
3. Validation panel в UI

Нужна маленькая, но обязательная панель:

Errors: N
Warnings: M
текущий summary
список первых 10–20 issues
jump to selected object по objectId
4. Save policy

Правило editor-а должно стать таким:

ошибки validation блокируют export/runtime-save,
предупреждения не блокируют, но показываются явно,
status text всегда сообщает, что произошло:
validation failed,
saved with warnings,
saved successfully.
Минимальные структуры, которые потом сюда хорошо встанут
struct EditorValidationState {
    std::vector<bunker::ValidationIssue> issues;
    int errorCount = 0;
    int warningCount = 0;
    std::string summary;
    std::string selectedIssueObjectId;
};

struct EditorSavePolicy {
    bool blockOnErrors = true;
    bool allowWarnings = true;
    bool autoValidateBeforeSave = true;
};
Что нельзя делать здесь

Не надо сейчас:

строить гигантскую undo/redo систему,
делать новый editor framework,
переносить весь Editor_Main.cpp по файлам до интеграции validation.

Сначала editor должен стать безопасным authoring-gate, а уже потом более красивым.

Критерий готовности

Editor_Main.cpp считается доведённым, когда:

LoadOrCreateEditorWorld(...) не делает прямой world.Save(...),
есть обязательный вызов ValidateWorldForRuntime(...) перед save/export,
editor умеет показать summary и список validation issues.
3. src/GameRuntime.cpp
Что этот файл делает сейчас

Это по-прежнему один из самых тяжёлых runtime-файлов. В нём виден большой recovery/backbone контур и остаётся длинная DescribeTerminalSync(...) цепочка по scriptTag: tower_sync, power_pylon, drone_station, rail_freight, orbital_uplink, rail_fortress, recovery_fabricator, industrial_gate, industrial_survey, industrial_outpost, assembly_cell и дальше.

Почему именно он следующий после editor

Потому что здесь есть архитектурный разрыв:
GameplayDescriptorRegistry.cpp уже канонизировал дескрипторы и alias-ы, но runtime-сообщения всё ещё держатся на отдельной ручной цепочке. Более того, в registry канонические имена уже rail_depot и rail_fortress_hub, а alias-ы rail_freight и rail_fortress нормализуются отдельно. Значит, runtime-сообщения сейчас живут чуть в стороне от канонического словаря.

Что именно сюда встраивать
1. Table-driven terminal sync text

Нужно заменить ручную if-цепочку DescribeTerminalSync(...) на таблицу, основанную на:

каноническом scriptTag,
normalize alias → canonical,
lookup текста синка по каноническому тегу.

Это сразу даст:

меньше шансов потерять новую authored-фичу,
меньше расхождения между editor/runtime/validation,
лёгкое расширение industrial chain.
2. Interaction dispatch через registry

Следующий шаг после terminal sync text — не делать всё ad-hoc по строкам, а завести table-driven mapping:

scriptTag
optional action handler
optional status text
optional progression flag mutation
3. Разделить “описание” и “эффект”

Сейчас часто одно и то же место отвечает и за смысл terminal-а, и за текст события. Надо явно разделить:

DescribeTerminalSync(tag)
ApplyTerminalSyncEffects(tag, profile, gameState, worldState)

Это даст ясность и для regression-тестов, и для будущих factory/knowledge/assistant extensions.

Минимальные структуры, которые потом сюда хорошо встанут
struct TerminalSyncSpec {
    std::string_view scriptTag;
    const char* syncText;
    bool affectsProgression = false;
    bool affectsWorldState = false;
};

struct TerminalSyncActionResult {
    std::string statusText;
    bool progressionAdvanced = false;
    bool worldStateChanged = false;
};
Дополнительный важный шаг

Когда будешь это трогать, сразу переводи на канонические теги, а alias оставляй только на уровне normalization.
То есть runtime должен знать не rail_freight, а rail_depot; не rail_fortress, а rail_fortress_hub. Это уже заложено в registry.

Что нельзя делать здесь

Не надо сейчас:

переписывать весь GameRuntime.cpp,
выносить все update-функции сразу,
трогать combat/weather/story одновременно.

Нужен один очень конкретный результат:
runtime должен перестать жить на ручных string-if цепочках там, где уже есть registry.

4. src/GameRuntimePipPad.cpp
Что этот файл уже умеет

Здесь уже живой DATA/archive слой: есть DATA / Archive Summary, статусы Archive Sync, Relay Packet, Debrief Record, счётчик archive sync awakenings, damaged/reconstructed carriers, список архивных носителей, выбор активной кассеты. Здесь же виден tank utility слой: Tow Coupler Mk.I, переключение utility-модуля, подпись Tow Coupler: +logistics, -mobility, +thermal load.

Чего в нём пока не хватает

По текущему осмотру здесь уже хорошо живут archive/tape и tank UI, но не виден явный отдельный music-control hook, а infrastructure/backbone не собран в отдельный сильный экран. Поэтому именно сюда идеально ложатся три вещи:

общий Knowledge слой,
infrastructure screen,
assistant/mode panel.
Что именно сюда встраивать
1. Расширить DATA из Tape UI в Knowledge UI

Не ломая текущий archive-экран, надо расширить его до общего экрана знаний:

Archive
Data Cards
Relay
Debrief
Echo

Текущие tapes остаются, но становятся частным случаем более широкой системы.

2. Добавить Infrastructure screen

Отдельная вкладка или подпанель:

Tower/Grid
Logistics
Fabrication
Relay/Services
Water/Decay/Erosion

Данные для неё уже есть в world field state persistence и в runtime update chain. В SessionProfiles.cpp сериализуется много world infrastructure флагов: towerSyncRecovered, localRelayAvailable, regionalGridOnline, caravanRouteActive, droneStationsActive, tradeNetworkActive, railFreightActive, orbitalUplinkActive, railFortressActive, recoveryFabricatorActive, industrialGateUnlocked и счётчики циклов/прогресса.

3. Music bonus как лёгкое расширение tape-layer

Раз отдельный Play Track-hook сейчас не виден, это место лучшее для его возврата:

если carrier музыкальный и не повреждён,
появляется кнопка Play,
в profile/runtime активируется небольшой временный бонус.
4. Assistant mode panel

Ещё одна компактная секция:

текущий режим assistant-а,
last advice,
suggested next node,
warning summary.
Минимальные структуры, которые потом сюда хорошо встанут
enum class PipDataCategory {
    Archive,
    DataCard,
    RelayPacket,
    Debrief,
    EchoTrace
};

struct PipInfrastructureSnapshot {
    bool towerOnline = false;
    bool relayOnline = false;
    bool logisticsOnline = false;
    bool fabricationOnline = false;
    bool medicalOnline = false;
    float etherErosion = 0.0f;
    float infrastructureDecay = 0.0f;
    int recoveryIndex = 0;
};

struct ActiveMusicBonus {
    std::string sourceId;
    std::string effectId;
    float remainingSeconds = 0.0f;
    bool active = false;
};
Что нельзя делать здесь

Не надо делать новый mega-UI.
Надо усилить уже существующий Pip-Pad как главный экран системной обратной связи.

Критерий готовности

GameRuntimePipPad.cpp считается доведённым на этом этапе, когда:

DATA уже не только tapes, а общий knowledge-layer,
есть Infrastructure screen,
есть место для assistant advice,
music bonus не потерян в общей archive-свалке.
5. src/SessionProfiles.cpp
Что этот файл уже делает

Он уже хранит и сериализует много критических слоёв проекта, включая world_field=... c длинным списком параметров мира: etherErosion, infrastructureDecay, towerSyncRecovered, localRelayAvailable, regionalGridOnline, caravanRouteActive, droneStationsActive, tradeNetworkActive, railFreightActive, orbitalUplinkActive, railFortressActive, recoveryFabricatorActive, industrialGateUnlocked, contamination и счётчики циклов.

Почему он ключевой для следующего шага

Почти всё, что ты хочешь вернуть или усилить, упрётся именно сюда:

knowledge/data cards,
active music bonus,
assistant state,
consciousness line,
richer infrastructure summary.

Именно поэтому SessionProfiles.cpp надо не ломать, а аккуратно расширять как центральный persistent contract.

Что именно сюда встраивать
1. Knowledge carriers

Текущие collectedTapes надо постепенно обобщить до более широкого persistent набора знаний. Не обязательно сразу удалять tapes — можно сделать новый слой рядом, а потом мигрировать.

2. Active music bonus

Если music bonus вернётся, его состояние должно жить здесь:

source id,
effect id,
remaining time,
active flag.
3. Assistant state

Сюда хорошо ложится:

current assistant mode,
enabled flag,
cockpit link,
maybe short persistent memory / seen hints.
4. Consciousness line

Это тоже persistent-слой, а не UI-слой. Значит, SessionProfiles.cpp — правильное место.

5. Future region/chunk contract

Если позже будешь добавлять region/chunk thinking, сюда можно хранить per-region state, не ломая текущий per-world save format.

Минимальные структуры, которые потом сюда хорошо встанут
struct KnowledgeCarrier {
    std::string id;
    std::string title;
    std::string kind;
    bool damaged = false;
    bool reconstructed = false;
    bool consumed = false;
    std::string rewardType;
    std::string rewardKey;
};

enum class AssistantMode {
    Navigation,
    Engineering,
    Tactical,
    Recovery,
    Archive
};

struct AssistantState {
    AssistantMode mode = AssistantMode::Navigation;
    bool enabled = true;
    bool cockpitLinked = false;
    std::string lastAdvice;
};

struct ConsciousnessLineState {
    float resonance = 0.0f;
    int diaryEvents = 0;
    int archiveMoments = 0;
    int socialLinks = 0;
    int turningPoints = 0;
};
Важное правило

Все новые persistent сущности надо добавлять так, чтобы:

старые профили продолжали загружаться,
отсутствие нового поля не ломало load,
normalize step мог мягко достраивать default values.

Это уже согласуется с тем, как у тебя работают profile roundtrip и smoke-check на normalize/migration logic.

6. src/GameplayDescriptorRegistry.cpp
Что в нём уже есть

Сейчас это уже сильный канонический словарь. В нём есть descriptor specs для archive_sync, terminal_sync, remote_link, workshop_service, tower_sync, power_pylon, drone_station, rail_depot, orbital_uplink, rail_fortress_hub, recovery_fabricator, industrial chain, lanline_service_hub, tank_service, medical_support, fey_ring, echo_trace, specialist_cryo, плюс alias-ы radio_tower, workshop_field_service, rail_freight, rail_fortress.

Что сюда надо добавить

Именно сюда должны первым делом попадать все новые канонические authored сущности, которые ты хочешь вернуть в активный код:

Возвратимые сущности
data_card
relay_packet
debrief_record
music_tape
assistant_anchor
region_gate
возможно factory_console

Причём сначала именно сюда, а не сразу в runtime.

Почему это критично

Если новая authored-фича появится вне registry, она:

не будет известна validation,
не будет известна editor presets,
не будет известно, нужен ли linkTarget,
не будет иметь единый canonical tag.
Что делать здесь дальше

Нужно расширить spec не только до “предпочтительный interaction/category”, но и до семантики уровня UI/validation/progression.

Минимальная расширенная модель
struct GameplayDescriptorSpec {
    std::string_view scriptTag;
    std::string_view label;
    InteractionType preferredInteraction;
    ObjectCategory preferredCategory;
    bool requiresLinkTarget = false;
    bool appearsInEditorDrafts = false;
    bool appearsInPipPad = false;
    bool contributesToProgression = false;
};
Критерий готовности

Ни одна новая gameplay-authored сущность не должна появляться в мире, пока её нет в registry.

7. src/WorldValidation.cpp
Что в нём уже есть

Validation уже:

проверяет registryId,
дубликаты,
пустой displayName,
битые registry-like linkTarget,
legacy alias warnings,
mismatch interaction/category,
missing required linkTarget,
unknown scriptTag,
transition without linkTarget.
Что сюда встраивать следующим

Когда начнёшь возвращать новые фичи, сюда надо первым делом добавлять:

1. Knowledge/Data rules
data_card должен иметь ожидаемый reward metadata
music_tape должен иметь effect id
relay_packet должен быть terminal/archive-compatible
debrief_record должен иметь title/body or source ref
2. Region/chunk rules
если объект имеет regionId, он должен ссылаться на существующий регион
transitions между регионами должны иметь linkTarget + target region
3. Assistant anchor rules
если есть assistant_anchor, он должен указывать на корректный semantic target
Главное правило

Validation должен расти вместе с registry, а не после него.

8. Launcher/src/LauncherSupport.cpp и Launcher_Main.cpp
Почему они не первые

Launcher у тебя уже близок к рабочему состоянию, и главное сейчас — не там. CMakeLists.txt показывает, что launcher уже выделен с LauncherSupport.cpp как отдельной частью, а основная боль по интеграции сейчас у runtime/editor/save-flow.

Что туда потом встраивать

Когда добьёшь main/editor/runtime:

readiness report,
demo-world summary,
validation-safe launch notice,
session/world consistency summary,
future assistant/session hints.

То есть launcher уже не надо спасать, его надо потом чуть усилить как операционный entry shell.

9. Реальный порядок внедрения по дням/ходам
Ход 1

src/main.cpp

atomics everywhere
honest save result
bootstrap helpers
Ход 2

Editor/src/Editor_Main.cpp

atomics in LoadOrCreateEditorWorld
validation summary
block export on errors
Ход 3

src/GameRuntime.cpp

table-driven DescribeTerminalSync
canonical tags only
split description/effect
Ход 4

src/GameRuntimePipPad.cpp

infrastructure screen
knowledge generalization
assistant mode panel
Ход 5

src/SessionProfiles.cpp

knowledge carrier persistence
active music bonus
assistant state
consciousness line
Ход 6

src/GameplayDescriptorRegistry.cpp + src/WorldValidation.cpp

register new tags
validation rules for them
10. Самая важная мысль

Сейчас проект уже не надо спасать.
Его надо собрать в одно целое.

То есть твоя следующая фаза — не “новые идеи”, а:

уже написанные модули сделать обязательными,
уже придуманные канонические фичи вернуть в активный runtime/editor,
уже существующие системы сделать видимыми игроку через Pip-Pad и demo-route.

Это особенно видно по текущему состоянию: registry и validation уже есть, smoke-check уже есть, world state persistence уже богатый, archive/tape UI уже живой, Tow Coupler уже живой, Lanline shell уже живой — но часть этого ещё не доведена до единого интеграционного контура.

Ниже даю четвёртую часть мастер-файла — уже как единый канон структур, enum’ов и контрактов, который можно сейчас кинуть в один временный файл, а потом спокойно разнести по SessionProfiles, GameplayDescriptorRegistry, WorldValidation, GameRuntime, GameRuntimePipPad, Editor и Launcher.

Я специально строю это не в отрыве от проекта: сейчас у тебя уже есть отдельные модули для registry, validation, session profiles, Pip-Pad, editor/launcher support и smoke-checks, а roadmap фиксирует, что vertical slice и большой набор систем уже реально живут в проекте — от Pip-Pad, Data Reconstruction, Tow Coupler, Field Service, SoulLine, Lanline Services, Power Grid, Rail Freight, Orbital Uplink, Recovery Fabricator и дальше. Поэтому ниже не “новая игра поверх старой”, а единый типовой слой для уже существующей архитектуры.

MASTER_FILE_PART_4__CORE_TYPES_AND_ENUMS
0. Зачем нужен этот файл

Этот файл нужен как единый временный канон, чтобы:

не плодить сущности под разными именами,
не терять фичи между md, runtime, editor и save/load,
заранее держать форму будущих систем,
потом разнести всё это по коду без переизобретения терминов.

Это особенно важно сейчас, потому что проект уже прошёл стадию “просто собрать всё в один giant file” и перешёл к стадии интеграции: у тебя есть отдельные GameplayDescriptorRegistry, WorldValidation, SessionProfiles, GameRuntimePipPad, LauncherSupport, EditorSupport, но часть канонических идей ещё живёт сразу в нескольких местах и требует общего словаря.

1. Общие правила именования и смысла
1.1. Один смысл — одно имя

Если сущность уже имеет каноническое имя, не вводить второе имя ради “удобства”.

Например:

если канонический тег rail_depot, то rail_freight допустим только как alias;
если канонический слой KnowledgeCarrier, то Tape — это subtype, а не отдельная несвязанная система;
если канонический слой AssistantState, то Nerv может быть display/lore label, но не отдельный второй persistent-контракт.
1.2. Любая новая gameplay-сущность обязана иметь:
canonical tag,
descriptor contract,
validation rules,
persistence contract, если она влияет на долгую сессию,
UI surface, если игрок должен её видеть.
1.3. Любая временная сущность должна быть явно отмечена

Нельзя оставлять безымянные “потом переделаю” состояния.

2. Канон базовых идентификаторов
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace bunker {

using RegistryId = std::string;
using WorldId = std::string;
using RegionId = std::string;
using DescriptorTag = std::string;
using CharacterId = std::string;
using SessionId = std::string;
using KnowledgeId = std::string;
using EffectId = std::string;
using ObjectiveId = std::string;
using SpecialistId = std::string;
using ServiceNodeId = std::string;

enum class CanonicalState : std::uint8_t {
    Unknown = 0,
    Draft,
    Active,
    Deprecated,
    AliasOnly
};

} // namespace bunker
Зачем это нужно

Сейчас у тебя уже есть миры, профили, сессионные состояния, registry ids, descriptor tags, world metadata и authored-сущности. Лучше заранее держать простые типовые названия, чтобы дальше не смешивать “id объекта”, “id мира”, “scriptTag”, “alias” и “UI label”. Это особенно полезно, потому что registry/validation уже стали отдельными модулями и дальше будут только расти.

3. Канон descriptor-слоя
namespace bunker {

enum class DescriptorDomain : std::uint8_t {
    Unknown = 0,
    Interaction,
    Infrastructure,
    Knowledge,
    Service,
    Transport,
    Recovery,
    Story,
    Lanline,
    Specialist,
    Transition
};

enum class DescriptorVisibility : std::uint8_t {
    Hidden = 0,
    EditorOnly,
    RuntimeOnly,
    Shared
};

struct DescriptorAlias {
    std::string legacyTag;
    std::string canonicalTag;
};

struct GameplayDescriptorSpec {
    DescriptorTag scriptTag;
    std::string label;
    DescriptorDomain domain = DescriptorDomain::Unknown;
    DescriptorVisibility visibility = DescriptorVisibility::Shared;

    int preferredInteraction = 0;
    int preferredCategory = 0;

    bool requiresLinkTarget = false;
    bool contributesToProgression = false;
    bool contributesToInfrastructure = false;
    bool contributesToKnowledge = false;
    bool appearsInPipPad = false;
    bool appearsInEditorDrafts = false;
    bool canBeValidationBlocking = false;
};

} // namespace bunker
Почему это полезно

У тебя уже есть вынесенный GameplayDescriptorRegistry, legacy alias support и validation, а roadmap отдельно фиксирует descriptor-presets, tower_sync / remote_link flow и semantic authoring fields. Значит, дальше descriptor надо мыслить не только как “tag + preferred interaction”, а как центр семантики для runtime, editor и validation.

4. Канон knowledge/data-слоя

Это один из главных кусков, который я бы закладывал заранее, чтобы не потерять Data Reconstruction, архивы, кассеты, data cards, relay packets и debrief-записи.

namespace bunker {

enum class KnowledgeCarrierType : std::uint8_t {
    Unknown = 0,
    Tape,
    DataCard,
    RelayPacket,
    DebriefRecord,
    ArchiveSync,
    EchoTrace,
    TerminalRecord
};

enum class KnowledgeRewardType : std::uint8_t {
    None = 0,
    Lore,
    Recipe,
    InfrastructureUnlock,
    SpecialistUnlock,
    ObjectiveProgress,
    MapIntel,
    MusicBonus,
    AssistantHint
};

enum class KnowledgeCondition : std::uint8_t {
    Intact = 0,
    Damaged,
    Corrupted,
    Reconstructed,
    Consumed
};

struct KnowledgeReward {
    KnowledgeRewardType type = KnowledgeRewardType::None;
    std::string rewardKey;
    int amount = 0;
};

struct KnowledgeCarrier {
    KnowledgeId id;
    std::string title;
    std::string sourceTag;
    KnowledgeCarrierType type = KnowledgeCarrierType::Unknown;
    KnowledgeCondition condition = KnowledgeCondition::Intact;

    bool discovered = false;
    bool consumed = false;
    bool replayable = false;
    bool grantsProgression = false;

    std::string summary;
    std::string body;
    std::string mediaKey;
    std::vector<KnowledgeReward> rewards;
};

struct KnowledgeLedger {
    std::vector<KnowledgeCarrier> entries;
    std::optional<KnowledgeId> selectedId;
    int reconstructedCount = 0;
    int consumedCount = 0;
};

} // namespace bunker
Зачем это нужно

Roadmap уже фиксирует Data Reconstruction, Pip-Pad AR / Echo Trace, cassette player / music bonus и общий archive/data-контур, а в текущем проекте Pip-Pad уже является системной точкой обратной связи. Значит, правильнее не плодить отдельные несвязанные мини-системы, а собрать их в один knowledge-layer, где tapes — это лишь один из типов носителей.

5. Канон music/cassette bonus
namespace bunker {

enum class MusicEffectType : std::uint8_t {
    None = 0,
    SteadyHands,
    SalvageFocus,
    MarchDrive,
    EchoResolve,
    ThermalDiscipline,
    FieldRecovery
};

struct MusicEffectSpec {
    EffectId id;
    std::string label;
    MusicEffectType type = MusicEffectType::None;
    float durationSeconds = 0.0f;
    float magnitude = 0.0f;
};

struct ActiveMusicBonus {
    KnowledgeId sourceKnowledgeId;
    EffectId effectId;
    std::string displayName;
    float remainingSeconds = 0.0f;
    bool active = false;
};

} // namespace bunker
Почему отдельно

Roadmap прямо отмечает ранний cassette player / music bonus слой, но если не закрепить его как отдельный контракт, он легко растворится внутри общего tape/archive UI. Лучше сразу задать форму.

6. Канон assistant / Nerv / cockpit guidance
namespace bunker {

enum class AssistantMode : std::uint8_t {
    Navigation = 0,
    Engineering,
    Tactical,
    Recovery,
    Archive,
    Lanline
};

enum class AssistantUrgency : std::uint8_t {
    Low = 0,
    Medium,
    High,
    Critical
};

struct AssistantAdvice {
    std::string id;
    AssistantMode mode = AssistantMode::Navigation;
    AssistantUrgency urgency = AssistantUrgency::Low;
    std::string title;
    std::string body;
    std::string relatedTargetId;
    bool acknowledged = false;
};

struct AssistantState {
    AssistantMode currentMode = AssistantMode::Navigation;
    bool enabled = true;
    bool cockpitLinked = false;
    bool canAutocycleHints = true;
    std::string displayName = "Nerv";
    std::optional<std::string> activeAdviceId;
    std::vector<AssistantAdvice> adviceLog;
};

} // namespace bunker
Почему это закладывать сейчас

Даже если полноценный Nerv как большая система ещё не введён, у тебя уже есть Pip-Pad, BT-72, infrastructure chain, Lanline, archive/data core и objective route. На этом фоне assistant mode panel можно очень естественно встроить как поверхностный слой советов без дорогой “настоящей ИИ-системы”.

7. Канон infrastructure/read-model

Это очень важный блок, потому что большая часть проекта уже вращается вокруг восстановления инфраструктуры, но игроку нужен единый snapshot состояния.

namespace bunker {

enum class InfrastructureNodeType : std::uint8_t {
    Unknown = 0,
    TowerSync,
    PowerPylon,
    DroneStation,
    TradeNetwork,
    RailDepot,
    OrbitalUplink,
    RailFortressHub,
    RecoveryFabricator,
    IndustrialGate,
    IndustrialSurvey,
    IndustrialOutpost,
    AssemblyCell,
    FoundryLine,
    ReactorYard,
    CapacitorBank,
    RelaySubstation,
    ServiceBay,
    WaterReclaimer,
    LanlineServiceHub,
    TankService,
    MedicalSupport,
    SpecialistCryo
};

enum class InfrastructureNodeState : std::uint8_t {
    Offline = 0,
    Available,
    Active,
    Blocked,
    Damaged,
    Degraded
};

struct InfrastructureNodeSnapshot {
    InfrastructureNodeType type = InfrastructureNodeType::Unknown;
    DescriptorTag sourceTag;
    RegistryId objectId;
    InfrastructureNodeState state = InfrastructureNodeState::Offline;
    std::string label;
    std::string note;
    bool discovered = false;
    bool contributesToRecoveryIndex = false;
};

struct InfrastructureReadModel {
    std::vector<InfrastructureNodeSnapshot> nodes;

    bool regionalGridOnline = false;
    bool localRelayAvailable = false;
    bool towerSyncRecovered = false;
    bool caravanRouteActive = false;
    bool droneStationsActive = false;
    bool tradeNetworkActive = false;
    bool railFreightActive = false;
    bool orbitalUplinkActive = false;
    bool railFortressActive = false;
    bool recoveryFabricatorActive = false;
    bool industrialGateUnlocked = false;

    float etherErosion = 0.0f;
    float infrastructureDecay = 0.0f;
    float routeContamination = 0.0f;
    int shelterRecoveryIndex = 0;
};

} // namespace bunker
Почему это особенно важно

Roadmap уже фиксирует большой длинный backbone: Power Grid, Power Pylon, Drone Stations, Trade Network, Route Contamination, Rail Freight Link, Orbital Uplink, Rail Fortress, Recovery Fabricator, Shelter Recovery Index и дальше. Если не сделать единый read-model, это всё будет чувствоваться как набор отдельных флагов, а не как восстановление системы мира.

8. Канон BT-72 / tank / utility / field-service
namespace bunker {

enum class TankUtilityModuleType : std::uint8_t {
    None = 0,
    TowCoupler,
    BucketShield,
    FieldRig,
    ThermalKit,
    ServiceRig
};

enum class TankServiceState : std::uint8_t {
    Nominal = 0,
    NeedsService,
    Critical,
    Offline
};

struct TankUtilityModuleState {
    TankUtilityModuleType type = TankUtilityModuleType::None;
    std::string moduleId;
    std::string label;
    bool installed = false;
    bool unlocked = false;
};

struct TankFieldServiceRecord {
    bool performed = false;
    float serviceQuality = 0.0f;
    float lastServiceWorldTime = 0.0f;
    std::string performedAtNodeId;
};

struct TankRuntimeExtensionState {
    std::vector<TankUtilityModuleState> utilityModules;
    TankServiceState serviceState = TankServiceState::Nominal;
    TankFieldServiceRecord fieldService;
    bool wirelessChargingAvailable = false;
    bool soulSyncLinked = false;
    bool soulLineReady = false;
    bool thermalModeUnlocked = false;
};

} // namespace bunker
Почему это прямо надо держать отдельно

Roadmap уже перечисляет wireless tank charging, Field Service, SoulSync / Sync-Link, SoulLine, Thermal Mode, Tow Coupler, field modification utility-slot и дальше. Это не “мелкие фичи”, а отдельная ось идентичности проекта, и её лучше держать как самостоятельный контракт, а не как россыпь bool-полей.

9. Канон world progression / shelter / recovery
namespace bunker {

enum class RecoveryMilestoneType : std::uint8_t {
    None = 0,
    FirstTowerOnline,
    PowerBackboneOnline,
    CaravanStabilized,
    DroneNetOnline,
    TradeNetEstablished,
    RailRouteEstablished,
    OrbitalSyncEstablished,
    FortressRouteEstablished,
    FabricationOnline,
    WaterSecurityEstablished
};

struct RecoveryMilestone {
    RecoveryMilestoneType type = RecoveryMilestoneType::None;
    bool completed = false;
    std::string completedByWorld;
    std::string completedByNode;
};

struct ShelterRecoveryState {
    int recoveryIndex = 0;
    int doctrineTier = 0;
    int assignedSpecialists = 0;
    int awakenedRecipes = 0;
    bool forwardCampUnlocked = false;
    bool scavengerTeamsActive = false;
    bool autopilotCaravanUnlocked = false;
    std::vector<RecoveryMilestone> milestones;
};

} // namespace bunker
Почему это логично

В roadmap уже есть Forward Camp, Scavenger Team, Awakened Recipes, Cryo Specialists, Shelter Doctrine, Autopilot Caravan, Shelter Recovery Index. Значит, shelter/recovery слой уже достаточно зрелый, чтобы быть не только россыпью story flags, а отдельной persistent/domain-моделью.

10. Канон consciousness / memory / resonance
namespace bunker {

enum class ResonanceEventType : std::uint8_t {
    None = 0,
    DiaryRead,
    ArchiveRecovered,
    EchoExperienced,
    SocialBondFormed,
    SurvivalTurningPoint,
    AssistantDeepLink,
    DebriefProcessed
};

struct ResonanceEvent {
    ResonanceEventType type = ResonanceEventType::None;
    std::string sourceId;
    float value = 0.0f;
};

struct ConsciousnessLineState {
    float resonance = 0.0f;
    int diaryEvents = 0;
    int archiveMoments = 0;
    int socialLinks = 0;
    int survivalTurningPoints = 0;
    std::vector<ResonanceEvent> eventLog;
};

} // namespace bunker
Почему это стоит заложить заранее

Это как раз тот слой, который проще всего потерять, потому что он не обязателен для сборки, но очень важен для атмосферы и identity. Если его не зафиксировать как типовой контракт, он так и останется красивой идеей в md, а не будущей системой.

11. Канон region / chunk / streaming-prep

Ты ещё не обязан делать реальный streaming прямо сейчас. Но контракт на данные стоит заложить уже сейчас.

namespace bunker {

struct RegionDescriptor {
    RegionId regionId;
    std::string label;
    bool isInterior = false;
    bool supportsStreaming = false;
    bool isCriticalDemoRegion = false;
};

struct RegionLink {
    RegionId fromRegion;
    RegionId toRegion;
    RegistryId viaObjectId;
    bool bidirectional = true;
};

struct RegionRuntimeState {
    RegionId regionId;
    bool loaded = false;
    bool visited = false;
    bool hasCheckpoint = false;
};

} // namespace bunker
Почему это сейчас уместно

Канон мира уже давно просит region/chunk мышление, а в vertical slice и authored transitions у тебя уже есть маршруты, gates, fey_ring, industrial chain и world links. Значит, можно пока не делать streaming subsystem, но уже перестать мыслить мир как бесформенный плоский список объектов.

12. Канон demo / readiness / diagnostics

Это не обязательно игровая механика, но очень полезный инженерный слой.

namespace bunker {

enum class ReadinessSeverity : std::uint8_t {
    Ok = 0,
    Warning,
    Blocking
};

struct ReadinessIssue {
    ReadinessSeverity severity = ReadinessSeverity::Ok;
    std::string code;
    std::string message;
    std::string relatedId;
};

struct DemoReadinessReport {
    bool launcherReady = false;
    bool gameReady = false;
    bool editorReady = false;
    bool validationReady = false;
    bool saveFlowReady = false;
    bool worldRouteReady = false;
    bool pipPadReady = false;
    bool lanlineReady = false;
    std::vector<ReadinessIssue> issues;
};

struct WorldIntegrationReport {
    int objectCount = 0;
    int gameplayTaggedObjects = 0;
    int invalidObjects = 0;
    int warningObjects = 0;
    bool hasStarterTower = false;
    bool hasTankAnchor = false;
    bool hasServiceNode = false;
    bool hasRecoveryNode = false;
    bool hasLanlineNode = false;
};

} // namespace bunker
Почему это надо

У тебя уже есть BunkerSmokeChecks, safe persistence, validation, launcher/game/editor split и подтверждённые сборки. Следующий взрослый шаг — иметь не только “собралось”, но и понятный интеграционный диагноз для demo-ready состояния.

13. Канон validation-домена
namespace bunker {

enum class ValidationSeverity : std::uint8_t {
    Info = 0,
    Warning,
    Error,
    Blocking
};

enum class ValidationDomain : std::uint8_t {
    Unknown = 0,
    Registry,
    Authoring,
    Descriptor,
    LinkGraph,
    RegionGraph,
    Knowledge,
    Infrastructure,
    Persistence
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Info;
    ValidationDomain domain = ValidationDomain::Unknown;
    std::string code;
    RegistryId objectId;
    std::string message;
};

struct ValidationSummary {
    int infos = 0;
    int warnings = 0;
    int errors = 0;
    int blocking = 0;
    std::string label;
};

} // namespace bunker
Почему это не лишнее

У тебя validation уже выделен в отдельный модуль и реально проверяет мир. Следующий логичный шаг — сделать его не просто “списком строк”, а нормальным typed-результатом, который одинаково понятен editor-у, tests и будущим диагностическим панелям.

14. Канон persistence/version/migration
namespace bunker {

enum class PersistenceObjectType : std::uint8_t {
    Unknown = 0,
    SessionProfile,
    WorldState,
    LanlineSnapshot,
    EditorWorkspace
};

struct PersistenceVersion {
    int major = 1;
    int minor = 0;
    int patch = 0;
};

struct MigrationNote {
    std::string fromKey;
    std::string toKey;
    std::string description;
};

struct PersistenceContract {
    PersistenceObjectType type = PersistenceObjectType::Unknown;
    PersistenceVersion version;
    bool supportsLegacyAliases = false;
    bool usesAtomicWrite = false;
    std::vector<MigrationNote> migrations;
};

} // namespace bunker
Зачем это готовить

У тебя уже есть alias-normalization, session profile roundtrip, world validation, atomic save и smoke-checks. Это идеальная база, чтобы дальше не “случайно мигрировать формат”, а фиксировать это осознанно.

15. Канон Lanline / session shell
namespace bunker {

enum class LanlineMode : std::uint8_t {
    Solo = 0,
    LanHost,
    LanClient
};

enum class LanlineLifecycle : std::uint8_t {
    Inactive = 0,
    HostLobbyOpen,
    HostJoinPending,
    HostRuntimeActive,
    HostClientAccepted,
    ClientRuntimeJoined,
    RuntimeActive
};

enum class LanlinePresenceType : std::uint8_t {
    None = 0,
    Friend,
    Voice,
    Relay,
    Service
};

struct LanlinePresenceRecord {
    std::string displayName;
    LanlinePresenceType type = LanlinePresenceType::None;
    bool online = false;
    bool ready = false;
};

struct LanlineRuntimeDigest {
    LanlineMode mode = LanlineMode::Solo;
    LanlineLifecycle lifecycle = LanlineLifecycle::Inactive;
    WorldId worldName;
    SessionId sessionId;
    std::vector<LanlinePresenceRecord> visiblePresence;
    std::string lastRelayMessage;
};

} // namespace bunker
Почему это сейчас полезно

У тебя уже есть launcher hardening, runtime launch state sync, Lanline session persistence/world glue и общая lobby logic. Значит, дальше полезно держать компактный digest-срез Lanline-состояния, который можно одинаково показывать в launcher, runtime и debug-панелях.

16. Канон objective / route / route-state
namespace bunker {

enum class ObjectivePhase : std::uint8_t {
    Hidden = 0,
    Available,
    Active,
    Completed,
    Failed
};

struct ObjectiveNode {
    ObjectiveId id;
    std::string title;
    std::string summary;
    ObjectivePhase phase = ObjectivePhase::Hidden;
    RegistryId relatedObjectId;
    DescriptorTag relatedTag;
};

struct RouteState {
    std::vector<ObjectiveNode> objectives;
    std::optional<ObjectiveId> activeObjectiveId;
    bool debriefUnlocked = false;
};

} // namespace bunker
Почему это к месту

Roadmap уже фиксирует рабочий стартовый вертикальный контур: крио -> Pip-Pad -> архив -> laska -> танк -> ковш -> bulkhead -> debris -> ghoul -> relay -> debrief, плюс objective-driven start route и zone events. Значит, route-state уже зрелый enough, чтобы иметь более явный типовой слой.

17. Временный единый контейнер, если всё это держать в одном файле

Если ты хочешь именно скинуть всё в один файл, можно сверху сделать такой агрегатор:

namespace bunker {

struct CanonFeatureStateBundle {
    KnowledgeLedger knowledge;
    InfrastructureReadModel infrastructure;
    TankRuntimeExtensionState tank;
    ShelterRecoveryState shelterRecovery;
    AssistantState assistant;
    ConsciousnessLineState consciousness;
    LanlineRuntimeDigest lanline;
    RouteState route;
    ActiveMusicBonus activeMusicBonus;
};

} // namespace bunker

Это не обязательно потом оставлять как реальный production-type. Но как единый временный master bundle для размышления и постепенного распила он очень удобен.

18. Как потом это разносить по текущему проекту
В src/GameplayDescriptorRegistry.cpp

Отправлять:

DescriptorDomain
DescriptorVisibility
GameplayDescriptorSpec
canonical tags
alias map
В src/WorldValidation.cpp

Отправлять:

ValidationSeverity
ValidationDomain
ValidationIssue
ValidationSummary
knowledge/region-specific validation rules
В src/SessionProfiles.cpp

Отправлять:

KnowledgeLedger
ActiveMusicBonus
AssistantState
ConsciousnessLineState
ShelterRecoveryState
RegionRuntimeState
В src/GameRuntimePipPad.cpp

Отправлять:

KnowledgeCarrier
infrastructure snapshot display
assistant advice display
active music bonus display
В src/GameRuntime.cpp

Отправлять:

tank runtime extension state
route state helpers
infrastructure read-model population
terminal sync action results
В Launcher/src/LauncherSupport.cpp

Отправлять:

LanlineRuntimeDigest
readiness report / demo status digest
В tests

Отправлять:

migration contract tests
validation summary tests
readiness/integration report tests

Это хорошо ложится на текущую структуру, потому что CMake уже подтверждает существование отдельных runtime/editor/launcher/support/validation/session/descriptor модулей и отдельной цели для smoke-checks.

19. Самые важные четыре вещи из всего этого

Если совсем жёстко ужимать, я бы считал обязательными именно эти четыре блока:

GameplayDescriptorSpec как центр семантики
KnowledgeLedger + KnowledgeCarrier как спасение Data Cards / tapes / reconstruction / archive
InfrastructureReadModel как видимая форма большого recovery/backbone слоя
AssistantState + ActiveMusicBonus + ConsciousnessLineState как защита identity проекта от “растворения в просто survival-systems”
20. Итог

Сейчас у тебя уже есть рабочий фундамент: сборка нескольких приложений, отдельные модули для registry/validation/profiles/Pip-Pad/support и живой roadmap с большим количеством уже внедрённых систем. Следующий уровень зрелости — это не ещё один монолит, а единый канон типов, чтобы потом всё остальное встраивалось в код без потери смысла и без новых расхождений между md, editor и runtime.
единый paste-ready master-файл, который можно просто сохранить как MASTER_CANON_INTEGRATION.txt или MASTER_CANON_INTEGRATION.md, а потом разнести по коду, задачам и модулям. Я собрал его под текущее состояние репозитория: у проекта уже есть отдельные BunkerGame, BunkerLauncher, BunkerEditor, BunkerSmokeChecks, а также вынесенные GameplayDescriptorRegistry, WorldValidation, SessionProfiles, GameRuntimePipPad, EditorSupport, LauncherSupport; roadmap фиксирует завершённый этап стабилизации и активную работу над архитектурой, стартовым вертикальным срезом и базовым боевым/RPG-слоем.

MASTER_CANON_INTEGRATION
version: working master
purpose: keep all important systems, names, contracts, integration logic and future insertions in one place until they are fully embedded into code

==================================================
1. CORE GOAL
==================================================

The project must not evolve by adding random features on top of unfinished systems.
The project must evolve by:
- preserving meaning
- canonicalizing names
- making old content compatible
- finishing integration between runtime, editor, launcher, save/load and validation
- only then expanding content

Main target:
Build a stable, showable, extendable Bunker Protocol vertical slice where:
- Launcher is the required entry point
- Game runtime is stable
- Editor is safe for authored content
- Save/load is reliable
- Pip-Pad is the main systemic feedback surface
- Lanline is a working meta-shell
- Shelter/Recovery/Infrastructure progression is visible and persistent
- Old ideas are not silently lost during migration

==================================================
2. WORKING RULES
==================================================

Rule 1:
Any gameplay feature is considered unfinished until it has:
- a canonical name
- a descriptor contract
- validation behavior
- persistence behavior if it affects long-term state
- player-visible feedback if it matters in runtime

Rule 2:
No new authored gameplay object may appear in worlds unless it is registered in the descriptor registry.

Rule 3:
No new persistent feature may be added without a default-safe load path.

Rule 4:
No feature is considered preserved if it only exists in markdown and not in runtime/editor flow.

Rule 5:
Old names may exist only as aliases. Runtime/editor/validation/save contracts must converge on canonical names.

Rule 6:
Do not delete identity features by accident just because they are not fully finished yet.

==================================================
3. WHAT COUNTS AS PROJECT IDENTITY
==================================================

These are core identity layers that must not disappear:

3.1. TANK / PLAYER / FIELD IDENTITY
- BT-72 as the central mechanical axis
- utility-slot based extension
- Tow Coupler
- Field Service
- wireless charging / service anchors
- SoulSync / Sync-Link
- SoulLine
- thermal mode
- stress awakening
- second wind / muscle memory style bodily specialization

3.2. DATA / MEMORY / INTERFACE IDENTITY
- Pip-Pad as the main systemic interface
- Data Reconstruction
- archive sync
- relay packets
- debrief records
- echo traces
- data cards as wider knowledge carriers
- cassette / music bonus flavor and gameplay link
- assistant guidance / Nerv-like layer
- consciousness / resonance / memory direction

3.3. WORLD / RECOVERY / INFRASTRUCTURE IDENTITY
- tower sync backbone
- power pylon
- drone station
- trade/recovery network
- caravan route
- route contamination
- rail freight / rail depot
- orbital uplink
- rail fortress
- recovery fabricator
- industrial gate / survey / outpost
- assembly / foundry / reactor / capacitor / relay / water chain
- shelter recovery index
- specialists / cryo unlock logic
- infrastructure decay and ether erosion

3.4. META / SESSION / LANLINE IDENTITY
- launcher-first entry flow
- world/session prep
- Lanline roster/session persistence
- relay chat
- voice presence
- host/client readiness logic
- runtime/launcher shared session state thinking

==================================================
4. WHAT IS ALREADY CONSIDERED REAL
==================================================

The following are considered real project layers, not just ideas:
- descriptor registry
- world validation
- session profile persistence
- Pip-Pad extracted into its own runtime file
- launcher/editor support files
- smoke checks target
- atomic persistence layer
- infrastructure/backbone progression branch
- Lanline services shell
- Tow Coupler runtime presence
- archive/data reconstruction presence

This means future work should finish integration, not restart architecture.

==================================================
5. MAIN CANONICAL NAMES
==================================================

5.1. canonical authored tags
archive_sync
terminal_sync
remote_link
workshop_service
tower_sync
power_pylon
drone_station
trade_network
rail_depot
orbital_uplink
rail_fortress_hub
recovery_fabricator
industrial_gate
industrial_survey
industrial_outpost
assembly_cell
foundry_line
reactor_yard
capacitor_bank
relay_substation
service_bay
water_reclaimer
lanline_service_hub
tank_service
medical_support
fey_ring
echo_trace
specialist_cryo
data_card
relay_packet
debrief_record
music_tape
assistant_anchor
region_gate
factory_console

5.2. legacy aliases allowed only as aliases
radio_tower -> tower_sync
workshop_field_service -> workshop_service
rail_freight -> rail_depot
rail_fortress -> rail_fortress_hub

Rule:
All runtime/editor/validation logic should operate on canonical names after normalization.

==================================================
6. MASTER CORE TYPES
==================================================

namespace bunker {

using RegistryId = std::string;
using WorldId = std::string;
using RegionId = std::string;
using DescriptorTag = std::string;
using CharacterId = std::string;
using SessionId = std::string;
using KnowledgeId = std::string;
using EffectId = std::string;
using ObjectiveId = std::string;
using SpecialistId = std::string;
using ServiceNodeId = std::string;

enum class CanonicalState : std::uint8_t {
    Unknown = 0,
    Draft,
    Active,
    Deprecated,
    AliasOnly
};

} // namespace bunker

==================================================
7. DESCRIPTOR CONTRACT
==================================================

namespace bunker {

enum class DescriptorDomain : std::uint8_t {
    Unknown = 0,
    Interaction,
    Infrastructure,
    Knowledge,
    Service,
    Transport,
    Recovery,
    Story,
    Lanline,
    Specialist,
    Transition
};

enum class DescriptorVisibility : std::uint8_t {
    Hidden = 0,
    EditorOnly,
    RuntimeOnly,
    Shared
};

struct DescriptorAlias {
    std::string legacyTag;
    std::string canonicalTag;
};

struct GameplayDescriptorSpec {
    DescriptorTag scriptTag;
    std::string label;
    DescriptorDomain domain = DescriptorDomain::Unknown;
    DescriptorVisibility visibility = DescriptorVisibility::Shared;

    int preferredInteraction = 0;
    int preferredCategory = 0;

    bool requiresLinkTarget = false;
    bool contributesToProgression = false;
    bool contributesToInfrastructure = false;
    bool contributesToKnowledge = false;
    bool appearsInPipPad = false;
    bool appearsInEditorDrafts = false;
    bool canBeValidationBlocking = false;
};

} // namespace bunker

Descriptor rules:
- Every gameplay tag must exist here first
- Validation must be derived from this where possible
- Editor presets must be derived from this where possible
- Runtime UI text should not drift away from this
- Alias maps live here, not all over the codebase

==================================================
8. KNOWLEDGE / DATA SYSTEM
==================================================

Purpose:
Unify tapes, data cards, relay packets, debrief records, archive fragments and echo traces into one larger knowledge layer.

namespace bunker {

enum class KnowledgeCarrierType : std::uint8_t {
    Unknown = 0,
    Tape,
    DataCard,
    RelayPacket,
    DebriefRecord,
    ArchiveSync,
    EchoTrace,
    TerminalRecord
};

enum class KnowledgeRewardType : std::uint8_t {
    None = 0,
    Lore,
    Recipe,
    InfrastructureUnlock,
    SpecialistUnlock,
    ObjectiveProgress,
    MapIntel,
    MusicBonus,
    AssistantHint
};

enum class KnowledgeCondition : std::uint8_t {
    Intact = 0,
    Damaged,
    Corrupted,
    Reconstructed,
    Consumed
};

struct KnowledgeReward {
    KnowledgeRewardType type = KnowledgeRewardType::None;
    std::string rewardKey;
    int amount = 0;
};

struct KnowledgeCarrier {
    KnowledgeId id;
    std::string title;
    std::string sourceTag;
    KnowledgeCarrierType type = KnowledgeCarrierType::Unknown;
    KnowledgeCondition condition = KnowledgeCondition::Intact;

    bool discovered = false;
    bool consumed = false;
    bool replayable = false;
    bool grantsProgression = false;

    std::string summary;
    std::string body;
    std::string mediaKey;
    std::vector<KnowledgeReward> rewards;
};

struct KnowledgeLedger {
    std::vector<KnowledgeCarrier> entries;
    std::optional<KnowledgeId> selectedId;
    int reconstructedCount = 0;
    int consumedCount = 0;
};

} // namespace bunker

Knowledge principles:
- Tape is not the whole data system, only one subtype
- Data Card must return as a first-class knowledge subtype
- Reconstruction should work on damaged/corrupted knowledge carriers, not just tapes
- Rewards can be lore, recipe, unlock, objective progress, map intel, assistant hints
- Pip-Pad DATA screen should become a knowledge screen, not just a tape list

==================================================
9. MUSIC / CASSETTE BONUS CONTRACT
==================================================

namespace bunker {

enum class MusicEffectType : std::uint8_t {
    None = 0,
    SteadyHands,
    SalvageFocus,
    MarchDrive,
    EchoResolve,
    ThermalDiscipline,
    FieldRecovery
};

struct MusicEffectSpec {
    EffectId id;
    std::string label;
    MusicEffectType type = MusicEffectType::None;
    float durationSeconds = 0.0f;
    float magnitude = 0.0f;
};

struct ActiveMusicBonus {
    KnowledgeId sourceKnowledgeId;
    EffectId effectId;
    std::string displayName;
    float remainingSeconds = 0.0f;
    bool active = false;
};

} // namespace bunker

Music principles:
- Music tapes should not disappear inside generic archive UI
- A playable intact music carrier may activate a small temporary bonus
- Music is not a separate giant subsystem; it is a thin extension on top of KnowledgeCarrier

==================================================
10. ASSISTANT / NERV / GUIDANCE CONTRACT
==================================================

namespace bunker {

enum class AssistantMode : std::uint8_t {
    Navigation = 0,
    Engineering,
    Tactical,
    Recovery,
    Archive,
    Lanline
};

enum class AssistantUrgency : std::uint8_t {
    Low = 0,
    Medium,
    High,
    Critical
};

struct AssistantAdvice {
    std::string id;
    AssistantMode mode = AssistantMode::Navigation;
    AssistantUrgency urgency = AssistantUrgency::Low;
    std::string title;
    std::string body;
    std::string relatedTargetId;
    bool acknowledged = false;
};

struct AssistantState {
    AssistantMode currentMode = AssistantMode::Navigation;
    bool enabled = true;
    bool cockpitLinked = false;
    bool canAutocycleHints = true;
    std::string displayName = "Nerv";
    std::optional<std::string> activeAdviceId;
    std::vector<AssistantAdvice> adviceLog;
};

} // namespace bunker

Assistant principles:
- Start as a guidance shell, not as full AI
- Use existing game state to generate hints
- Modes map naturally onto current project systems:
  - Navigation: next tower, next anchor, next transition
  - Engineering: tank service, power, repair, modules
  - Tactical: heat, damage, threat, risk
  - Recovery: infrastructure, recovery index, next unlock
  - Archive: reconstruction, data, lore goals
  - Lanline: presence, relay, session warnings

==================================================
11. INFRASTRUCTURE / BACKBONE CONTRACT
==================================================

namespace bunker {

enum class InfrastructureNodeType : std::uint8_t {
    Unknown = 0,
    TowerSync,
    PowerPylon,
    DroneStation,
    TradeNetwork,
    RailDepot,
    OrbitalUplink,
    RailFortressHub,
    RecoveryFabricator,
    IndustrialGate,
    IndustrialSurvey,
    IndustrialOutpost,
    AssemblyCell,
    FoundryLine,
    ReactorYard,
    CapacitorBank,
    RelaySubstation,
    ServiceBay,
    WaterReclaimer,
    LanlineServiceHub,
    TankService,
    MedicalSupport,
    SpecialistCryo
};

enum class InfrastructureNodeState : std::uint8_t {
    Offline = 0,
    Available,
    Active,
    Blocked,
    Damaged,
    Degraded
};

struct InfrastructureNodeSnapshot {
    InfrastructureNodeType type = InfrastructureNodeType::Unknown;
    DescriptorTag sourceTag;
    RegistryId objectId;
    InfrastructureNodeState state = InfrastructureNodeState::Offline;
    std::string label;
    std::string note;
    bool discovered = false;
    bool contributesToRecoveryIndex = false;
};

struct InfrastructureReadModel {
    std::vector<InfrastructureNodeSnapshot> nodes;

    bool regionalGridOnline = false;
    bool localRelayAvailable = false;
    bool towerSyncRecovered = false;
    bool caravanRouteActive = false;
    bool droneStationsActive = false;
    bool tradeNetworkActive = false;
    bool railFreightActive = false;
    bool orbitalUplinkActive = false;
    bool railFortressActive = false;
    bool recoveryFabricatorActive = false;
    bool industrialGateUnlocked = false;

    float etherErosion = 0.0f;
    float infrastructureDecay = 0.0f;
    float routeContamination = 0.0f;
    int shelterRecoveryIndex = 0;
};

} // namespace bunker

Infrastructure principles:
- Infrastructure must become visible to the player as one system, not scattered bools
- Pip-Pad must get an Infrastructure screen
- The current industrial/recovery backbone is already rich enough to deserve a read model
- The player should understand:
  - what is online
  - what is blocked
  - what is decaying
  - what the next unlock is
  - what benefit each node gives

==================================================
12. TANK / UTILITY / FIELD SERVICE CONTRACT
==================================================

namespace bunker {

enum class TankUtilityModuleType : std::uint8_t {
    None = 0,
    TowCoupler,
    BucketShield,
    FieldRig,
    ThermalKit,
    ServiceRig
};

enum class TankServiceState : std::uint8_t {
    Nominal = 0,
    NeedsService,
    Critical,
    Offline
};

struct TankUtilityModuleState {
    TankUtilityModuleType type = TankUtilityModuleType::None;
    std::string moduleId;
    std::string label;
    bool installed = false;
    bool unlocked = false;
};

struct TankFieldServiceRecord {
    bool performed = false;
    float serviceQuality = 0.0f;
    float lastServiceWorldTime = 0.0f;
    std::string performedAtNodeId;
};

struct TankRuntimeExtensionState {
    std::vector<TankUtilityModuleState> utilityModules;
    TankServiceState serviceState = TankServiceState::Nominal;
    TankFieldServiceRecord fieldService;
    bool wirelessChargingAvailable = false;
    bool soulSyncLinked = false;
    bool soulLineReady = false;
    bool thermalModeUnlocked = false;
};

} // namespace bunker

Tank principles:
- Tank-related identity must not dissolve into random booleans
- Tow Coupler stays a real first-class module
- Field Service should remain a distinct layer
- Tank service state and utility state should be visible in Pip-Pad
- SoulSync/SoulLine should remain explicit parts of the tank-player bond

==================================================
13. SHELTER / RECOVERY / SPECIALIST CONTRACT
==================================================

namespace bunker {

enum class RecoveryMilestoneType : std::uint8_t {
    None = 0,
    FirstTowerOnline,
    PowerBackboneOnline,
    CaravanStabilized,
    DroneNetOnline,
    TradeNetEstablished,
    RailRouteEstablished,
    OrbitalSyncEstablished,
    FortressRouteEstablished,
    FabricationOnline,
    WaterSecurityEstablished
};

struct RecoveryMilestone {
    RecoveryMilestoneType type = RecoveryMilestoneType::None;
    bool completed = false;
    std::string completedByWorld;
    std::string completedByNode;
};

struct ShelterRecoveryState {
    int recoveryIndex = 0;
    int doctrineTier = 0;
    int assignedSpecialists = 0;
    int awakenedRecipes = 0;
    bool forwardCampUnlocked = false;
    bool scavengerTeamsActive = false;
    bool autopilotCaravanUnlocked = false;
    std::vector<RecoveryMilestone> milestones;
};

} // namespace bunker

Recovery principles:
- Shelter progression must feel systemic, not just textual
- Recovery index should remain a meaningful value
- Specialists, recipes, doctrine and outposts should live under one broader recovery umbrella

==================================================
14. CONSCIOUSNESS / MEMORY / RESONANCE CONTRACT
==================================================

namespace bunker {

enum class ResonanceEventType : std::uint8_t {
    None = 0,
    DiaryRead,
    ArchiveRecovered,
    EchoExperienced,
    SocialBondFormed,
    SurvivalTurningPoint,
    AssistantDeepLink,
    DebriefProcessed
};

struct ResonanceEvent {
    ResonanceEventType type = ResonanceEventType::None;
    std::string sourceId;
    float value = 0.0f;
};

struct ConsciousnessLineState {
    float resonance = 0.0f;
    int diaryEvents = 0;
    int archiveMoments = 0;
    int socialLinks = 0;
    int survivalTurningPoints = 0;
    std::vector<ResonanceEvent> eventLog;
};

} // namespace bunker

Consciousness principles:
- This is a hidden or semi-hidden identity layer
- It should influence rare route, archive, awakening, assistant and stress outcomes
- It does not need a huge UI early
- It must exist in persistence before it can meaningfully affect narrative/gameplay

==================================================
15. REGION / CHUNK / STREAMING-PREP CONTRACT
==================================================

namespace bunker {

struct RegionDescriptor {
    RegionId regionId;
    std::string label;
    bool isInterior = false;
    bool supportsStreaming = false;
    bool isCriticalDemoRegion = false;
};

struct RegionLink {
    RegionId fromRegion;
    RegionId toRegion;
    RegistryId viaObjectId;
    bool bidirectional = true;
};

struct RegionRuntimeState {
    RegionId regionId;
    bool loaded = false;
    bool visited = false;
    bool hasCheckpoint = false;
};

} // namespace bunker

Region principles:
- Do not build full streaming yet if scope is too large
- Start by making regions/chunks explicit in data and authored objects
- Author transitions should eventually know not only linkTarget but also destination region
- This preserves the future streaming path without forcing an engine rewrite now

==================================================
16. LANLINE / SESSION DIGEST CONTRACT
==================================================

namespace bunker {

enum class LanlineMode : std::uint8_t {
    Solo = 0,
    LanHost,
    LanClient
};

enum class LanlineLifecycle : std::uint8_t {
    Inactive = 0,
    HostLobbyOpen,
    HostJoinPending,
    HostRuntimeActive,
    HostClientAccepted,
    ClientRuntimeJoined,
    RuntimeActive
};

enum class LanlinePresenceType : std::uint8_t {
    None = 0,
    Friend,
    Voice,
    Relay,
    Service
};

struct LanlinePresenceRecord {
    std::string displayName;
    LanlinePresenceType type = LanlinePresenceType::None;
    bool online = false;
    bool ready = false;
};

struct LanlineRuntimeDigest {
    LanlineMode mode = LanlineMode::Solo;
    LanlineLifecycle lifecycle = LanlineLifecycle::Inactive;
    WorldId worldName;
    SessionId sessionId;
    std::vector<LanlinePresenceRecord> visiblePresence;
    std::string lastRelayMessage;
};

} // namespace bunker

Lanline principles:
- Lanline is not fake chrome; it is a real shell layer
- Launcher and runtime should be able to render a compact common digest
- Session presence, readiness and relay state should not be scattered as unrelated fields

==================================================
17. OBJECTIVE / ROUTE CONTRACT
==================================================

namespace bunker {

enum class ObjectivePhase : std::uint8_t {
    Hidden = 0,
    Available,
    Active,
    Completed,
    Failed
};

struct ObjectiveNode {
    ObjectiveId id;
    std::string title;
    std::string summary;
    ObjectivePhase phase = ObjectivePhase::Hidden;
    RegistryId relatedObjectId;
    DescriptorTag relatedTag;
};

struct RouteState {
    std::vector<ObjectiveNode> objectives;
    std::optional<ObjectiveId> activeObjectiveId;
    bool debriefUnlocked = false;
};

} // namespace bunker

Route principles:
- Startup route must remain explicit and inspectable
- Story progress should remain compatible with authored world objects and tags
- Pip-Pad should be able to show active route state clearly

==================================================
18. VALIDATION CONTRACT
==================================================

namespace bunker {

enum class ValidationSeverity : std::uint8_t {
    Info = 0,
    Warning,
    Error,
    Blocking
};

enum class ValidationDomain : std::uint8_t {
    Unknown = 0,
    Registry,
    Authoring,
    Descriptor,
    LinkGraph,
    RegionGraph,
    Knowledge,
    Infrastructure,
    Persistence
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Info;
    ValidationDomain domain = ValidationDomain::Unknown;
    std::string code;
    RegistryId objectId;
    std::string message;
};

struct ValidationSummary {
    int infos = 0;
    int warnings = 0;
    int errors = 0;
    int blocking = 0;
    std::string label;
};

} // namespace bunker

Validation minimum rules:
- empty registryId => blocking
- duplicate registryId => blocking
- broken linkTarget => blocking when required for gameplay semantics
- missing required linkTarget => blocking
- unknown scriptTag => error or blocking depending on context
- legacy alias present => warning
- preferred interaction mismatch => warning
- preferred category mismatch => warning
- transition without target => warning or error depending on context
- region mismatch or invalid region => validation domain: RegionGraph
- malformed knowledge item => validation domain: Knowledge

Editor rule:
- save/export blocked on blocking or error severity
- warnings allowed, but shown clearly
- validation summary must be visible before save/export

==================================================
19. PERSISTENCE / MIGRATION CONTRACT
==================================================

namespace bunker {

enum class PersistenceObjectType : std::uint8_t {
    Unknown = 0,
    SessionProfile,
    WorldState,
    LanlineSnapshot,
    EditorWorkspace
};

struct PersistenceVersion {
    int major = 1;
    int minor = 0;
    int patch = 0;
};

struct MigrationNote {
    std::string fromKey;
    std::string toKey;
    std::string description;
};

struct PersistenceContract {
    PersistenceObjectType type = PersistenceObjectType::Unknown;
    PersistenceVersion version;
    bool supportsLegacyAliases = false;
    bool usesAtomicWrite = false;
    std::vector<MigrationNote> migrations;
};

} // namespace bunker

Persistence principles:
- All important long-lived state must load safely with defaults
- Old profiles should not explode when new fields are added
- Normalize step should remain first-class
- World/profile save success must be honest and coordinated
- Atomic writes are mandatory where corruption risk matters

==================================================
20. DEMO / READINESS / DIAGNOSTICS CONTRACT
==================================================

namespace bunker {

enum class ReadinessSeverity : std::uint8_t {
    Ok = 0,
    Warning,
    Blocking
};

struct ReadinessIssue {
    ReadinessSeverity severity = ReadinessSeverity::Ok;
    std::string code;
    std::string message;
    std::string relatedId;
};

struct DemoReadinessReport {
    bool launcherReady = false;
    bool gameReady = false;
    bool editorReady = false;
    bool validationReady = false;
    bool saveFlowReady = false;
    bool worldRouteReady = false;
    bool pipPadReady = false;
    bool lanlineReady = false;
    std::vector<ReadinessIssue> issues;
};

struct WorldIntegrationReport {
    int objectCount = 0;
    int gameplayTaggedObjects = 0;
    int invalidObjects = 0;
    int warningObjects = 0;
    bool hasStarterTower = false;
    bool hasTankAnchor = false;
    bool hasServiceNode = false;
    bool hasRecoveryNode = false;
    bool hasLanlineNode = false;
};

struct RuntimeBootstrapReport {
    bool profileLoaded = false;
    bool worldLoaded = false;
    bool worldGenerated = false;
    bool starterInfrastructureReady = false;
    bool launchTicketApplied = false;
    bool staticEraserLoaded = false;
    std::string summary;
};

struct SaveOutcome {
    bool worldSaved = false;
    bool profileSaved = false;
    bool eraserSaved = false;
    std::string message;
};

} // namespace bunker

Readiness principles:
- "build passed" is not enough
- the project should know whether it is demo-ready
- launcher/game/editor/persistence/validation/Pip-Pad/Lanline should each be able to report readiness
- smoke checks should grow toward integration checks, not just compilation confidence

==================================================
21. CANON FEATURE STATE BUNDLE
==================================================

namespace bunker {

struct CanonFeatureStateBundle {
    KnowledgeLedger knowledge;
    InfrastructureReadModel infrastructure;
    TankRuntimeExtensionState tank;
    ShelterRecoveryState shelterRecovery;
    AssistantState assistant;
    ConsciousnessLineState consciousness;
    LanlineRuntimeDigest lanline;
    RouteState route;
    ActiveMusicBonus activeMusicBonus;
};

} // namespace bunker

Note:
This bundle is not required to exist forever as a production object.
It exists as a temporary master thinking tool so the project does not lose coherence while systems are being integrated.

==================================================
22. FILE-BY-FILE FUTURE TARGETS
==================================================

22.1. src/main.cpp
Target:
- full atomic save integration
- honest save status
- bootstrap report
- no direct save calls left
Do not use it for:
- editor validation
- giant gameplay rewrites

22.2. Editor/src/Editor_Main.cpp
Target:
- validation gate before save/export
- atomic save when creating/saving worlds
- visible validation summary panel
Do not use it for:
- giant framework rewrite before safety exists

22.3. src/GameRuntime.cpp
Target:
- move away from long string-if chains where registry already exists
- table-driven terminal sync text
- table-driven terminal sync effects
- canonical tags only after normalization
Do not use it for:
- all-at-once rewrites

22.4. src/GameRuntimePipPad.cpp
Target:
- general knowledge screen
- infrastructure screen
- assistant panel
- visible tank/service state
- music bonus display
Do not use it for:
- overdesigned UI before data contracts exist

22.5. src/SessionProfiles.cpp
Target:
- persistent contracts for knowledge, assistant, music bonus, consciousness, region state
Rule:
- always load older profiles safely

22.6. src/GameplayDescriptorRegistry.cpp
Target:
- be the single source of truth for authored gameplay semantics

22.7. src/WorldValidation.cpp
Target:
- grow with every new canonical descriptor feature

22.8. Launcher/src/LauncherSupport.cpp
Target:
- digest-level session readiness, world summary, demo entry clarity
Not first priority.

==================================================
23. ORDER OF INTEGRATION
==================================================

Order 1:
main.cpp
- full atomic save flow
- honest F5 and shutdown behavior

Order 2:
Editor_Main.cpp
- validation everywhere important
- atomic world create/save

Order 3:
GameRuntime.cpp
- descriptor-driven terminal behavior
- split description from effect

Order 4:
GameRuntimePipPad.cpp
- infrastructure
- knowledge
- assistant
- music bonus surface

Order 5:
SessionProfiles.cpp
- knowledge ledger persistence
- assistant state persistence
- consciousness persistence
- region readiness

Order 6:
GameplayDescriptorRegistry.cpp + WorldValidation.cpp
- register and validate any newly revived feature

==================================================
24. WHAT MUST NOT BE LOST FROM NOW ON
==================================================

Do not accidentally lose:
- alias compatibility
- descriptor canon
- validation safety
- atomic save flow
- Data Reconstruction
- archive/data identity
- Tow Coupler
- Field Service
- SoulLine / SoulSync
- Lanline meta shell
- infrastructure backbone
- recovery index and doctrine direction
- assistant identity potential
- consciousness/resonance direction
- future region/chunk contract

==================================================
25. WHAT COUNTS AS "FEATURE RETURNED SUCCESSFULLY"
==================================================

A feature is considered returned only if:
- it has a canonical type/name
- it is represented in data/contracts
- it can survive save/load if needed
- it appears in runtime or editor in a meaningful way
- it is not only mentioned in markdown

Examples:
- Data Cards are not truly back if they only exist as notes
- Music bonus is not back if tapes exist but no playable gameplay effect exists
- Assistant is not back if only the word "Nerv" exists in docs
- Infrastructure is not fully back if backend flags exist but the player cannot inspect them in Pip-Pad

==================================================
26. CURRENT DEVELOPMENT PHILOSOPHY
==================================================

The project is no longer in the phase:
"just get something on screen"

The project is now in the phase:
"finish integrating already-created systems so the game becomes coherent, robust and showable"

Therefore:
- reduce drift
- reduce silent feature loss
- reduce parallel naming
- reduce string-only logic where canonical registry exists
- increase editor safety
- increase runtime clarity
- increase persistence honesty
- increase player-visible system feedback

==================================================
27. FINAL FORMULA
==================================================

Do not build a new layer on top of disconnected systems.
Turn existing systems into one consistent loop:
launcher -> session/world selection -> runtime -> authored world -> infrastructure/recovery -> Pip-Pad feedback -> save/load -> return

And preserve the identity pillars:
BT-72
Pip-Pad
Knowledge/Data
Recovery/Infrastructure
Lanline
Assistant
Resonance
Authoring safety

END OF MASTER CANON

Ниже — жёсткий формат без лишней prose:
приоритет → файл → что туда встраивать → минимальные структуры → результат.
Основание для порядка: у тебя уже есть отдельные модули GameplayDescriptorRegistry, WorldValidation, SessionProfiles, GameRuntimePipPad, EditorSupport, LauncherSupport, плюс отдельный BunkerSmokeChecks; roadmap фиксирует, что стабилизация основы завершена, Launcher v1 завершён, а архитектура/вертикальный срез/RPG-слой ещё в работе.

PRIORITY 0 — НЕ ЛОМАТЬ КАНОН

Нельзя терять:

BT-72 / utility-slot / Tow Coupler / Field Service / SoulLine / SoulSync
Pip-Pad / Data Reconstruction / Echo Trace
Lanline shell / relay / voice presence / launcher-first flow
Power Grid / Rail / Orbital / Recovery backbone
per-world persistence / validation / descriptor canon
cassette/music bonus как identity-фичу, даже если она пока маленькая
future-layer: assistant/Nerv, consciousness/resonance, region/chunk contract
PRIORITY 1 — SAVE / LOAD / SAFETY
1.1 src/main.cpp

Встраивать:

только атомарные сейвы
честный save status
bootstrap report
убрать все прямые world.Save(...) и прямые SaveSessionProfile(...)

Добавить структуры:

struct RuntimeBootstrapReport {
    bool profileLoaded = false;
    bool worldLoaded = false;
    bool worldGenerated = false;
    bool starterInfrastructureReady = false;
    bool launchTicketApplied = false;
    bool staticEraserLoaded = false;
    std::string summary;
};

struct SaveOutcome {
    bool worldSaved = false;
    bool profileSaved = false;
    bool eraserSaved = false;
    std::string message;
};

Результат:

F5 и shutdown сохраняют мир/профиль согласованно
нет ложного “save committed”
runtime становится инженерно надёжнее
1.2 Editor/src/Editor_Main.cpp

Встраивать:

validation перед save/export
atomic save в create/save path
validation summary panel
блокировку export при error/blocking

Добавить структуры:

struct EditorValidationState {
    std::vector<bunker::ValidationIssue> issues;
    int errorCount = 0;
    int warningCount = 0;
    std::string summary;
    std::string selectedIssueObjectId;
};

struct EditorSavePolicy {
    bool blockOnErrors = true;
    bool allowWarnings = true;
    bool autoValidateBeforeSave = true;
};

Результат:

editor перестаёт быть “сильным, но опасным”
authored world не ломается тихо
PRIORITY 2 — DESCRIPTOR CANON
2.1 src/GameplayDescriptorRegistry.cpp

Встраивать:

единый canonical registry для всех gameplay tags
legacy aliases только здесь
новые теги сначала сюда, потом в runtime/editor/validation

Держать здесь:

enum class DescriptorDomain : std::uint8_t {
    Unknown = 0,
    Interaction,
    Infrastructure,
    Knowledge,
    Service,
    Transport,
    Recovery,
    Story,
    Lanline,
    Specialist,
    Transition
};

enum class DescriptorVisibility : std::uint8_t {
    Hidden = 0,
    EditorOnly,
    RuntimeOnly,
    Shared
};

struct DescriptorAlias {
    std::string legacyTag;
    std::string canonicalTag;
};

struct GameplayDescriptorSpec {
    DescriptorTag scriptTag;
    std::string label;
    DescriptorDomain domain = DescriptorDomain::Unknown;
    DescriptorVisibility visibility = DescriptorVisibility::Shared;
    int preferredInteraction = 0;
    int preferredCategory = 0;
    bool requiresLinkTarget = false;
    bool contributesToProgression = false;
    bool contributesToInfrastructure = false;
    bool contributesToKnowledge = false;
    bool appearsInPipPad = false;
    bool appearsInEditorDrafts = false;
    bool canBeValidationBlocking = false;
};

Новые канонические теги, которые держать тут:

data_card
relay_packet
debrief_record
music_tape
assistant_anchor
region_gate
factory_console

Результат:

один источник правды
меньше string-chaos
проще validation и editor drafts
PRIORITY 3 — VALIDATION AS LAW
3.1 src/WorldValidation.cpp

Встраивать:

typed validation domains
knowledge rules
region rules
assistant anchor rules
blocking vs warning policy

Добавить структуры:

enum class ValidationSeverity : std::uint8_t {
    Info = 0,
    Warning,
    Error,
    Blocking
};

enum class ValidationDomain : std::uint8_t {
    Unknown = 0,
    Registry,
    Authoring,
    Descriptor,
    LinkGraph,
    RegionGraph,
    Knowledge,
    Infrastructure,
    Persistence
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Info;
    ValidationDomain domain = ValidationDomain::Unknown;
    std::string code;
    RegistryId objectId;
    std::string message;
};

struct ValidationSummary {
    int infos = 0;
    int warnings = 0;
    int errors = 0;
    int blocking = 0;
    std::string label;
};

Новые правила:

data_card без reward metadata → error
music_tape без effect id → warning/error
region_gate без region/link target → blocking
assistant_anchor без semantic target → error
unknown canonical tag → error/blocking

Результат:

validation растёт вместе с фичами
editor/export не пропускают битые authored-сущности
PRIORITY 4 — PIP-PAD KEEPS THE GAME TOGETHER
4.1 src/GameRuntimePipPad.cpp

Встраивать:

общий knowledge screen
infrastructure screen
assistant panel
active music bonus panel
tank/service state panel

Добавить структуры:

enum class PipDataCategory : std::uint8_t {
    Archive = 0,
    DataCard,
    RelayPacket,
    Debrief,
    EchoTrace
};

struct PipInfrastructureSnapshot {
    bool towerOnline = false;
    bool relayOnline = false;
    bool logisticsOnline = false;
    bool fabricationOnline = false;
    bool medicalOnline = false;
    float etherErosion = 0.0f;
    float infrastructureDecay = 0.0f;
    int recoveryIndex = 0;
};

struct ActiveMusicBonus {
    std::string sourceId;
    std::string effectId;
    float remainingSeconds = 0.0f;
    bool active = false;
};

Разделы Pip-Pad, которые должны быть:

DATA / Knowledge
Infrastructure
Tank / Service
Assistant
Route / Objectives
Lanline digest

Результат:

игрок видит систему мира, а не только текстовые последствия
identity проекта не растворяется

Roadmap уже подтверждает, что Pip-Pad AR / Echo Trace, Data Reconstruction, Tow Coupler, Field Service, SoulLine, Lanline Services, Power Grid, Rail Freight Link, Orbital Uplink, Recovery Fabricator и Shelter Recovery Index входят в живой контур проекта, так что именно Pip-Pad должен стать общей точкой их отображения.

PRIORITY 5 — RUNTIME LOGIC MUST STOP DRIFTING
5.1 src/GameRuntime.cpp

Встраивать:

table-driven terminal sync text
table-driven sync effects
split “description” from “effect”
canonical tags only after normalization

Добавить структуры:

struct TerminalSyncSpec {
    std::string_view scriptTag;
    const char* syncText;
    bool affectsProgression = false;
    bool affectsWorldState = false;
};

struct TerminalSyncActionResult {
    std::string statusText;
    bool progressionAdvanced = false;
    bool worldStateChanged = false;
};

Разделить:

DescribeTerminalSync(tag)
ApplyTerminalSyncEffects(tag, ...)

Результат:

runtime перестаёт расходиться с registry
industrial/recovery chain проще расширять
меньше ручных if-цепочек
PRIORITY 6 — KNOWLEDGE SYSTEM MUST BECOME WIDER THAN TAPE
6.1 src/SessionProfiles.cpp

Встраивать:

knowledge ledger
assistant state
active music bonus
consciousness line
future region state

Добавить структуры:

enum class KnowledgeCarrierType : std::uint8_t {
    Unknown = 0,
    Tape,
    DataCard,
    RelayPacket,
    DebriefRecord,
    ArchiveSync,
    EchoTrace,
    TerminalRecord
};

enum class KnowledgeRewardType : std::uint8_t {
    None = 0,
    Lore,
    Recipe,
    InfrastructureUnlock,
    SpecialistUnlock,
    ObjectiveProgress,
    MapIntel,
    MusicBonus,
    AssistantHint
};

enum class KnowledgeCondition : std::uint8_t {
    Intact = 0,
    Damaged,
    Corrupted,
    Reconstructed,
    Consumed
};

struct KnowledgeReward {
    KnowledgeRewardType type = KnowledgeRewardType::None;
    std::string rewardKey;
    int amount = 0;
};

struct KnowledgeCarrier {
    KnowledgeId id;
    std::string title;
    std::string sourceTag;
    KnowledgeCarrierType type = KnowledgeCarrierType::Unknown;
    KnowledgeCondition condition = KnowledgeCondition::Intact;
    bool discovered = false;
    bool consumed = false;
    bool replayable = false;
    bool grantsProgression = false;
    std::string summary;
    std::string body;
    std::string mediaKey;
    std::vector<KnowledgeReward> rewards;
};

struct KnowledgeLedger {
    std::vector<KnowledgeCarrier> entries;
    std::optional<KnowledgeId> selectedId;
    int reconstructedCount = 0;
    int consumedCount = 0;
};

Правило:

tapes не удалять
tapes расширять в KnowledgeCarrier
старые сейвы должны грузиться без поломки

Результат:

Data Cards реально возвращаются
archive/tape/reconstruction становится широкой knowledge-системой
PRIORITY 7 — ASSISTANT / NERV / RESONANCE
7.1 src/SessionProfiles.cpp + src/GameRuntimePipPad.cpp

Встраивать:

persistent assistant state
advice log
mode switching
soft UI surface в Pip-Pad

Структуры:

enum class AssistantMode : std::uint8_t {
    Navigation = 0,
    Engineering,
    Tactical,
    Recovery,
    Archive,
    Lanline
};

enum class AssistantUrgency : std::uint8_t {
    Low = 0,
    Medium,
    High,
    Critical
};

struct AssistantAdvice {
    std::string id;
    AssistantMode mode = AssistantMode::Navigation;
    AssistantUrgency urgency = AssistantUrgency::Low;
    std::string title;
    std::string body;
    std::string relatedTargetId;
    bool acknowledged = false;
};

struct AssistantState {
    AssistantMode currentMode = AssistantMode::Navigation;
    bool enabled = true;
    bool cockpitLinked = false;
    bool canAutocycleHints = true;
    std::string displayName = "Nerv";
    std::optional<std::string> activeAdviceId;
    std::vector<AssistantAdvice> adviceLog;
};

Режимы:

Navigation
Engineering
Tactical
Recovery
Archive
Lanline

Результат:

assistant не остаётся только красивой идеей в md
он начинает жить как практический советчик поверх текущих систем
7.2 src/SessionProfiles.cpp + src/StoryRoute.cpp + src/SkillSystem.cpp

Встраивать:

consciousness / resonance hidden layer

Структуры:

enum class ResonanceEventType : std::uint8_t {
    None = 0,
    DiaryRead,
    ArchiveRecovered,
    EchoExperienced,
    SocialBondFormed,
    SurvivalTurningPoint,
    AssistantDeepLink,
    DebriefProcessed
};

struct ResonanceEvent {
    ResonanceEventType type = ResonanceEventType::None;
    std::string sourceId;
    float value = 0.0f;
};

struct ConsciousnessLineState {
    float resonance = 0.0f;
    int diaryEvents = 0;
    int archiveMoments = 0;
    int socialLinks = 0;
    int survivalTurningPoints = 0;
    std::vector<ResonanceEvent> eventLog;
};

Результат:

identity-слой не потеряется
позже можно мягко влиять на archive/awakening/story
PRIORITY 8 — TANK IDENTITY MUST STAY EXPLICIT
8.1 src/GameRuntime.cpp + src/GameRuntimePipPad.cpp + src/SessionProfiles.cpp

Встраивать:

utility module state
service state
field service record
soul sync / soul line state

Структуры:

enum class TankUtilityModuleType : std::uint8_t {
    None = 0,
    TowCoupler,
    BucketShield,
    FieldRig,
    ThermalKit,
    ServiceRig
};

enum class TankServiceState : std::uint8_t {
    Nominal = 0,
    NeedsService,
    Critical,
    Offline
};

struct TankUtilityModuleState {
    TankUtilityModuleType type = TankUtilityModuleType::None;
    std::string moduleId;
    std::string label;
    bool installed = false;
    bool unlocked = false;
};

struct TankFieldServiceRecord {
    bool performed = false;
    float serviceQuality = 0.0f;
    float lastServiceWorldTime = 0.0f;
    std::string performedAtNodeId;
};

struct TankRuntimeExtensionState {
    std::vector<TankUtilityModuleState> utilityModules;
    TankServiceState serviceState = TankServiceState::Nominal;
    TankFieldServiceRecord fieldService;
    bool wirelessChargingAvailable = false;
    bool soulSyncLinked = false;
    bool soulLineReady = false;
    bool thermalModeUnlocked = false;
};

Результат:

Tow Coupler, Field Service, SoulLine не расползаются по случайным bool-полям
tank-loop становится видимым и устойчивым

Roadmap прямо подтверждает ранние слои wireless tank charging, Field Service, SoulSync / Sync-Link, SoulLine, Thermal Mode, Tow Coupler / trailer utility module, так что эти контракты уже соответствуют живому направлению проекта.

PRIORITY 9 — RECOVERY / SHELTER STATE
9.1 src/SessionProfiles.cpp

Встраивать:

recovery milestones
doctrine tier
assigned specialists
awakened recipes
forward camp / scavenger / caravan unlocks

Структуры:

enum class RecoveryMilestoneType : std::uint8_t {
    None = 0,
    FirstTowerOnline,
    PowerBackboneOnline,
    CaravanStabilized,
    DroneNetOnline,
    TradeNetEstablished,
    RailRouteEstablished,
    OrbitalSyncEstablished,
    FortressRouteEstablished,
    FabricationOnline,
    WaterSecurityEstablished
};

struct RecoveryMilestone {
    RecoveryMilestoneType type = RecoveryMilestoneType::None;
    bool completed = false;
    std::string completedByWorld;
    std::string completedByNode;
};

struct ShelterRecoveryState {
    int recoveryIndex = 0;
    int doctrineTier = 0;
    int assignedSpecialists = 0;
    int awakenedRecipes = 0;
    bool forwardCampUnlocked = false;
    bool scavengerTeamsActive = false;
    bool autopilotCaravanUnlocked = false;
    std::vector<RecoveryMilestone> milestones;
};

Результат:

shelter/recovery становится не только набором флагов
проще показать progression и в Pip-Pad, и в launcher summary
PRIORITY 10 — REGION / CHUNK PREP
10.1 World-связанные файлы + editor

Встраивать:

regionId
region links
per-region runtime state
пока без настоящего streaming

Структуры:

struct RegionDescriptor {
    RegionId regionId;
    std::string label;
    bool isInterior = false;
    bool supportsStreaming = false;
    bool isCriticalDemoRegion = false;
};

struct RegionLink {
    RegionId fromRegion;
    RegionId toRegion;
    RegistryId viaObjectId;
    bool bidirectional = true;
};

struct RegionRuntimeState {
    RegionId regionId;
    bool loaded = false;
    bool visited = false;
    bool hasCheckpoint = false;
};

Результат:

future streaming path не теряется
authored transitions начинают мыслиться как graph, а не только как linkTarget
PRIORITY 11 — LANLINE DIGEST
11.1 LauncherSupport.cpp + runtime Pip-Pad

Встраивать:

compact session/world digest
readiness/world summary
visible presence/relay state

Структуры:

enum class LanlineMode : std::uint8_t {
    Solo = 0,
    LanHost,
    LanClient
};

enum class LanlineLifecycle : std::uint8_t {
    Inactive = 0,
    HostLobbyOpen,
    HostJoinPending,
    HostRuntimeActive,
    HostClientAccepted,
    ClientRuntimeJoined,
    RuntimeActive
};

enum class LanlinePresenceType : std::uint8_t {
    None = 0,
    Friend,
    Voice,
    Relay,
    Service
};

struct LanlinePresenceRecord {
    std::string displayName;
    LanlinePresenceType type = LanlinePresenceType::None;
    bool online = false;
    bool ready = false;
};

struct LanlineRuntimeDigest {
    LanlineMode mode = LanlineMode::Solo;
    LanlineLifecycle lifecycle = LanlineLifecycle::Inactive;
    WorldId worldName;
    SessionId sessionId;
    std::vector<LanlinePresenceRecord> visiblePresence;
    std::string lastRelayMessage;
};

Результат:

launcher и runtime говорят об одной сессии одним языком
Lanline остаётся системным shell, а не просто атмосферой
PRIORITY 12 — READINESS / DEMO CHECK
12.1 tests/SmokeChecks.cpp + support/debug panels

Встраивать:

demo readiness
world integration report
typed readiness issues

Структуры:

enum class ReadinessSeverity : std::uint8_t {
    Ok = 0,
    Warning,
    Blocking
};

struct ReadinessIssue {
    ReadinessSeverity severity = ReadinessSeverity::Ok;
    std::string code;
    std::string message;
    std::string relatedId;
};

struct DemoReadinessReport {
    bool launcherReady = false;
    bool gameReady = false;
    bool editorReady = false;
    bool validationReady = false;
    bool saveFlowReady = false;
    bool worldRouteReady = false;
    bool pipPadReady = false;
    bool lanlineReady = false;
    std::vector<ReadinessIssue> issues;
};

struct WorldIntegrationReport {
    int objectCount = 0;
    int gameplayTaggedObjects = 0;
    int invalidObjects = 0;
    int warningObjects = 0;
    bool hasStarterTower = false;
    bool hasTankAnchor = false;
    bool hasServiceNode = false;
    bool hasRecoveryNode = false;
    bool hasLanlineNode = false;
};

Результат:

проект умеет говорить “мы готовы к демо / мы не готовы”
это лучше, чем просто “вроде работает”
ПОРЯДОК РЕАЛЬНОГО ВНЕДРЕНИЯ
Ход 1

src/main.cpp

atomics
honest save status
bootstrap report
Ход 2

Editor/src/Editor_Main.cpp

validation gate
atomic editor save
summary panel
Ход 3

src/GameRuntime.cpp

table-driven terminal sync
canonical tags only
description/effect split
Ход 4

src/GameRuntimePipPad.cpp

infrastructure screen
knowledge screen
assistant panel
music bonus surface
Ход 5

src/SessionProfiles.cpp

knowledge ledger
assistant
music bonus
consciousness
shelter recovery
Ход 6

src/GameplayDescriptorRegistry.cpp

new canonical tags
stronger spec semantics
Ход 7

src/WorldValidation.cpp

rules for new tags/domains
Ход 8

LauncherSupport.cpp

digest/readiness polish
ЕСЛИ УЖАТЬ ДО САМОГО ВАЖНОГО

Сначала делать только это:

main.cpp — safe persistence
Editor_Main.cpp — validation gate
GameRuntime.cpp — registry-driven terminal logic
GameRuntimePipPad.cpp — infrastructure + knowledge UI
SessionProfiles.cpp — knowledge/assistant/consciousness persistence

Тогда даю временный единый header-файл, который можно просто создать и держать как общий канон типов, пока ты не разнесёшь всё по модулям.

Файл

include/MasterCanonTypes.hpp

Код
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bunker {

// ==================================================
// 1. BASE IDS / COMMON TYPES
// ==================================================

using RegistryId = std::string;
using WorldId = std::string;
using RegionId = std::string;
using DescriptorTag = std::string;
using CharacterId = std::string;
using SessionId = std::string;
using KnowledgeId = std::string;
using EffectId = std::string;
using ObjectiveId = std::string;
using SpecialistId = std::string;
using ServiceNodeId = std::string;

enum class CanonicalState : std::uint8_t {
    Unknown = 0,
    Draft,
    Active,
    Deprecated,
    AliasOnly
};

// ==================================================
// 2. CANONICAL TAGS / ALIASES
// ==================================================

struct DescriptorAlias {
    std::string_view legacyTag;
    std::string_view canonicalTag;
};

inline constexpr std::string_view kCanonicalDescriptorTags[] = {
    "archive_sync",
    "terminal_sync",
    "remote_link",
    "workshop_service",
    "tower_sync",
    "power_pylon",
    "drone_station",
    "trade_network",
    "rail_depot",
    "orbital_uplink",
    "rail_fortress_hub",
    "recovery_fabricator",
    "industrial_gate",
    "industrial_survey",
    "industrial_outpost",
    "assembly_cell",
    "foundry_line",
    "reactor_yard",
    "capacitor_bank",
    "relay_substation",
    "service_bay",
    "water_reclaimer",
    "lanline_service_hub",
    "tank_service",
    "medical_support",
    "fey_ring",
    "echo_trace",
    "specialist_cryo",
    "data_card",
    "relay_packet",
    "debrief_record",
    "music_tape",
    "assistant_anchor",
    "region_gate",
    "factory_console"
};

inline constexpr DescriptorAlias kDescriptorAliases[] = {
    {"radio_tower", "tower_sync"},
    {"workshop_field_service", "workshop_service"},
    {"rail_freight", "rail_depot"},
    {"rail_fortress", "rail_fortress_hub"}
};

inline std::string_view NormalizeDescriptorTag(std::string_view tag) {
    for (const auto& alias : kDescriptorAliases) {
        if (alias.legacyTag == tag) {
            return alias.canonicalTag;
        }
    }
    return tag;
}

inline bool IsCanonicalDescriptorTag(std::string_view tag) {
    for (const auto canonical : kCanonicalDescriptorTags) {
        if (canonical == tag) {
            return true;
        }
    }
    return false;
}

// ==================================================
// 3. DESCRIPTOR CONTRACT
// ==================================================

enum class DescriptorDomain : std::uint8_t {
    Unknown = 0,
    Interaction,
    Infrastructure,
    Knowledge,
    Service,
    Transport,
    Recovery,
    Story,
    Lanline,
    Specialist,
    Transition
};

enum class DescriptorVisibility : std::uint8_t {
    Hidden = 0,
    EditorOnly,
    RuntimeOnly,
    Shared
};

struct GameplayDescriptorSpec {
    DescriptorTag scriptTag;
    std::string label;
    DescriptorDomain domain = DescriptorDomain::Unknown;
    DescriptorVisibility visibility = DescriptorVisibility::Shared;

    int preferredInteraction = 0;
    int preferredCategory = 0;

    bool requiresLinkTarget = false;
    bool contributesToProgression = false;
    bool contributesToInfrastructure = false;
    bool contributesToKnowledge = false;
    bool appearsInPipPad = false;
    bool appearsInEditorDrafts = false;
    bool canBeValidationBlocking = false;
};

// ==================================================
// 4. KNOWLEDGE / DATA CONTRACT
// ==================================================

enum class KnowledgeCarrierType : std::uint8_t {
    Unknown = 0,
    Tape,
    DataCard,
    RelayPacket,
    DebriefRecord,
    ArchiveSync,
    EchoTrace,
    TerminalRecord
};

enum class KnowledgeRewardType : std::uint8_t {
    None = 0,
    Lore,
    Recipe,
    InfrastructureUnlock,
    SpecialistUnlock,
    ObjectiveProgress,
    MapIntel,
    MusicBonus,
    AssistantHint
};

enum class KnowledgeCondition : std::uint8_t {
    Intact = 0,
    Damaged,
    Corrupted,
    Reconstructed,
    Consumed
};

struct KnowledgeReward {
    KnowledgeRewardType type = KnowledgeRewardType::None;
    std::string rewardKey;
    int amount = 0;
};

struct KnowledgeCarrier {
    KnowledgeId id;
    std::string title;
    std::string sourceTag;
    KnowledgeCarrierType type = KnowledgeCarrierType::Unknown;
    KnowledgeCondition condition = KnowledgeCondition::Intact;

    bool discovered = false;
    bool consumed = false;
    bool replayable = false;
    bool grantsProgression = false;

    std::string summary;
    std::string body;
    std::string mediaKey;
    std::vector<KnowledgeReward> rewards;
};

struct KnowledgeLedger {
    std::vector<KnowledgeCarrier> entries;
    std::optional<KnowledgeId> selectedId;
    int reconstructedCount = 0;
    int consumedCount = 0;
};

// ==================================================
// 5. MUSIC / CASSETTE BONUS CONTRACT
// ==================================================

enum class MusicEffectType : std::uint8_t {
    None = 0,
    SteadyHands,
    SalvageFocus,
    MarchDrive,
    EchoResolve,
    ThermalDiscipline,
    FieldRecovery
};

struct MusicEffectSpec {
    EffectId id;
    std::string label;
    MusicEffectType type = MusicEffectType::None;
    float durationSeconds = 0.0f;
    float magnitude = 0.0f;
};

struct ActiveMusicBonus {
    KnowledgeId sourceKnowledgeId;
    EffectId effectId;
    std::string displayName;
    float remainingSeconds = 0.0f;
    bool active = false;
};

// ==================================================
// 6. ASSISTANT / NERV CONTRACT
// ==================================================

enum class AssistantMode : std::uint8_t {
    Navigation = 0,
    Engineering,
    Tactical,
    Recovery,
    Archive,
    Lanline
};

enum class AssistantUrgency : std::uint8_t {
    Low = 0,
    Medium,
    High,
    Critical
};

struct AssistantAdvice {
    std::string id;
    AssistantMode mode = AssistantMode::Navigation;
    AssistantUrgency urgency = AssistantUrgency::Low;
    std::string title;
    std::string body;
    std::string relatedTargetId;
    bool acknowledged = false;
};

struct AssistantState {
    AssistantMode currentMode = AssistantMode::Navigation;
    bool enabled = true;
    bool cockpitLinked = false;
    bool canAutocycleHints = true;
    std::string displayName = "Nerv";
    std::optional<std::string> activeAdviceId;
    std::vector<AssistantAdvice> adviceLog;
};

// ==================================================
// 7. INFRASTRUCTURE / BACKBONE CONTRACT
// ==================================================

enum class InfrastructureNodeType : std::uint8_t {
    Unknown = 0,
    TowerSync,
    PowerPylon,
    DroneStation,
    TradeNetwork,
    RailDepot,
    OrbitalUplink,
    RailFortressHub,
    RecoveryFabricator,
    IndustrialGate,
    IndustrialSurvey,
    IndustrialOutpost,
    AssemblyCell,
    FoundryLine,
    ReactorYard,
    CapacitorBank,
    RelaySubstation,
    ServiceBay,
    WaterReclaimer,
    LanlineServiceHub,
    TankService,
    MedicalSupport,
    SpecialistCryo
};

enum class InfrastructureNodeState : std::uint8_t {
    Offline = 0,
    Available,
    Active,
    Blocked,
    Damaged,
    Degraded
};

struct InfrastructureNodeSnapshot {
    InfrastructureNodeType type = InfrastructureNodeType::Unknown;
    DescriptorTag sourceTag;
    RegistryId objectId;
    InfrastructureNodeState state = InfrastructureNodeState::Offline;
    std::string label;
    std::string note;
    bool discovered = false;
    bool contributesToRecoveryIndex = false;
};

struct InfrastructureReadModel {
    std::vector<InfrastructureNodeSnapshot> nodes;

    bool regionalGridOnline = false;
    bool localRelayAvailable = false;
    bool towerSyncRecovered = false;
    bool caravanRouteActive = false;
    bool droneStationsActive = false;
    bool tradeNetworkActive = false;
    bool railFreightActive = false;
    bool orbitalUplinkActive = false;
    bool railFortressActive = false;
    bool recoveryFabricatorActive = false;
    bool industrialGateUnlocked = false;

    float etherErosion = 0.0f;
    float infrastructureDecay = 0.0f;
    float routeContamination = 0.0f;
    int shelterRecoveryIndex = 0;
};

// ==================================================
// 8. TANK / UTILITY / FIELD SERVICE CONTRACT
// ==================================================

enum class TankUtilityModuleType : std::uint8_t {
    None = 0,
    TowCoupler,
    BucketShield,
    FieldRig,
    ThermalKit,
    ServiceRig
};

enum class TankServiceState : std::uint8_t {
    Nominal = 0,
    NeedsService,
    Critical,
    Offline
};

struct TankUtilityModuleState {
    TankUtilityModuleType type = TankUtilityModuleType::None;
    std::string moduleId;
    std::string label;
    bool installed = false;
    bool unlocked = false;
};

struct TankFieldServiceRecord {
    bool performed = false;
    float serviceQuality = 0.0f;
    float lastServiceWorldTime = 0.0f;
    std::string performedAtNodeId;
};

struct TankRuntimeExtensionState {
    std::vector<TankUtilityModuleState> utilityModules;
    TankServiceState serviceState = TankServiceState::Nominal;
    TankFieldServiceRecord fieldService;
    bool wirelessChargingAvailable = false;
    bool soulSyncLinked = false;
    bool soulLineReady = false;
    bool thermalModeUnlocked = false;
};

// ==================================================
// 9. SHELTER / RECOVERY CONTRACT
// ==================================================

enum class RecoveryMilestoneType : std::uint8_t {
    None = 0,
    FirstTowerOnline,
    PowerBackboneOnline,
    CaravanStabilized,
    DroneNetOnline,
    TradeNetEstablished,
    RailRouteEstablished,
    OrbitalSyncEstablished,
    FortressRouteEstablished,
    FabricationOnline,
    WaterSecurityEstablished
};

struct RecoveryMilestone {
    RecoveryMilestoneType type = RecoveryMilestoneType::None;
    bool completed = false;
    std::string completedByWorld;
    std::string completedByNode;
};

struct ShelterRecoveryState {
    int recoveryIndex = 0;
    int doctrineTier = 0;
    int assignedSpecialists = 0;
    int awakenedRecipes = 0;
    bool forwardCampUnlocked = false;
    bool scavengerTeamsActive = false;
    bool autopilotCaravanUnlocked = false;
    std::vector<RecoveryMilestone> milestones;
};

// ==================================================
// 10. CONSCIOUSNESS / RESONANCE CONTRACT
// ==================================================

enum class ResonanceEventType : std::uint8_t {
    None = 0,
    DiaryRead,
    ArchiveRecovered,
    EchoExperienced,
    SocialBondFormed,
    SurvivalTurningPoint,
    AssistantDeepLink,
    DebriefProcessed
};

struct ResonanceEvent {
    ResonanceEventType type = ResonanceEventType::None;
    std::string sourceId;
    float value = 0.0f;
};

struct ConsciousnessLineState {
    float resonance = 0.0f;
    int diaryEvents = 0;
    int archiveMoments = 0;
    int socialLinks = 0;
    int survivalTurningPoints = 0;
    std::vector<ResonanceEvent> eventLog;
};

// ==================================================
// 11. REGION / CHUNK / STREAMING-PREP CONTRACT
// ==================================================

struct RegionDescriptor {
    RegionId regionId;
    std::string label;
    bool isInterior = false;
    bool supportsStreaming = false;
    bool isCriticalDemoRegion = false;
};

struct RegionLink {
    RegionId fromRegion;
    RegionId toRegion;
    RegistryId viaObjectId;
    bool bidirectional = true;
};

struct RegionRuntimeState {
    RegionId regionId;
    bool loaded = false;
    bool visited = false;
    bool hasCheckpoint = false;
};

// ==================================================
// 12. LANLINE / SESSION DIGEST CONTRACT
// ==================================================

enum class LanlineMode : std::uint8_t {
    Solo = 0,
    LanHost,
    LanClient
};

enum class LanlineLifecycle : std::uint8_t {
    Inactive = 0,
    HostLobbyOpen,
    HostJoinPending,
    HostRuntimeActive,
    HostClientAccepted,
    ClientRuntimeJoined,
    RuntimeActive
};

enum class LanlinePresenceType : std::uint8_t {
    None = 0,
    Friend,
    Voice,
    Relay,
    Service
};

struct LanlinePresenceRecord {
    std::string displayName;
    LanlinePresenceType type = LanlinePresenceType::None;
    bool online = false;
    bool ready = false;
};

struct LanlineRuntimeDigest {
    LanlineMode mode = LanlineMode::Solo;
    LanlineLifecycle lifecycle = LanlineLifecycle::Inactive;
    WorldId worldName;
    SessionId sessionId;
    std::vector<LanlinePresenceRecord> visiblePresence;
    std::string lastRelayMessage;
};

// ==================================================
// 13. OBJECTIVE / ROUTE CONTRACT
// ==================================================

enum class ObjectivePhase : std::uint8_t {
    Hidden = 0,
    Available,
    Active,
    Completed,
    Failed
};

struct ObjectiveNode {
    ObjectiveId id;
    std::string title;
    std::string summary;
    ObjectivePhase phase = ObjectivePhase::Hidden;
    RegistryId relatedObjectId;
    DescriptorTag relatedTag;
};

struct RouteState {
    std::vector<ObjectiveNode> objectives;
    std::optional<ObjectiveId> activeObjectiveId;
    bool debriefUnlocked = false;
};

// ==================================================
// 14. VALIDATION CONTRACT
// ==================================================

enum class ValidationSeverity : std::uint8_t {
    Info = 0,
    Warning,
    Error,
    Blocking
};

enum class ValidationDomain : std::uint8_t {
    Unknown = 0,
    Registry,
    Authoring,
    Descriptor,
    LinkGraph,
    RegionGraph,
    Knowledge,
    Infrastructure,
    Persistence
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Info;
    ValidationDomain domain = ValidationDomain::Unknown;
    RegistryId objectId;
    std::string code;
    std::string message;
};

struct ValidationSummary {
    int infos = 0;
    int warnings = 0;
    int errors = 0;
    int blocking = 0;
    std::string label;
};

// ==================================================
// 15. PERSISTENCE / MIGRATION CONTRACT
// ==================================================

enum class PersistenceObjectType : std::uint8_t {
    Unknown = 0,
    SessionProfile,
    WorldState,
    LanlineSnapshot,
    EditorWorkspace
};

struct PersistenceVersion {
    int major = 1;
    int minor = 0;
    int patch = 0;
};

struct MigrationNote {
    std::string fromKey;
    std::string toKey;
    std::string description;
};

struct PersistenceContract {
    PersistenceObjectType type = PersistenceObjectType::Unknown;
    PersistenceVersion version;
    bool supportsLegacyAliases = false;
    bool usesAtomicWrite = false;
    std::vector<MigrationNote> migrations;
};

// ==================================================
// 16. DEMO / READINESS / DIAGNOSTICS CONTRACT
// ==================================================

enum class ReadinessSeverity : std::uint8_t {
    Ok = 0,
    Warning,
    Blocking
};

struct ReadinessIssue {
    ReadinessSeverity severity = ReadinessSeverity::Ok;
    std::string code;
    std::string message;
    std::string relatedId;
};

struct DemoReadinessReport {
    bool launcherReady = false;
    bool gameReady = false;
    bool editorReady = false;
    bool validationReady = false;
    bool saveFlowReady = false;
    bool worldRouteReady = false;
    bool pipPadReady = false;
    bool lanlineReady = false;
    std::vector<ReadinessIssue> issues;
};

struct WorldIntegrationReport {
    int objectCount = 0;
    int gameplayTaggedObjects = 0;
    int invalidObjects = 0;
    int warningObjects = 0;
    bool hasStarterTower = false;
    bool hasTankAnchor = false;
    bool hasServiceNode = false;
    bool hasRecoveryNode = false;
    bool hasLanlineNode = false;
};

struct RuntimeBootstrapReport {
    bool profileLoaded = false;
    bool worldLoaded = false;
    bool worldGenerated = false;
    bool starterInfrastructureReady = false;
    bool launchTicketApplied = false;
    bool staticEraserLoaded = false;
    std::string summary;
};

struct SaveOutcome {
    bool worldSaved = false;
    bool profileSaved = false;
    bool eraserSaved = false;
    std::string message;
};

// ==================================================
// 17. TEMPORARY AGGREGATE BUNDLE
// ==================================================

struct CanonFeatureStateBundle {
    KnowledgeLedger knowledge;
    InfrastructureReadModel infrastructure;
    TankRuntimeExtensionState tank;
    ShelterRecoveryState shelterRecovery;
    AssistantState assistant;
    ConsciousnessLineState consciousness;
    LanlineRuntimeDigest lanline;
    RouteState route;
    ActiveMusicBonus activeMusicBonus;
};

} // namespace bunker
Как использовать пока без боли

Пока никуда массово не подключай.
Просто создай файл:

include/MasterCanonTypes.hpp

А дальше уже поэтапно:

SessionProfiles.cpp → knowledge / assistant / consciousness / shelter
GameRuntimePipPad.cpp → infrastructure / assistant / music / knowledge UI
GameplayDescriptorRegistry.cpp → descriptor поля и канонические теги
WorldValidation.cpp → validation domain / issue / summary
GameRuntime.cpp → tank / route / terminal sync structures
LauncherSupport.cpp → Lanline digest / readiness
main.cpp → bootstrap report / save outcome

жесткая выжимка для нашего редактора, если брать не весь файл с разбором Creation Kit, а только то, что реально подходит под Bunker Protocol / Game_Project.

Сразу главный фильтр:
мы не переносим чужую архитектуру как есть, не копируем Bethesda-подход, а берем только полезные инженерные роли модулей и адаптируем их под уже зафиксированный канон проекта:
Launcher + Game + Editor, solo + LAN first, BunkerEditor как отдельный toolset уровня Creation Kit, бинарный формат мира BWLD/BWL2, Registry ID, authored world, world preview, object library, prefab/library workflow, semantic authoring и экспорт обратно в runtime-мир.

Что берем из файла точно
1. Реестр объектов и typed ID

Из файла очень полезна идея, что редактор не должен работать с “просто указателями”, а должен иметь центральный реестр сущностей, где каждый объект живет по своему ID, а связи между объектами идут через ID, а не через прямые адреса памяти. В самом файле это прямо описано как UID/FormID, слабые ссылки, XREF и переназначение временных ID при сохранении. Это идеально ложится на уже подтвержденную для проекта идею Registry_ID.

Для нашего редактора это значит:

у каждого authored object должен быть стабильный Registry ID;
ссылки между объектами должны храниться как targetRegistryId, а не как pointer/reference на runtime-объект;
редактор должен уметь находить “кто на кого ссылается”;
при дублировании и импорте должен выдаваться новый ID;
при сохранении нужен remap временных ID в постоянные.
2. Weak references и cross-reference map

Из файла надо брать не весь CK-style form database, а именно:

weak reference модель;
XREF карту;
предупреждение при удалении объекта, на который есть ссылки.
Это прямо полезно для нашего BunkerEditor, потому что у нас уже есть authored interaction/category/scriptTag/linkTarget flow, а значит связи начнут расти.

Нужный результат:

удаляешь tower_sync → редактор предупреждает, что на него ссылаются lanline_service_hub, power_pylon, relay_substation;
удаляешь prefab base object → видно, сколько размещений это затронет;
удаляешь fey_ring → видно, какие route-узлы ломаются.
3. Property editor / property grid

Из файла надо брать идею Property Sheets / Property Grid, но в современном виде, без копирования MFC/старого UI. В проекте редактор уже умеет редактировать позицию, размеры, здоровье, loot, interaction, category, spawn и метаданные мира, а значит property-layer уже есть и его надо развивать как единый инспектор объекта, а не как набор разрозненных полей.

Для нас это значит:

один правый инспектор объекта;
секции Transform, Gameplay, Interaction, Semantic, Loot, Visual, Runtime Notes;
если объект tank_service, показывать service-specific fields;
если объект fey_ring, показывать route fields;
если объект lanline_service_hub, показывать relay/service fields.
4. World preview + viewport interaction

Из файла стоит брать:

gizmos,
raycast/select,
snapping,
layer visibility,
bounds/debug overlays.
Это прямо совпадает с тем, что уже идет у тебя в Editor v1: world preview, selection, spawn marker, object move, viewport navigation в духе Creation Kit, interaction-helper overlay. То есть это не новая идея, а направление, которое уже подтверждено roadmap и которое надо продолжать.

Для нашего редактора это означает:

2D preview надо дальше вести к гибридному authoring viewport;
клик по объекту → select;
drag → move;
snap to grid;
focus/double click;
debug overlay для interaction radius, service radius, spawn зоны, power links.
5. Layer manager и visibility filters

Из файла полезен слой Layers & Visibility: это очень подходит для authored worlds с большим количеством сущностей. У тебя уже есть object search/filter и category-filter, значит следующий логичный шаг — layer manager и сохранение видимости слоев внутри editor session.

Нужные editor layers:

Terrain
Structures
Gameplay
Loot
Service
Fey
Spawn
Debug
Markers
Rail/Industrial
6. Dirty flags + Undo/Redo

Из файла это одна из самых полезных вещей для редактора:
не весь большой CK stack, а именно:

dirty state у объекта;
dirty state у мира;
undo/redo как стек дельт.

Для нас это означает:

любое изменение объекта помечает объект как dirty;
экспорт мира очищает dirty-флаг;
undo/redo хранит не весь мир целиком, а команду/дельту:
add object
remove object
move object
duplicate object
change property
assign spawn
assign semantic preset
7. Validation + warnings window

Из файла надо брать именно это.
В нем явно выделены Validation Module, Warnings & Logs, thread-safe logging, categorized warnings. Для нашего редактора это критично, потому что editor уже перестал быть просто map tool и стал toolset.

Нужные проверки редактора:

duplicate Registry ID
fey_ring без route target
lanline_service_hub без tower_sync
tank_service без service_bay / hangar context
medical_support без service backbone
spawn вне walkable/start-safe зоны
объект с interaction, но без radius/semantic target
broken linkTarget
orphaned prefab reference
invalid world export metadata
8. Virtual lists / object window / cell view

Из файла надо брать принцип, что большие списки должны быть виртуализированными, а не тупо рендерить все. Это подходит под Object Window, Cell View, prefab library, object palette и warnings list.

Для редактора это означает:

Object Window
Cell View
Prefab Library
Warnings
Search Results
должны быть готовы к большим authored worlds.
9. Binary world format и asset manifest

Из файла полезен не ESM/ESP как таковой, а идея, что мир должен хранить:

стабильную структуру;
ссылки на ресурсы;
секции/чанки;
версию формата;
asset manifest.
Это хорошо ложится на уже существующий BWLD/BWL2-контур проекта.

Для нас:

BWL2 надо держать как основной authored world format;
в заголовке мира должны быть:
version
world metadata
next available registry id
asset manifest
layer table
object table
prefab references
semantic links
10. Import assistant / small smart import window

Это прямо совпадает с каноном проекта: у тебя уже зафиксировано, что editor должен быть не только map editor, а полноценный authoring toolset, плюс есть отдельная идея маленького окна интеллектуального импорта по картинке/референсу. Это полезно брать как editor-side Import Assistant, а не как full AI generator.

Для нас это значит:

отдельное маленькое окно импорта;
drag/drop картинки;
ручная классификация:
prop
item
environment
structure
scene module
потом отправка в palette/prefab draft, а не магическое автосоздание готового мира.
Что брать ограниченно, не целиком
1. Plug-in / override logic

Из файла идея override-подмены полезна, но не как ESM/ESP-клон прямо сейчас. Нам нужна легкая layered world override model только для будущего:

base world
override layer
mission layer
temporary session layer.

Сейчас для редактора это значит:

заложить формат мира так, чтобы override потом был возможен;
но не строить сейчас полный mod stack как у Bethesda.
2. Asset provider / external libraries

Из файла можно взять идею AssetProvider, asset manifest, VFS, внешних библиотек ресурсов. Это полезно архитектурно. Но не надо сейчас уводить editor в реальную BA2/Fallout pipeline, если это не часть ближайшего production-плана. В файле это обсуждается как направление, но для нашего проекта это только дальняя опция.

Для нас:

да: AssetProvider
да: virtual paths
да: resource manifest
нет сейчас: обязательная поддержка BA2/DDS/Fallout 4 в production editor
3. Quest/dialogue/AI graph editing

Из файла это интересно, но для текущего Editor v1 брать это рано.
Сейчас редактор уже в работе как world/prefab/semantic authoring toolset; значит сначала нужно дожать мир, объекты, interaction, layers, warnings, import pipeline. Квестовый граф, dialogue tree и AI packages — это уже Editor v2+, не ближайший must-have.

Что не берем сейчас вообще
1. MFC / WinAPI / old CK shell

Нам не нужен старый Windows-монолит из CK. Из файла полезны роли окон, но не сама старая оболочка. Брать надо современную модульную форму уже существующего редактора, а не копировать MFC-подход.

2. Papyrus VM и полная скриптовая машина

Для нашего редактора сейчас это лишнее. Из файла стоит запомнить, что нужен мост между данными и логикой, но не строить полноценный CK-like VM слой на этом этапе.

3. ESM/ESP exact format

Не брать.
У нас свой BWLD/BWL2, свой editor, свой runtime. Из файла берем только принципы:

records
references
overrides
validation
asset manifest.
Но не формат Bethesda.
4. Полный BA2/DDS/Fallout asset compatibility как обязательную цель редактора

Это не должно сейчас уводить редактор в сторону. В твоем проекте канонически важнее:

authored worlds
item/object authoring
interaction semantics
export to runtime format
import assistant
а не совместимость с Fallout-архивами.
5. Security / firmware / чужие экосистемы

Это уже отдельно зафиксировано как нерелевантное для production stack проекта. Брать нельзя.

Что это значит именно для нашего редактора

Наша каноническая роль редактора уже зафиксирована:
он должен быть не облегчённым map tool, а полноценным authoring-комплексом уровня Creation Kit, адаптированным под Bunker Protocol, и работать не только с картой, но и с предметами, контейнерами, объектами мира, техническими и интерактивными сущностями.

Значит, редактор надо строить как набор этих слоев:

Editor Core
world open/save/export
current world session
dirty state
undo/redo
selected object context
Registry & References
Registry ID
weak references
XREF map
duplicate protection
remap on duplicate/import/save
World Authoring
object palette
prefab library
placement
spawn tools
semantic presets
layer manager
Viewport / Preview
select
move
focus
pan/zoom
snap
interaction overlays
debug markers
Inspectors
transform
category
interaction
loot
health
semantic tags
runtime notes
Validation / Warnings
errors
warnings
quick fix hints
export blockers
Import Assistant
drag/drop image
classify
create prefab draft
assign category
send to library
Export Pipeline
BWLD/BWL2
asset manifest
metadata
layer table
object refs
semantic links
Практический приоритет для внедрения

Если переводить это в реальную очередь для твоего проекта, то из всего файла для нашего editor надо внедрять в таком порядке:

Registry / weak refs / XREF
Dirty flags + Undo/Redo
Warnings / validation window
Layer Manager
Unified Property Inspector
World/object semantic references
Import Assistant
Asset manifest / export discipline
Override-ready world format
Только потом — сложные graphs / scripting editors
Готовая формулировка для ИИ

Можно вставить в файл как рабочее правило:

Берем из разобранного файла Creation Kit не конкретную реализацию Bethesda, а роли модулей, полезные для нашего BunkerEditor:

Registry ID и weak references
cross-reference map
property inspector
gizmo/raycast/snap
layer manager
dirty flags
undo/redo
warnings/validation
object window / cell view / virtual lists
binary world serialization with asset manifest
import assistant

Не берем сейчас:

MFC/старую оболочку
Papyrus VM
точный ESM/ESP формат
обязательную BA2/Fallout compatibility
full quest/dialogue/AI compiler
security / firmware / нерелевантные внешние ветки

Редактор проекта остается отдельным приложением уровня Creation Kit, но адаптированным под Bunker Protocol, BWLD/BWL2, Registry ID, authored world, BT-72, service/fey/industrial semantics и pipeline импорта пользовательских референсов.
Основа — наш проект.
Главный источник правды — текущий Bunker Protocol: его roadmap, текущий editor/runtime split, BWLD/BWL2, Registry ID, authored world, Launcher / Game / Editor, service/fey/industrial semantics. В текущих материалах прямо зафиксировано, что BunkerEditor должен вырасти в полноценный toolset уровня Creation Kit, но остаться редактором именно для нашего проекта, а не чужим клоном.

2. Файл с анализом CK — это надстройка.
Он нужен, чтобы подсветить, каких модулей и дисциплины обычно не хватает редактору:

typed ID / registry;
weak refs / XREF;
property inspector;
object window / cell view;
gizmos, raycast, snap;
warnings / validation;
chunked world serialization;
asset/provider layer.

3. Берем только совместимое с нашим каноном.
То, что уже совпадает с проектом:

редактор как отдельное приложение;
world authoring;
object/property editing;
export в бинарный world format;
Registry ID;
import pipeline;
Object Window, Cell View, Layer Manager, snap to grid.

4. Не берем буквально fallout-специфику.
Не надо тащить в проект:

MFC/старую оболочку CK;
Papyrus VM;
точный ESM/ESP пайплайн;
обязательную BA2/DDS/Fallout 4 совместимость как текущую цель;
чужую архитектуру целиком.
Даже сам файл местами уходит в эту сторону, но для нас это только дальние технические идеи, не production-приоритет.

5. Сам файл местами уже устарел относительно нашего текущего состояния.
В нем есть куски, где проект описан как очень ранний “скелет”, которому только предстоит ожить как редактор. Но по текущему состоянию это уже не так: Editor v1 у нас реально умеет загружать текущий runtime-мир, экспортировать *.bwld, валидировать конфликт Registry ID, дублировать объекты, работать с preview, prefab/library workflow, scriptTag/linkTarget и semantic presets. То есть файл надо читать как полезный инженерный комментарий, но не как точную диагностику текущего состояния редактора.

Если зафиксировать это одной фразой:

анализ Creation Kit из Fallout 4 — это дополнительный референсный слой для нашего BunkerEditor, который помогает понять, какие модули должен иметь серьезный редактор, но все решения принимаются только через канон и текущее состояние нашего проекта.

Дальше правильный способ использовать этот файл такой:

брать из него роли модулей;
сверять с текущим Editor v1;
добавлять только то, что усиливает наш pipeline;
не превращать проект в Fallout-tool clone.
Прописываю что именно добавим, если брать твой текущий проект как основу, а файл с разбором Creation Kit — только как дополнение по полезным модулям, без копирования чужой архитектуры.

Что добавим в редактор в первую очередь
1. Реестр объектов

Добавим нормальный Registry ID слой:

уникальный ID у каждого объекта мира;
генератор новых ID;
защита от дублей;
remap ID при дублировании и импорте;
weak references между объектами через ID, а не через прямые ссылки.
2. Cross-reference система

Добавим карту связей:

кто на кого ссылается;
предупреждение при удалении объекта, если он используется;
быстрый поиск зависимостей;
отдельный блок References в инспекторе объекта.
3. Единый Property Inspector

Сделаем один сильный правый инспектор:

Transform
Gameplay
Interaction
Semantic
Loot
Visual
Runtime Notes

И специальные секции для:

lanline_service_hub
fey_ring
tank_service
medical_support
4. Undo / Redo

Добавим нормальный стек действий:

add object
remove object
duplicate
move
rotate
scale
property change
semantic preset apply
spawn move
5. Warnings / Validation

Добавим окно предупреждений редактора:

duplicate Registry ID
broken linkTarget
fey_ring без route target
lanline_service_hub без tower_sync
tank_service без service_bay/hangar контекста
medical_support без backbone-контекста
объект вне допустимой зоны
некорректный export
6. Layer Manager

Добавим управляемые слои:

Terrain
Structures
Gameplay
Loot
Service
Fey
Spawn
Debug
Rail
Industrial

С возможностью:

скрывать/показывать;
блокировать редактирование слоя;
фильтровать поиск по слоям.
7. Улучшенный viewport

Добавим:

raycast select;
gizmo move/rotate/scale;
snap to grid;
focus on selected;
bounds overlay;
interaction radius overlay;
service radius overlay;
route/link visualization для service/fey объектов.
8. Object Window / Palette

Сделаем нормальное окно объектов:

поиск;
фильтр по категории;
фильтр по слою;
фильтр по semantic tag;
быстрый placement;
recent objects;
favorites.
9. Prefab / Library слой

Усилим библиотеку:

сохранение prefab из выбранных объектов;
prefab metadata;
preview и category;
повторное использование prefab в мирах;
предупреждения при изменении base prefab;
future support для prefab overrides.
10. Import Assistant

Добавим маленькое окно импорта:

drag/drop картинки или референса;
ручная классификация:
prop
item
structure
environment
scene module
создание prefab draft;
отправка в palette/library.
Что добавим в формат мира
11. Усиление BWLD/BWL2

Добавим/дожмем:

version header;
next available registry id;
asset manifest;
layer table;
object table;
semantic links;
prefab references;
export validation.
12. Override-ready структура

Не полный ESM/ESP-клон, а задел на будущее:

base world;
authoring override;
mission/session override;
merge-ready chunks.
Что добавим в связи editor ↔ runtime
13. Semantic world glue

Добавим editor/runtime bridge для:

tower_sync
lanline_service_hub
fey_ring
tank_service
medical_support
relay_substation
service_bay
water_reclaimer
14. Runtime-facing export discipline

Сделаем так, чтобы editor экспортировал мир без “скрытых ручных правок”:

export blockers;
export summary;
missing refs summary;
auto-fix простых ошибок.
Что добавим в сам проект вокруг редактора
15. Service/world glue

Дожмем:

unlock Lanline Services строго через tower_sync;
authored service anchors;
Fey Ring Network как authored world layer;
tank/medical/service hooks из editor в runtime.
16. Gameplay hooks

Добавим нормальные связи:

tank_service → реальные service kits;
medical_support → реальные medical supply hooks;
fey_ring → route state / schedule hooks;
service terminals → authored world interactions.
17. Relay / Registry инструменты

Добавим helper-слой:

поиск объекта по Registry ID;
поиск по scriptTag;
поиск по linkTarget;
поиск ссылок на объект;
jump-to-object from warnings/reference list.
Что не добавляем сейчас

Чтобы не расползтись:

не делаем Papyrus-подобную VM;
не делаем старый CK-style монолит;
не делаем полный quest/dialogue/AI editor прямо сейчас;
не делаем Fallout-совместимость как основную цель;
не делаем BA2/DDS интеграцию как обязательный production слой.
Самый правильный порядок внедрения
Registry ID + weak refs + XREF
Warnings / Validation
Undo / Redo
Layer Manager
Unified Property Inspector
Viewport gizmos / overlays / snap
Prefab / Library
Import Assistant
BWLD/BWL2 export discipline
Service/Fey authored glue
Короткая формула

Для нашего редактора добавляем не “Creation Kit целиком”, а его самые полезные функции:

реестр объектов,
связи по ID,
инспектор свойств,
viewport-инструменты,
слои,
validation,
undo/redo,
prefab library,
import assistant,
сильный world export.

Считаем этот файл дополнительным референсом по ролям модулей редактора, а основу решений держим на нашем проекте и его каноне. Дальше логично собрать из этого уже чистое, короткое Editor_TZ.md без мусора и без Bethesda-перегруза

фиксирую состояние без путаницы:

Текущий export/history path теперь закрыт не только backend-ом, но и production-useful navigation layer:

есть `LoadWorldExportHistory(...)`, snapshot/baseline paths и загрузка baseline snapshot;
есть history query/filter/preset слой в `WorldExport`;
есть smoke-тест `RunExportHistoryCheckpointSelectionSmoke()`, который теперь проверяет latest shipping/prototype/blocked selection, baseline-updated filter, compare preset resolution и no-match fallback;
в `Editor_Main.cpp` теперь есть filter row, compare preset combo, jump actions, compact filtered audit list, preview snapshot, historical diff, baseline diff и focus/fix по regressions.

Следующий точный ход после этого слоя уже закрыт:

`undo/redo` и `viewport authoring`

Следующий рабочий пакет теперь:

`prefab/library strengthening`, затем `import assistant`, затем `export discipline / world-format tightening`

Иначе говоря:
compare/history path заново не перепридумывать — он уже закрыт и дальше нужен только как поддерживаемый production слой editor/export pipeline.
Тогда это можно использовать только как локальный технический полигон, а не как основу публичного контента проекта.
Самый правильный ход сейчас такой:
Что можно делать


локально проверить AssetProvider/virtual path resolver;


проверить, что мир хранит пути и ссылки на ресурсы, а не сами тяжелые картинки внутри world-файла;


протестировать editor-side texture browser и preview на внешней библиотеке ресурсов;


держать это как внешний source library, а не как часть канонического контента проекта. В твоем файле как раз описана полезная идея: .wld/world format должен хранить ссылки и manifest, а AssetProvider решает, брать ресурс из обычной папки или из внешней библиотеки/архива.  


Чего не делать


не коммитить эти BA/картинки в публичный репозиторий;


не делать их частью shipping build;


не строить канонический визуальный стиль проекта на чужих ассетах;


не превращать редактор в “Fallout importer” как основную цель. В том же файле это уходит далеко в сторону и как production-цель для нашего проекта не годится. 


Как это использовать с пользой для проекта


через внешний AssetProvider;


через asset manifest в world/export pipeline;


через texture/material browser в editor;


только для внутренней проверки пайплайна:


path resolution


preview


material assignment


fallback loading


external library indexing.
Файл прямо подталкивает именно к такому подходу: внешний ресурсный слой, virtual path resolver и ссылки на текстуры, а не запихивание картинок прямо в мир.  




Если по-человечески:
как временный тестовый ресурсный банк — ок.
Как часть публичного/финального проекта — нет.
И для нашего курса это даже удобно: можно не трогать канон проекта, а спокойно проверить:


AssetProvider


texture lookup


external resource manifest


editor preview/browser


А уже потом заменить все это на свои ассеты.
Следующий практический шаг отсюда — добавить в план редактора внешнюю resource library support без вшивания чужих файлов в сам проект.

Разберем дальше уже по-взрослому.

Где проект сейчас реально сильный

Архитектура уже очень крепкая: проект собирает отдельные BunkerGame, BunkerLauncher, BunkerEditor, а также отдельный BunkerSmokeChecks; в BunkerEditor уже подключены WorldExport.cpp, WorldSemanticAuthoring.cpp и WorldValidation.cpp, а в game/launcher/editor используется общий набор модулей вроде GameplayDescriptorRegistry.cpp и SessionProfiles.cpp. Это уже не “набор экспериментов”, а нормальный product split с тестовым target.

ROADMAP.md тоже подтверждает зрелость: Этап 1 завершен, Launcher v1 отмечен как завершенный, а в списке подтвержденных сборок есть длинная цепочка успешных build_verify_ninja и build_verify_ninja_fresh, включая editor-side world preview, prefab/library workflow, BWL2, semantic authoring, Lanline Services persistence/world glue и launcher hardening. Это очень сильный сигнал, что позвоночник проекта уже собран.

Что видно по последнему прерванному таску

По твоим вставкам видно, что export/history path уже не в зачатке. В WorldExport backend уже есть:

пути для report / audit trail / snapshot / baseline;
загрузка истории через LoadWorldExportHistory(...);
сводка history entry;
построение validation snapshot;
загрузка baseline snapshot.
То есть backend уже умеет хранить и читать историю экспортов и validation-артефактов.

Smoke-тест RunExportHistoryCheckpointSelectionSmoke() уже проверяет важную вещь: два экспорта одного и того же мира, порядок history entries как newest-first, сравнение текущего состояния с новым и старым historical checkpoint, object-aware regressions/improvements и корректный разбор policyLabel/decisionLabel. Значит selected checkpoint compare уже реально сделан и тестируется, а не только обещан.

В Editor_Main.cpp это уже доведено до usable UI: есть baseline diff, historical checkpoint selection, preview historical snapshot, object-level regressions/improvements, кнопки Focus и Fix для baseline и historical regressions. То есть плоский список уже частично преодолен — ручной выбор checkpoint и сравнение уже живут в editor.

Что в этом слое еще не дожато

Судя по тому, что уже есть, недостает не “системы истории”, а именно последнего production-useful navigation layer:

быстрых фильтров по audit history;
compare presets;
jump-кнопок вида last successful shipping, last prototype, last blocked, last baseline updated;
helper-layer в WorldExport, который будет находить “последнюю подходящую запись”, а не заставлять editor жить на одном длинном combo. Это прямо вытекает из текущего состояния кода и твоего описания прерванного таска.

То есть самый точный вывод тут такой: таск не провален и не пустой — он уже на 70–80% сделан, осталось добить быстрый слой навигации и пресетов поверх уже работающего compare/history path.

Разбор по папкам
include/

Самый ближайший кандидат — include/WorldExport.hpp. Там уже есть типы для baseline/snapshot/history/result, так что следующий логичный шаг — добавить query/filter/preset слой, а не изобретать новые сущности elsewhere. Это хорошая точка для:

history filters,
compare preset enums,
selection/query structs,
helper declarations для latest matching entry.
src/

Главный ближайший файл — src/WorldExport.cpp. Там уже есть все кирпичи для history loading и snapshot parsing, значит именно сюда надо положить:

фильтрацию history entries,
выбор latest matching export,
compare preset resolution,
compact summary/badge helpers.
Это даст production-useful слой без ломки существующего backend.
Editor/

Editor_Main.cpp уже тащит на себе runtime/export окно, preview, baseline diff, historical compare и action-кнопки. Значит здесь следующий ход — не переписывать UI, а надстроить:

filter row,
quick buttons,
compare preset combo,
compact filtered history view,
fallback messages, если подходящего checkpoint нет.
tests/

tests/SmokeChecks.cpp уже хорошо покрывает historical checkpoint selection. Следующий честный шаг — не писать новый test harness, а просто расширить существующий smoke-блок проверками на:

latest shipping selection,
latest blocked selection,
prototype-only filter,
no-match fallback.
docs

ROADMAP.md после этого нужно будет синхронизировать коротко и по факту: не “мы придумали историю экспортов”, а “добавлены быстрые history filters и compare presets”. Это особенно важно, потому что roadmap у тебя уже очень насыщенный, и ему нужна точная синхронизация с реальным состоянием кода.

Что кроме export/history сейчас выглядит самым дорогим хвостом

После этого слоя самыми дорогими открытыми блоками выглядят:

Registry ID / XREF / strong reference discipline в editor;
unified property inspector;
layer manager;
undo/redo;
дальнейшее hardening validation/warnings;
deeper semantic authoring glue между editor и runtime.
Это логично, потому что editor уже вырос из “viewer”, и теперь дорогие дыры — не базовые, а tool-grade.
На чем бы я сосредоточился дальше

Когда токены вернутся, я бы делал строго по порядку:

WorldExport.hpp — query/filter/preset types.
WorldExport.cpp — latest matching entry + filtering + quick compare resolution.
Editor_Main.cpp — быстрые кнопки и filtered selection поверх уже существующего UI.
SmokeChecks.cpp — тесты на новые быстрые режимы.
ROADMAP.md — короткий sync по факту.

Это сейчас самый дешевый по риску и самый полезный по отдаче ход, потому что existing compare/history layer уже живой.
что останется по большой версии после закрытия export/history filters + compare presets.

Что будет считаться закрытым после этого слоя

Если добить:

history filters,
quick compare presets,
jump к last successful shipping / prototype / blocked / baseline,
smoke на эти режимы,

то editor получит нормальный production-useful export audit workflow, а не просто набор preview-окон. Это логично, потому что backend истории, baseline/snapshot compare и UI для historical checkpoint уже есть, а недостает именно удобного navigation layer поверх них.

После этого можно будет считать, что у редактора экспортный контур уже взрослый:

world export,
validation report,
audit trail,
shipping baseline,
historical compare,
object-aware regressions,
быстрый переход по ключевым checkpoint’ам.
Что останется самым дорогим после этого
1. Editor-hardening

Это будет следующий большой блок:

Registry ID как полный жесткий слой,
weak refs / XREF,
unified property inspector,
layer manager,
undo/redo,
более сильный warnings/validation UX.
Это уже тот уровень, который реально приближает BunkerEditor к сильному production toolset, а не просто к редактору карты. Сам проект уже явно идет в эту сторону: editor выделен в отдельный target и обрастает специализированными модулями вроде EditorSupport.cpp, PrefabLibrary.cpp, WorldExport.cpp, WorldSemanticAuthoring.cpp, WorldValidation.cpp. (raw.githubusercontent.com
)
2. Start vertical slice

По roadmap Этап 3. Вертикальный срез старта все еще “в работе”, хотя база старта уже собрана. Значит после export/history слоя одной из главных больших задач останется не новая система, а дожим стартового опыта как цельной игровой зоны и маршрута, а не просто цепочки уже существующих систем. (raw.githubusercontent.com
)

3. Combat / RPG / BT-72 depth

Roadmap по-прежнему держит Этап 4. Базовый боевой и RPG-слой как “в работе”. То есть для большой версии потребуется не просто наличие танка, сервиса, прогрессии и SPECIAL-подобных идей, а именно достаточная глубина и сцепка этих систем. (raw.githubusercontent.com
)

4. Recovery / industry / logistics depth

Это уже одна из сильнейших сторон проекта, но для большой версии ее надо не просто продолжать линейно, а превращать в настоящий mid-game operational core loop. По roadmap backbone уже серьезно вырос: power grid, drone stations, trade network, rail freight, orbital uplink, rail fortress, recovery fabricator, service bay, water reclaimer и дальше. Большая версия — это когда это не только checkpoints, а устойчивый игровой хребет. (raw.githubusercontent.com
)

5. Service / Fey / runtime glue

Lanline Services уже заметно окреп: есть persistence, dual-currency logic, editor-side service/fey anchors и tower-sync-aware unlock. Но для большой версии останется дожать:

полную runtime/launcher/editor консистентность,
более глубокие gameplay hooks,
service/fey world coupling,
честный статус чата/voice как shell vs real transport.
Это уже не фундаментальная дыра, а зрелая интеграционная работа. (raw.githubusercontent.com
)
6. Repo / release discipline

Это не игровая механика, но до большой версии все равно придется:

дочистить repo hygiene,
не держать лишние build artifacts в публичном репо,
синхронизировать ROADMAP.md без противоречий,
лучше закрепить release flow.
Сейчас код уже серьезный, а репозиторий по форме все еще частично dev-tree. (raw.githubusercontent.com
)
Что уже не выглядит страшной проблемой

Вот что уже не главная угроза:

трехчастная архитектура Launcher / Game / Editor;
сам факт существования редактора;
world export как таковой;
history/baseline/audit как направление;
service persistence как факт;
industrial backbone как направление.
Это уже реально есть и живет в коде/roadmap, а не только в идеях. (raw.githubusercontent.com
) (raw.githubusercontent.com
)
Моя обновленная оценка после закрытия этого слоя

Если закрыть именно export/history filters + compare presets, я бы оценил проект так:

до крепкой v1: 82–85%
до большой полноценной версии: 55–60%

Почему рост не гигантский: этот слой не делает новую игру, но сильно повышает production maturity editor/export pipeline, а это очень важно именно для большой версии. У тебя уже собран позвоночник проекта, и сейчас ценность все чаще дают не новые “большие идеи”, а такие finish layers. (raw.githubusercontent.com
) (raw.githubusercontent.com
)

Самый правильный маршрут после этого

После закрытия history/filter/preset слоя я бы двигался так:

Registry ID / XREF / warnings hardening
unified property inspector
layer manager
undo/redo
start vertical slice polish
deeper BT-72 / combat / RPG
richer recovery/industry/logistics core
service/fey full glue
release discipline / cleanup

Это и будет самый прямой путь от “сильная системная ранняя версия” к “большой полноценной версии”.
Вот следующие 10 ходов подряд после текущего export/history слоя.

Судя по коду, у тебя уже есть: history backend, baseline/snapshot compare, historical checkpoint selection, object-aware regressions/improvements и UI для baseline/history diff. Значит дальше надо идти не в новые большие идеи, а в editor-hardening и world authoring maturity.

1. Добить быстрые history filters и compare presets

Закрыть текущий прерванный слой до конца:

quick buttons: Last successful shipping, Last prototype, Last blocked, Last baseline
filter row
compare preset selection
smoke-тесты на быстрые режимы
Это последний кусок поверх уже живого history workflow.
2. Ввести полный Registry ID слой

Сделать объектный реестр редактора жестким:

уникальный ID у каждого объекта
генерация новых ID
remap при duplicate/import
поиск по ID
запрет дублей
Это следующий настоящий фундамент editor-grade уровня.
3. Добавить XREF / weak references

После Registry ID сделать:

incoming/outgoing references
предупреждение при удалении объекта, на который ссылаются
jump-to-reference
reference block в inspector
Это сразу усилит semantic authoring и validation.
4. Усилить validation/warnings окно

Не просто список ошибок, а production warnings:

broken refs
duplicate IDs
missing semantic anchors
invalid service/fey links
export blockers
quick fix suggestions
Текущий validation/export путь уже сильный, значит warnings теперь надо дотянуть до tool-grade UX.
5. Сделать unified property inspector

Собрать один сильный инспектор вместо россыпи полей:

Transform
Gameplay
Interaction
Semantic
Loot
Visual
Runtime Notes
спецсекции для lanline_service_hub, fey_ring, tank_service, medical_support
6. Добавить Layer Manager

Нужны слои:

Terrain
Structures
Gameplay
Loot
Service
Fey
Spawn
Debug
Rail
Industrial

И действия:

show/hide
lock/unlock
filter by layer
assign selected to layer
7. Ввести Undo / Redo

Минимальный стек команд:

add/remove object
duplicate
move/rotate/scale
property change
semantic preset apply
spawn move
link change
Без этого editor уже будет ощущаться узким местом.
8. Дожать viewport tools

Сделать viewport по-настоящему рабочим:

raycast select
move/rotate/scale gizmo
snap to grid
focus on selected
bounds overlay
interaction/service radius overlay
route/link visualization для service/fey объектов
9. Усилить Prefab / Library

Следующий production шаг:

сохранить prefab из selection
metadata/category/preview
prefab usage tracking
warnings on broken prefab refs
future-ready base for prefab overrides
10. Полировать стартовый vertical slice

После editor-hardening вернуться к игре:

цельность стартового мира
onboarding
BT-72 сцепка
recovery/service/fey hooks в первом маршруте
убрать пустоты и “системность ради системности”
Это уже шаг от сильного toolset к большой версии самой игры.
Самый правильный порядок по файлам

Если очень жестко:

WorldExport.hpp/.cpp
Editor_Main.cpp
tests/SmokeChecks.cpp
World / SessionProfiles для Registry ID и refs
WorldValidation / editor warnings
inspector UI
layer UI/state
undo/redo stack
viewport tools
prefab/library + start slice polish
Коротко
Вот один прямой маршрут, без развилок.
Сейчас текущий слой — это export/history path. Он уже живой: есть audit history, baseline/snapshot compare, historical checkpoint selection, object-aware regressions/improvements и UI под это в editor. Значит сначала его надо дожать до конца, а не перескакивать дальше.  
Маршрут такой


Добить export/history


quick filters


compare presets


jump to last successful shipping / prototype / blocked / baseline


smoke-тесты на эти режимы. 




Сделать жесткий Registry ID слой


уникальные ID


remap при duplicate/import


поиск по ID


запрет дублей




Добавить XREF / weak refs


кто на кого ссылается


warning при удалении


jump-to-reference




Дожать editor-hardening


warnings/validation UX


unified property inspector


layer manager


undo/redo




Дожать viewport как рабочий tool


raycast select


gizmo


snap


overlays для service/fey/link logic




Усилить prefab/library


prefab save/reuse


usage tracking


prefab warnings




Вернуться к игре


polish стартового vertical slice


BT-72 / combat / RPG depth


recovery/industry/logistics как настоящий core loop




Потом уже большая версия


full service/fey glue


content expansion


release/repo discipline




Самое короткое
Сначала добиваем editor pipeline, потом дожимаем game slice, потом расширяем игру.
Если еще короче:
export/history → Registry/XREF → editor-hardening → viewport/prefab → start slice polish → big version.

Да. Самое важное еще вот что:
Первое: не потеряй точку входа после паузы.
Когда токены вернутся, пусть ИИ начинает не “с нуля”, а ровно с текущего места: WorldExport уже умеет history, baseline, snapshot compare и historical checkpoint selection, так что надо добивать именно quick filters/presets, а не переписывать слой заново.  
Второе: зафиксируй один ближайший checkpoint в тексте.
Лучше прямо в Use_this_One.md или рядом коротко записать:


что уже есть;


что осталось;


какие 4 файла трогать следующими;


что не трогать.
Иначе следующий ИИ может снова расползтись.


Третье: не открывай сейчас новые большие ветки.
До конца текущего слоя не надо:


новый editor subsystem,


новый gameplay subsystem,


новый import pipeline,


новый Fallout-asset слой.
Сначала закрыть текущий production-useful кусок.


Четвертое: не тащи чужие BA/картинки в shipping-путь.
Их можно держать только как локальный технический полигон для preview/provider/browser, но не как основу публичной сборки.
Пятое: после quick filters сразу идти в editor-hardening, а не в новую красивую фичу:


Registry ID


XREF


warnings/validation


unified inspector


layer manager


undo/redo


Если хочешь, я могу прямо сейчас написать тебе сверхкороткий checkpoint note на 10 строк, который ты просто вставишь в Use_this_One.md.

мир **делают разработчики в редакторе**;
- игрок **не редактирует карту как редактором**;
- но игрок **может строить внутри переносимого Camp/AIMP** как по логике Fallout 76;
- **BT-72, Camp/AIMP и мастерские — три разные системы**;
- `State of Decay 2` берем как пример того, **как authored мир живет через игровые механики**, а не как прямую копию игры;
- Windsurf должен **внедрять максимум возможного**, а не уходить в обсуждения.

Скопируй и вставь это в Windsurf:

```text id="296z8x"
Ты работаешь над текущим проектом Bunker Protocol / Game_Project.

Твоя задача:
не спорить с направлением, не расползаться в новые документы и не перепридумывать проект, а собрать и внедрить максимум возможного из текущего репозитория, `Next.md`, `Next_split/*`, `ROADMAP.md` и канонических правил ниже.

Главный принцип:
меньше текста — больше реальной реализации.

==================================================
1. ЖЕСТКИЙ КАНОН ПРОЕКТА
==================================================

Проект:
- `Bunker Protocol`

Архитектура:
- `BunkerLauncher`
- `BunkerGame`
- `BunkerEditor`
- `BunkerSmokeChecks`

Основные pillars:
- BT-72
- recovery
- heavy-tech
- industry
- logistics
- persistence
- authored world
- editor как production tool
- launcher как обязательная точка входа

Сетевой и сервисный слой:
- `solo + LAN first`
- `Lanline - optime`
- `Lanline Services`
- `Fey Ring Network`

Экономика:
- за игровые деньги: ресурсы, repair kits, medical, service items
- за реальные деньги: только cosmetics / symbolic support
- без оружия, готовых танков и боевых преимуществ за реальные деньги

Service unlock:
- через восстановление инфраструктуры
- в первую очередь через `tower_sync`

==================================================
2. ОЧЕНЬ ВАЖНО: МИР, CAMP, МАСТЕРСКИЕ, BT-72
==================================================

Это четыре разные вещи, их нельзя смешивать.

------------------------------
2.1. Authored world
------------------------------

Мир делается разработчиками в редакторе.
Разработчик через `BunkerEditor`:
- создает карту;
- расставляет дома, мастерские, сервисные узлы, маршруты, точки интереса;
- собирает authored геометрию и authored world states.

Игрок не должен:
- редактировать всю карту как редактором;
- рисовать карту;
- произвольно перестраивать authored мир без игровых ограничений;
- глобально менять геометрию мира.

------------------------------
2.2. Camp / AIMP
------------------------------

У игрока есть отдельная игровая система строительства:
- `Camp / AIMP`

Это:
- переносимая и разворачиваемая игроком зона;
- аналог по общему принципу Fallout 76 / Fallout 4 style camp logic;
- игрок носит ее с собой, ставит, может перенести и развернуть в другом месте по игровым правилам.

Внутри радиуса Camp/AIMP игрок может:
- ставить свои объекты;
- перемещать свои постройки;
- перестраивать свою camp-зону;
- организовывать локальную базу / полевой узел / сервисную точку.

Но:
- только в пределах Camp/AIMP;
- не по всей карте;
- не как полноценным редактором мира.

------------------------------
2.3. Мастерские
------------------------------

Мастерские — это отдельная система authored мира.

Это:
- заранее расставленные разработчиками точки мира;
- объекты, которые можно зачистить, захватить, восстановить, использовать;
- сервисные / производственные / логистические узлы authored мира.

Мастерская — это НЕ Camp/AIMP.
Мастерская — это НЕ BT-72.

Игрок может:
- находить мастерскую;
- зачищать;
- восстанавливать;
- подключать;
- использовать в recovery/industry/service loop.

------------------------------
2.4. BT-72
------------------------------

BT-72 — это отдельная система.
Не Camp.
Не мастерская.

BT-72 — это центральный напарник-платформа:
- по роли близок к титану из Titanfall 2;
- но оформлен как развиваемая классовая боевая платформа;
- с модификацией и обслуживанием в духе Fallout 76 / Fallout 4;
- с читаемой модульной специализацией как в WoT Blitz.

BT-72 — это:
- напарник;
- боевая платформа;
- инженерная платформа;
- ключ к тяжелой расчистке;
- часть progression;
- часть recovery loop;
- часть service and modification loop.

==================================================
3. КАКОЙ ПРИНЦИП БЕРЕМ ОТ STATE OF DECAY 2
==================================================

State of Decay 2 использовать только как пример того,
как authored мир взаимодействует с игровыми механиками.

Берем:
- базу / домашний узел;
- вылазку;
- риск;
- добычу;
- возврат;
- реальное изменение состояния базы и мира;
- ощущение, что вылазка имеет цену и последствия.

Не копируем игру буквально.

Для Bunker Protocol это переводится так:
- authored карта создается разработчиком один раз;
- игрок живет внутри нее через игровые механики;
- игрок не редактирует карту как редактором;
- игрок меняет authored мир через:
  - зачистку,
  - расчистку,
  - восстановление,
  - захват,
  - запуск узлов,
  - восстановление инфраструктуры,
  - открытие маршрутов,
  - эксплуатацию мастерских,
  - развертывание Camp/AIMP,
  - использование BT-72.

==================================================
4. ИГРОК МОЖЕТ И НЕ МОЖЕТ
==================================================

Игрок может:
- зачистить дом;
- зачистить объект;
- очистить мастерскую;
- восстановить сервисный узел;
- расчистить завал;
- включить релейку;
- открыть маршрут;
- развернуть Camp/AIMP;
- строить и двигать свои объекты внутри Camp/AIMP;
- обслуживать и модифицировать BT-72;
- захватывать и использовать мастерские;
- переводить authored мир из мертвого состояния в рабочее через игровые механики.

Игрок не может:
- свободно редактировать всю карту;
- двигать authored здания по всей карте как редактором;
- рисовать карту;
- свободно менять authored геометрию без ограничений;
- превращать игру в sandbox editor mode.

==================================================
5. РОЛЬ BT-72
==================================================

BT-72 считать не “просто танком”, а центральной напарник-платформой.

Использовать следующие опорные правила:

- линк с BT-72 — важная часть раннего маршрута;
- BT-72 участвует в progression;
- BT-72 нужен для тяжелой расчистки и тяжелой боевой работы;
- пешком можно исследовать, лутать, входить в дома, поднимать мелкие задачи;
- с BT-72 игрок:
  - проламывает,
  - удерживает,
  - тащит,
  - расчищает,
  - обслуживает тяжелый recovery loop,
  - работает с частью опасных зон.

Развитие BT-72:
- классовая роль;
- модульность;
- обслуживание;
- улучшения;
- сервис;
- ремонт;
- энергосистема;
- damage states;
- utility rigs;
- боевые модули;
- инженерные модули.

Workshop/service around BT-72:
- не “просто починка танка”;
- а узел обслуживания, модификации, пополнения, ремонта и развития билда.

==================================================
6. ТЕКУЩИЙ ПРИОРИТЕТ РАЗРАБОТКИ
==================================================

Не начинать проект заново.
Не строить новые красивые editor-feature просто ради editor-feature.

Следующие реальные направления:

1. перечитать `Next.md`
2. перечитать `Next_split/*`
3. перечитать `ROADMAP.md`
4. проверить, что уже реально внедрено
5. продолжить с текущего пакета, который ближе всего к завершению игры

Текущий приоритет считать таким:
- `Launcher / Lanline Services / runtime return`
- затем `start vertical slice polish`
- затем `BT-72 / combat / RPG depth`
- затем `recovery / industry / logistics` как более плотный mid-game backbone

Не уходить обратно в бесконечный editor-hardening, если код уже ушел дальше.

==================================================
7. ЧТО ДЕЛАТЬ С .MD ФАЙЛАМИ
==================================================

Твоя задача — не плодить новые документы, а сокращать старые за счет внедрения.

Для каждого `.md`:
- если это живой канон — оставить, но сократить до актуального;
- если это active todo — внедрить и удалить выполненное;
- если это backlog — оставить как backlog;
- если это summary, который уже дублирует код — сократить или удалить после переноса ценного;
- если это устаревший промежуточный документ — убрать после переноса полезного.

Особенно внимательно обработать:
- `Next.md`
- `Next_split/*`
- `ROADMAP.md`
- `MASTER_PROMPT.md`
- `KOROTKOE_TZ_CLEAN.md`
- `Editor_TZ_short.md`
- `LANLINE_BOUNDARY.md`
- `AI_Wright_here.md`
- `SOURCE_EXTRACTION_REPORT.md`
- `trash.md`
- `Use_this_One.md`

Главный принцип:
не держать в тексте то, что уже может жить в коде, тестах и короткой живой документации.

==================================================
8. ИСТОЧНИКИ ПРАВДЫ
==================================================

Приоритет истины:
1. текущий код
2. тесты / smoke checks
3. `ROADMAP.md`
4. `Next.md`
5. `Next_split/*`
6. `Use_this_One.md`
7. остальные `.md`
8. анализ Creation Kit — только как инженерный референс

Если старый текст противоречит текущему коду:
- верить коду;
- адаптировать текст;
- не откатывать проект назад.

==================================================
9. ЧЕГО НЕЛЬЗЯ ДЕЛАТЬ
==================================================

Нельзя:
- отклоняться от темы проекта;
- превращать игру в generic survival-shooter;
- превращать проект в прямой Fallout-клон;
- копировать чужой код;
- ломать сборку;
- ломать split `Launcher / Game / Editor`;
- делать pay-to-win;
- продавать оружие, готовые танки, боевые преимущества;
- превращать `Lanline Services` в настоящий интернет-браузер;
- превращать `BunkerEditor` в прямой клон Creation Kit;
- превращать authored world в player-side full editor.

==================================================
10. BA / ЧУЖИЕ ТЕКСТУРЫ
==================================================

Если локально есть BA и картинки:
- использовать только как технический полигон;
- не коммитить;
- не включать в shipping build;
- не делать проект визуально зависимым от них.

Разрешено только для:
- AssetProvider
- virtual path resolver
- preview/browser
- external library indexing
- material assignment testing

==================================================
11. РАБОЧИЙ ЦИКЛ
==================================================

На каждый заход:
1. перечитай нужные `.md`
2. перечитай нужные `.hpp/.cpp`
3. проверь, что уже реализовано
4. внедри максимум возможного
5. проверь сборку
6. проверь smoke/tests
7. сократи `Next.md`
8. сократи `Next_split/*`
9. сократи дублирующие `.md`, если их смысл уже ушел в код
10. кратко обнови `ROADMAP.md`
11. все блокеры запиши в `Use_this_One.md`

==================================================
12. DEFINITION OF DONE
==================================================

Слой считается закрытым только если:
- код реально внедрен;
- сборка проходит;
- smoke/tests проходят;
- соответствующие пункты удалены из `Next.md` / `Next_split/*`;
- дублирующие `.md` сокращены;
- `ROADMAP.md` синхронизирован;
- не осталось текста, который просто повторяет уже реализованный код.

==================================================
13. ГЛАВНЫЙ ПРИНЦИП
==================================================

Не анализируй бесконечно.
Не спорь с направлением.
Не оставляй реализуемые вещи в тексте.

Просто:
- бери из проекта и `.md` все, что уже готово к встройке;
- внедряй максимум возможного;
- сокращай документы по факту внедрения;
- не делай мир за разработчика;
- делай системы, чтобы разработчик сам собирал мир в редакторе;
- двигай проект к реальному завершению.
```

И отдельно короткий **канонический блок**, который можно кинуть ему следом, если захочешь:

```text id="9984kc"
Запомни жестко:

- BT-72, Camp/AIMP и мастерские — три разные системы.
- Мир делает разработчик в редакторе.
- Игрок не редактирует всю карту как редактором.
- Игрок может свободно строить только в пределах переносимого Camp/AIMP.
- Мастерские — authored world nodes, которые можно зачищать, захватывать, восстанавливать и использовать.
- BT-72 — центральная напарник-платформа: бой, расчистка, сервис, модификация, progression.
- State of Decay 2 использовать только как пример того, как authored мир живет через игровые механики.
- Не делать игру “редактором для игрока”.
```

Если хочешь, я еще сделаю **ультракороткую версию на 10 строк**, чтобы ты отправил ее сразу после этого как “не забудь”.

# First Playable Route — System Sequence

Важно:
- это НЕ карта;
- это НЕ authored layout;
- это НЕ level design;
- это последовательность шагов первого играбельного маршрута;
- карту и расстановку разработчик делает сам в редакторе;
- сейчас нужно реализовывать только системную поддержку этого маршрута.

## Канон маршрута

1. **Интро / видеоролик**
   - коротко задает тон мира;
   - показывает аварию, упадок, состояние бункера;
   - подводит к пробуждению и будущей роли BT-72.

2. **Пробуждение в криокамере**
   - игрок получает контроль;
   - первые системные сообщения;
   - базовое понимание, что бункер в аварийном состоянии.

3. **Внутренний проход по бункеру**
   - ранний пеший отрезок;
   - первые радтараканы;
   - первая дубинка / примитивное оружие ближнего боя;
   - базовый лут, двери, панели, простые взаимодействия;
   - это учебный пеший слой до получения Pip-Pad.

4. **Предвосхищение Pip-Pad**
   - до получения Pip-Pad игрок находит:
     - бумажные чертежи,
     - старые версии,
     - заметки,
     - техдокументы,
     - наброски на столах.
   - Pip-Pad не должен лежать как магический предмет без контекста.

5. **Получение Pip-Pad**
   - открывает базовую системную сводку;
   - objective summary;
   - архивные/технические следы;
   - ранние recovery hints;
   - player-facing diagnostics.

6. **Контакт с BT-72**
   - игрок находит не готовый танк, а:
     - корпус,
     - отдельно ядро,
     - голозаписи,
     - чертежи,
     - сервисные следы.
   - BT-72 не должен выдаваться мгновенно как готовая награда.

7. **Восстановление BT-72 как ранняя техническая цель**
   - игрок находит голозаписи/чертежи:
     - на полке,
     - на столе,
     - в ящике/тумбочке,
     - на мобильном столе с инструментами.
   - по мере нахождения:
     - можно открывать схемы,
     - собирать материалы,
     - восстанавливать части поэтапно
       ИЛИ
     - собрать несколько записей и закрыть восстановление более крупным пакетом.
   - ранний loop:
     - поиск
     - понимание
     - материалы
     - восстановление

8. **Sync / Link с BT-72**
   - отдельный большой payoff;
   - cockpit / HUD / смена ритма игры;
   - фиксирует переход от чисто пешего слоя к машине-партнеру.

9. **Учебный танковый кусок**
   - движение;
   - cockpit / camera;
   - базовый HUD;
   - одна-две простые функции;
   - первое понимание ресурса и веса машины.

10. **Получение схемы модуля расчистки**
   - тяжелая расчистка не дается бесплатно;
   - игрок находит голозапись/чертеж модуля расчистки;
   - собирает материалы;
   - устанавливает модуль.

11. **Тяжелая расчистка**
   - пешком не пройти;
   - BT-72 + модуль расчистки обязательны;
   - это первый большой инженерный payoff машины.

12. **Первый бой**
   - короткий, понятный authored combat encounter;
   - проверка BT-72 как боевой платформы;
   - без тупых волн зомби;
   - важны ясная цель, риск и последствия.

13. **Первый сервис / передышка**
   - после боя игрок должен получить:
     - первый сервисный payoff,
     - частичную починку,
     - пополнение,
     - workshop / service point / field service feeling.
   - это не конец уровня, а вход в дальнейший recovery loop.

14. **Намек на большую городскую зону**
   - впереди большая зона, которую потом нужно:
     - расчищать от гулей,
     - от роботов / протектронов,
     - от машин,
     - от мусора,
     - от завалов.
   - это не надо сейчас строить как карту;
   - нужно только системно поддержать такой следующий этап.

15. **Tower / Recovery payoff**
   - первый реальный системный результат;
   - включение/восстановление узла;
   - изменение recovery status;
   - ощущение, что вылазка реально сдвинула мир.

16. **Возврат / дебриф / следующий хук**
   - summary;
   - новые задачи;
   - доп. события;
   - новые системные направления;
   - без тупого wave-survival режима.

## Что реализовывать сейчас

Реализовывать НЕ карту, а только системную поддержку маршрута:

- intro / start state hooks
- cryo awakening flow
- early melee + vermin encounter support
- Pip-Pad acquisition flow
- blueprint / holo-record discovery flow
- staged BT-72 restoration logic
- BT-72 sync/link flow
- tank tutorial support
- clearance module unlock/install flow
- heavy clearance gameplay support
- first combat encounter support
- first service/rest flow
- tower/recovery payoff hooks
- debrief / next hook flow

## Очень важно

Не делать сейчас:
- карту;
- authored layout;
- level geometry;
- world placement;
- actual city layout.

Это делает разработчик в редакторе сам.

Сейчас нужно только:
- состояния,
- последовательность,
- progression hooks,
- unlocks,
- save/load,
- UI,
- system glue,
- BT-72 systems,
- service/recovery hooks,
- combat/RPG support.
