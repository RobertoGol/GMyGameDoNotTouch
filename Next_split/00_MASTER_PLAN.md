# Master Plan

Короткая выжимка из `Next.md`.

## Контекст

- Проект уже не пустой прототип, а поздний vertical slice / ранняя системная `v1`.
- Базовая форма проекта считается канонической: `BunkerLauncher`, `BunkerGame`, `BunkerEditor`.
- `Lanline - optime` считается каноническим названием LAN-слоя.
- Вопросы к автору держать в `Use_this_One.md`.
- Идеи автора и дальний backlog держать в `trash.md`.

## Ближайший приоритет

1. Добить стабильность `BunkerLauncher`.
2. Закрыть launcher до честного `v1`.
3. После кода синхронизировать `ROADMAP.md`.
4. Затем двигать `Lanline Services` и сервисный runtime-layer.

## Уже зафиксировано как база

- запуск игры через launcher
- рабочий editor
- ранний recovery backbone
- `Relay Substation`
- `Service Bay`
- `Water Reclaimer`
- ранний `Lanline` session-state / launcher-runtime glue

## Правила из старого Next

- не делать крупный распил файлов без явного желания
- не уходить в абстрактный рефакторинг
- новый слой закрывать как минимум через runtime + editor + docs
- не держать рабочие патчи только в заметках, если они уже реально внедрены
