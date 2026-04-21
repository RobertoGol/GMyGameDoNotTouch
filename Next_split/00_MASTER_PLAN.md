# Master Plan

Короткая актуальная выжимка из `Next.md`.

## Контекст

- канон проекта остается прежним: `BunkerLauncher`, `BunkerGame`, `BunkerEditor`
- `solo + LAN first`, `Lanline - optime`, `Lanline Services`, `Fey Ring Network`
- сначала код и smoke-checks, потом чистка `Next.md` / `ROADMAP.md`

## Уже закрыто в текущем editor-hardening проходе

- `export/history path`
- `Registry ID / weak refs / XREF`
- `warnings / validation hardening`
- `unified property inspector`
- `layer manager`
- `undo/redo`
- `viewport authoring`: grid-step snap, bounds gizmos, spawn drag, XREF/service overlays

## Следующий рабочий пакет

1. `start vertical slice polish`
2. `BT-72 / combat / RPG depth`
3. затем углубление `recovery / industry / logistics`

## Только что закрыто

- `prefab/library strengthening`
- `import assistant hardening`
- `export discipline / world-format tightening`
- `Launcher / Lanline Services / runtime return`

## Правила

- не перепридумывать архитектуру и не делать большой рефакторинг ради красоты
- если пункт уже живет в коде, не держать его как основной todo в md
- после реального внедрения синхронизировать `ROADMAP.md` и сокращать дублирующие заметки
