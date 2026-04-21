# include/LanlineServices.hpp

Уже закрыто:

- формальный save/load bridge для `Lanline Services`: `LanlineServicesSave`, `MakeLanlineServicesStateFromSave(...)`, `BuildLanlineServicesSave(...)`, `DefaultLanlineServicesSavePath()`, `Save...`, `Load...`
- `SupportOrder` расширен до стабильного persistence-контракта: `orderId`, `itemId`, `itemLabel`, `destinationNode`, `state`, `paymentCurrency`, `createdAtUnix`, `etaUnix`
- dual-currency и анти-`pay-to-win` границы закреплены в shared catalog rules
- unlock-state теперь держит explicit `tower / relay / service / water / fey` flags поверх tier, а launcher/runtime умеют синхронизировать service snapshot обратно в `SessionProfile`

Следующее отсюда:

- довести service orders от UI/persistence до более явного runtime effect layer
- расширять launcher/runtime summaries без превращения `Lanline Services` в интернет-браузер или полноценный online backend
