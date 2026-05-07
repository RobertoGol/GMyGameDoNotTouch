Я опираюсь на текущий лог/состояние: стабильная ветка Testing-imgui-viewports, последний стабильный milestone 8f0759b editor: add 3d viewport and planning shells, .codex/ как локальное состояние инструмента, и важное открытие: MapObject уже имеет float z = 0.0f, но его нужно нормально провести через save/load/editor/preview/placement workflow. Также по старому канону проекта видно важное ограничение: map/layout/world geometry остаётся human-owned, а ИИ не должен уходить в большой editor-R&D без конкретного blocker/task.

По Blender-логике: базовые transform-операции — move/rotate/scale через G/R/S, gizmo и axis constraints; transform orientation влияет на оси gizmo, а snapping нужен для контролируемого размещения объектов.

Ниже — готовый файл. Назови его, например:

AI_PROJECT_BRIEF.md

И положи в корень проекта D:\Projects\Game_Project. Новый чат/Codex должен начинать с чтения этого файла.

AI_PROJECT_BRIEF.md

0. Назначение файла

Этот файл нужен для нового AI/Codex-чата.

Он объясняет:

что это за проект;

какая текущая стабильная точка;

что уже сделано;

что сломано или недоделано;

какие правила разработки обязательны;

какие задачи идут следующими;

что нельзя делать без отдельного approval;

какие проценты готовности примерно сейчас;

куда лучше сфокусироваться.

Этот файл НЕ является автогенерируемым логом.Этот файл НЕ заменяет git, AI_FULL_LOG.md, PROJECT_CANON_AND_STATUS.md или тесты.Этот файл — стартовый brief для ИИ.

1. Проект

Путь проекта:

D:\Projects\Game_Project

Рабочая ветка:

Testing-imgui-viewports

Последняя стабильная известная точка:

8f0759b editor: add 3d viewport and planning shells

Важные стабильные commits:

8f0759b editor: add 3d viewport and planning shells
4eeb42f editor: enable imgui viewport windows
d18225b vendor: update imgui docking snapshot for viewports

Состояние .codex/:

.codex/ — локальное состояние инструмента Codex.
Не добавлять в git.
Не удалять.
Не чистить через git clean.
Оставлять untouched.
2. Главная идея игры

Проект — не просто редактор. Это игра/прототип с собственным runtime, editor tooling и будущим world/asset pipeline.

Главные направления:

Bunker / wasteland / recovery / industry / logistics loop;
BT-72 / транспорт / сервис / танк как важная часть игры;
Camp / AIMP / мастерские / восстановление / инфраструктура;
world/runtime/editor должны быть синхронизированы;
базовая игра важнее бесконечного editor-R&D.

ИИ не должен уводить проект в бесконечный редактор и забывать про игру.

3. Жёсткие engineering-правила

Работать как старый senior engineer.

Обязательные принципы:

KISS    — проще, меньше, прямее.
YAGNI   — не строить будущие системы без нужды.
Clean Code — код должен читаться как книга.
Small patches — одна задача, один маленький патч.
No palace architecture — не строить дворец кнопок.
No speculative systems — не добавлять системы "на потом".
No hidden side effects — побочные эффекты должны быть очевидны.

Код должен быть:

простым;
локальным;
читаемым;
легко откатываемым;
проверяемым;
без умных абстракций ради абстракций.

Плохой подход:

"Сделать всё как Creation Kit + Blender + BA2 + BWLD + physics + editor UI"

Хороший подход:

"Подключить существующий MapObject.z к save/load/editor preview и Shift+P."
4. Git-правила

ИИ НЕ должен делать без отдельного явного approval:

git add
git commit
git push
git clean
git reset --hard
git merge
git rebase
git switch
изменения master

Перед любыми изменениями запускать и логировать:

git status -sb
git branch --show-current
git log --oneline --decorate -8
git --no-pager diff --stat

Если ветка не Testing-imgui-viewports — остановиться.

Если есть неожиданные tracked modifications — остановиться и спросить.

Если есть только .codex/ untracked — оставить untouched.

5. Логирование

Все действия ИИ должны логироваться в:

AI_FULL_LOG.md

Правило:

append-only

Логировать:

command;
output summary;
exit code;
inspection;
edit plan;
files touched;
build result;
ctest result;
launch result;
manual check status;
final finding.

AI_FULL_LOG.md не коммитить без отдельного approval.

6. Build/test baseline

Стандартная проверка:

Get-Process BunkerEditor -ErrorAction SilentlyContinue | Stop-Process -Force

git diff --check
cmake --build build_finish_msvc --config Debug
ctest --test-dir build_finish_msvc -C Debug --output-on-failure

Если build/test падает:

остановиться;
дать точную ошибку;
не коммитить;
не лечить рандомно.

Если BunkerEditor.exe блокирует linker:

закрыть процесс;
повторить build один раз;
логировать причину.
7. Что уже сделано
7.1 ImGui multi-viewports

Сделано:

ImGui viewport windows включены;
окна можно выносить за пределы главного окна;
detached windows работают;
Render Window / Object Window / Cell View можно открывать/выносить;
закрытие detached Render Window не крашит редактор;
Render Window можно открыть снова через Windows menu.

