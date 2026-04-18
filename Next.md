Ниже — единый полный план дальнейшей разработки, собранный из того, что мы уже выяснили по коду, ROADMAP.md, KOROTKOE_TZ_CLEAN.md, MASTER_PROMPT.md, SOURCE_EXTRACTION_REPORT.md и текущему состоянию репозитория. Я объединяю сюда и ближайшие шаги, и путь до большой полноценной версии. Канонически проект остается разделенным на BunkerLauncher, BunkerGame и BunkerEditor; Launcher — обязательная точка входа, Editor — отдельный production tool, а LAN-слой фиксируем как Lanline - optime. Вопросы ИИ идут в Use_this_One.md, идеи автора живут в trash.md.

Сразу фиксирую отправную точку: сейчас проект — поздний vertical slice / ранняя системная v1, а не пустой прототип. По roadmap уже закрыты или частично закрыты стартовый маршрут, ранний бой/RPG, BT-72, recovery backbone, Relay Substation, Service Bay, Water Reclaimer, рабочий editor и launcher/session flow. Но при этом Launcher v1 еще не закрыт до конца, Editor v1 еще в работе, Data Cards только частично начаты, а LAN foundation и factories/energy/infrastructure еще не доведены по существу.

Моя рабочая оценка остается такой:
до первой крепкой v1 — примерно 65–75%,
до большой полноценной версии — примерно 35–45%.
Это не цифры из документа, а инженерная оценка по уже собранным слоям и по незакрытым pillars.

1. Базовые правила на весь оставшийся путь
Ничего не распиливать и не рефакторить крупно, пока ты сам этого не захочешь. Работаем внутри текущей структуры, без “резки” Launcher_Main.cpp, Editor_Main.cpp, GameRuntime.cpp.
Не делать заглушки, если можно сделать реальный маленький рабочий слой.
Не отходить от проекта: не превращать его в generic shooter, sandbox без recovery или просто map editor.
Каждый новый слой закрывать целиком: runtime + editor + docs + cleanup.
Не прыгать на новые большие идеи, пока предыдущий системный checkpoint не доведен.
Вопросы ИИ — только в Use_this_One.md.
trash.md — это backlog идей автора; если идея уже реально внедрена, ее надо отражать в рабочих md и убирать из backlog. Эти правила прямо закреплены в roadmap и master prompt.
2. Что уже считаем закрытым и не тратим на это основной фокус

Это важно, чтобы не крутиться по кругу.

Уже можно считать рабочей базой:

трехприложенческую структуру продукта;
обязательный запуск игры через launcher;
стартовый bunker-route и recovery progression после него;
ранний танковый контур BT-72;
chain Rail Freight -> Orbital Uplink -> Rail Fortress -> Recovery Fabricator -> Industrial Gate -> Survey -> Outpost -> Assembly -> Foundry -> Reactor -> Capacitor -> Relay Substation -> Service Bay -> Water Reclaimer;
рабочий BunkerEditor с viewport, object authoring, prefab flow, descriptor presets и export handoff;
early Lanline - optime session-state и launcher/runtime связку.

Особенно важно: Water Reclaimer уже зафиксирован как текущий подтвержденный checkpoint и не должен быть снова “переделываемым бесконечно”, если не всплывет конкретный баг. Это прямо отражено и в ROADMAP.md, и в Fin_ask.md.

ПОЛНЫЙ ПЛАН ДАЛЬНЕЙШЕЙ РАЗРАБОТКИ
Этап 1. Стабилизация обязательной точки входа — BunkerLauncher

Это самый следующий практический шаг.

Что сделать
Починить bug-риск с выбором мира в Launcher_Main.cpp, где в список для ImGui::Combo кладутся указатели через world.string().c_str().
Укрепить запуск BunkerGame.exe и BunkerEditor.exe, чтобы он не зависел от случайного current_path.
Добавить безопасные проверки индексов для:
выбранного мира,
выбранного персонажа,
LAN snapshot’ов.
Обновлять список миров и knownLanlineSessions не только один раз при старте, а так, чтобы UI не устаревал.
Сохранить launcher как обязательную, устойчивую точку входа в игру и редактор.
По roadmap Launcher v1 уже почти завершен, но еще не закрыт.
Что считается завершением этапа
BunkerLauncher стабильно выбирает мир и персонажа;
корректно запускает BunkerGame и BunkerEditor;
не падает на битом выборе или пустом списке;
не показывает устаревший список миров/LAN snapshot’ов;
после правок все три цели снова собираются.
Этап 2. Полностью добить Launcher v1

После stability pass — не уходить в новый gameplay-layer, а закрыть лаунчер до конца.

Что сделать
Доделать реальный LAN browser/discovery.
Добавить ping.
Добавить сетевую диагностику:
понятный session status,
host/client diagnostics,
basic failure feedback.
Сохранить и развить каноническое имя LAN-режима: Lanline - optime.
Сделать так, чтобы Lanline - optime одинаково читался:
в launcher,
в game runtime,
в roadmap/docs.
Это прямо перечислено как остаток по launcher в roadmap.
Что считается завершением этапа
Launcher v1 можно честно отметить как завершенный;
session browser, ping и diagnostics уже не “архитектурны”, а реально работают;
Lanline - optime становится канонически закрепленным LAN-слоем по всему проекту.
Этап 3. Сборка и очистка репозитория

После лаунчера нужно закрепить чистое техническое состояние.

Что сделать
Пересобрать:
BunkerGame,
BunkerLauncher,
BunkerEditor.
Оставить один основной актуальный build и максимум один fallback.
Удалить лишние build_* директории.
Убрать из рабочей практики лишние корневые build-артефакты вроде случайных CMakeCache.txt, если они не нужны.
Держать репозиторий в более чистом состоянии и перестать плодить сборки без причины.
По дереву проекта лишних build-папок реально много.
Что считается завершением этапа
есть одна главная рабочая сборка;
есть, при желании, одна fallback-сборка;
репозиторий перестает быть складом локальных артефактов;
status проекта становится легче читать и сопровождать.
Этап 4. Обновление документации под реальность

После cleanup надо привести md-память в точное соответствие с кодом.

Что сделать
Обновить ROADMAP.md после завершения launcher-работ и build cleanup.
Убедиться, что:
Water Reclaimer помечен закрытым и больше не висит как “текущий незавершенный” слой;
Lanline - optime зафиксирован как каноническое имя LAN-слоя;
stage-статусы соответствуют текущему коду.
Не переписывать все подряд — только честно фиксировать checkpoint’ы и остатки.
Если в trash.md есть идеи, уже реально внедренные в проект, переносить их в рабочую память и убирать из backlog.
Это тоже прямо закреплено в правилах roadmap.
Что считается завершением этапа
docs перестают отставать от кода;
roadmap снова можно использовать как честный журнал состояния проекта;
backlog в trash.md становится чище.
Этап 5. Закрытие LAN foundation уже внутри runtime

После launcher-слоя логично идти в реальный LAN-фундамент игры, а не в случайную новую механику.

Что сделать
Довести до usable состояния:
host/join,
discovery,
ping,
session presence,
вход/выход игроков.
Закрепить структуру синхронизации:
session state,
player presence,
world match,
snapshot metadata.
Добавить базовые runtime-уведомления:
кто вошел,
кто вышел,
к какому миру привязана сессия.
Продолжать считать модель проекта solo + LAN first, а не MMO-first.
Это прямо канон ТЗ и master prompt.
Что считается завершением этапа
LAN в проекте существует не только в launcher UX, а как реальный runtime-слой;
Lanline - optime становится настоящей игровой session-моделью;
LAN foundation в roadmap можно перевести из “не начат по существу” в реальный рабочий этап.
Этап 6. Добивка ранней игры до настоящей v1

После обязательных системных слоев надо довести раннюю игру до состояния, где она уже ощущается как первая цельная версия, а не просто сильный vertical slice.

Что сделать
Tighten стартовый маршрут и post-start continuity:
крио,
Pip-Pad,
архив,
первый hostile-контур,
BT-72,
расчистка,
relay,
debrief,
recovery backbone.
Довести objective-flow, mission log и player-facing continuity до более цельного состояния.
Добить soft fail / respawn / checkpoint rhythm.
Убрать остаточные “слишком ранние” общие формулировки в runtime, если они еще всплывают.
Сделать раннюю игру устойчивой как самостоятельный опыт.
Roadmap уже показывает, что основа для этого есть, но это все еще “в работе”.
Что считается завершением этапа
можно честно говорить: “первая цельная играбельная версия уже есть”;
ранняя игра не ощущается набором техчекпойнтов;
игрок понимает progression от старта к recovery backbone без смысловых дыр.
Этап 7. Закрытие базового боевого и RPG-слоя

Это уже путь от v1 к более плотной игре.

Что сделать
Довести пеший бой:
урон,
feedback,
hit response,
читаемость угроз.
Довести RPG-слой:
SPECIAL,
HP,
MP/energy,
inventory,
carry weight,
leveling,
perks,
awakening.
Продолжить развивать не только бонусы через UI, но и логику, где поведение игрока реально формирует билд.
Закрыть ранний “food/toxin / stress / awakened recipes / doctrine / specialists” слой так, чтобы это стало устойчивой общей системой, а не набором интересных отдельно стоящих механик.
Этот слой уже сильно продвинут, но roadmap по нему все еще стоит как “в работе”.
Что считается завершением этапа
у игрока уже есть внятный ранний RPG loop;
пеший режим и recovery loop работают как одна система;
awakening — не экзотика, а реально работающий pillar проекта.
Этап 8. Доведение танка и тяжелой техники до полноценного pillar-слоя

BT-72 уже есть, но до большой версии нужно больше.

Что сделать
Довести BT-72:
cockpit,
HUD,
damage state,
modules,
thermal,
energy,
service,
utility-slot continuity.
Усилить связку:
пеший режим ↔ танк,
танк ↔ service,
танк ↔ logistics,
танк ↔ recovery.
Развивать heavy-vehicle philosophy проекта:
техника как progression,
техника как инструмент восстановления,
техника как logistics layer.
Потом уже расширяться к другим классам техники из ТЗ:
мотоцикл,
машина,
грузовик,
кран,
ЖД-платформы,
и позднее другие тяжелые роли.
Что считается завершением этапа
BT-72 ощущается не “ранним системным каркасом”, а реальным центральным слоем игры;
тяжелая техника становится одним из несущих pillars полной версии.
Этап 9. Доведение persistence и мира до системного уровня
Что сделать
Дальше укреплять per-world state:
destroyed/removed objects,
authored state,
infrastructure state,
route state,
camp/backbone state.
Довести world continuity:
start worlds,
custom worlds,
selected world from profile,
runtime/editor handoff.
Двигаться к более зрелому миру из канона:
chunk thinking,
streaming,
verticality,
lifts,
richer zones.
Не ломать текущий BWLD/BWL2 контур без критической причины.
Persistence уже неплохой, но до большой версии ему еще расти.
Что считается завершением этапа
мир реально помнит изменения системно и стабильно;
editor и runtime живут в одном согласованном world pipeline;
карта перестает быть просто authored сценой и становится настоящим persistent world state.
Этап 10. Полное закрытие Editor v1

Editor уже production-useful, но не закрыт.

