# Lanline Boundary

`Lanline` в текущем проекте фиксируется как `LAN-first session shell`, а не как полноценный сетевой gameplay-stack.

Что входит в границу `Lanline` сейчас:

- launcher/runtime session bootstrap через `launch ticket`
- session snapshot/state mirror между `BunkerLauncher` и `BunkerGame`
- lobby/seats/ready-state/presence
- relay chat mirror
- voice activity/settings/push-to-talk presence без raw audio transport
- service-layer UI для support, cosmetics, tank service и medical flows
- world/profile glue, который отражает `Lanline`-состояние в persistence

Что сознательно не обещается как часть `Lanline` сейчас:

- real-time combat replication
- authoritative multiplayer simulation
- rollback / lag compensation
- distributed inventory/game-state ownership
- raw voice/audio streaming
- dedicated online backend beyond local/session snapshot model

Практическое правило для следующих проходов:

- если фича касается launcher/runtime/session/presence/service-shell, она может жить в `Lanline`
- если фича требует сетевой синхронизации боёвки, физики, AI или полноценного co-op state authority, это уже отдельный multiplayer stack, а не `Lanline`

Техническое следствие:

- `Lanline` можно дальше чистить и расширять как продуктовый UX/session слой
- код не должен маскировать под `Lanline` то, что на самом деле является будущей сетевой игрой
