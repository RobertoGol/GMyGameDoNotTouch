# src/StoryRoute.cpp

Что повторялось в `Next.md`:

- реализовать objective helper-ы по текущему `selectedWorld`
- брать данные из `profile.worldFieldStates`
- использовать флаги:
  - `towerSyncRecovered`
  - `feyRingIntercityUnlocked`
  - `feyRingInterserverUnlocked`

Смысл: story/objective flow должен читать уже существующий world progression, а не дублировать его.
