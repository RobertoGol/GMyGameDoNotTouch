# Next

## Живой статус

Этот верхний блок теперь главный.
Старый длинный текст ниже оставлен как архив контекста, чтобы ничего не терялось.
### Еще осталось из старого плана

- верхний блок старого плана сейчас закрыт

### Закрыто в этом проходе

- канонизирован общий `scriptTag` registry: editor/runtime/validation теперь понимают одинаковые теги и legacy-aliases
- `WorldValidation` больше не считает каждый семантический `linkTarget` битой ссылкой; жёстко проверяются только ссылки, похожие на `registryId`
- editor presets переведены на канонические теги для `tower_sync`, `workshop_service`, `rail_depot`, `rail_fortress_hub`, `industrial_gate`, `fey_ring`, `tank_service`
- runtime interaction и terminal sync теперь понимают alias-теги без ручного ремонта старых world-файлов
- smoke-check покрывает alias normalization и validation contract
- `World::Load` теперь канонизирует legacy `scriptTag`, а `World::FindObjectByScriptTag` / `HasScriptTag` работают через normalized lookup
- editor warnings по `fey_ring` / `lanline_service_hub` / `tank_service` тоже учитывают legacy-aliases, а не только новые теги
- `GameplayDescriptorRegistry` вынесен из header в [src/GameplayDescriptorRegistry.cpp], `WorldValidation` вынесен в [src/WorldValidation.cpp]

### Правило на следующие проходы

Сначала закрываем пункты из этого верхнего блока.
Старые заметки ниже не удаляются, а служат архивом и пояснением.

Продуктовая граница `Lanline` зафиксирована отдельно: `LANLINE_BOUNDARY.md`.

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
