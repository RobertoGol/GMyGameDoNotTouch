# include/LanlineServices.hpp

Основные задачи из `Next.md`:

- ввести или дотянуть save-модель:
  - `LanlineServicesSave`
  - `MakeLanlineServicesStateFromSave(...)`
  - `BuildLanlineServicesSave(...)`
  - `DefaultLanlineServicesSavePath()`
  - `SaveLanlineServicesSave(...)`
  - `LoadLanlineServicesSave(...)`
- расширить `SupportOrder`:
  - `orderId`
  - `itemId`
  - `itemLabel`
  - `destinationNode`
  - `state`
  - `createdAtUnix`
  - `etaUnix`
  - `paymentCurrency`
- сохранить жесткую границу: никаких weapon unlocks, готовых танков и `pay-to-win` через support
- держать dual-currency модель:
  - `InGame`
  - `SymbolicSupport`

Смысл этого файла в старом `Next.md`: сделать `Lanline Services` не просто UI-панелью, а формальным контрактом для launcher/runtime/persistence.