Что сделать
Довести библиотеку объектов и prefab workflow.
Довести интерактивности, контейнеры и loot authoring.
Довести export/import flow.
Доделать preview/playtest-сценарии.
Довести import assistant как реальный мост от картинки/референса к игровому объекту.
Усилить валидацию:
Registry ID,
link consistency,
semantic object correctness.
Сохранить курс на Creation Kit-подобный tool, а не на “игрушечный редактор”.
Это прямо закреплено и в ТЗ, и в roadmap, и видно по текущему Editor_Main.cpp.
Что считается завершением этапа
Editor v1 можно честно назвать завершенным;
он становится не просто “рабочим”, а полноценным первым production toolset.
Этап 11. Полноценный Data Cards / archive / reconstruction слой
Что сделать
Углубить Data / Archive / Reconstruction.
Сделать Data Cards одним из центральных carriers:
lore,
blueprints,
recipes,
archival truth,
recovery unlocks,
industrial data.
Связать это с:
Pip-Pad,
terminals,
objectives,
progression,
reconstruction.
Развить кассеты, записи и data fragments до полноценного лор/прогрессионного пласта.
В каноне это один из главных типов контента, а в roadmap пока только частично начато.
Что считается завершением этапа
мир начинает гораздо сильнее “говорить” через данные, а не только через системные узлы;
Data Cards становятся одной из визитных карточек проекта.
Этап 12. Реальный factory / energy / infrastructure backbone

Это один из самых больших незакрытых блоков до полноценной версии.

Что сделать
Начать не “по идее”, а реально:
factory graph,
energy grid,
производственные узлы,
feeding/backbone logic.
Сделать ранние заводы, переработку, assembly и энергию настоящим средним игровым циклом.
Связать production не только с флагами, а с ресурсами, питанием, очередями, зависимостями и world state.
Подготовить UI/pipeline под industrial management.
Продолжить логику из уже введенных узлов, чтобы они стали не только progression-этапами, но и настоящими operational systems.
Это прямо в ТЗ и master prompt отмечено как обязательный pillar, а в roadmap еще “не начат по существу”.
Что считается завершением этапа
у проекта появляется настоящий mid-game industrial core loop;
recovery fantasy поднимается на уровень “не только чинить узлы, но и реально оживлять инфраструктуру”.
Этап 13. Железная дорога, логистика и стратегический recovery scale
Что сделать
Углубить уже существующие rail/logistics nodes:
routes,
damage,
repair,
supply,
heavy transport,
dependency chains.
Сделать железную дорогу мета-системой, а не просто серией authored объектов.
Развить связку:
rail ↔ factories,
rail ↔ power,
rail ↔ shelter recovery,
rail ↔ deeper zone control.
Довести логистику до стратегического масштаба восстановления мира.
В ТЗ железная дорога — не декор, а обязательный системный слой.
Что считается завершением этапа
recovery становится не локальным, а территориальным;
игра получает свой полноценный strategic/industrial масштаб.
Этап 14. Расширение мира и контента
Что сделать
Добавлять больше зон:
бункеры,
поверхность,
разрушенные города,
индустриальные зоны,
природные зоны.
Добавлять глубину врагов и опасных зон.
Усиливать цветовые/смысловые зоны сложности.
Делать больше authored content поверх уже готового системного каркаса.
Расширять мир не хаотично, а по production pipeline через editor.
Это уже путь не к “v1”, а к большой игре по замыслу.
Что считается завершением этапа
игра ощущается как большой мир, а не как одна сильная ранняя ветка.
Этап 15. ИИ-помощник / Nerv / системный ассистент
Что сделать
Вернуться к линии из старых веток:
navigation mode,
engineering mode,
tactical mode.
Развить помощника как реальную системную сущность, связанную с:
HUD,
сенсорами,
погодой,
техникой,
производством,
интерпретацией мира.
Не делать магический wallhack; держать его как слабый, техничный, world-aware assist layer.
Это явно отмечено в старых notes и source extraction.
Что считается завершением этапа
ИИ-помощник становится одним из узнаваемых системных слоев проекта, а не просто голосом или tooltip-системой.
Этап 16. Полировка, баланс, производительность и релизный контур

Это уже финальный этап до полноценной версии.

Что сделать
Баланс:
бой,
progression,
recovery,
industry,
logistics.
Производительность и устойчивость:
runtime,
saves,
editor,
LAN sessions.
UI polish:
launcher,
HUD,
Pip-Pad,
tank HUD,
editor,
factory interfaces.
Релизная дисциплина:
clean repo,
reproducible builds,
packaging,
version checkpoints.
Что считается завершением этапа
проект уже выглядит как полноценная большая игра, а не как сильная dev-сборка.
СВЕРХКОРОТКАЯ ОЧЕРЕДЬ “ЧТО ДЕЛАТЬ ПРЯМО СЕЙЧАС”

Если выжать весь план в порядок без лишнего, то он такой:

Launcher stability pass
Launcher v1 completion
Rebuild всех 3 приложений
Build cleanup
ROADMAP sync
LAN foundation в runtime
Добивка ранней игры до полноценной v1
Combat/RPG closure
BT-72 / heavy-tech closure
Persistence/world deepening
Editor v1 closure
Data Cards / archive / reconstruction
Factory / energy / infrastructure backbone
Rail / logistics strategic scale
World/content expansion
AI assistant / Nerv
Polish / balance / release discipline
ИТОГ

Если совсем честно и жестко:

Ближайшая цель — не “придумать еще что-нибудь крутое”, а:

закрыть Launcher v1,
привести репозиторий в порядок,
закрепить Lanline - optime,
довести игру до честной первой цельной v1.

Большая полноценная версия начинается там, где:

LAN foundation уже рабочий,
Editor v1 закрыт,
Data Cards стали настоящим слоем,
factories/energy/logistics уже не “следующий мечтательный этап”, а реальный mid/late-game loop.

Это и есть путь от текущих 35–45% до полной версии.

Если хочешь, следующим сообщением я превращу это в ударный рабочий чеклист на 10 ближайших ходов, совсем без длинных объяснений.

Ниже — проработанные примеры кода под ближайшие задачи и потом дальнейший план по порядку. Я держусь текущего состояния репозитория: сейчас у BunkerLauncher уже есть выбор мира, Lanline - optime, snapshot’ы и session-state, но в коде видны конкретные точки, которые надо довести, а в roadmap прямо осталось закрыть real LAN browser/discovery, ping и diagnostics.

1. BunkerLauncher: сначала стабилизация, без рефакторинга

Сейчас в Launcher_Main.cpp есть четыре реальные проблемы:

worldLabels собирается через world.string().c_str(), то есть указатели висят на временных строках;
TryLaunchSiblingExecutable() ищет exe от current_path(), а не от папки самого лаунчера;
PrepareSelectedCharacter() индексирует linkedCharacters[idx] без страховки;
список snapshot’ов и миров грузится один раз до цикла и не обновляется.
1.1. Безопасные world labels для ImGui::Combo

Это самый первый фикс.

std::vector<std::string> worldLabelStorage;
std::vector<const char*> worldLabels;

auto RebuildWorldLabels = [&]() {
    worldLabelStorage.clear();
    worldLabels.clear();

    worldLabelStorage.reserve(worlds.size());
    worldLabels.reserve(worlds.size());

    for (const auto& world : worlds) {
        worldLabelStorage.push_back(world.string());
        worldLabels.push_back(worldLabelStorage.back().c_str());
    }
};

Инициализация:

auto worlds = DiscoverWorlds();
std::vector<std::string> worldLabelStorage;
std::vector<const char*> worldLabels;
RebuildWorldLabels();

Почему так: сейчас ImGui::Combo("World", ...) получает массив const char*, но у тебя он строится из временных std::string, поэтому хранить нужно сначала сами строки, а уже потом их c_str(). Это точечный фикс, без изменения архитектуры.

1.2. Зажим индексов перед доступом к массивам

Чтобы не словить выход за границы на выборе мира, персонажа или snapshot’а.

template <typename T>
int ClampIndex(int value, const std::vector<T>& items) {
    if (items.empty()) {
        return -1;
    }
    if (value < 0) {
        return 0;
    }
    if (value >= static_cast<int>(items.size())) {
        return static_cast<int>(items.size()) - 1;
    }
    return value;
}

template <std::size_t N>
int ClampArrayIndex(int value, const char* const (&)[N]) {
    if constexpr (N == 0) {
        return -1;
    }
    if (value < 0) {
        return 0;
    }
    if (value >= static_cast<int>(N)) {
        return static_cast<int>(N) - 1;
    }
    return value;
}

Применение перед UI и перед launch:

launcherState.selectedCharacter = ClampArrayIndex(launcherState.selectedCharacter, characters);
launcherState.sessionModeIndex = ClampArrayIndex(launcherState.sessionModeIndex, sessionModes);
launcherState.selectedWorldIndex = ClampIndex(launcherState.selectedWorldIndex, worlds);
launcherState.selectedLanlineSnapshot = knownLanlineSessions.empty()
    ? -1
    : ClampIndex(launcherState.selectedLanlineSnapshot, knownLanlineSessions);

И в PrepareSelectedCharacter():

void PrepareSelectedCharacter(bunker::SessionProfile& sessionProfile,
                              const LauncherState& launcherState,
                              const char* const* characters,
                              std::size_t characterCount) {
    const std::size_t idx = static_cast<std::size_t>(
        std::clamp(launcherState.selectedCharacter, 0, static_cast<int>(characterCount - 1)));

    sessionProfile.character.displayName = characters[idx];

    if (idx < sessionProfile.account.linkedCharacters.size()) {
        sessionProfile.character.characterId = sessionProfile.account.linkedCharacters[idx];
    } else if (!sessionProfile.account.linkedCharacters.empty()) {
        sessionProfile.character.characterId = sessionProfile.account.linkedCharacters.front();
    } else {
        sessionProfile.character.characterId = "@fallback_character";
    }

    sessionProfile.account.username = launcherState.login;
}

Это закрывает самый неприятный класс тихих крашей в текущем launcher flow. Сейчас в коде прямой доступ идет без такой страховки.

1.3. Поиск BunkerGame.exe и BunkerEditor.exe рядом с лаунчером, а не от current_path()

Сейчас TryLaunchSiblingExecutable() опирается на std::filesystem::current_path(). Это хрупко.

Ниже минимальный безопасный вариант под Windows:

#ifdef _WIN32
#include <windows.h>
#endif

std::filesystem::path GetExecutableDirectory() {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH] = {};
    const DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(buffer).parent_path();
#else
    return std::filesystem::current_path();
#endif
}

bool TryLaunchSiblingExecutable(const char* executableName, std::string& statusText) {
    const auto candidate = GetExecutableDirectory() / executableName;
    if (!std::filesystem::exists(candidate)) {
        statusText = std::string("Executable not found near launcher: ") + candidate.string();
        return false;
    }

    const std::string command = "\"" + candidate.string() + "\"";
    statusText = "Launching " + candidate.filename().string();
    std::system(command.c_str());
    return true;
}

Это не меняет стиль проекта, но делает запуск намного устойчивее.

1.4. Refresh для миров и Lanline snapshot’ов

Сейчас worlds и known sessions читаются один раз до main-loop. При таком UI лучше иметь ручной refresh.

void RefreshLauncherData(std::vector<std::filesystem::path>& worlds,
                         std::vector<std::string>& worldLabelStorage,
                         std::vector<const char*>& worldLabels,
                         std>& worldLabelStorage,
                         std::vector<const char*>& worldLabels,
                         std::vector<bunker::LanlineSessionState>& knownLanlineSessions,
                         LauncherState& launcherState,
                         const bunker::SessionProfile& sessionProfile) {
    worlds = DiscoverWorlds();
    knownLanlineSessions = bunker::DiscoverLanlineSessionSnapshots();

    worldLabelStorage.clear();
    worldLabels.clear();
    worldLabelStorage.reserve(worlds.size());
    worldLabels.reserve(worlds.size());

    for (const auto& world : worlds) {
        worldLabelStorage.push_back(world.string());
        worldLabels.push_back(worldLabelStorage.back().c_str());
    }

    launcherState.selectedWorldIndex = ClampIndex(launcherState.selectedWorldIndex, worlds);
    launcherState.selectedLanlineSnapshot = knownLanlineSessions.empty()
        ? -1
        : ClampIndex(launcherState.selectedLanlineSnapshot, knownLanlineSessions);

    for (std::size_t i = 0; i < worlds.size(); ++i) {
        if (worlds[i].string() == sessionProfile.selectedWorld) {
            launcherState.selectedWorldIndex = static_cast<int>(i);
            break;
        }
    }
}

