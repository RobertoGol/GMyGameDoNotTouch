# Launcher/src/Launcher_Main.cpp

Что из `Next.md` относится сюда:

- исправить риск с `ImGui::Combo`, если список миров собирается через временные `world.string().c_str()`
- сделать запуск `BunkerGame.exe` и `BunkerEditor.exe` устойчивым к случайному `current_path`
- добавить проверки индексов для:
  - выбранного мира
  - выбранного персонажа
  - `knownLanlineSessions`
- обновлять список миров и LAN snapshot-ов не только один раз при старте
- подключить `LanlineServicesSave`:
  - загрузка при старте launcher
  - сохранение после изменений или на выходе
- реально рисовать панель `Lanline Services`, если состояние уже поднимается, но UI не вызывается

Связанные файлы:

- `include/LanlineServices.hpp`
- `src/LanlineServices.cpp`
- `include/SessionProfiles.hpp`
- `ROADMAP.md`
