# Source Extraction Report

## Назначение

Этот файл фиксирует, что именно было вытащено из старых папок проекта и как это должно использоваться в `Game_Project`.
Цель не в архивировании всего подряд, а в отделении:

- канонических идей и механик;
- уже начатых технических подсистем;
- UI/UX референсов;
- нерелевантных веток, которые не надо переносить в игру.

## Общий вывод

Сейчас у проекта есть четыре разных источника:

- `../Новая папка` — главный дизайн-источник и старая модульная схема игры и редактора.
- `../Project_M` — самый полезный технический прототип движка/runtime.
- `../Aegis-9300` — полезен как референс терминального UI и окна логина/лаунчера.
- `../void-project` — не источник игрового кода; максимум источник стилистики терминальных формулировок и именования.

Итоговая стратегия для `Game_Project`:

- брать канон игры, редактора и форматов из `Новая папка`;
- брать технические паттерны и подсистемы runtime из `Project_M`;
- брать только визуальные и UX-идеи лаунчера из `Aegis-9300`;
- не переносить ядро, язык и security-механику из `void-project`.

## Папка `../Новая папка`

### Что полезно

Это основной источник по:

- жанру и формату игры;
- камерам `1st/3rd person`;
- схеме `Launcher -> Game -> Editor`;
- редактору уровня `Creation Kit`;
- импорту эскизов и их превращению в игровые объекты;
- формату идентификаторов сущностей;
- RPG-модели персонажа;
- ранней модульной архитектуре игры.

### Что уже подтверждено документами

Из `Project_docs.ini`, `Project.ini`, `Project.md` подтверждаются:

- `Bunker Protocol` как action-RPG / immersive sim;
- не изометрия, а основной пользовательский вид в стиле `Fallout`;
- отдельный `World Editor`;
- бинарный формат мира `.wld` / `BWLD`;
- подготовка ассетов через `Asset Studio`;
- логика реестра сущностей, персонажа, навыков, UI и камеры.

### Что полезно из кода

Старые модули, которые стоит переосмыслить и перенести в новый вид:

- `../Новая папка/include/Registry_ID.hpp`
  - готовая схема типов сущностей: `User`, `Character`, `Monster`, `Boss`, `Item`, `Structure`, `Transport`
- `../Новая папка/include/Avatar_State.hpp`
  - базовая модель `S.P.E.C.I.A.L.`, HP/MP, инвентарь, вес
- `../Новая папка/include/Engine_Core.hpp`
  - схема состояний: `Boot`, `Launcher`, `WorldLoading`, `ActivePlay`, `Paused`
- `../Новая папка/include/World_Resources.hpp`
  - ранняя идея формата мира и объектных слоёв
- `../Новая папка/include/Camera_Avatar.hpp`
  - источник для гибридной камеры
- `../Новая папка/include/Input_Direct.hpp`
  - источник для слоя ввода и поддержки трекбола
- `../Новая папка/WorldEditor/include/Editor_Core.hpp`
  - минимальная логика редактора: выбор объекта, translate, snap, duplicate
- `../Новая папка/WorldEditor/include/UI_CellView.hpp`
  - концепт окна сцены
- `../Новая папка/WorldEditor/include/UI_ObjectWindow.hpp`
  - концепт окна свойств объекта
- `../Новая папка/WorldEditor/src/Editor_Exporter.cpp`
  - источник для export-контура из редактора в runtime
- `../Новая папка/src/Launcher_Main.cpp`
  - источник для идеи лаунчера как отдельного приложения

### Что переносить в первую очередь

- реестр ID сущностей;
- состояние персонажа и профиля;
- state-machine приложения;
- editor workflow: `palette -> scene -> export`;
- input abstraction с местом под trackball;
- формальную спецификацию объектов мира.

## Папка `../Project_M`

### Что полезно

Это главный технический донор. В отличие от `Новая папка`, тут уже есть не только идеи, но и реальные runtime-подсистемы.

### Подсистемы, которые уже существуют

По структуре `include/slinger` и `src/...` подтверждены:

- память и стриминг:
  - `memory/MemoryManager`
  - `io/VirtualFileSystem`
  - `platform/win/WinMappedFile`
- загрузка контента:
  - `assets/AssetManager`
  - `assets/TextureManager`
  - `assets/DataCard`
  - `assets/WicMemoryStream`
- окно и рендер:
  - `platform/win/Win32Window`
  - `render/Dx11Renderer`
  - `render/FrameGraph`
  - `render/ChunkRenderer`
  - `render/HudTextRenderer`
  - `render/TitleScreenRenderer`
- мир:
  - `world/Chunk`
  - `world/ChunkCoord`
  - `world/ChunkStreamer`
  - `world/WorldStreamer`
  - `world/WorldAtlas`
  - `world/ObjectPool`
- игра:
  - `game/PlayerController`
  - `game/Weather`
  - `game/NervSystem`
  - `game/WeaponSystem`
  - `game/DeathSystem`
  - `game/QuestSystem`
  - `game/TankAssistantComponent`
- физика:
  - `physics/PhysicsWorld`
  - `physics/SyncSystem`
  - `physics/SimplePhysicsSystem`
  - `physics/JoltBridge`
  - `physics/JoltSystem`
- persistence:
  - `persistence/StaticEraser`
  - `persistence/StaticEraserFacade`
  - `persistence/StaticEraserSqlite`
  - `persistence/SoulLine`
  - `persistence/SoulLineFacade`
  - `persistence/SoulLineSqlite`
  - `persistence/SqliteDb`
