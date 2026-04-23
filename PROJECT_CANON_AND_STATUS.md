# PROJECT_CANON_AND_STATUS.md

## 0. Роль этого файла

Этот файл объединяет смысл и полезное содержимое из:
- `ROADMAP.md`
- `MASTER_PROMPT.md`
- `KOROTKOE_TZ_CLEAN.md`
- `HUMAN_READING_NOTES_CLEAN.md`
- `SOURCE_EXTRACTION_REPORT.md`
- `Editor_TZ_short.md`

Его задача — держать в одном месте:
- канон проекта;
- текущий статус;
- архитектурные правила;
- происхождение идей и допустимые источники;
- роль редактора;
- краткий активный фокус.

Это **не** архив старых заметок и не backlog.  
Рабочие активные задачи остаются в `Next.md` и `Next_split/*`.

---

## 1. Проект

### Название
**Bunker Protocol**

### Общая форма игры
- solo + LAN first
- launcher — обязательная точка входа
- editor — отдельный production tool
- game runtime — отдельное приложение

### Основные pillars
- BT-72
- recovery
- heavy-tech
- industry
- logistics
- persistence
- authored world
- bunker-to-surface progression
- service / support loops
- world states instead of freeform player editing

### Базовая цель
Довести проект до **первой цельной играбельной версии**, где:
- есть интро и пробуждение;
- есть bunker-start;
- есть ранний progression через доступы, Pip-Pad и технические следы;
- есть восстановление BT-72;
- есть sync/link;
- есть выход через ангар и выезд на поверхность;
- есть первый бой, первый сервис и первый recovery payoff;
- есть дальнейший хук в recovery / industry / logistics loop.

---

## 2. Архитектура проекта

### Приложения
- `BunkerLauncher`
- `BunkerGame`
- `BunkerEditor`
- `BunkerSmokeChecks`

### Жесткие правила
- `Launcher`, `Game`, `Editor` остаются отдельными приложениями.
- `Launcher` обязателен как пользовательская точка входа.
- `Editor` не является runtime-зависимостью игрока и может не входить в пользовательскую сборку.
- Новые функции сначала встраиваются в код и тесты, а не в новые `.md`.

### Роль launcher
Launcher отвечает за:
- вход;
- выбор мира / профиля / персонажа;
- системные уведомления;
- старт игрового runtime;
- shell-layer для `Lanline - optime`, `Lanline Services`, `Fey Ring Network`.

### Роль runtime
Runtime отвечает за:
- actual gameplay;
- progression;
- BT-72;
- combat / RPG;
- recovery / industry / logistics;
- world state transitions;
- authored-world interaction.

### Роль editor
Editor отвечает за:
- authored world creation;
- object placement;
- semantic authoring;
- prefab/library workflow;
- export;
- validation;
- preview;
- content authoring for the developer, not the player.

---

## 3. Что считать каноном мира

### Authored world
Карту и authored мир собирает разработчик в редакторе.

Игрок:
- не редактирует карту как редактором;
- не рисует карту;
- не меняет authored геометрию свободно;
- взаимодействует с authored миром только через игровые механики и world states.

### Camp / AIMP
Отдельная игровая система:
- переносимая;
- разворачиваемая;
- позволяет свободное строительство только внутри своего радиуса;
- не заменяет editor;
- не равна мастерским;
- не равна BT-72.

### Мастерские
Это authored world nodes:
- их можно найти;
- зачистить;
- захватить;
- восстановить;
- использовать;
- включить в recovery / industry / service loop.

### BT-72
Отдельная центральная система:
- не Camp;
- не мастерская;
- титаноподобная напарник-платформа;
- боевая и инженерная платформа;
- часть progression;
- часть service/modification loop;
- часть recovery backbone.

---

## 4. Сетевой и сервисный слой

### Канон
- `solo + LAN first`
- `Lanline - optime`
- `Lanline Services`
- `Fey Ring Network`

### Границы
Это не fully authoritative online MMO stack.  
Это:
- session shell;
- launcher/runtime glue;
- service/support UI layer;
- world/profile/service consistency.

---

## 5. Экономика и ограничения

### За игровые деньги
- ресурсы
- repair kits
- medical
- service items
- recovery-support items

### За реальные деньги
- только cosmetics / symbolic support

### Нельзя
- оружие за реальные деньги
- готовые танки
- боевые преимущества
- pay-to-win

---

## 6. Источники канона и что из них брать

