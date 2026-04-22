# include/SessionProfiles.hpp

Из `Next.md` сюда вынесены идеи по persistence и world glue:

- `LanlineServicesProfile`
  - `relayCredits`
  - `ownedCosmetics`
  - `pendingSupportOrders`
  - `serviceHubKnown`
  - `cosmeticsShopSeen`
- `WorldFieldState`
  - сервисные и world flags для `towerSyncRecovered`, `localRelayAvailable`, `regionalGridOnline`
  - recovery flags для `relaySubstationActive`, `serviceBayActive`, `waterReclaimerActive`
  - `feyRingIntercityUnlocked`, `feyRingInterserverUnlocked`
  - счетчики сервисных циклов и кредитов
- `SessionProfile`
  - `worldFieldStates`
  - `selectedWorld`
  - `sessionMode`
  - `lanlineServices`
  - launcher announcement read-state
  - `fieldCheckpointKnown`, `fieldCheckpointX`, `fieldCheckpointY`
  - `FirstPlayableRouteProgress` для persisted первого маршрута: intro/clues/early vermin/BT-72 restore/clearance/service/recovery/debrief
- helper-нормализация:
  - `NormalizeLanlineServicesProfile`
  - `NormalizeWorldFieldState`

Смысл этого блока: держать сервисный прогресс, launcher read-state и world-state не в UI, а в профиле сессии.