- фабрики и энергия:
  - `factory/FactoryGraph`
  - `power/EnergyGrid`
- сеть:
  - `net/LanSession`
- скрипты:
  - `scripts/LuaJitBridge`
- инфраструктура:
  - `jobs/JobSystem`
  - `ecs/Registry`
  - `ecs/TransformComponent`
  - `ecs/ChunkComponent`
- движок:
  - `engine/SlingerEngine`

### Что особенно ценно для `Game_Project`

- уже есть `LAN`-скелет, а это совпадает с текущим каноном `solo + LAN first`
- уже есть `Nerv`, погода, танковый runtime и dual-pass представление кабины
- уже есть `StaticEraser` и `SoulLine`, то есть часть долгосрочной канонической логики не нужно придумывать заново
- уже есть `FactoryGraph` и `EnergyGrid`, пусть и не в финальном виде
- уже есть стриминг и memory discipline, что критично для открытого мира

### Что переносить в первую очередь

- `LanSession` как базу для сетевого слоя;
- `MemoryManager + VirtualFileSystem` как основу для asset streaming;
- `StaticEraser` как базу для постоянного очищения завалов;
- `SoulLine` как базу скрытой долгосрочной прогрессии;
- `FactoryGraph + EnergyGrid` как ядро будущих заводов;
- `PlayerController + NervSystem + Weather + WeaponSystem` как основу игрового цикла;
- `SlingerEngine` как пример компоновки runtime-подсистем.

### Что не надо переносить напрямую

- старые имена `slinger::*` как финальный public API;
- текущую структуру проекта целиком;
- DX11-only привязку как окончательное решение без переоценки;
- экспериментальные fallback-решения, которые были сделаны только для автономного тестового запуска.

## Папка `../Aegis-9300`

### Что полезно

Это не игра и не редактор. Это Tauri/Svelte-проект с терминальным UI.

По структуре найдены:

- `src/App.svelte`
- `src/lib/AuthWindow.svelte`
- `src/lib/Terminal.svelte`
- `src/lib/TopBar.svelte`
- `src/lib/SideMenu.svelte`
- `src-tauri/src/auth.rs`
- `src-tauri/src/security.rs`
- `src-tauri/src/gamepad.rs`

### Что брать

Только UX-идеи:

- стилистику окна логина;
- подачу статуса в терминальном блоке;
- визуальную подачу кнопок и заголовков;
- структуру лаунчерного окна.

### Что не брать

- Rust/Tauri как основу для игры;
- security-логику;
- SSH/secure-vault сценарии;
- всё, что не относится к UX лаунчера.

## Папка `../void-project`

### Что это по факту

Это отдельная экосистема со своим языком, runtime и системной логикой. Для `Game_Project` она не является корректной кодовой базой-донором.

### Что можно взять

- стилистику некоторых терминальных формулировок;
- naming mood для системных сообщений;
- отдельные идеи UI-текста.

### Что нельзя переносить

- `.ctos/.vc/.web` как архитектуру игры;
- BIOS / firmware / hidden vault / raw inject / stealth / self-destruct механику;
- security и системные практики, не относящиеся к игре.

Это надо считать внешним экспериментом, а не частью игрового production stack.

## Что уже можно считать извлечённым в канон проекта

После просмотра этих папок в `Game_Project` уже можно считать подтвержденными следующие столпы:

- игра строится как `Launcher + Game + Editor`;
- основной режим игры: `solo + LAN first`;
- основной вид: `1st/3rd person`, плюс режимы кабины/танка;
- редактор — отдельный toolset уровня `Creation Kit`;
- импорт эскизов — обязательная часть pipeline;
- танки, заводы, ЖД, Data Cards, `Nerv`, `SoulLine`, `StaticEraser` — это системные механики, а не второстепенные идеи;
- техническая база должна уметь стриминг, persistence, LAN и дальнейшее расширение.

## Решение для `Game_Project`

### Что считаю канонической базой

- дизайн и игровые правила: `../Новая папка`
- runtime-подсистемы: `../Project_M`
- launcher UX: `../Aegis-9300`

### Что считаю побочным или нерелевантным

- `../void-project` как кодовая база
- security-ветки из `Aegis-9300`

## Прямой план переноса в код

Следующие модули надо внедрять в `Game_Project` в ближайших итерациях:

1. `RegistryId`
   - новый runtime-модуль на основе `../Новая папка/include/Registry_ID.hpp`
2. `CharacterState / Profile`
   - новый слой на основе `Avatar_State.hpp`
3. `AppState / SessionFlow`
   - на основе `Engine_Core.hpp` и уже сделанного `BunkerLauncher`
4. `LAN session layer`
   - на основе `../Project_M/include/slinger/net/LanSession.h`
5. `World streaming and persistence`
   - на основе `Project_M` + текущего `BWLD`-контура `Game_Project`
6. `StaticEraser + SoulLine`
   - как отдельные доменные подсистемы
7. `Factory + EnergyGrid`
   - как целевой mid-game слой
8. `Editor asset pipeline`
   - `Asset Studio`, `Cell View`, `Object Window`, `Exporter`, `Import Assistant`

## Практический вывод

Если переводить всё это в одну фразу:

`Новая папка` говорит, какую игру мы строим.
`Project_M` показывает, какие подсистемы уже почти готовы.
`Aegis-9300` даёт стиль лаунчера.
`void-project` в production игры не нужен.

Следующий шаг после этого файла: переносить найденные модули в `Game_Project` как новую, чистую архитектуру, а не собирать ещё одну абстрактную документацию.