Кнопка в UI:

if (ImGui::Button("Refresh Worlds / Lanline", ImVec2(-1.0f, 28.0f))) {
    RefreshLauncherData(worlds, worldLabelStorage, worldLabels, knownLanlineSessions, launcherState, sessionProfile);
    launcherState.statusText = "Launcher data refreshed.";
}
2. Lanline - optime: следующий реальный шаг — live diagnostics, а не фантазии

Сейчас LanlineSession уже существует как файловое session-state API: пишутся и читаются .state snapshot’ы с session_id, mode, world, host, updated_at, player, event, а и launcher, и runtime r закрыты real browser/discovery, ping и diagnostics. fileciteturn29file1 fileciteturn26file2turn26file4

Значит ближайший правильный шаг — не придумывать большой мультиплеер, а добавить живую проверку достижимости host endpoint поверх уже существующего LanlineSessionState.

2.1. Минимальная живая диагностика хоста

Подход: не ICMP, а порт-проба. Для LAN это практичнее.

struct LanlineDiagnostics {
    bool hostReachable = false;
    bool worldMatch = false;
    int pingMs = -1;
    std::string lastError;
};

Вариант под Windows/Winsock:

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

LanlineDiagnostics ProbeLanlineHost(const bunker::LanlineSessionState& session,
                                    const std::string& selectedWorld,
                                    int timeoutMs = 250) {
    LanlineDiagnostics out{};
    out.worldMatch = (session.worldName == selectedWorld);

#ifdef _WIN32
    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        out.lastError = "WSAStartup failed";
        return out;
    }

    const auto colonPos = session.hostEndpoint.find(':');
    const std::string host = colonPos == std::string::npos ? session.hostEndpoint : session.hostEndpoint.substr(0, colonPos);
    const std::string port = colonPos == std::string::npos ? "27015" : session.hostEndpoint.substr(colonPos + 1);

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &result) != 0) {
        out.lastError = "getaddrinfo failed";
        WSACleanup();
        return out;
    }

    SOCKET sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) {
        out.lastError = "socket failed";
        freeaddrinfo(result);
        WSACleanup();
        return out;
    }

    u_long nonBlocking = 1;
    ioctlsocket(sock, FIONBIO, &nonBlocking);

    const auto started = std::chrono::steady_clock::now();
    connect(sock, result->ai_addr, static_cast<int>(result->ai_addrlen));

    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(sock, &writeSet);

    timeval tv{};
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;

    const int ready = select(0, nullptr, &writeSet, nullptr, &tv);
    if (ready > 0 && FD_ISSET(sock, &writeSet)) {
        out.hostReachable = true;
        out.pingMs = static_cast<int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - started).count());
    } else {
        out.lastError = "host timeout";
    }

    closesocket(sock);
    freeaddrinfo(result);
    WSACleanup();
#else
    out.lastError = "not implemented on this platform";
#endif

    return out;
}

Это уже не заглушка: это реальная живая проверка, которую можно честно показывать пользователю как early LAN diagnostics.

2.2. Отрисовка диагностики по выбранному snapshot’у
if (launcherState.selectedLanlineSnapshot >= 0 &&
    launcherState.selectedLanlineSnapshot < static_cast<int>(knownLanlineSessions.size())) {
    const auto& selectedSnapshot =
        knownLanlineSessions[static_cast<std::size_t>(launcherState.selectedLanlineSnapshot)];

    const auto diagnostics = ProbeLanlineHost(
        selectedSnapshot,
        worlds[static_cast<std::size_t>(launcherState.selectedWorldIndex)].string());

    ImGui::Separator();
    ImGui::Text("Lanline Diagnostics");
    ImGui::BulletText("Host: %s", diagnostics.hostReachable ? "reachable" : "offline/unreachable");
    ImGui::BulletText("Ping: %s",
        diagnostics.pingMs >= 0 ? (std::to_string(diagnostics.pingMs) + " ms").c_str() : "n/a");
    ImGui::BulletText("World match: %s", diagnostics.worldMatch ? "yes" : "no");

    if (!diagnostics.lastError.empty()) {
        ImGui::TextDisabled("Details: %s", diagnostics.lastError.c_str());
    }
}

Это хорошо встает прямо в текущий Known Lanline Sessions блок.

2.3. Мягкий discovery без сетевого переписывания

Пока не хочешь большой сетевой распил, значит discovery делаем так:

filesystem snapshots остаются как пассивный discovery layer;
live host probe добавляет “живой/мертвый” статус;
потом уже можно добавлять настоящий UDP broadcast.

То есть ближайшая честная ступень — **snapshot discovery + reachability probe + ping-like ti из того, чего не хватает launcher v1. Текущий roadmap прямо просит именно это. fileciteturn26file2turn26file5

3. BunkerGame: продублировать диагностику в runtime Lanline - optime

В runtime уже есть вкладка Lanline - optime, где игра показывает session ID, mode, went log и known sessions. Это значит, что diagnostics надо не только в launcher, но и туда. fileciteturn26file4

Пример runtime-добавки
LanlineSessionState sessionState;
const bool hasSessionState = LoadLanlineSessionState(sessionState);

if (hasSessionState) {
    const auto diagnostics = ProbeLanlineHost(sessionState, sessionState.worldName);

    ImGui::Separator();
    ImGui::Text("Diagnostics");
    ImGui::BulletText("Host reachable: %s", diagnostics.hostReachable ? "yes" : "no");
    ImGui::BulletText("Ping: %s",
        diagnostics.pingMs >= 0 ? (std::to_string(diagnostics.pingMs) + " ms").c_str() : "n/a");
    if (!diagnostics.lastError.empty()) {
        ImGui::TextDisabled("%s", diagnostics.lastError.c_str());
    }
}

Так launcher и game будут показывать один и тот же честный operational state.

4. Build cleanup: отдельный маленький скрипт

Это не gameplay-код, но это тоже одна из задач.

PowerShell-скрипт
$root = "D:\Projects\Game_Project"

$keep = @(
    "build_verify_ninja_fresh",
    "build_verify_ninja"
)

Get-ChildItem $root -Directory | Where-Object {
    $_.Name -like "build_*" -and ($keep -notcontains $_.Name)
} | ForEach-Object {
    Write-Host "Removing $($_.FullName)"
    Remove-Item $_.FullName -Recurse -Force
}

Если хочешь оставить только один build:

$keep = @("build_verify_ninja_fresh")

У тебя реально много build_*, и их стоит чиститодну основную рабочую и максимум одну fallback-сборку. Это уже часть agreed policy проекта. fileciteturn11file1

5. Что делать дальше — уже по порядку

Ниже — дальнейший план после этих кодовых правок, без лишних развилок.

Шаг 1. Закрыть Launcher stability pass

Сначала:

world labels,
clamp индексов,
launch path,
refresh worlds/snapshots.
Это чистая стабилизация обязательной точки входа проекта. Сейчас это cherуже обязателен по архитектуре, аLauncher v1` по roadmap почти завершен. fileciteturn29file0turn26file2
Шаг 2. Дать Lanline - optime живую диагностику

После stability pass:

host reachability,
ping-like timing,
world match,
last error/details.
Сейчас и launsion-state и snapshots, так что diagnostics ляжет прямо поверх имеющегося слоя. fileciteturn29file1turn26file4
Шаг 3. Протянуть одинаковую диагностику в BunkerGame

Чтобы Lanline - optime ощущался единым слоем, а не “лан внутри лаунчера и отдельно где-то еще”.

Шаг 4. Пересобрать все 3 приложения

После этого обязательно проверить:

BunkerGame,
BunkerLauncher,
`Bunkerа — каноническая, и ломать ее нельзя. Это закреплено и в ТЗ, и в master prompt. fileciteturn12file0turn11file2
Шаг 5. Почистить build-папки

После подтвержденной сборки:

оставить одну главную build,
максимум одну fallback,
остальное удалить.
Шаг 6. Обновить ROADMAP.md

После launcher/LAN-работ надо честно зафиксировать:

что закрыто,
какая сборка подтве по Launcher v1.
Roadmap у тебя уже ведется как журнал checkpoint’ов, так и продолжать. fileciteturn26file2
Шаг 7. После этого — уже не launcher, а настоящий LAN foundation

То есть следующий большой слой:

real discovery,
host/join,
presence,
runtime sync model,
вход/выто еще не начато по существу, а для полной версии это один из ключевых pillars. fileciteturn11file0turn12file0
Шаг 8. Только потом — следующий большой gameplay/checkpoint-слой

Идти снова так же, как ты уже делал:

один слой,
довести до конца,
runtime + editor + docs,
-.
Этот метод у тебя уже сработал на Relay Substation -> Service Bay -> Water Reclaimer. fileciteturn11file0
6. Самая практичная очередь прямо сейчас

Если совсем коротко, то делай так:

починить Launcher_Main.cpp
добавить live diagnostics в Lanline - optime
показать те же diagnostics в BunkerGame
пересобрать 3 targets
почистить лишние build_*
обновить ROADMAP.md
переходить к полноценному LAN foundation

Если хочешь, следующим сообщением я соберу тебе готовый patch-plan по Launcher_Main.cpp буквально блоками: “вставить сюда”, “заменить это”, “добавить после этого”.

можно добавить, но правильно делать это не как “настоящий браузер внутри игры” прямо сейчас, а как внутренний сервисный слой проекта.

Иначе есть риск сломать канон: у проекта сейчас solo + LAN first, интернет не должен быть жесткой зависимостью, launcher/game/editor остаются раздельными, а ближайшие приоритеты — Launcher v1, LAN foundation, ранняя v1, а не тяжелая онлайн-обвязка.

Что именно я бы сделал

Не “браузер сайтов”, а внутриигровой сервисный хаб.

Название слоя можно дать в духе проекта, например:

Relay Net
ShelterNet
Lanline Services
Portal Console

Самый аккуратный вариант — Lanline Services как сетевой/социальный слой поверх Lanline - optime.

Что туда можно включить
1. Запрос поддержки / донат-магазин

Но строго в твоих правилах:

никакого готового оружия
никаких готовых танков
никаких готовых боевых преимуществ

Разрешено:

материалы
скины
косметика
возможно сервисные удобства
возможно ускорители небоевых контуров, если они не ломают баланс

Я бы разделил на 3 типа:

Materials
Cosmetics
Support Packs

Примеры:

basic salvage crate
repair composites
clean water reserve
workshop paint skin
BT-72 hull livery
relay operator badge

Это хорошо ложится на recovery fantasy проекта и не превращает игру в pay-to-win.

2. Таймеры и расписание кольц фей / порталов фей

Это очень хороший слой, если встроить его в мир правильно.

Что показывать:

время открытия
время закрытия
следующий цикл
направление
доступные города / серверы / узлы
статус перегрузки или нестабильности

Пример полей:

Node: Shelter 17 / East Ring
Destination: Iron Span
Window opens in: 12m
Cycle duration: 8m
Load state: Stable
Transit class: Personnel / Light Cargo / Relay Packet

Это можно красиво увязать с миром:

кольца фей как diegetic транспортно-сетевой слой
порталы фей как межгородская / межсерверная / межсессионная логистика
это усиливает атмосферу Log Horizon-влияния, но без ухода в MMO-first
3. Поиск друзей

Можно добавить:

поиск по нику
список друзей
статус: online / in session / offline
текущий world/session
invite/join request
mute/block

Это хорошо ложится на Lanline - optime и на будущий social layer.

4. Чат

Да, но лучше как внутренний системный чат, а не как встроенный “веб-мессенджер”.

Каналы:

Local Session
Party
Friends
System
Relay

Примеры:

сообщения игроков
системные статусы сессии
приглашения
лог переходов/подключений

Это еще и совпадает со старым направлением проекта, где чат и системные логи уже мыслились важной частью мира.

5. Настройки голосовой связи

Да, это логично.

Минимум:

включить/выключить голос
input device
output device
push-to-talk / open mic
sensitivity
volume per user
mute player
test mic

Но это тоже лучше делать как voice settings layer, а не как огромную VoIP-систему сразу.

Как это правильно встроить в проект
Не делать сейчас реальный web browser

Полноценный встроенный браузер — плохая идея на текущем этапе:

тяжело технически
лишняя зависимость
безопасность
утяжеление проекта
конфликт с текущим GLFW + OpenGL + ImGui стеком

Правильнее:

сделать внутренний UI-хаб, который выглядит как мини-браузер
но работает на собственных экранах/вкладках проекта

То есть визуально это “браузер”, а по факту — встроенный сервисный интерфейс.

Где его лучше открыть

Я бы сделал 2 точки входа:

В игре

вкладка в Pip-Pad
или отдельный Relay / Lanline / Services экран

В лаунчере

облегченная версия того же сервиса
друзья / сообщения / сессии / расписание порталов

Так ты не сломаешь разделение продукта:

launcher — до входа
game — внутри сессии
editor — отдельно
Что я бы назвал каноничным составом этого слоя
Новый системный слой

Lanline Services

Внутри него:

Friends
Chat
Voice
Support
Portal Schedule
Requests / Deliveries
Новый миро-сетевой слой

Fey Ring Network

Внутри:

кольца фей
порталы фей
окна активности
межгородские маршруты
межсерверные/межсессионные переходы
нагрузка / стабильность / таймеры
Что надо сделать по порядку
Этап A. Не код, а фиксация канона

Сначала решить и записать:

это именно внутренний сервисный хаб, а не “настоящий браузер интернета”
донат только:
материалы
скины
косметика
сервисные штуки
без:
оружия
готовых танков
готовых боевых преимуществ
кольца фей / порталы фей — это канонический сетевой/транспортный слой проекта
Этап B. UI-скелет

Сделать вкладки:

Friends
Chat
Voice
Support
Portal Schedule

Пока без тяжелой backend-магии, но не как заглушки, а как реальная data-driven оболочка.

Этап C. Friends + Chat

Сначала:

friend list
search by nickname
invite/request
session presence
текстовый чат

Это самый полезный живой social слой.

Этап D. Portal Schedule

Сделать:

список узлов
таймеры
направления
статус открытия
расписание

Это будет и красиво, и полезно, и атмосферно.

Этап E. Support / Store

Сделать:

категории
список допустимых паков
цены/условия
request/delivery flow
журнал выдач

И жестко держать запрет на pay-to-win.

Этап F. Voice Settings

Сначала только:

настройки
mute
input/output
push-to-talk
test mic

А уже потом, если надо, расширять до полноценной голосовой связи.

Что я бы не делал сейчас

Пока не надо:

реальный Chromium/CEF/WebView внутри проекта
полноценный интернет-магазин
тяжелую серверную экономику
покупку боевых вещей
сложную глобальную межсерверную MMO-логику

Это уже слишком рано для текущей стадии проекта.

Как это влияет на общий план

Если добавлять этот слой, то не прямо сейчас вместо launcher/LAN, а так:

закончить Launcher stability
закончить Launcher v1
довести Lanline - optime
после этого открыть новый системный слой:
Lanline Services + Fey Ring Network

То есть этот слой — следующий большой social/service layer после LAN foundation, а не замена текущим ближайшим задачам.

Мой итог

Идея хорошая.
Но реализовывать ее надо как:

внутренний сервисный хаб
без настоящего веб-браузера
без pay-to-win
с опорой на Lanline - optime
с каноническим слоем Fey Ring / Portal Schedule

Самый правильный вариант названия и структуры сейчас:

Lanline Services — друзья, чат, голос, поддержка, запросы
Fey Ring Network — кольца фей, порталы, таймеры, маршруты

Если хочешь, следующим сообщением я распишу это уже в виде полного feature-design документа: экраны, вкладки, данные, поля, логика и очередность внедрения.

Ниже — полная проработка этого слоя так, чтобы он не ломал текущий канон проекта.

Ключевая идея: делать не настоящий веб-браузер, а внутренний сервисный хаб мира. Это лучше подходит проекту, потому что у тебя канон сейчас solo + LAN first, интерфейс должен быть частью мира, продукт остается разделенным на BunkerLauncher / BunkerGame / BunkerEditor, а из Aegis-9300 брать надо именно UX-идеи, а не веб-стек или security-логику.

И еще важный якорь: BunkerGame уже сейчас штатно запускается только через BunkerLauncher, а значит такой сервисный слой логично делать как общий launcher/game сервисный интерфейс, а не как отдельную внешнюю веб-оболочку.

1. Каноническая форма нового слоя
Рабочее название слоя

Lanline Services

Это общий сервисный слой поверх:

Lanline - optime
social features
support requests
Fey-переходов
системных уведомлений
Подсистема маршрутов

Fey Ring Network

Это уже не “браузер”, а diegetic-сеть мира:

кольца фей
порталы фей
маршруты между городами
маршруты между серверами/узлами
окна открытия
таймеры
загрузка линии
нестабильность перехода
Почему это хорошо ложится в проект

Такой слой не ломает pillars:

solo + LAN first сохраняется;
UI остается частью мира;
не нарушается split Launcher / Game / Editor;
можно использовать терминальный и системный стиль UI;
social/network/service слой встраивается в уже существующий Lanline - optime.
2. Что должно входить в Lanline Services
2.1. Friends

Нужно поддерживать:

поиск друзей по нику;
список друзей;
статус:
offline
online
in session
in portal transit
do not disturb
текущая сессия;
текущий мир;
invite / join request;
block / mute.
2.2. Chat

Каналы:

Direct
Friends
Party
Session
Relay
System

Это хорошо укладывается в уже существующий дух проекта, где чат и системные логи мыслятся как важная часть мира.

2.3. Voice

Минимальный обязательный слой:

enable / disable;
push-to-talk / open mic;
input device;
output device;
input sensitivity;
master voice volume;
per-user volume;
mute player;
mic test.
2.4. Support / Requests

Это не pay-to-win магазин, а служба поддержки и снабжения.

Разрешено:

материалы;
скины;
косметика;
сервисные визуальные штуки;
non-combat support items;
возможно чистая вода, стройматериалы, декоративные наборы, эмблемы.

Запрещено:

готовое оружие;
готовые танки;
собранные боевые модули;
боевые бонусы, которых нельзя получить обычной игрой;
anything that breaks balance.

Это важно, потому что проект строится как системная recovery-игра, а не как магазин боевой мощи. Плюс ТЗ прямо говорит не сводить игру к MMO/service-first модели.

2.5. Portal Schedule / Fey Ring Network

Тут должен быть отдельный экран:

кольцо/портал;
origin;
destination;
opens in;
closes in;
duration;
cooldown;
status;
transit class;
capacity/load;
stable / unstable;
city / server class;
queue size.

То есть это не “магическое меню телепорта”, а реальная сеть переходов мира.

2.6. Deliveries

Связанный слой:

what was requested;
request state;
ETA;
delivered at;
failed / delayed;
destination node;
claim at shelter / relay / service terminal.
3. Что я считаю правильной UX-структурой
В BunkerLauncher

Облегченная версия:

Lanline
Friends
Chat
Portal Schedule
Support
Voice Settings

Это хорошо подходит на этапе до входа в игру.

В BunkerGame

Полная версия — как вкладка в Pip-Pad или Relay-экран:

Overview
Friends
Chat
Voice
Support
Fey Rings
Deliveries
В BunkerEditor

Ничего из этого туда напрямую не тянуть.
Editor должен оставаться production tool, не social client. Это согласуется с каноном проекта, где editor — отдельный content pipeline toolset, а не часть player runtime.

4. Режимы работы слоя

Нужно заложить 3 режима.

4.1. Offline Local
нет внешней сети;
доступны только локальные данные;
chat/history может быть пустым;
portal schedule может показывать только локальные mock/cached cycles;
support requests недоступны или откладываются.
4.2. Lanline Local
есть Lanline - optime;
доступны friends/session roster/presence;
доступен session chat;
portal schedule может работать в LAN-режиме как shared world-schedule;
voice settings доступны;
support requests могут быть только локальными или отключены.
4.3. Relay Online
есть внешний relay/service node;
доступны friends across nodes;
available support ordering;
real portal schedule between cities/servers;
expanded relay chat;
delivery tracking.

Это хорошо, потому что не ломает current scope: сейчас проект все еще solo + LAN first, а online можно наращивать сверху позже.

5. Канонические ограничения для monetization

Вот это надо сразу зафиксировать жестко:

Разрешено
skins;
liveries;
operator badges;
terminal themes;
BT-72 cosmetic paints;
decorative camp props;
materials;
salvage bundles;
industrial components;
clean water packs;
workshop cloth/insignia packs.
Запрещено
weapons;
completed tanks;
direct combat modules;
exclusive power advantages;
uncraftable battle ammo;
any “buy victory” layer.
Хорошая формулировка внутри мира

Это не “донат-магазин”, а:

Support Requests
Relay Supply
Auxiliary Deliveries
Shelter Aid
Lanline Support
6. Данные и логика: как это хранить

Ниже уже проработанные примеры кода. Они не требуют встроенного Chromium/CEF и нормально ложатся на твой текущий GLFW + ImGui слой. Это важно, потому что current repo уже построен вокруг launcher/game UI на ImGui, а не на веб-движке.

