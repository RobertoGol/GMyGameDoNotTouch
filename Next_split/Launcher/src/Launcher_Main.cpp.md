# Launcher/src/Launcher_Main.cpp

Уже закрыто:

- world/session indexes и launcher refresh path больше не висят на старом одноразовом состоянии
- запуск sibling executables не зависит от случайного `current_path`
- `LanlineServicesSave` грузится/сохраняется из launcher
- launcher реально рисует shared `Lanline Services` panel и теперь синхронизирует её обратно в `SessionProfile/WorldFieldState`
- launcher main screen получил compact announcement widget в левом верхнем углу: local build notice, `Dismiss`, optional details и persisted read-state без сети и без modal popup
- launch path теперь carry-ит `BT-72` second-seat state: launcher snapshot, join-target copy и `LaunchTicketInfo` держат `seat role / second seat policy / trusted gunner`, а lobby summary показывает seat assignment

Следующее:

- launcher-side polish вокруг richer vertical-slice summaries и surface-exit route cues, а не базовый service glue
