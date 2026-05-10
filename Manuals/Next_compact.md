# Next

## 0. Правило этого файла

Этот файл — не архив и не dump старых заметок.

Он держит только:
- жесткий канон текущего прохода;
- активный канон первого играбельного маршрута;
- active gameplay/system priorities;
- краткий живой статус;
- следующий рабочий пакет;
- индекс `Next_split/*`.

Все длинные исторические разборы и архивные хвосты должны храниться вне этого файла.

---

## 1. Жесткий канон текущего прохода

### Пока НЕ делать
- карту;
- authored layout;
- level design;
- surface city layout;
- расстановку помещений, декора, укреплений и маршрутов руками;
- literal copying узнаваемых bunker/vault решений.

### Делать только
- системы вне карты;
- progression hooks;
- runtime / launcher / profile / save-load;
- BT-72;
- Camp / AIMP;
- мастерские как игровые системы;
- combat / RPG;
- recovery / industry / logistics;
- UI / validation / smoke-tests.

### Жесткие правила мира
- мир собирает разработчик в редакторе;
- игрок не редактирует карту как редактором;
- свободное строительство у игрока есть только внутри переносимого Camp/AIMP;
- BT-72, Camp/AIMP и мастерские — разные системы;
- мастерские — authored world nodes;
- BT-72 — центральная напарник-платформа;
- State of Decay 2 использовать только как пример того, как authored мир живет через игровые механики.

---

## 2. Start Route Canon — First Playable Route

### Общая последовательность
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

### Что считать каноном старта
- убежище не одноместное;
- есть жилой, служебный и инженерный слои;
- до Pip-Pad часть доступа идет через карты/жетоны/override;
- Pip-Pad заранее предвосхищен средой;
- BT-72 не выдается готовым;
- ангар — отдельный переходный слой;
- выход на поверхность происходит на BT-72, а не пешком;
- первое место выхода — старт surface loop;
- BT-72 поддерживает второе место и gunner role по разрешению основного пилота.

### Что реализовывать сейчас
- cryo start state;
- multi-occupancy shelter context;
- early melee + vermin support;
- access card / early clearance logic before Pip-Pad;
- Pip-Pad acquisition flow;
- blueprint / holo-record discovery flow;
- staged BT-72 restoration logic;
- core installation / activation;
- sync/link flow;
- internal hangar tutorial slice;
- bunker exit unlock logic;
- hangar gate / lift transition support;
- first surface arrival support;
- first heavy clearance support;
- first combat encounter support;
- first service/rest flow;
- tower / recovery payoff hooks;
- debrief / next hook flow;
- BT-72 second seat permission + gunner role support.

---

## 3. Legacy Reactive Tech Stack — Final

Это не отдельная технодемка.
Это слой качества поверх:
- start route;
- BT-72;
- first combat;
- first service/rest;
- recovery payoff.

### Обязательно
- honest AI perception без wallhack и без неадекватного hearing range;
- Awareness;
- Trigger Discipline;
- Pathfinding;
- Interactables;
- Shatterable Glass;
- Projectile Effects;
- Muzzle Fire / Gun Fire;
- Shock Wave;
- Animation Cycles;
- Foliage Interaction;
- Breakable light vegetation;
- Rain splatter / water ripples / limited reflections;
- role-separated enemy logic:
  - люди = тактика + self-preservation;
  - гули = rush melee;
  - роботы = паттерны и зоны контроля.

### Ограниченно
- modular mechanical damage;
- limited momentum damage for clearance;
- limited debris;
- limited light destruction;
- limited water response;
- limited mirrors/reflections;
- crowds only minimally.

### Не делать core-фичами базовой игры
- Stalker behavior;
- Levolution;
- Igniting Foliage;
- Fire Propagation;
- full fluid sim;
- total destruction sandbox;
- always-on friendly fire.

### Friendly fire
Допустим только как режим/настройка:
- PvE
- PvP
- PvW

---

## 4. Краткий живой статус

### Уже считаем закрытым
- основной editor/toolchain проход;
- export/history/validation spine;
- weak refs / XREF;
- layers / inspector / shared undo-redo / viewport hardening;
- prefab/library v1;
- import assistant;
- tightened export discipline;
- Lanline Services profile/runtime sync;
- launcher announcement widget;
- tank_service BT-72 payoff;
- перевод first playable route в отдельный persistence/objective/runtime слой.

### Не спорить с этим списком без прямой проверки кода.

---

## 5. Следующий рабочий пакет

1. `Launcher / Lanline Services / runtime return`
2. `start vertical slice polish`
3. `BT-72 / combat / RPG depth`
4. `recovery / industry / logistics` как более плотный mid-game backbone

Главная цель:
довести базовую игру до законченного состояния,
а не открывать новые параллельные ветки.

---

## 6. Как работать с этим файлом

Для каждого пункта:
- если реализовано полностью — удалить;
- если реализовано частично — доделать и удалить;
- если pending — внедрить;
- если устарело — переписать или убрать;
- если есть блокер — записать в `Use_this_One.md`.

Не оставлять в `Next.md` длинные исторические хвосты.

---

## 7. Индекс Next_split

- `Next_split/README.md`
- `Next_split/00_MASTER_PLAN.md`

### Launcher
- `Next_split/Launcher/src/Launcher_Main.cpp.md`

### Services
- `Next_split/include/SessionProfiles.hpp.md`
- `Next_split/include/LanlineServices.hpp.md`
- `Next_split/src/LanlineServices.cpp.md`

### Runtime / World
- `Next_split/include/GameRuntime.hpp.md`
- `Next_split/src/GameRuntime.cpp.md`
- `Next_split/include/World.hpp.md`
- `Next_split/src/World.cpp.md`

### Story / Progression
- `Next_split/include/Progression.hpp.md`
- `Next_split/src/Progression.cpp.md`
- `Next_split/include/StoryRoute.hpp.md`
- `Next_split/src/StoryRoute.cpp.md`

### Tank / Editor
- `Next_split/include/HangarSystem.hpp.md`
- `Next_split/Editor/src/Editor_Main.cpp.md`

### Docs
- `Next_split/ROADMAP.tasks.md`
- `Next_split/Use_this_One.tasks.md`
- `Next_split/trash.tasks.md`

---

## 8. Финальное правило

`Next.md` держит только:
- канон;
- активный маршрут;
- активные задачи;
- краткий статус;
- следующий пакет;
- индекс split-файлов.

Архив старых заметок в этот файл больше не возвращать.