6.1. Базовые enum и модели
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace bunker {

enum class ServiceHubMode {
    OfflineLocal,
    LanlineLocal,
    RelayOnline
};

enum class FriendStatus {
    Offline,
    Online,
    InSession,
    InPortalTransit,
    DoNotDisturb
};

enum class ChatChannelType {
    Direct,
    Friends,
    Party,
    Session,
    Relay,
    System
};

enum class SupportCategory {
    Materials,
    Skins,
    Cosmetics,
    Utility
};

enum class SupportOrderState {
    Draft,
    Submitted,
    Approved,
    Rejected,
    Delivering,
    Delivered,
    Expired
};

enum class FeyGateState {
    Closed,
    OpeningSoon,
    Open,
    Cooldown,
    Unstable
};

struct FriendEntry {
    std::string accountId;
    std::string nickname;
    FriendStatus status = FriendStatus::Offline;
    std::string currentSessionId;
    std::string currentWorld;
    bool voiceMuted = false;
    float voiceVolume = 1.0f;
};

struct ChatMessage {
    std::string id;
    ChatChannelType channelType = ChatChannelType::System;
    std::string channelId;
    std::string senderId;
    std::string senderLabel;
    std::string body;
    std::int64_t unixSeconds = 0;
    bool systemMessage = false;
    bool localEcho = false;
};

struct ChatChannel {
    std::string id;
    std::string displayName;
    ChatChannelType type = ChatChannelType::System;
    std::vector<ChatMessage> messages;
};

struct VoiceSettings {
    bool enabled = true;
    bool pushToTalk = true;
    float inputSensitivity = 0.55f;
    float inputGain = 1.0f;
    float outputGain = 1.0f;
    int selectedInputDevice = 0;
    int selectedOutputDevice = 0;
    char pushToTalkKey[16] = "V";
};

struct SupportCatalogItem {
    std::string id;
    std::string label;
    std::string description;
    SupportCategory category = SupportCategory::Materials;
    int priceCredits = 0;

    bool grantsWeapon = false;
    bool grantsCompletedTank = false;
    bool combatAdvantage = false;

    std::vector<std::string> contents;
};

struct SupportOrder {
    std::string orderId;
    std::string itemId;
    std::string itemLabel;
    SupportOrderState state = SupportOrderState::Draft;
    std::string destinationNode;
    std::int64_t createdAt = 0;
    std::int64_t etaUnix = 0;
};

struct FeyGateCycle {
    std::string id;
    std::string originLabel;
    std::string destinationLabel;
    std::string routeClass;      // City / Server / Relay / Cargo
    FeyGateState state = FeyGateState::Closed;
    std::int64_t opensAtUnix = 0;
    std::int64_t closesAtUnix = 0;
    std::int64_t cooldownEndsUnix = 0;
    int queueSize = 0;
    int capacity = 0;
    bool interServer = false;
    bool unstable = false;
};

struct LanlineServicesState {
    ServiceHubMode mode = ServiceHubMode::OfflineLocal;
    std::vector<FriendEntry> friends;
    std::vector<ChatChannel> channels;
    std::vector<SupportCatalogItem> supportCatalog;
    std::vector<SupportOrder> supportOrders;
    std::vector<FeyGateCycle> feyGateCycles;
    VoiceSettings voice;
};

inline bool IsAllowedSupportItem(const SupportCatalogItem& item) {
    return !item.grantsWeapon
        && !item.grantsCompletedTank
        && !item.combatAdvantage;
}

} // namespace bunker
6.2. Канонический начальный каталог support-товаров
inline std::vector<SupportCatalogItem> MakeDefaultSupportCatalog() {
    return {
        {
            "support_salvage_small",
            "Salvage Crate / Small",
            "Basic recovery materials for shelter upkeep and workshop stock.",
            SupportCategory::Materials,
            120,
            false, false, false,
            {"bulk_salvage", "repair_parts", "circuit_scrap"}
        },
        {
            "support_clean_water",
            "Clean Water Reserve",
            "Purified water stock for shelter and expedition logistics.",
            SupportCategory::Utility,
            80,
            false, false, false,
            {"clean_water", "water_filter_media"}
        },
        {
            "skin_bt72_ashgray",
            "BT-72 Hull Livery: Ash Gray",
            "Cosmetic paint set for BT-72. No gameplay effect.",
            SupportCategory::Skins,
            160,
            false, false, false,
            {"skin_bt72_ashgray"}
        },
        {
            "cosmetic_operator_badge",
            "Relay Operator Badge",
            "Profile cosmetic and terminal insignia set.",
            SupportCategory::Cosmetics,
            60,
            false, false, false,
            {"badge_relay_operator", "terminal_theme_relay"}
        },
        {
            "support_illegal_weapon_pack",
            "DEBUG Forbidden Pack",
            "Should never appear in UI.",
            SupportCategory::Utility,
            9999,
            true, false, true,
            {"weapon_plasma_x"}
        }
    };
}

Отрисовка должна показывать только допустимые позиции.

6.3. Таймеры и состояние Fey Ring Network
inline FeyGateState ComputeFeyGateState(const FeyGateCycle& gate, std::int64_t nowUnix) {
    if (gate.unstable) {
        return FeyGateState::Unstable;
    }
    if (nowUnix >= gate.opensAtUnix && nowUnix < gate.closesAtUnix) {
        return FeyGateState::Open;
    }
    if (nowUnix < gate.opensAtUnix && (gate.opensAtUnix - nowUnix) <= 300) {
        return FeyGateState::OpeningSoon;
    }
    if (nowUnix >= gate.closesAtUnix && nowUnix < gate.cooldownEndsUnix) {
        return FeyGateState::Cooldown;
    }
    return FeyGateState::Closed;
}

inline std::string FormatCountdown(std::int64_t seconds) {
    if (seconds <= 0) {
        return "00:00";
    }
    const auto minutes = seconds / 60;
    const auto remSeconds = seconds % 60;
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%02lld:%02lld",
        static_cast<long long>(minutes),
        static_cast<long long>(remSeconds));
    return buffer;
}
6.4. Friends + search
inline bool FriendMatchesSearch(const FriendEntry& entry, const std::string& query) {
    if (query.empty()) {
        return true;
    }

    auto lower = [](std::string value) {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    };

    const std::string q = lower(query);
    return lower(entry.nickname).find(q) != std::string::npos
        || lower(entry.accountId).find(q) != std::string::npos
        || lower(entry.currentWorld).find(q) != std::string::npos;
}
7. UI: как это рисовать в игре и лаунчере
7.1. Главный хаб Lanline Services
#include "imgui.h"

