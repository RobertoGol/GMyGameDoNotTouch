# src/LanlineServices.cpp

Уже закрыто:

- `ServicesUnlockState` строится не из абстрактного “магического интернета”, а из `WorldFieldState` + operational checks recovery/backbone
- shared save/load bridge реально работает и покрыт smoke roundtrip
- launcher/runtime sync-ят `Lanline Services` state с `SessionProfile`, чтобы credits/cosmetics/pending orders не расходились по разным сохранениям
- support catalog остаётся dual-currency и anti-pay-to-win

Следующее:

- привязать support-order delivery к более явным runtime/logistics эффектам
- дожать service/fey summaries в launcher/runtime поверх уже существующего persistence/glue
