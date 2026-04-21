Ниже — чистый Editor_TZ.md, где файл с разбором Creation Kit используется только как дополнение и инженерный референс, а не как источник для копирования чужой архитектуры. В файле, который ты дал, реально полезны идеи про UID/реестр, property grid, viewport/gizmo, validation/warnings, world serialization и слоистую организацию редактора.

Editor_TZ.md
1. Роль редактора

BunkerEditor — это отдельное приложение в составе проекта, полноценный authoring toolset для мира, объектов, интерактивных сущностей, service/fey/industrial узлов и экспортного world pipeline.

Редактор не является:

копией Creation Kit;
универсальным редактором “для всего”;
внешним моддинг-софтом;
браузером ассетов из чужих игр;
quest/dialogue IDE на текущем этапе.

Редактор является:

основным инструментом сборки игровых миров;
инструментом authoring для объектов, контейнеров, спавнов, сервисных узлов, маршрутов и связей;
источником экспортируемого runtime-мира;
частью канонической архитектуры Launcher / Game / Editor.
2. Как использовать анализ Creation Kit

Анализ Creation Kit из Fallout 4 используется только как дополнительный инженерный референс по ролям модулей редактора:

реестр объектов и ID;
property inspector;
viewport interaction;
validation/warnings;
сериализация мира;
asset/provider layer;
object window / cell view / layer logic.

Мы не копируем:

MFC/WinAPI-монолит;
Papyrus VM;
точный ESM/ESP pipeline;
Fallout-specific форматы как production-цель;
чужой код, названия классов и внутренние структуры.
3. Основные цели BunkerEditor
3.1. Авторинг мира

Редактор должен позволять:

создавать и открывать миры;
сохранять и экспортировать миры в runtime-формат;
размещать объекты;
редактировать spawn/markers;
ставить service/fey/industrial anchors;
работать с prefab/library системой;
валидировать мир перед экспортом.
3.2. Авторинг объектов

Редактор должен позволять:

создавать объект;
дублировать объект;
удалять объект;
редактировать свойства объекта;
применять semantic preset;
связывать объект с другим объектом через ID или semantic target;
быстро искать и фильтровать объекты.
3.3. Экспорт в runtime

Редактор должен экспортировать мир так, чтобы:

runtime читал его без ручных доправок;
все ID и ссылки были валидны;
broken links ловились до экспорта;
world metadata, layers и object table были консистентны.
4. Что добавляем в редактор
4.1. Registry ID / Object Registry
Цель

Каждый объект редактора должен иметь уникальный стабильный Registry ID.

Что нужно
генератор новых ID;
хранение Registry ID в world data;
проверка на дубли;
remap ID при дублировании объекта;
remap временных ID в постоянные при сохранении/экспорте;
поиск объекта по ID;
ссылки между объектами через ID, а не через прямые ссылки.
Почему

Из файла с CK самый полезный слой — это идея центрального ID/реестра и weak references. Это реально нужно редактору, чтобы не было хаоса в связях и удалениях.

4.2. Weak References и Cross-Reference Map
Цель

Редактор должен знать:

кто на кого ссылается;
что сломается при удалении объекта;
какие связи у объекта есть входящие и исходящие.
Что нужно
targetRegistryId как основная модель ссылок;
карта XREF:
incoming references
outgoing references
блок References в инспекторе объекта;
предупреждение при удалении объекта с активными ссылками;
jump-to-reference.
Примеры
lanline_service_hub ссылается на tower_sync;
fey_ring ссылается на route target;
prefab placement ссылается на prefab base;
terminal ссылается на service node.
4.3. Unified Property Inspector
Цель

Вместо множества разрозненных полей нужен единый инспектор выбранного объекта.

Обязательные секции
Transform
Gameplay
Interaction
Semantic
Loot
Visual
Runtime Notes
Специализированные секции

Для отдельных semantic типов:

lanline_service_hub
fey_ring
tank_service
medical_support
relay_substation
service_bay
water_reclaimer
Поведение

Инспектор должен:

адаптироваться к типу объекта;
скрывать нерелевантные поля;
показывать warnings прямо внутри инспектора;
давать быстрые переходы к связанным объектам.

Из файла с CK берем именно идею property/editor sheet, а не старую UI-реализацию.

4.4. Viewport / World Preview
Цель

Viewport должен быть реальным рабочим authoring-пространством, а не просто картинкой.

Уже есть в текущем репозитории
выбор объекта кликом;
raycast-like selection в preview;
move drag по выбранному объекту;
grid-step snap;
focus on selected;
camera pan / zoom;
bounds gizmos для `width/depth`;
interaction radius overlay;
service radius overlay;
route/XREF visualization;
drag player spawn.

Дожать дальше
rotate gizmo, если появится явный orientation field в world data;
scale/shape authoring beyond current `width/depth` handles;
дальнейшую полировку hybrid viewport.
Для service/fey объектов

Показывать:

link lines;
route direction;
service radius;
anchor relation;
invalid link markers.
4.5. Layer Manager
Цель

Редактор должен уметь работать со сложными мирами через слои.

Слои

Минимальный набор:

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
Возможности
show/hide layer;
lock/unlock layer;
filter by layer;
assign selected objects to layer;
mass select by layer.

Из CK-анализа берем сам принцип layer/visibility filtering.