Примерно готово:

ImGui detachable windows: 85%

Осталось:

стабилизировать UX окон;
не ломать detach behavior при новых viewport input изменениях.
7.2 3D Render Window shell

Сделано:

Render Window / Viewport открывается;
есть 3D proxy boxes/markers;
есть selected highlight;
есть camera orbit/zoom;
есть planning shell для levels/floors/layers;
есть Locations / Cells planning shell;
build/ctest проходили на стабильной точке.

Примерно готово:

3D viewport shell: 35%

Осталось:

нормальная сетка/floor;
реальное размещение по XYZ;
стабильный transform gizmo;
поворот объектов;
surface placement;
save/load vertical data;
real asset/mesh preview later.
7.3 Object / Cell / Editor shell

Сделано:

Object Window есть;
Cell View есть;
World Authoring есть;
Export / Runtime есть;
Import Assistant есть;
Asset Palette есть;
planning shells есть.

Примерно готово:

Editor window structure: 35%

Осталось:

нормальный Object Browser;
нормальный Cell/reference browser;
Reference Inspector;
Preview Object;
workflow как в world editor;
не делать это одним огромным патчем.
8. Важнейшее открытие: Z уже есть

В include/MapObject.hpp уже есть:

float z = 0.0f;

Значит:

НЕ добавлять новое поле z;
НЕ добавлять elevation;
НЕ делать второй vertical field;
использовать существующий MapObject.z.

Текущая проблема:

z существует, но не везде подключён;
save/load/prefab/editor/preview могут его игнорировать;
поэтому предметы пока нельзя нормально ставить на стол/полку/поверхность.

Главная ближайшая задача:

провести существующий MapObject.z через весь минимальный vertical placement pipeline
9. Почему Z важен

Нужно уметь:

ставить предмет на стол;
ставить предмет на полку;
ставить предмет на кровать;
ставить предмет на контейнер;
ставить предмет на крышу;
делать платформы;
делать этажи;
делать двойные этажи/арены;
делать ramps/stairs later.

Без сохранённого z предмет существует только на плоскости x/y.

Это ломает реальный world authoring.

10. Целевая модель координат

Для MapObject:

x = горизонтальная координата X
y = горизонтальная координата Y / depth-plane coordinate
z = вертикальное основание объекта
width = размер по X
depth = размер по Y/depth
height = вертикальный размер

Низ объекта:

object.z

Верх объекта:

object.z + object.height

Footprint:

x ± width / 2
y ± depth / 2
11. Целевая модель уровней

Уровни / этажи / floors:

Level 0  = базовый пол
Level +1 = верхний этаж
Level -1 = нижний этаж / подвал
Double-height = высокий объём / арена

Но важно:

Сейчас НЕ делать полноценную систему уровней.

Сначала:

MapObject.z;
save/load;
editor display;
/ - vertical movement;
Shift+P place-on-surface.

Потом:

floors/layers;
stairs;
ramps;
magnets;
surface types;
level metadata.
12. Трансформации объектов — целевая модель

Нужно прийти к Blender-like workflow, но маленькими шагами.

12.1 Move

Объект должен двигаться по трём осям:

X
Y
Z

Минимальная модель:

X/Y — уже должны быть persistent через object.x/object.y;
Z — должен быть persistent через object.z.
12.2 Rotate

Нужен поворот по осям:

Rotate X
Rotate Y
Rotate Z

Поворот должен идти нарастающе:

пользователь нажимает/удерживает rotation command;
вращает по выбранной оси;
угол меняется step-by-step;
позже snap rotation.

Но сейчас важно:

проверить есть ли rotation fields;
если нет — НЕ фейкать persistence;
сделать отдельный schema migration later.
12.3 Scale

Scale later:

только если есть поля;
иначе отдельный future task.
13. Blender-like reference

Ориентир по workflow:

Move / Rotate / Scale через G/R/S;
оси X/Y/Z ограничивают transform;
transform orientation влияет на направление gizmo;
snapping нужен для точного размещения;
transform panel показывает позицию/поворот/масштаб.

Для BunkerEditor не нужно копировать Blender UI.
Нужно взять идею:

понятный gizmo;
понятные оси;
понятный snap;
понятный transform state;
мышь и клавиатура не конфликтуют.
14. Что нельзя делать сейчас

НЕ делать сейчас:

BA2/BSA parser
D:\Projects\Ner scan
real mesh renderer
real textured preview
physics falling
full Creation Kit clone
full Blender clone
rotation schema migration
scale schema migration
multi-cell persistence
new giant UI shell
new huge editor architecture

Особенно нельзя:

снова добавлять 500–1300 строк UI-shell без рабочей механики;
снова делать competing gizmo systems;
снова миксовать viewport controls + window organization + BA2 + schema.
15. Ближайшая задача №1 — Vertical Placement MVP
Цель

Сделать так, чтобы предмет можно было поставить выше пола и сохранить это.

Например:

коробку на стол;
пистолет на ящик;
предмет на полку;
объект на платформу.
Нужно сделать
Подтвердить, что MapObject.z уже есть.
Не добавлять новое поле.
Подключить z к world save/load.
Подключить z к prefab save/load, если prefab хранит MapObject.
Подключить z к editor selected-object UI.
Подключить z к undo comparison.
Подключить z к 3D preview:
proxy base at z;
proxy top at z + height.
Сделать keyboard vertical movement:
KeypadAdd raises z;
KeypadSubtract lowers z.
Сделать Shift+P:
найти highest overlapping support object;
поставить selected object на support.z + support.height;
если support нет — поставить на z = 0.
Не делать
mouse gizmo
rotation
physics
floor system
BA2
new windows
renderer rewrite
Примерная готовность этой задачи
Vertical placement MVP: 15%

Почему не 0%:

MapObject.z уже существует.

Почему не больше:

не подтверждено, что z проходит через save/load/prefab/export/preview/UI.
16. Vertical placement MVP — точный Codex task

Новая задача для Codex:

Task: wire existing MapObject.z into vertical placement MVP.

Do not add new fields.
Do not add physics.
Do not add mouse gizmo.
Do not add UI shell.
Do not parse BA2/BSA.

Allowed edits:
- include/MapObject.hpp only if inspection proves tiny needed comment/default adjustment
- src/World.cpp
- src/PrefabLibrary.cpp
- src/WorldExport.cpp only if object export ignores z and update is safe
- src/WorldValidation.cpp only if validation directly prints/checks object position
- Editor/src/Editor_Main.cpp

Main requirements:
- old worlds without z load with z=0
- new saves preserve z
- prefab save/load preserves z
- editor selected object UI exposes Z
- 3D preview draws object at z
- KeypadAdd raises z
- KeypadSubtract lowers z
- Shift+P places selected object on highest overlapping support surface
- build/test pass
- launch editor
- stop for manual check
17. Shift+P surface placement algorithm

KISS algorithm:

selected = selected object

for each candidate object:
    if candidate is selected:
        skip

    if selected footprint overlaps candidate footprint:
        candidateTop = candidate.z + candidate.height
        choose highest candidateTop

if found:
    selected.z = highest candidateTop
else:
    selected.z = 0

Footprint overlap:

selected minX = selected.x - selected.width / 2
selected maxX = selected.x + selected.width / 2
selected minY = selected.y - selected.depth / 2
selected maxY = selected.y + selected.depth / 2

candidate minX = candidate.x - candidate.width / 2
candidate maxX = candidate.x + candidate.width / 2
candidate minY = candidate.y - candidate.depth / 2
candidate maxY = candidate.y + candidate.depth / 2

overlap if:
selected.maxX >= candidate.minX
selected.minX <= candidate.maxX
selected.maxY >= candidate.minY
selected.minY <= candidate.maxY

This is enough for MVP.

No mesh collision.
No normals.
No ramps.
No physics.

18. Manual check for Vertical Placement MVP

После build/test/launch проверить:

Render Window открывается.
Старый мир загружается.
Объекты без z стоят на полу.
Выделенный объект имеет поле Z в UI.
KeypadAdd поднимает объект.
KeypadSubtract опускает объект.
Объект визуально меняет высоту в 3D viewport.
Snap применяется к Z.
Shift+P над столом/ящиком ставит объект на верхнюю поверхность.
Shift+P без support ставит объект на пол.
Save world.
Reload world.
Z сохранился.
MMB camera работает.
Wheel zoom работает.
Detached Render Window работает.
Object Window / Cell View не сломаны.
.codex/ untouched.
BA2/BSA не трогались.
D:\Projects\Ner не сканировался.
19. Ближайшая задача №2 — Rotation fields

После Vertical Placement MVP.

Цель

Предмет должен уметь поворачиваться по осям:

rotationX
rotationY
rotationZ

или другой выбранный проектом формат.

Сначала inspect:
есть ли rotation fields;
есть ли export support;
есть ли runtime support;
есть ли old world compatibility plan.
Если нет fields:

Нужна отдельная schema migration.

Не делать вместе с z.

Примерная готовность:

Rotation persistence: 0–10%
20. Ближайшая задача №3 — Transform Gizmo

После Z и rotation model.

Цель

Нормальный X/Y/Z gizmo как editor interaction.

Требования:

один gizmo path;
не два competing systems;
draw and hit-test use same projected points;
X/Y/Z всегда видны;
X drag moves x;
Y drag moves y;
Z drag moves z;
rotation gizmo отдельно later;
MMB camera не ломается;
dragging viewport не двигает window.

Примерная готовность:

Mouse transform gizmo: 10–15%

Почему:

была попытка, но она конфликтовала со старым preview gizmo.
21. Ближайшая задача №4 — Object Browser / Cell View

После базового transform.

Цель:

нормальный Object Browser;
нормальный Cell/reference browser;
Reference Inspector;
Preview Object.

Не делать раньше Z/transform, потому что UI без рабочей механики бесполезен.

Примерная готовность:

Editor workflow UI: 25–35%
22. Ближайшая задача №5 — Asset pipeline / BA2 / BWLD refs

Позже.

Цель:

Asset Sources / Browser;
local-only BA2/BSA index;
no extraction at first;
BWLD asset reference fields;
preview cache;
real mesh preview.

Не делать сейчас.

Примерная готовность:

Asset pipeline: 0–5%
23. Текущая общая оценка проекта

Это приблизительно, не математическая истина.

Base runtime / smoke stability
55–65%

Причина:

build/ctest проходят;
есть runtime/editor/launcher/smoke checks;
но game loop/content ещё не завершены.
Editor detachable windows
80–85%

Причина:

ImGui viewports работают;
detached windows работают;
нужно не ломать.
3D editor viewport
30–40%

Причина:

proxy boxes есть;
camera есть;
selection есть;
нет нормального persistent transform pipeline.
Vertical placement
15%

Причина:

MapObject.z уже есть;
но save/load/editor/preview/surface placement надо довести.
Rotation/orientation
5–10%

Причина:

preview attempts были;
persisted model пока нет.
Object/cell authoring workflow
25–35%

Причина:

окна есть;
workflow пока сырой.
Asset/BA2/BWLD pipeline
0–5%

Причина:

только идеи/планирование;
нельзя трогать без отдельного шага.
Общая готовность editor как usable world authoring tool
25–30%
Общая готовность проекта до базовой playable/showable игры
45–55%
24. Сколько осталось

Примерно осталось:

Editor до usable world authoring: 70–75%
Game до нормального playable/showable состояния: 45–55%
Asset pipeline: 95–100%

Главный bottleneck сейчас:

нормальный world authoring transform pipeline

То есть:

XYZ;
save/load;
placement;
rotation;
object workflow.
25. Что доделать — полный список
Must-have ближайшие
Провести MapObject.z через save/load.
Добавить Z в editor selected object UI.
Рисовать 3D proxy на z.
KeypadAdd/KeypadSubtract для z.
Shift+P place-on-surface.
Save/reload проверка z.
Undo comparison включает z.
Prefab save/load включает z.
Потом
Rotation fields / schema plan.
Rotation editor UI.
Rotation preview becomes persisted rotation.
X/Y/Z transform gizmo.
Single input path for viewport.
Object Browser polish.
Cell View polish.
Reference Inspector.
Preview Object.
Proper snapping/magnets.
Floors/levels persistence.
Stairs/ramps planning.
Asset refs.
BA2/BSA local-only index.
Mesh/material preview.
Runtime support for vertical/rotation if needed.
26. Фокус-варианты
Focus A — самый правильный сейчас
Vertical Placement MVP

Почему:

нужен для предметов на столах/полках;
уже есть MapObject.z;
маленькая KISS-задача;
даёт реальный gameplay/editor value.
Focus B — после A
Rotation persistence

Почему:

без поворота предметов editor бесполезен;
нужно по осям как в Blender;
но сначала нужна data/schema clarity.
Focus C — после A+B
Mouse Transform Gizmo

Почему:

мышью удобно двигать;
но gizmo без сохранённого z/rotation снова будет фейком.
Focus D — потом
Object Browser / Cell View polish

Почему:

UI нужен;
но UI без transform workflow не двигает игру.
Focus E — позже
BA2/BWLD asset bridge

Почему:

важно;
но рано, пока basic world authoring не готов.
27. Следующий правильный prompt для Codex

Сначала убедиться, что нет старых модификаций:

cd D:\Projects\Game_Project
Get-Process BunkerEditor -ErrorAction SilentlyContinue | Stop-Process -Force
git status -sb

Если Editor/src/Editor_Main.cpp modified от старой попытки:

git restore -- Editor/src/Editor_Main.cpp
git status -sb

Потом дать Codex задачу:

Continue from clean baseline.

Task: vertical placement MVP using existing MapObject.z.

Do not add new z/elevation fields.
Do not add mouse gizmo.
Do not add UI shell.
Do not touch BA2/BSA.
Do not change rotation/scale.
Do not implement physics.

Inspect:
- include/MapObject.hpp
- src/World.cpp
- src/PrefabLibrary.cpp
- src/WorldExport.cpp
- src/WorldValidation.cpp
- Editor/src/Editor_Main.cpp

Implement:
- persist existing MapObject.z in world save/load
- backward compatibility: old files without z load as z=0
- persist z in prefab save/load if prefabs serialize MapObject
- selected object UI exposes Z
- undo comparison includes z
- 3D preview draws proxy at z and top at z+height
- KeypadAdd raises selectedObject.z
- KeypadSubtract lowers selectedObject.z
- Shift+P places selected object on highest overlapping support surface
- no physics, no mesh collision

Run:
git diff --check
cmake --build build_finish_msvc --config Debug
ctest --test-dir build_finish_msvc -C Debug --output-on-failure
launch BunkerEditor.exe

Stop for manual check.
Do not commit.
Do not push.
28. Абсолютные запреты для ИИ

ИИ не должен:

- делать большой UI-shell;
- снова делать Creation Kit clone;
- снова делать Blender clone целиком;
- добавлять 1000 строк без manual value;
- трогать BA2/BSA;
- сканировать D:\Projects\Ner;
- менять master;
- делать commit/push без approval;
- добавлять новые файлы без approval;
- менять BWLD format без отдельной задачи;
- добавлять rotation/scale вместе с z;
- переписывать renderer;
- трогать physics;
- ломать detached windows;
- ломать build/ctest;
- скрывать ошибки.
29. Финальная стратегия

Правильный порядок:

1. Vertical Placement MVP
2. Rotation Persistence MVP
3. Clean Transform Gizmo MVP
4. Object/Cell/Reference workflow polish
5. Floors/Levels system
6. Asset bridge / BA2 / BWLD refs
7. Real preview/rendering
8. DLC/mod authoring workflow

Не менять порядок без причины.

30. One-sentence project focus

Сейчас проекту нужен не новый красивый UI, а рабочий world authoring core:

selected object -> position XYZ -> save/load -> preview -> place on surfaces -> rotate -> snap -> then UI polish

Сейчас, после 98-часовой смены, я бы не начинал с большого редактора. Начни с **Vertical Placement MVP**, потому что это даст реальный прогресс: предметы наконец можно будет ставить на столы/полки/платформы и сохранять это.
::contentReference[oaicite:3]{index=3}
надо добавить раздел про merge strategy / совмещение веток, иначе ИИ может начать пушить/мерджить как попало.

Вставь в AI_PROJECT_BRIEF.md вот этот раздел, лучше после Git-правил.

## GitHub / Merge strategy

Главное правило:

```text
ИИ не делает merge сам.
ИИ не делает pull request сам.
ИИ не нажимает Merge на GitHub.
ИИ только готовит ветку, проверяет build/test и пишет отчёт.
Merge делает человек после ручной проверки.
Рабочая модель веток

Основная стабильная ветка:

master

Рабочие feature/milestone ветки:

Testing-imgui-viewports
ck-ng-bridge
vertical-placement-mvp
editor-transform-gizmo
...

Правило:

master = только проверенное стабильное состояние.
feature branch = разработка и эксперименты.
Когда можно делать merge в master

Merge разрешён только если выполнено всё:

Рабочая ветка чистая, кроме допустимого .codex/.
Нет незакоммиченных tracked изменений.
AI_FULL_LOG.md не staged и не committed, если отдельно не разрешено.
.codex/ не staged и не committed.
Build прошёл.
ctest прошёл.
Редактор запустился.
Ручная проверка пройдена.
Пользователь явно написал: approved for merge или аналогично.

Стандартная проверка перед merge:

git status -sb
git log --oneline --decorate -8
git diff --check
cmake --build build_finish_msvc --config Debug
ctest --test-dir build_finish_msvc -C Debug --output-on-failure
Предпочтительный способ merge

Предпочтительно через GitHub Pull Request:

feature branch -> master

GitHub Pull Request нужен, чтобы:

увидеть diff;
проверить, что в PR попали только нужные файлы;
не затащить .codex/;
не затащить AI_FULL_LOG.md;
иметь понятную историю.

GitHub Pull Request показывает изменения между topic/head branch и base branch, а merge PR можно делать разными способами: обычный merge commit, squash или rebase, в зависимости от настроек репозитория. Если есть conflicts, их надо решить до merge. GitHub также указывает, что draft PR нельзя merge-ить.

Какой merge method использовать

Для этого проекта по умолчанию:

Squash and merge

Почему:

история чище;
много мелких Codex-коммитов превращаются в один понятный milestone commit;
проще откатывать.

Исключение:
если ветка содержит несколько реально важных отдельных commits, можно выбрать обычный merge commit, но только с approval.

Названия PR / squash commit

Формат:

editor: add vertical placement MVP
editor: stabilize viewport transform gizmo
editor: enable imgui viewport windows
pipeline: add CK bridge specification

Не использовать:

fix
update
misc
codex changes
final
stuff
Что нельзя merge-ить

Нельзя merge-ить, если в PR есть:

.codex/
AI_FULL_LOG.md
imgui.ini
Log.md
temporary files
debug dumps
exports/
profiles/
world/ случайно изменённые данные
D:\Projects\Ner content
BA2/BSA extracted files

Если такие файлы появились в diff:

остановиться;
убрать из staging/commit;
не делать merge.
Что делать перед Pull Request

На feature branch:

git status -sb
git diff --check
cmake --build build_finish_msvc --config Debug
ctest --test-dir build_finish_msvc -C Debug --output-on-failure
git log --oneline --decorate -5

Потом push feature branch:

git push origin <feature-branch>

После этого открыть Pull Request на GitHub:

base: master
compare: <feature-branch>
Что проверить в GitHub PR

Перед merge проверить:

Files changed
Commits
PR title
PR description
No unwanted files
No AI_FULL_LOG.md unless explicitly approved
No .codex/
Only expected project files

Если GitHub показывает merge conflict:

не merge-ить;
решить conflict локально или через GitHub conflict editor только если conflict простой;
после conflict resolution снова build/test.

GitHub documentation notes that merge conflicts happen when branches have competing commits, and conflicts must be resolved before a pull request can be merged. Simple conflicts can sometimes be resolved on GitHub; complex conflicts require command-line or another Git client.

Local merge alternative

Если нужно merge-ить локально, делать только после approval.

Safe local flow:

git switch master
git pull origin master
git merge --no-ff <feature-branch>
cmake --build build_finish_msvc --config Debug
ctest --test-dir build_finish_msvc -C Debug --output-on-failure
git push origin master

Но по умолчанию лучше Pull Request.

После успешного merge

После merge в master:

git switch master
git pull origin master
git status -sb
git log --oneline --decorate -8

Feature branch можно удалить только после отдельного approval:

git branch -d <feature-branch>
git push origin --delete <feature-branch>

По умолчанию старые ветки не удалять сразу. Сначала убедиться, что master стабилен.

Merge stop conditions

Остановиться и не merge-ить, если:

build failed
ctest failed
manual check failed
unexpected files in diff
merge conflicts unclear
branch is wrong
master has moved and feature branch is outdated
AI_FULL_LOG.md staged accidentally
.codex/ staged accidentally
world/export/profile data changed accidentally
AI behavior around merge

ИИ может:

проверить статус;
подготовить commit на feature branch, если пользователь явно разрешил;
подготовить PR description text;
написать manual checklist;
сказать, какие файлы должны попасть в PR.

ИИ не может без явного approval:

merge в master;
delete branch;
force-push;
resolve conflicts guesses;
push master;
press GitHub Merge button;
скрывать conflicts.

Коротко: **feature branch → PR → squash merge в master только после build/ctest/manual approval**. GitHub официально рекомендует решать merge conflicts до merge PR, а если conflict простой — его можно решить через GitHub UI; если сложный, надо решать локально/через command line. :contentReference[oaicite:0]{index=0}
::contentReference[oaicite:1]{index=1}
---

# FINAL OVERRIDE / CURRENT CANON FOR AI

## Read this first

If anything above conflicts with this section, this section wins.

This file is `AllIn.md`.
It is the startup brief for AI/Codex sessions.

The AI must:
1. Read this file.
2. Follow `FINAL OVERRIDE / CURRENT CANON FOR AI`.
3. Work in small KISS steps.
4. Not build speculative editor systems.
5. Not commit/push/merge without explicit approval.

---

## Current project state

Project path:

```text
D:\Projects\Game_Project