namespace bunker {

const char* ToLabel(FriendStatus status) {
    switch (status) {
        case FriendStatus::Offline: return "Offline";
        case FriendStatus::Online: return "Online";
        case FriendStatus::InSession: return "In Session";
        case FriendStatus::InPortalTransit: return "In Portal Transit";
        case FriendStatus::DoNotDisturb: return "Do Not Disturb";
    }
    return "Unknown";
}

const char* ToLabel(ServiceHubMode mode) {
    switch (mode) {
        case ServiceHubMode::OfflineLocal: return "Offline Local";
        case ServiceHubMode::LanlineLocal: return "Lanline Local";
        case ServiceHubMode::RelayOnline: return "Relay Online";
    }
    return "Unknown";
}

void DrawSupportTab(LanlineServicesState& state);
void DrawFeyGateTab(LanlineServicesState& state, std::int64_t nowUnix);
void DrawFriendsTab(LanlineServicesState& state, char* searchBuffer, std::size_t searchSize);
void DrawChatTab(LanlineServicesState& state, int& selectedChannelIndex, char* messageBuffer, std::size_t messageSize);
void DrawVoiceTab(LanlineServicesState& state);

void DrawLanlineServicesHub(LanlineServicesState& state, std::int64_t nowUnix) {
    ImGui::Begin("Lanline Services");
    ImGui::Text("Mode: %s", ToLabel(state.mode));
    ImGui::Separator();

    static char friendSearch[128] = "";
    static char messageInput[256] = "";
    static int selectedChannelIndex = 0;

    if (ImGui::BeginTabBar("LanlineServicesTabs")) {
        if (ImGui::BeginTabItem("Friends")) {
            DrawFriendsTab(state, friendSearch, sizeof(friendSearch));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Chat")) {
            DrawChatTab(state, selectedChannelIndex, messageInput, sizeof(messageInput));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Voice")) {
            DrawVoiceTab(state);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Support")) {
            DrawSupportTab(state);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Fey Rings")) {
            DrawFeyGateTab(state, nowUnix);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace bunker
7.2. Support / store экран без pay-to-win
void bunker::DrawSupportTab(LanlineServicesState& state) {
    ImGui::TextWrapped("Lanline Support: materials, skins, cosmetics and utility requests only.");
    ImGui::TextDisabled("No weapons. No completed tanks. No direct combat advantage.");
    ImGui::Separator();

    for (const auto& item : state.supportCatalog) {
        if (!IsAllowedSupportItem(item)) {
            continue;
        }

        ImGui::PushID(item.id.c_str());
        ImGui::Text("%s", item.label.c_str());
        ImGui::TextWrapped("%s", item.description.c_str());
        ImGui::BulletText("Cost: %d credits", item.priceCredits);

        if (ImGui::TreeNode("Contents")) {
            for (const auto& content : item.contents) {
                ImGui::BulletText("%s", content.c_str());
            }
            ImGui::TreePop();
        }

        if (ImGui::Button("Request")) {
            SupportOrder order;
            order.orderId = "order_" + item.id + "_" + std::to_string(state.supportOrders.size() + 1);
            order.itemId = item.id;
            order.itemLabel = item.label;
            order.state = SupportOrderState::Draft;
            order.destinationNode = "Shelter 17";
            order.createdAt = 0; // fill with real unix time
            order.etaUnix = 0;   // fill with scheduler later
            state.supportOrders.push_back(order);
        }

        ImGui::Separator();
        ImGui::PopID();
    }

    if (!state.supportOrders.empty()) {
        ImGui::Text("Orders");
        for (const auto& order : state.supportOrders) {
            ImGui::BulletText("%s -> %s", order.itemLabel.c_str(), order.destinationNode.c_str());
        }
    }
}
7.3. Экран расписания Fey Ring Network
const char* ToLabel(bunker::FeyGateState state) {
    switch (state) {
        case bunker::FeyGateState::Closed: return "Closed";
        case bunker::FeyGateState::OpeningSoon: return "Opening Soon";
        case bunker::FeyGateState::Open: return "Open";
        case bunker::FeyGateState::Cooldown: return "Cooldown";
        case bunker::FeyGateState::Unstable: return "Unstable";
    }
    return "Unknown";
}

void bunker::DrawFeyGateTab(LanlineServicesState& state, std::int64_t nowUnix) {
    ImGui::TextWrapped("Fey Ring Network: inter-city and inter-server transit windows.");
    ImGui::Separator();

    for (auto& gate : state.feyGateCycles) {
        gate.state = ComputeFeyGateState(gate, nowUnix);

        ImGui::PushID(gate.id.c_str());
        ImGui::Text("%s -> %s", gate.originLabel.c_str(), gate.destinationLabel.c_str());
        ImGui::BulletText("Class: %s", gate.routeClass.c_str());
        ImGui::BulletText("State: %s", ToLabel(gate.state));
        ImGui::BulletText("Queue: %d / %d", gate.queueSize, gate.capacity);

        if (gate.state == FeyGateState::Open) {
            ImGui::BulletText("Closes in: %s",
                FormatCountdown(gate.closesAtUnix - nowUnix).c_str());
        } else if (gate.state == FeyGateState::OpeningSoon || gate.state == FeyGateState::Closed) {
            ImGui::BulletText("Opens in: %s",
                FormatCountdown(gate.opensAtUnix - nowUnix).c_str());
        } else if (gate.state == FeyGateState::Cooldown) {
            ImGui::BulletText("Cooldown ends in: %s",
                FormatCountdown(gate.cooldownEndsUnix - nowUnix).c_str());
        }

        if (gate.interServer) {
            ImGui::TextDisabled("Inter-server transit");
        } else {
            ImGui::TextDisabled("Inter-city transit");
        }

        ImGui::Separator();
        ImGui::PopID();
    }
}
7.4. Friends UI
void bunker::DrawFriendsTab(LanlineServicesState& state, char* searchBuffer, std::size_t searchSize) {
    ImGui::InputTextWithHint("Search", "nickname / account / world", searchBuffer, searchSize);
    ImGui::Separator();

    for (auto& entry : state.friends) {
        if (!FriendMatchesSearch(entry, searchBuffer)) {
            continue;
        }

        ImGui::PushID(entry.accountId.c_str());
        ImGui::Text("%s", entry.nickname.c_str());
        ImGui::BulletText("Status: %s", ToLabel(entry.status));
        if (!entry.currentWorld.empty()) {
            ImGui::BulletText("World: %s", entry.currentWorld.c_str());
        }
        if (!entry.currentSessionId.empty()) {
            ImGui::BulletText("Session: %s", entry.currentSessionId.c_str());
        }

        if (ImGui::Button(entry.voiceMuted ? "Unmute Voice" : "Mute Voice")) {
            entry.voiceMuted = !entry.voiceMuted;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("Voice Volume", &entry.voiceVolume, 0.0f, 1.5f);

        ImGui::SameLine();
        ImGui::Button("Invite");
        ImGui::SameLine();
        ImGui::Button("Message");

        ImGui::Separator();
        ImGui::PopID();
    }
}
7.5. Chat UI
void bunker::DrawChatTab(LanlineServicesState& state, int& selectedChannelIndex, char* messageBuffer, std::size_t messageSize) {
    if (state.channels.empty()) {
        ImGui::TextDisabled("No channels available.");
        return;
    }

    selectedChannelIndex = std::clamp(selectedChannelIndex, 0, static_cast<int>(state.channels.size()) - 1);

    if (ImGui::BeginListBox("Channels", ImVec2(220.0f, 140.0f))) {
        for (int i = 0; i < static_cast<int>(state.channels.size()); ++i) {
            const bool selected = (i == selectedChannelIndex);
            if (ImGui::Selectable(state.channels[i].displayName.c_str(), selected)) {
                selectedChannelIndex = i;
            }
        }
        ImGui::EndListBox();
    }

    ImGui::Separator();

    auto& channel = state.channels[static_cast<std::size_t>(selectedChannelIndex)];
    ImGui::Text("Channel: %s", channel.displayName.c_str());

    ImGui::BeginChild("ChatLog", ImVec2(0.0f, 260.0f), true);
    for (const auto& msg : channel.messages) {
        if (msg.systemMessage) {
            ImGui::TextDisabled("[%s] %s", msg.senderLabel.c_str(), msg.body.c_str());
        } else {
            ImGui::TextWrapped("%s: %s", msg.senderLabel.c_str(), msg.body.c_str());
        }
    }
    ImGui::EndChild();

    ImGui::InputText("Message", messageBuffer, messageSize);
    if (ImGui::Button("Send") && std::strlen(messageBuffer) > 0) {
        ChatMessage msg;
        msg.id = "msg_" + std::to_string(channel.messages.size() + 1);
        msg.channelId = channel.id;
        msg.channelType = channel.type;
        msg.senderId = "local_player";
        msg.senderLabel = "You";
        msg.body = messageBuffer;
        msg.localEcho = true;
        channel.messages.push_back(msg);
        messageBuffer[0] = '\0';
    }
}
7.6. Voice settings UI
void bunker::DrawVoiceTab(LanlineServicesState& state) {
    auto& voice = state.voice;

    ImGui::Checkbox("Enable Voice", &voice.enabled);
    ImGui::Checkbox("Push To Talk", &voice.pushToTalk);
    ImGui::InputText("PTT Key", voice.pushToTalkKey, IM_ARRAYSIZE(voice.pushToTalkKey));

    ImGui::SliderFloat("Input Sensitivity", &voice.inputSensitivity, 0.0f, 1.0f);
    ImGui::SliderFloat("Input Gain", &voice.inputGain, 0.0f, 2.0f);
    ImGui::SliderFloat("Output Gain", &voice.outputGain, 0.0f, 2.0f);

    ImGui::InputInt("Input Device", &voice.selectedInputDevice);
    ImGui::InputInt("Output Device", &voice.selectedOutputDevice);

    if (ImGui::Button("Test Microphone")) {
        // hook into future mic test
    }
}
8. Как это внедрять без ломки проекта

Правильный путь:

Шаг 1. Сначала модель

Добавить новый модуль:

include/LanlineServices.hpp
src/LanlineServices.cpp

Это не распил существующего кода, а локальный новый слой.

Шаг 2. Сначала только shell UI

Сделать:

friends tab
chat tab
voice tab
support tab
fey rings tab

Но без тяжелой сетевой/платежной магии.

Шаг 3. Сначала launcher + runtime, не editor
легкая версия в BunkerLauncher
полная версия в BunkerGame
Шаг 4. Сначала local/LAN mode

То есть:

friends presence
session chat
voice settings
portal schedules с таймерами
support catalog без реального внешнего платежного backend
Шаг 5. Только потом online relay

Когда Lanline - optime и LAN foundation окрепнут, тогда уже:

real online friends
relay presence
real cross-server schedules
real support ordering backend
9. Что не надо делать сейчас

Не надо сейчас:

тащить CEF / Chromium / WebView;
строить настоящий браузер сайтов;
городить серверный магазин;
делать боевые pay-to-win вещи;
смешивать это с editor;
делать жесткую internet dependency.

Это противоречит текущему правильному вектору проекта: solo + LAN first, UI как часть мира, launcher/game/editor split, и только UX-идеи из внешних UI-веток, а не их стек.

10. Дальнейший план по порядку

Вот правильная очередь с учетом всего проекта и нового слоя.

Ближайшие шаги
Закрыть Launcher stability pass
Закрыть Launcher v1
Дать Lanline - optime live diagnostics
Пересобрать 3 приложения
Почистить build-папки
Обновить ROADMAP.md
После этого
Сделать shell-версию Lanline Services
friends
chat
voice settings
support catalog
fey schedule
Сделать runtime-версию Lanline Services
в Pip-Pad / relay panel
Сделать launcher-версию Lanline Services
облегченный social/service entry hub
Довести Fey Ring Network
timers
states
routes
city/server distinction
queue/load
Довести support/request flow
draft
submitted
delivery
claim
После укрепления LAN foundation
live friend presence
relay chat
future voice transport
online relay backend
ИТОГ

Самая правильная форма этой идеи:

не браузер интернета
а внутренний сервисный интерфейс мира

Каноническая структура:

Lanline Services — friends / chat / voice / support
Fey Ring Network — portal timers / routes / schedules / transitions

И это реально хорошо подходит твоему проекту.

Следующим сообщением я могу дать уже еще более прикладной набор: какие именно новые файлы создать, какие функции объявить, и куда именно вызывать DrawLanlineServicesHub(...) в BunkerLauncher и BunkerGame.

сильно улучшает идею.

Теперь слой становится не “встроенным браузером с самого начала”, а заслуживаемой системной сетью, которая открывается только после первого реального шага по восстановлению мира. Это очень хорошо попадает в канон проекта: сначала игрок поднимает инфраструктуру, потом получает доступ к более широким сервисам. В проекте уже есть tower_sync / power grid-логика, а значит такой unlock естественно встраивается в существующий recovery-flow.

И второй важный момент:
заказывать детали для танка и аптечки — можно, если это оформлено как:

расходники,
ремонтные наборы,
сервисные комплекты,
med supplies,

а не как:

готовое оружие,
готовые танки,
готовые боевые модули,
готовая “мощь”.
Это не ломает баланс и хорошо ложится на recovery/service fantasy проекта.
Обновленная каноническая версия слоя
Новый принцип активации

Lanline Services и связанный “мини-браузер” не активны с самого начала.

Они открываются только после того, как игрок:

доходит до первой вышки;
восстанавливает ее;
выполняет tower sync;
поднимает первый стабильный узел сети.

То есть это уже не просто меню, а результат восстановления связи.

Это очень хорошее дизайнерское правило, потому что:

усиливает progression;
делает сервисы частью мира;
не выбрасывает все social/service features в лицо слишком рано;
связывает UI с recovery.

В проекте это особенно уместно, потому что ранний Power Grid уже существует как системный слой, а tower sync уже оформлен и в editor presets, и в runtime progression.

Что именно теперь открывает первая вышка

После первой активированной вышки открывается ограниченная версия сервиса.

Открывается сразу
Lanline Services
Friends
Chat
Voice Settings
Fey Ring Schedule
Support Requests
Не обязательно сразу открывается полностью

Можно сделать по ступеням.

После первой вышки
чат
поиск друзей
базовые настройки голоса
расписание колец/порталов
базовые support requests
После второй/третьей важной сетевой точки
расширенные relay services
межгородские переходы
межсерверные окна
улучшенные заказы
richer presence/status

То есть сеть может расти вместе с инфраструктурой.

Обновленный дизайн support/request слоя

Теперь он должен включать не только материалы и скины, но и:

Разрешенные service items
Материалы
salvage crates
repair parts
industrial composites
clean water reserve
filter media
wiring bundles
sealants
lubricants
Косметика
скины
ливреи
insignia
terminal themes
camp banners
profile badges
Танковые сервисные наборы

Это очень хорошее добавление.

Например:

ходовка
suspension patch kit
track repair set
wheel bearing service pack
башня
turret servo kit
stabilization maintenance set
turret repair bundle
движок / power core
engine service kit
cooling pack
power-core maintenance set
сенсоры
optics repair pack
sensor lens kit
relay calibration pack
корпус
hull patch plates
armor welding set
Медицинские и полевые расходники
medkit
trauma kit
field dressing pack
stim reserve
anti-toxin pack
expedition medical crate
Что запрещено по-прежнему

Даже после открытия башни все равно запрещено:

покупать готовое оружие;
покупать готовые танки;
покупать собранные боевые машины;
покупать уникальные боевые преимущества;
покупать “победу”.

Разрешен сервис и снабжение, а не мощь в готовом виде.

Новая правильная структура экрана
Внутри Lanline Services

Вкладки:

Overview
Friends
Chat
Voice
Support
Tank Service
Medical
Fey Rings
Что где
Support
материалы
косметика
общие utility-наборы
Tank Service
сервисные наборы по подсистемам танка
заявки на доставку
ETA
список совместимости
Medical
аптечки
перевязочные наборы
анти-токсины
emergency stock
Fey Rings
расписание
кольца фей
порталы фей
маршруты
таймеры
статусы
Новый progression-gate для слоя

Это очень важно.

Стадии открытия
Stage 0 — до вышки

Сервисный слой недоступен.
В UI можно показывать:

Lanline Services: offline
Relay access unavailable
Tower synchronization required
Stage 1 — после первой активной вышки

Открывается ограниченный relay-access:

friends
chat
voice settings
local support requests
local portal schedule
Stage 2 — после стабильной сети / power backbone

Открываются:

tank service deliveries
medical requests
richer relay schedule
cross-node services
Stage 3 — после развитой сети / portal backbone

Открываются:

inter-city portal windows
inter-server schedules
expanded relay services
broader social network presence

Это прекрасно связывает сервисы с восстановлением мира.

Проработанные примеры кода

Ниже — обновленные примеры с учетом:

unlock через первую вышку,
tank service kits,
medical kits,
portal timers.
1. Флаг открытия сервиса
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace bunker {

enum class ServicesUnlockTier {
    Locked,
    TowerLinked,
    BackboneStable,
    RelayExpanded
};

struct ServicesUnlockState {
    ServicesUnlockTier tier = ServicesUnlockTier::Locked;
    bool firstTowerActivated = false;
    bool localRelayAvailable = false;
    bool backboneStable = false;
    bool intercityPortalsUnlocked = false;
    bool interserverPortalsUnlocked = false;
};

inline bool IsLanlineServicesUnlocked(const ServicesUnlockState& state) {
    return state.firstTowerActivated && state.tier != ServicesUnlockTier::Locked;
}

inline bool IsTankServiceUnlocked(const ServicesUnlockState& state) {
    return state.tier >= ServicesUnlockTier::BackboneStable;
}

inline bool IsMedicalSupportUnlocked(const ServicesUnlockState& state) {
    return state.tier >= ServicesUnlockTier::TowerLinked;
}

inline bool IsIntercityPortalScheduleUnlocked(const ServicesUnlockState& state) {
    return state.tier >= ServicesUnlockTier::BackboneStable;
}

inline bool IsInterserverPortalScheduleUnlocked(const ServicesUnlockState& state) {
    return state.tier >= ServicesUnlockTier::RelayExpanded;
}

} // namespace bunker
2. Привязка unlock к миру/прогрессу

Это можно считать из world/profile state, а не хранить как чистую отдельную магию.

inline bunker::ServicesUnlockState BuildServicesUnlockState(
    bool towerSyncRestored,
    bool localGridOnline,
    bool backboneStable,
    bool relayExpanded)
{
    bunker::ServicesUnlockState state{};
    state.firstTowerActivated = towerSyncRestored;
    state.localRelayAvailable = towerSyncRestored && localGridOnline;
    state.backboneStable = backboneStable;
    state.intercityPortalsUnlocked = backboneStable;
    state.interserverPortalsUnlocked = relayExpanded;

    if (!towerSyncRestored) {
        state.tier = bunker::ServicesUnlockTier::Locked;
    } else if (towerSyncRestored && !backboneStable) {
        state.tier = bunker::ServicesUnlockTier::TowerLinked;
    } else if (backboneStable && !relayExpanded) {
        state.tier = bunker::ServicesUnlockTier::BackboneStable;
    } else {
        state.tier = bunker::ServicesUnlockTier::RelayExpanded;
    }

    return state;
}
3. Новые категории support items
enum class SupportCategory {
    Materials,
    Skins,
    Cosmetics,
    Utility,
    TankService,
    Medical
};

enum class TankSubsystem {
    None,
    Hull,
    Suspension,
    Turret,
    Engine,
    Sensors,
    PowerCore
};

struct SupportCatalogItem {
    std::string id;
    std::string label;
    std::string description;
    SupportCategory category = SupportCategory::Materials;
    int priceCredits = 0;

    bool grantsWeapon = false;
    bool grantsCompletedTank = false;
    bool combatAdvantage = false;

    TankSubsystem tankSubsystem = TankSubsystem::None;

    std::vector<std::string> contents;
};

inline bool IsAllowedSupportItem(const SupportCatalogItem& item) {
    return !item.grantsWeapon
        && !item.grantsCompletedTank
        && !item.combatAdvantage;
}
4. Каталог новых заказов: танковые сервисные наборы и аптечки
inline std::vector<SupportCatalogItem> MakeDefaultSupportCatalog() {
    return {
        {
            "support_salvage_small",
            "Salvage Crate / Small",
            "Basic recovery materials for shelter upkeep and workshop stock.",
            SupportCategory::Materials,
            120,
            false, false, false,
            TankSubsystem::None,
            {"bulk_salvage", "repair_parts", "circuit_scrap"}
        },
        {
            "skin_bt72_ashgray",
            "BT-72 Hull Livery: Ash Gray",
            "Cosmetic paint set for BT-72. No gameplay effect.",
            SupportCategory::Skins,
            160,
            false, false, false,
            TankSubsystem::None,
            {"skin_bt72_ashgray"}
        },
        {
            "tank_suspension_kit",
            "BT-72 Suspension Repair Kit",
            "Service bundle for ходовка / suspension and track maintenance.",
            SupportCategory::TankService,
            210,
            false, false, false,
            TankSubsystem::Suspension,
            {"track_patch", "bearing_set", "grease_pack", "alignment_tools"}
        },
        {
            "tank_turret_kit",
            "BT-72 Turret Service Kit",
            "Turret servo, stabilization and bearing maintenance bundle.",
            SupportCategory::TankService,
            230,
            false, false, false,
            TankSubsystem::Turret,
            {"servo_patch", "turret_bearing", "stabilizer_fluid"}
        },
        {
            "tank_engine_kit",
            "BT-72 Engine Service Kit",
            "Field maintenance kit for engine and cooling assembly.",
            SupportCategory::TankService,
            250,
            false, false, false,
            TankSubsystem::Engine,
            {"engine_seal", "coolant_pack", "injector_cleanser"}
        },
        {
            "tank_sensor_kit",
            "BT-72 Sensor Recovery Kit",
            "Optics, relay and calibration tools for damaged sensor arrays.",
            SupportCategory::TankService,
            185,
            false, false, false,
            TankSubsystem::Sensors,
            {"lens_pack", "sensor_relay", "calibration_spool"}
        },
        {
            "medkit_standard",
            "Field Medkit",
            "Standard expedition med supply for operator recovery.",
            SupportCategory::Medical,
            75,
            false, false, false,
            TankSubsystem::None,
            {"medkit", "bandage_roll", "sterile_patch"}
        },
        {
            "medkit_trauma",
            "Trauma Response Pack",
            "Advanced trauma bundle for severe field damage recovery.",
            SupportCategory::Medical,
            120,
            false, false, false,
            TankSubsystem::None,
            {"trauma_kit", "injector", "coagulant_pack"}
        }
    };
}
5. Защита UI до активации первой вышки
void DrawLanlineServicesLockedScreen(const bunker::ServicesUnlockState& unlockState) {
    ImGui::Begin("Lanline Services");
    ImGui::Text("Lanline Services: offline");
    ImGui::Separator();

    if (!unlockState.firstTowerActivated) {
        ImGui::TextWrapped("Relay access unavailable. Restore and synchronize the first tower to open Lanline Services.");
        ImGui::BulletText("Requirement: first tower restored");
        ImGui::BulletText("Requirement: tower sync completed");
    } else {
        ImGui::TextWrapped("Relay node detected, but services are still restricted.");
    }

    ImGui::End();
}
6. Главный экран теперь зависит от стадии unlock
void DrawLanlineServicesHub(bunker::LanlineServicesState& state,
                            const bunker::ServicesUnlockState& unlockState,
                            std::int64_t nowUnix) {
    if (!bunker::IsLanlineServicesUnlocked(unlockState)) {
        DrawLanlineServicesLockedScreen(unlockState);
        return;
    }

    ImGui::Begin("Lanline Services");
    ImGui::Text("Lanline Services Online");
    ImGui::Separator();

    static char friendSearch[128] = "";
    static char messageInput[256] = "";
    static int selectedChannelIndex = 0;

    if (ImGui::BeginTabBar("LanlineServicesTabs")) {
        if (ImGui::BeginTabItem("Friends")) {
            DrawFriendsTab(state, friendSearch, sizeof(friendSearch));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Chat")) {
            DrawChatTab(state, selectedChannelIndex, messageInput, sizeof(messageInput));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Voice")) {
            DrawVoiceTab(state);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Support")) {
            DrawSupportTab(state, unlockState);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Fey Rings")) {
            DrawFeyGateTab(state, unlockState, nowUnix);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
7. Support tab с разделением на обычные заказы, танковый сервис и медицину
const char* ToLabel(bunker::SupportCategory category) {
    switch (category) {
        case bunker::SupportCategory::Materials: return "Materials";
        case bunker::SupportCategory::Skins: return "Skins";
        case bunker::SupportCategory::Cosmetics: return "Cosmetics";
        case bunker::SupportCategory::Utility: return "Utility";
        case bunker::SupportCategory::TankService: return "Tank Service";
        case bunker::SupportCategory::Medical: return "Medical";
    }
    return "Unknown";
}

void DrawSupportTab(bunker::LanlineServicesState& state,
                    const bunker::ServicesUnlockState& unlockState) {
    ImGui::TextWrapped("Relay support requests. No weapons, no completed tanks, no direct combat advantages.");
    ImGui::Separator();

    for (const auto& item : state.supportCatalog) {
        if (!bunker::IsAllowedSupportItem(item)) {
            continue;
        }

        if (item.category == bunker::SupportCategory::TankService &&
            !bunker::IsTankServiceUnlocked(unlockState)) {
            continue;
        }

        if (item.category == bunker::SupportCategory::Medical &&
            !bunker::IsMedicalSupportUnlocked(unlockState)) {
            continue;
        }

        ImGui::PushID(item.id.c_str());
        ImGui::Text("[%s] %s", ToLabel(item.category), item.label.c_str());
        ImGui::TextWrapped("%s", item.description.c_str());
        ImGui::BulletText("Cost: %d credits", item.priceCredits);

        if (item.category == bunker::SupportCategory::TankService) {
            ImGui::TextDisabled("Tank subsystem service request");
        }

        if (ImGui::TreeNode("Contents")) {
            for (const auto& content : item.contents) {
                ImGui::BulletText("%s", content.c_str());
            }
            ImGui::TreePop();
        }

        if (ImGui::Button("Request")) {
            bunker::SupportOrder order;
            order.orderId = "order_" + item.id + "_" + std::to_string(state.supportOrders.size() + 1);
            order.itemId = item.id;
            order.itemLabel = item.label;
            order.state = bunker::SupportOrderState::Draft;
            order.destinationNode = "Shelter 17";
            state.supportOrders.push_back(order);
        }

        ImGui::Separator();
        ImGui::PopID();
    }
}
8. Fey Ring Network с ограничением по стадии unlock
void DrawFeyGateTab(bunker::LanlineServicesState& state,
                    const bunker::ServicesUnlockState& unlockState,
                    std::int64_t nowUnix) {
    ImGui::TextWrapped("Fey Ring Network: transit windows and relay routes.");
    ImGui::Separator();

    for (auto& gate : state.feyGateCycles) {
        if (gate.interServer && !bunker::IsInterserverPortalScheduleUnlocked(unlockState)) {
            continue;
        }
        if (!gate.interServer && !bunker::IsIntercityPortalScheduleUnlocked(unlockState)) {
            continue;
        }

        gate.state = bunker::ComputeFeyGateState(gate, nowUnix);

        ImGui::PushID(gate.id.c_str());
        ImGui::Text("%s -> %s", gate.originLabel.c_str(), gate.destinationLabel.c_str());
        ImGui::BulletText("State: %s", ToLabel(gate.state));
        ImGui::BulletText("Queue: %d / %d", gate.queueSize, gate.capacity);

        if (gate.state == bunker::FeyGateState::Open) {
            ImGui::BulletText("Closes in: %s",
                bunker::FormatCountdown(gate.closesAtUnix - nowUnix).c_str());
        } else {
            ImGui::BulletText("Opens in: %s",
                bunker::FormatCountdown(gate.opensAtUnix - nowUnix).c_str());
        }

        ImGui::TextDisabled(gate.interServer ? "Inter-server portal" : "Inter-city portal");
        ImGui::Separator();
        ImGui::PopID();
    }
}
9. Пример стартовых циклов Fey Ring Network
inline std::vector<bunker::FeyGateCycle> MakeDefaultFeyGateCycles(std::int64_t nowUnix) {
    return {
        {
            "ring_shelter17_ironspan",
            "Shelter 17 Ring",
            "Iron Span",
            "City",
            bunker::FeyGateState::Closed,
            nowUnix + 240,
            nowUnix + 720,
            nowUnix + 1080,
            2,
            12,
            false,
            false
        },
        {
            "portal_shelter17_relay_shard",
            "Shelter 17 Relay",
            "Relay Shard Node",
            "Server",
            bunker::FeyGateState::Closed,
            nowUnix + 900,
            nowUnix + 1260,
            nowUnix + 1800,
            0,
            24,
            true,
            false
        }
    };
}
Как это теперь правильно внедрять
Ближайшая очередь
сначала launcher stability pass
потом launcher v1
потом live diagnostics для Lanline - optime
потом shell-версия Lanline Services
добавить unlock через первую вышку
добавить support catalog
добавить tank service + medical requests
добавить Fey Ring Network timers
потом уже richer social/runtime features
Почему именно так

Потому что это не должно заменить текущий план, а должно лечь следующим большим service/social слоем после того, как у тебя стабилизирован launcher и усилен Lanline - optime.

Итог

Теперь идея стала гораздо лучше, чем в первой формулировке.

Правильная каноническая версия такая:

это не свободный браузер интернета
это внутренний сервисный хаб мира
он открывается только после первой активированной вышки
там можно:
искать друзей
чатиться
настраивать голос
смотреть расписание колец и порталов фей
заказывать материалы
заказывать скины
заказывать танковые сервисные наборы
заказывать аптечки и медрасходники
нельзя:
покупать оружие
покупать танки
покупать готовую боевую силу

Это очень хорошо встает в твой проект.

Следующим сообщением я могу сделать уже совсем прикладной integration plan:

какие файлы создать,
какие поля добавить в state/profile,
где именно вызывать этот UI в BunkerLauncher и BunkerGame,
и как это связать с tower_sync без ломки текущего кода.

правильная модель.

Каноническое правило экономики

За игровые деньги:

ресурсы;
ремонтные наборы;
аптечки;
сервисные расходники;
материалы для восстановления и вылазок.

За реальные деньги:

только скины;
только косметика;
только визуальные темы/эмблемы/оформление;
только как символическая плата, без боевого преимущества.

Это хорошо ложится в канон проекта, потому что игра держится на recovery / logistics / service fantasy, а не на продаже силы. Плюс текущий вектор проекта — solo + LAN first, а не service-first MMO, так что pay-to-win сюда действительно не подходит.

Как это должно работать в мире

Lanline Services открывается только после первой восстановленной и синхронизированной вышки через tower_sync. Это уже очень хорошо встраивается в текущий проект, потому что tower_sync и power grid у тебя уже существуют как реальные runtime/editor слои.

После первой вышки открываются:

чат;
друзья;
голосовые настройки;
расписание Fey Ring Network;
заказы за игровые деньги;
косметический каталог.

Но экономически эти части должны быть разделены жестко.

1. Финальная модель магазина/сервиса
1.1. Вкладки

Внутри Lanline Services:

Supplies
Tank Service
Medical
Cosmetics
Friends
Chat
Voice
Fey Rings
1.2. Валюты
Игровая валюта

Назови ее чем-то мирным и системным, например:

Relay Credits
Shelter Credits
Recovery Scrip
Lanline Credits

Лучший вариант для проекта: Recovery Scrip или Relay Credits.

Этой валютой оплачиваются:

ресурсы;
repair kits;
service packs;
medkits;
field supplies.
Реальная валюта

В UI лучше не писать грубо “донат”.
Лучше назвать:

Support Purchase
Symbolic Support
Relay Support
Shelter Support

То есть косметика покупается как поддержка проекта/узла, а не как “покупка силы”.

2. Что можно продавать за игровые деньги
2.1. Supplies
salvage crate;
repair parts;
wiring bundles;
sealants;
filter media;
clean water reserve;
field batteries;
industrial composites.
2.2. Tank Service

Очень хорошая идея — продавать не оружие, а именно сервисные комплекты по подсистемам танка:

Suspension Repair Kit
Track Patch Set
Turret Service Kit
Engine Service Pack
Cooling Pack
Sensor Recovery Kit
Power Core Maintenance Kit
Hull Patch Plates
2.3. Medical
medkit;
trauma pack;
anti-toxin pack;
field dressing kit;
expedition medical crate.

Это полностью соответствует духу проекта: recovery, service, logistics, survival. А не “магазин силы”.

3. Что можно продавать за реальные деньги

Только:

скины BT-72;
ливреи;
insignia;
terminal themes;
profile badges;
camp banners;
UI themes;
cosmetic voice/radio effects, если когда-нибудь понадобятся.

Нельзя:

weapons;
tanks;
combat modules;
exclusive stat boosts;
anything that changes balance.
4. Самое важное правило

Нужно в коде зашить жесткий запрет:

предмет за реальные деньги не может быть боевым;
предмет за реальные деньги не может давать прямое преимущество;
предмет за реальные деньги не может быть техникой или оружием;
расходники и ремонтные наборы идут только за игровую валюту.

Это лучше не держать “по договоренности”, а закрепить логикой каталога.

5. Проработанный код
5.1. Добавляем тип валюты
enum class StoreCurrency {
    InGame,
    RealMoneySymbolic
};
5.2. Обновляем модель товара
enum class SupportCategory {
    Supplies,
    TankService,
    Medical,
    Skins,
    Cosmetics
};

enum class TankSubsystem {
    None,
    Hull,
    Suspension,
    Turret,
    Engine,
    Sensors,
    PowerCore
};

struct ServiceCatalogItem {
    std::string id;
    std::string label;
    std::string description;

    SupportCategory category = SupportCategory::Supplies;
    StoreCurrency currency = StoreCurrency::InGame;

    int inGamePrice = 0;
    std::string realMoneyLabel; // например "Support Tier A"

    bool grantsWeapon = false;
    bool grantsCompletedTank = false;
    bool grantsCombatAdvantage = false;

    TankSubsystem tankSubsystem = TankSubsystem::None;

    std::vector<std::string> contents;
};
5.3. Жесткая валидация допустимости
bool IsAllowedCatalogItem(const ServiceCatalogItem& item) {
    if (item.grantsWeapon) {
        return false;
    }
    if (item.grantsCompletedTank) {
        return false;
    }
    if (item.grantsCombatAdvantage) {
        return false;
    }

    if (item.currency == StoreCurrency::RealMoneySymbolic) {
        if (item.category != SupportCategory::Skins &&
            item.category != SupportCategory::Cosmetics) {
            return false;
        }
    }

    if (item.currency == StoreCurrency::InGame) {
        if (item.category == SupportCategory::Skins ||
            item.category == SupportCategory::Cosmetics) {
            return false;
        }
    }

    return true;
}

Это уже закрывает главный риск: никто случайно не положит “turret cannon pack” в real-money каталог.

5.4. Пример правильного каталога
std::vector<ServiceCatalogItem> MakeDefaultLanlineCatalog() {
    return {
        {
            "supply_salvage_small",
            "Small Salvage Crate",
            "Basic recovery materials for workshop and shelter upkeep.",
            SupportCategory::Supplies,
            StoreCurrency::InGame,
            120,
            "",
            false, false, false,
            TankSubsystem::None,
            {"bulk_salvage", "repair_parts", "circuit_scrap"}
        },
        {
            "tank_engine_service",
            "BT-72 Engine Service Pack",
            "Field maintenance set for engine and cooling recovery.",
            SupportCategory::TankService,
            StoreCurrency::InGame,
            240,
            "",
            false, false, false,
            TankSubsystem::Engine,
            {"engine_seal", "coolant_pack", "injector_cleanser"}
        },
        {
            "tank_turret_service",
            "BT-72 Turret Service Kit",
            "Maintenance bundle for turret servo and stabilization system.",
            SupportCategory::TankService,
            StoreCurrency::InGame,
            220,
            "",
            false, false, false,
            TankSubsystem::Turret,
            {"servo_patch", "bearing_set", "stabilizer_fluid"}
        },
        {
            "medkit_field",
            "Field Medkit",
            "Standard operator recovery pack for expeditions.",
            SupportCategory::Medical,
            StoreCurrency::InGame,
            75,
            "",
            false, false, false,
            TankSubsystem::None,
            {"medkit", "bandage_roll", "sterile_patch"}
        },
        {
            "skin_bt72_ashgray",
            "BT-72 Hull Livery: Ash Gray",
            "Cosmetic livery for BT-72. No gameplay effect.",
            SupportCategory::Skins,
            StoreCurrency::RealMoneySymbolic,
            0,
            "Symbolic Support A",
            false, false, false,
            TankSubsystem::None,
            {"skin_bt72_ashgray"}
        },
        {
            "cosmetic_relay_badge",
            "Relay Operator Badge",
            "Profile badge and terminal insignia set.",
            SupportCategory::Cosmetics,
            StoreCurrency::RealMoneySymbolic,
            0,
            "Symbolic Support B",
            false, false, false,
            TankSubsystem::None,
            {"badge_relay_operator", "terminal_theme_relay"}
        }
    };
}
5.5. Разделение экранов по валютам
const char* ToLabel(StoreCurrency currency) {
    switch (currency) {
        case StoreCurrency::InGame: return "In-Game";
        case StoreCurrency::RealMoneySymbolic: return "Symbolic Support";
    }
    return "Unknown";
}
Supplies / Tank Service / Medical
void DrawOperationalStore(const std::vector<ServiceCatalogItem>& catalog) {
    for (const auto& item : catalog) {
        if (!IsAllowedCatalogItem(item)) {
            continue;
        }
        if (item.currency != StoreCurrency::InGame) {
            continue;
        }

        ImGui::PushID(item.id.c_str());
        ImGui::Text("%s", item.label.c_str());
        ImGui::TextWrapped("%s", item.description.c_str());
        ImGui::BulletText("Price: %d Relay Credits", item.inGamePrice);

        if (ImGui::TreeNode("Contents")) {
            for (const auto& entry : item.contents) {
                ImGui::BulletText("%s", entry.c_str());
            }
            ImGui::TreePop();
        }

        if (ImGui::Button("Order")) {
            // create in-game support order
        }

        ImGui::Separator();
        ImGui::PopID();
    }
}
Cosmetics
void DrawCosmeticStore(const std::vector<ServiceCatalogItem>& catalog) {
    ImGui::TextWrapped("Cosmetics only. Symbolic support. No gameplay advantage.");
    ImGui::Separator();

    for (const auto& item : catalog) {
        if (!IsAllowedCatalogItem(item)) {
            continue;
        }
        if (item.currency != StoreCurrency::RealMoneySymbolic) {
            continue;
        }

        ImGui::PushID(item.id.c_str());
        ImGui::Text("%s", item.label.c_str());
        ImGui::TextWrapped("%s", item.description.c_str());
        ImGui::BulletText("Support: %s", item.realMoneyLabel.c_str());

        if (ImGui::TreeNode("Includes")) {
            for (const auto& entry : item.contents) {
                ImGui::BulletText("%s", entry.c_str());
            }
            ImGui::TreePop();
        }

        if (ImGui::Button("Support Project")) {
            // open symbolic support flow
        }

        ImGui::Separator();
        ImGui::PopID();
    }
}
6. Как это выглядит в UX
До первой вышки

Показываем:

Lanline Services: offline
Relay access unavailable
Restore and synchronize first tower to enable services
После первой вышки

Показываем:

Supplies
Medical
Friends
Chat
Voice
Fey Rings
После стабильного backbone

Открываем:

Tank Service
расширенные маршруты Fey Rings
Косметика

Можно либо:

открыть сразу после первой вышки как “Relay Support”;
либо тоже привязать к устойчивой сети.

Я бы открыл после первой вышки, потому что по смыслу это уже “вышли в сеть”.

7. Обновленный дальнейший план

Теперь правильная очередь такая:

Закрыть Launcher stability pass
Закрыть Launcher v1
Дать live diagnostics для Lanline - optime
Сделать shell-версию Lanline Services
Добавить unlock через первую вышку
Добавить Fey Ring Network timers
Добавить friends / chat / voice settings
Потом уже расширять до richer online relay
Итог

Теперь схема стала совсем правильной:

Игровая валюта:

ресурсы
ремонтные наборы
аптечки
сервисные комплекты

Реальные деньги:

только скины
только косметика
только символическая поддержка

И это уже выглядит как зрелый, каноничный слой для твоего проекта, а не как сомнительный донат-магазин.

Следующим сообщением я могу дать уже готовый integration plan по файлам:

какой новый .hpp/.cpp создать,
что добавить в профиль/state,
где рисовать этот UI в BunkerLauncher,
где рисовать в BunkerGame,
и как привязать unlock к tower_sync.
