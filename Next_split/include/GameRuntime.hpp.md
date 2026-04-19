# include/GameRuntime.hpp

По `Next.md` сюда предлагались небольшие поля состояния:

- `lanlineServicesVisible`
- `feyRingScheduleVisible`
- `supportTerminalNearby`
- `tankServiceNearby`
- `medicalSupportNearby`
- `supportRefreshTimer`
- `feyRingRefreshTimer`
- `lastSupportAction`
- `lastPortalAction`

Логика старая не требует распила `GameRuntime.hpp`, только аккуратного добавления минимального service-state.