### `Новая папка`
Полезно брать:
- образ игры;
- UX мира;
- раннее мышление редактора;
- account / character separation;
- camera/avatar logic;
- chat/log importance;
- world/resource layering.

Не переносить как код бездумно.

### `Project_M`
Главный зрелый источник для:
- runtime behavior;
- LAN contour;
- техника;
- persistence;
- world logic;
- BT-72 / tank thinking;
- weather;
- world-mode / interact loop;
- data-card style systems.

### `Aegis-9300`
Полезно как:
- launcher mood;
- terminal feel;
- dry system UI references.

### `void-project`
Полезно только как:
- tonal reference для системного языка;
- не как архитектурная база.

---

## 7. Роль редактора

### Что редактор уже должен значить
Editor — это не toy map tool, а production authoring tool.

Его смысл:
- world authoring;
- semantic authoring;
- object authoring;
- prefab/library reuse;
- validation;
- export;
- developer-facing content pipeline.

### Что он не должен быть
- player-facing world editor;
- generic engine IDE;
- direct clone of Creation Kit.

### Что из анализа Creation Kit допустимо брать
По смыслу:
- registry / typed IDs;
- weak refs / XREF;
- property inspector;
- layer logic;
- object windows / palette;
- gizmos / raycast / snap;
- validation / warnings;
- structured world serialization;
- asset/provider layer.

Что нельзя брать буквально:
- MFC shell;
- Papyrus VM;
- ESM/ESP exact pipeline;
- Fallout-specific production core;
- прямое копирование узнаваемых решений.

---

## 8. Текущий статус проекта

### Статус этапов
- `Этап 1. Стабилизация основы` — завершен
- `Этап 2. Каркас архитектуры` — в работе
- `Этап 3. Вертикальный срез старта` — начат
- `Этап 4. Базовый боевой и RPG-слой` — в работе
- `Launcher v1` — завершен

### Что уже считать сильно продвинутым
- editor/toolchain spine;
- export / validation / history;
- semantic contracts;
- weak refs / XREF;
- layer manager / inspector / viewport hardening;
- prefab/library v1;
- import assistant;
- tightened export discipline;
- Lanline Services profile/runtime sync;
- launcher announcement widget;
- BT-72 service-kit payoff;
- first playable route как отдельный persistence/objective/runtime layer;
- access-card gated start route и `BT-72` second-seat / gunner permission flow.

### Что теперь главное
Главная незакрытая работа уже не в инструментах, а в самой игре:
1. start vertical slice polish
2. BT-72 / combat / RPG depth
3. recovery / industry / logistics как плотный mid-game backbone

---

## 9. Первый играбельный маршрут — только как system sequence

Это **не** карта.  
Это **не** layout.  
Это системный канон первого маршрута.

### Последовательность
интро  
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

### Что реализовывать для этого
- hangar tutorial support;
- bunker exit unlock logic;
- lift/gate transition support;
- first surface arrival hooks;

---

## 10. Legacy Reactive Tech Stack — практический смысл

Это не отдельная технодемка.  
Это слой качества поверх:
- start route;
- BT-72;
- first combat;
- first service/rest;
- recovery payoff.

### Обязательно
- honest AI perception without wallhack;
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

### Не делать core-фичами
- Stalker behavior
- Levolution
- Igniting Foliage
- Fire Propagation
- full fluid sim
- total destruction sandbox
- always-on friendly fire

### Friendly fire
Допустим только как режим/настройка:
- PvE
- PvP
- PvW

---

## 11. Как использовать backlog

`trash.md` — это backlog идей.
Правила:
- учитывать идеи как backlog;
- если идея реально внедрена, задокументировать это в рабочих файлах состояния;
- удалить идею из `trash.md`, чтобы backlog отражал только невнедренное.

---

## 12. Что делать дальше

Активный рабочий буфер — `Next.md` и `Next_split/*`.

Этот файл не должен снова превращаться в todo dump.

### Следующий пакет
1. Launcher / Lanline Services / runtime return
2. start vertical slice polish
3. BT-72 / combat / RPG depth
4. recovery / industry / logistics

---

## 13. Финальное правило

Этот файл — единый:
- канон проекта;
- краткий статус;
- происхождение идей;
- роль редактора;
- роль старых веток;
- роль первого маршрута;
- общий вектор до базовой завершенной игры.

Рабочие задачи — не сюда, а в:
- `Next.md`
- `Next_split/*`
- `Use_this_One.md`