Current working branch:

Testing-imgui-viewports

Current stable milestone:

8f0759b editor: add 3d viewport and planning shells

Known important stable commits:

8f0759b editor: add 3d viewport and planning shells
4eeb42f editor: enable imgui viewport windows
d18225b vendor: update imgui docking snapshot for viewports

.codex/ is local Codex/tool state.

Rules for .codex/:

Do not add it.
Do not delete it.
Do not clean it.
Leave it untouched.
Main engineering rule

Work like an old-school senior engineer.

Required principles:

KISS
YAGNI
Clean Code
Small patch
One task at a time
No speculative systems
No hidden side effects
No giant UI shell
No rewrite unless explicitly approved

The project must move forward through small working features, not through huge UI shells.

Bad:

Make it like Creation Kit + Blender + BA2 + BWLD + physics + full editor UI all at once.

Good:

Wire existing MapObject.z into save/load/editor UI/3D preview/Keypad +/-/Shift+P.
Absolute current priority

Current top priority:

Vertical Placement MVP using existing MapObject.z

Important fact:

float z = 0.0f;

already exists in MapObject.

Therefore:

Do not add another z field.
Do not add elevation as a duplicate.
Do not add verticalOffset as a duplicate.
Use existing MapObject.z.

The current task is NOT to invent vertical data.

The current task is to wire existing MapObject.z through the project.

Why this matters

The game/editor must support placing objects above the floor:

item on table
object on shelf
crate on platform
prop on bed
object on container roof
future floors
future ramps
future stairs
future double-height spaces

Without persisted z, objects only exist on a flat x/y plane.

That breaks real world authoring.

Coordinate model

Use this interpretation:

object.x      = horizontal X position
object.y      = horizontal Y/depth position
object.z      = vertical base/elevation
object.width  = size on X
object.depth  = size on Y/depth
object.height = vertical size

Object bottom:

object.z

Object top:

object.z + object.height

Object footprint:

x ± width / 2
y ± depth / 2
Current task: Vertical Placement MVP

Implement only this:

Inspect existing MapObject.z.
Reuse existing MapObject.z.
Persist z in world save/load.
Preserve backward compatibility:
old worlds without z load as z = 0.
Persist z in prefab save/load if prefabs serialize MapObject.
Add selected-object Z field in editor UI.
Include z in selected-object undo comparison if needed.
Draw 3D proxy boxes at object.z.
Draw object top at object.z + object.height.
Add KeypadAdd to raise selected object z.
Add KeypadSubtract to lower selected object z.
Add Shift+P place-on-surface:
find highest overlapping support object;
set selected object z = support.z + support.height;
if no support, set selected object z = 0.

Do not do anything else.

Do NOT do in this task

Do not implement:

mouse gizmo
rotation
scale
physics
falling simulation
floor/level persistence
BA2/BSA parser
D:\Projects\Ner scan
asset browser
Creation Kit UI shell
Blender clone
new windows
renderer rewrite
runtime renderer rewrite
large refactor

