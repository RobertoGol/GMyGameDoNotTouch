# Launcher/src/Launcher_Main.cpp

Уже закрыто:

- world/session indexes и launcher refresh path больше не висят на старом одноразовом состоянии
- запуск sibling executables не зависит от случайного `current_path`
- `LanlineServicesSave` грузится/сохраняется из launcher
- launcher реально рисует shared `Lanline Services` panel и теперь синхронизирует её обратно в `SessionProfile/WorldFieldState`
- launcher main screen получил compact announcement widget в левом верхнем углу: local build notice, `Dismiss`, optional details и persisted read-state без сети и без modal popup

Следующее:

- launcher-side polish и richer vertical-slice summaries, а не базовый service glue
