# src/StoryRoute.cpp

Что повторялось в `Next.md`:

- реализовать objective helper-ы по текущему `selectedWorld`
- выделить checkpoint/objective preview для launcher
- держать checklist `BT-72 restore -> clearance -> first service -> recovery node -> debrief`
- брать данные из `profile.worldFieldStates`
- использовать флаги:
  - `towerSyncRecovered`
  - `feyRingIntercityUnlocked`
  - `feyRingInterserverUnlocked`

Смысл: story/objective flow должен читать уже существующий world progression и persisted first playable route, а не дублировать его по разным UI.