Do not touch:

master;
BA2/BSA files;
external/imgui;
CMake unless explicitly necessary;
.codex/;
unrelated source files.
Allowed files for current task

Allowed edits:

src/World.cpp
src/PrefabLibrary.cpp
src/WorldExport.cpp
src/WorldValidation.cpp
Editor/src/Editor_Main.cpp
include/MapObject.hpp only if absolutely needed, but do not add another z

Do not edit other files unless build proves a tiny mechanical update is required.

If a required edit affects a file outside this list, stop and report before editing.

Pre-check commands

Before any edit, run and append to AI_FULL_LOG.md:

git status -sb
git branch --show-current
git log --oneline --decorate -8
git --no-pager diff --stat

If branch is not:

Testing-imgui-viewports

stop.

If unexpected tracked files are modified, stop.

If only .codex/ is untracked, leave it untouched and continue.

Required inspections

Inspect before editing:

include/MapObject.hpp
src/World.cpp
src/PrefabLibrary.cpp
src/WorldExport.cpp
src/WorldValidation.cpp
Editor/src/Editor_Main.cpp

Find:

how MapObject is saved;
how MapObject is loaded;
whether old files can omit fields;
whether prefabs save/load MapObject;
whether export ignores z;
where selected object UI edits x/y/width/depth/height;
where undo comparison happens;
where 3D preview builds object proxy boxes.
Shift+P surface placement algorithm

Use this simple MVP algorithm.

For selected object:

for each candidate object:
    skip if candidate is selected

    if selected footprint overlaps candidate footprint:
        candidateTop = candidate.z + candidate.height
        choose highest candidateTop

if support found:
    selected.z = highest candidateTop
else:
    selected.z = 0

Footprint overlap:

selected.minX = selected.x - selected.width / 2
selected.maxX = selected.x + selected.width / 2
selected.minY = selected.y - selected.depth / 2
selected.maxY = selected.y + selected.depth / 2

candidate.minX = candidate.x - candidate.width / 2
candidate.maxX = candidate.x + candidate.width / 2
candidate.minY = candidate.y - candidate.depth / 2
candidate.maxY = candidate.y + candidate.depth / 2

overlap if:
selected.maxX >= candidate.minX
selected.minX <= candidate.maxX
selected.maxY >= candidate.minY
selected.minY <= candidate.maxY

No mesh collision.
No physics.
No slopes.
No ramps.
No normals.
No complex table geometry.

This is proxy-based placement only.

Build/test

Before build, close editor:

Get-Process BunkerEditor -ErrorAction SilentlyContinue | Stop-Process -Force

Then run:

git diff --check
cmake --build build_finish_msvc --config Debug
ctest --test-dir build_finish_msvc -C Debug --output-on-failure

If build fails:

stop;
report exact error;
do not commit.

If tests fail:

stop;
report exact failure;
do not commit.

If build/test pass, launch:

.\build_finish_msvc\Debug\BunkerEditor.exe

Do not claim manual GUI behavior was verified from shell.

Manual check must be marked pending until user verifies.

Manual checklist for current task

User must verify:

Old world loads.
Objects without saved z appear on floor.
Selected object has editable Z field.
KeypadAdd raises selected object.
KeypadSubtract lowers selected object.
Object visibly moves up/down in 3D preview.
Snap applies to Z movement.
Shift+P over support object places selected object on top.
Shift+P without support places object on floor.
Save/reload keeps z.
Render Window still opens.
Render Window still detaches by title bar.
Drag inside viewport does not move the whole window.
MMB orbit still works.
Wheel zoom still works.
Object Window / Cell View still open.
.codex/ untouched.
No BA2/BSA read/extract/index.
No scan of D:\Projects\Ner.
Git rules

AI must not run without explicit user approval:

git add
git commit
git push
git clean
git reset --hard
git merge
git rebase
git switch
branch deletion
force push

AI_FULL_LOG.md is append-only and should not be committed unless explicitly approved.

.codex/ must not be committed.

Never use:

git add .

If commit is later approved, add files explicitly.

GitHub merge rules

AI does not merge by itself.

Human does the merge.

Preferred flow:

feature branch -> GitHub Pull Request -> master

Default method:

Squash and merge

Merge to master only after:

build passed;
ctest passed;
editor launched;
manual check passed;
no unwanted files in diff;
user explicitly approved merge.

Do not merge if PR contains:

.codex/
AI_FULL_LOG.md
imgui.ini
Log.md
temporary files
debug dumps
exports/
profiles/
world/ accidental data changes
D:\Projects\Ner content
BA2/BSA extracted files
Final report required

At the end, report in Russian:

what was changed;
whether existing MapObject.z was reused;
how z is saved;
how z is loaded;
how old worlds stay compatible;
whether prefabs preserve z;
whether export/validation was updated or why not;
how keyboard vertical movement works;
how Shift+P works;
files touched;
git status -sb;
git diff --stat;
build result;
ctest result;
launch result;
manual check pending;
risks;
next smallest recommended task.

