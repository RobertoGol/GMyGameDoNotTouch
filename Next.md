# Next

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

