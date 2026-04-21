# src/LanlineServices.cpp

Уже закрыто:

- `ServicesUnlockState` строится не из абстрактного “магического интернета”, а из `WorldFieldState` + operational checks recovery/backbone
- shared save/load bridge реально работает и покрыт smoke roundtrip
- launcher/runtime sync-ят `Lanline Services` state с `SessionProfile`, чтобы credits/cosmetics/pending orders не расходились по разным сохранениям
- support orders теперь имеют реальный runtime payoff: shared layer ведет `advance/count/claim`, а delivered parcels доходят до inventory/profile вместо вечного зависания в panel-state
- support catalog остаётся dual-currency и anti-pay-to-win

Следующее:

- дожать service/fey summaries в launcher/runtime поверх уже существующего persistence/glue
- future hooks: deeper logistics/system effects поверх уже работающего delivery flow