Do not commit.
Do not push.
Stop for approval.

Roadmap after current task

Do not start these now.

Recommended order:

1. Vertical Placement MVP
2. Rotation Persistence MVP
3. Clean Transform Gizmo MVP
4. Object/Cell/Reference workflow polish
5. Floors/Levels system
6. Asset bridge / BA2 / BWLD refs
7. Real mesh/textured preview
8. DLC/mod authoring workflow

Current one-sentence focus:

selected object -> position XYZ -> save/load -> preview -> place on surfaces -> rotate -> snap -> then UI polish

После этого в новом чате просто скажи:

```text
Read AllIn.md first. The FINAL OVERRIDE / CURRENT CANON FOR AI section overrides earlier conflicting text. Then execute
тогда в AllIn.md нужно добавить отдельный финальный раздел про объединение веток, потому что сейчас там уже есть правила “ИИ не merge-ит сам” и “feature branch → PR → master”, но надо жёстко зафиксировать: главная ветка — master, все полезные ветки нужно постепенно свести в неё через проверку, а не слить всё разом. 
Вставь в конец AllIn.md, после блока про .codex/, вот это:
---## Branch consolidation / master as main branchThe final main branch for the project is:```textmaster
All useful feature branches must eventually be reviewed and merged into master.
Important:
Do not merge all branches blindly.
Do not auto-merge old experiment branches.
Do not use git merge just because a branch exists.
Every branch must be inspected first.
Goal
The goal is to consolidate useful work into master while keeping history clean and avoiding broken editor experiments.
Branch policy
master = main stable branchfeature branches = temporary work branchesold experimental branches = review before merge or archive
Before merging any branch
For every branch, inspect:
git fetch origingit branch -agit status -sbgit log --oneline --decorate --graph --all -20
For each candidate branch, check:
git log --oneline master..<branch-name>git diff --stat master...<branch-name>git diff --name-only master...<branch-name>
Do not merge if the branch contains:


broken editor UI experiments;


accidental .codex/;


AI_FULL_LOG.md;


imgui.ini;


Log.md;


temporary files;


exported/generated files;


accidental world/profile data;


BA2/BSA/extracted assets;


changes unrelated to the branch purpose.


Preferred merge method
Preferred method:
GitHub Pull Request -> Squash and merge -> master
Reason:


cleaner master history;


one milestone commit per feature;


easier revert;


easier review.


Default PR direction:
base: mastercompare: feature branch
Merge order
Recommended order:
1. Stable infrastructure/editor commits2. ImGui viewport/detachable window work3. 3D viewport stable shell work4. Vertical Placement MVP5. Rotation persistence6. Transform gizmo7. Object/Cell/Reference workflow8. Asset bridge / BA2 / BWLD work
Do not merge later systems before their required base systems.
For example:


Do not merge transform gizmo before vertical placement is stable.


Do not merge BA2/BWLD work before asset bridge plan is approved.


Do not merge old UI-shell experiments unless they are manually reviewed and still wanted.


Required checks before merging to master
Before any branch can be merged into master, run:
git status -sbgit diff --checkcmake --build build_finish_msvc --config Debugctest --test-dir build_finish_msvc -C Debug --output-on-failure
For editor/UI branches, also launch:
.\build_finish_msvc\Debug\BunkerEditor.exe
Manual GUI check must pass before merge.
AI restrictions
AI must not:


merge into master;


push to master;


delete branches;


force push;


resolve merge conflicts by guessing;


include .codex/;


include AI_FULL_LOG.md;


include generated/exported files;


hide conflicts.


AI may:


inspect branches;


summarize what each branch contains;


recommend merge order;


prepare PR description text;


prepare a manual checklist;


tell the user which branches look safe or unsafe.


Human approval required
Merge into master is allowed only after the user explicitly says:
approved for merge
or gives equivalent explicit approval.
After successful merge
After a branch is merged into master:
git switch mastergit pull origin mastergit status -sbgit log --oneline --decorate -8
Do not delete the feature branch immediately unless explicitly approved.
Branch deletion requires separate approval:
git branch -d <branch-name>git push origin --delete <branch-name>
Current consolidation principle
Do not try to save every experiment.
Keep:


stable viewport/window work;


stable editor infrastructure;


vertical placement;


clean transform systems;


useful runtime/export/validation changes.


Discard or archive:


broken gizmo attempts;


giant UI-shell experiments that did not solve current gameplay/editor needs;


speculative BA2/BWLD work done before approval;


anything that breaks build/test/manual editor behavior.


Final rule
master must stay boring and stable.
Every merge into master must answer:
Does this make the game/editor more usable right now?Does it build?Does it pass tests?Was it manually checked?Can it be reverted cleanly?
И отдельно: когда будешь реально готов “сводить ветки”, не начинай с merge. Сначала дай Codex задачу **только на audit веток**:```textInspect all local and remote branches.Do not merge.Do not delete.Do not push.List each branch, latest commit, files changed versus master, and recommendation:- merge candidate- archive- ignore- needs manual review
Это спасёт от ситуации, где старый сломанный viewport/gizmo patch случайно попадёт в master.
