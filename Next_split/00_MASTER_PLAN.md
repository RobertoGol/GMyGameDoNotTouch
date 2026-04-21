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

1. `prefab/library strengthening`
2. `import assistant hardening`
3. `export discipline / world-format tightening`
4. затем возврат к `Launcher / Lanline Services / runtime` хвостам по `Next_split/*`

## Правила

- не перепридумывать архитектуру и не делать большой рефакторинг ради красоты
- если пункт уже живет в коде, не держать его как основной todo в md
- после реального внедрения синхронизировать `ROADMAP.md` и сокращать дублирующие заметки