4.6. Undo / Redo
Цель

Любое значимое действие должно откатываться.

Уже есть в текущем репозитории
shared undo stack по дельтам/командам;
coalescing повторных object/world edits;
undo/redo для add/remove/update/world metadata/batch semantic actions;
dirty-state относительно последнего load/export.

Минимальный стек команд
add object
remove object
duplicate object
move
rotate
scale
property edit
semantic preset apply
spawn move
layer assign
link change
Требование

Undo/redo должен работать по дельтам/командам, а не полным пересохранением всего мира.

4.7. Warnings / Validation Window
Цель

Редактор должен ловить проблемы до экспорта.

Обязательные проверки
duplicate Registry ID
broken targetRegistryId
broken linkTarget
fey_ring без route target
lanline_service_hub без tower_sync
tank_service без service context
medical_support без service/backbone context
invalid spawn position
orphaned prefab placement
export metadata mismatch
missing semantic anchor
Категории
Error
Warning
Info
Что важно

Warnings должны:

вести к объекту;
показывать, что именно не так;
по возможности предлагать quick fix.

Из CK-файла это один из самых полезных модулей.

4.8. Object Window / Palette / Search
Цель

Дать быстрый доступ к объектам, prefab’ам и semantic presets.

Функции
поиск по имени;
поиск по Registry ID;
поиск по scriptTag;
поиск по linkTarget;
поиск по категории;
поиск по слою;
поиск по semantic type;
recent objects;
favorites.
Представления
Object Window
Palette
Search Results
Prefab Library
4.9. Prefab / Library System
Цель

Повторное использование набора объектов как prefab.

Нужно
создать prefab из выбранных объектов;
сохранить prefab metadata;
предпросмотр prefab;
категории prefab;
re-place prefab в мир;
отслеживать prefab usage;
предупреждать о поломке prefab links.
Будущее

Подготовить базу для:

prefab overrides;
prefab sync/update;
prefab dependency warnings.
4.10. Import Assistant
Цель

Маленькое окно импорта референсов в редакторный pipeline.

Что делает
принимает картинку или референс;
спрашивает тип:
prop
item
structure
environment
scene module
создает draft object/prefab;
отправляет в palette/library;
не создает “готовый мир магией”.
Что важно

Import Assistant — это помощник authoring’а, а не full AI world generator.

5. Формат мира
5.1. Основной world format

Используем наш world format (BWLD/BWL2) как основной редакторный и экспортный формат.

В заголовке мира должны быть
format version;
world metadata;
next available registry id;
asset manifest;
layer table;
object table;
semantic link table;
prefab references.
Требования
format versioning;
forward-safe структура;
export validation;
возможность дальнейшего layered override.

Из CK-анализа берем не ESM/ESP, а сам принцип versioned structured world format.

5.2. Override-ready architecture

Не делаем сейчас полный Bethesda-style plugin stack, но готовим структуру под:

base world;
authoring override;
temporary session override;
mission layer.
6. Связь редактора с runtime
6.1. Semantic anchors

Редактор должен authorить и экспортировать:

tower_sync
lanline_service_hub
fey_ring
tank_service
medical_support
relay_substation
service_bay
water_reclaimer
Требование

Runtime должен видеть именно эти authored semantics, а не жить на отдельной ручной логике.

6.2. Export discipline

Перед экспортом редактор должен:

валидировать мир;
строить summary;
указывать ошибки и блокеры;
не экспортировать разрушенный world state без предупреждения.
7. Что берем из анализа Creation Kit, а что нет
Берем
object registry / UID idea
weak references
cross-reference map
property inspector
viewport gizmos / raycast / snap
warnings / validation
virtualized object lists
versioned world serialization
asset/provider layer как архитектурную идею
Не берем сейчас
MFC/WinAPI монолит
Papyrus VM
полный ESM/ESP clone
Fallout-specific asset compatibility как production-приоритет
full quest/dialogue/AI editor
все, что уводит редактор в “универсальный Fallout-tool clone”
8. Порядок внедрения
Этап 1
Registry ID
weak refs
XREF
Этап 2
Warnings / Validation
Object Inspector
Этап 3
Undo / Redo
Layer Manager
Статус: закрыто в текущем коде
Этап 4
Gizmo / Raycast / Snap / Viewport overlays
Статус: рабочий `v1` уже есть; дальше только hardening
Этап 5
Prefab / Library
Object Window / Palette
Статус: следующий активный пакет
Этап 6
Import Assistant
Статус: следующий после prefab/library
Этап 7
Export discipline
World format tightening
Override-ready structure
Этап 8
Full semantic glue with runtime
9. Definition of Done для Editor v1+

Редактор считается дожатым до следующего серьезного checkpoint, когда:

у всех объектов есть Registry ID;
ссылки идут через ID и валидируются;
есть warnings/validation;
есть undo/redo;
есть unified inspector;
есть layer manager;
viewport позволяет реально authorить мир;
prefab/library usable;
import assistant usable;
export в BWLD/BWL2 стабилен;
service/fey/industrial semantics authorятся и читаются runtime.
10. Короткая формула

BunkerEditor развивается как современный редактор мира и объектов для Bunker Protocol, который берет из анализа Creation Kit только полезные инженерные функции редактора, но остается редактором нашего проекта, а не клоном Bethesda-toolchain.
