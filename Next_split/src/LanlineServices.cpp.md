# src/LanlineServices.cpp

Из `Next.md` по этому файлу:

## Unlock logic

- увод unlock-логики от абстрактного `regional grid` в сторону `WorldFieldState`
- строить `ServicesUnlockState` от первой вышки и сервисного backbone:
  - `towerSyncRecovered`
  - `relaySubstationActive`
  - `serviceBayActive`
  - `waterReclaimerActive`
  - `feyRingIntercityUnlocked`
  - `feyRingInterserverUnlocked`

## Save/load bridge

- `MakeLanlineServicesStateFromSave(...)`
- `BuildLanlineServicesSave(...)`
- реальный serialize/deserialize в файл состояния

## UI and catalog

- разводить support-поток по понятным вкладкам
- не продавать оружие, готовые танки и прямые боевые преимущества
- держать адекватный default catalog

Связанные места:

- `include/LanlineServices.hpp`
- `Launcher/src/Launcher_Main.cpp`
- `src/GameRuntime.cpp`
