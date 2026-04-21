# include/LanlineServices.hpp

Уже закрыто:

- формальный save/load bridge для `Lanline Services`: `LanlineServicesSave`, `MakeLanlineServicesStateFromSave(...)`, `BuildLanlineServicesSave(...)`, `DefaultLanlineServicesSavePath()`, `Save...`, `Load...`
- `SupportOrder` расширен до стабильного persistence-контракта: `orderId`, `itemId`, `itemLabel`, `destinationNode`, `state`, `paymentCurrency`, `createdAtUnix`, `etaUnix`
- dual-currency и анти-`pay-to-win` границы закреплены в shared catalog rules
- unlock-state теперь держит explicit `tower / relay / service / water / fey` flags поверх tier, а launcher/runtime умеют синхронизировать service snapshot обратно в `SessionProfile`
- shared order helpers теперь ведут lifecycle `Queued / Routed / Delivered / Claimed`, а runtime может claim-ить delivered parcels в `SessionProfile` без отдельной ad-hoc логики в UI

Следующее отсюда:

- расширять launcher/runtime summaries без превращения `Lanline Services` в интернет-браузер или полноценный online backend
- future hooks: richer logistics/service consequences поверх уже существующего order-delivery flow
