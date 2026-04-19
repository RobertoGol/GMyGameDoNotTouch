# src/GameRuntime.cpp

Из `Next.md`:

- добавить helper `IsNearTaggedObject(...)`
- через `World::FindObjectByScriptTag(...)` проверять proximity к:
  - `lanline_service_hub`
  - `tank_service`
  - `medical_support`
- обновлять флаги близости в runtime
- сделать `Lanline Services` реальной gameplay UI, а не только launcher-shell

Смысл: service-layer должен существовать и внутри игры, а не только в launcher.
