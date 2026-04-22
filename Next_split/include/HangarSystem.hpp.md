# include/HangarSystem.hpp

Уже закрыто:

- `ConsumeTankServiceKit(...)` больше не является плоским generic repair-hook: `track / servo / engine / lens` kits теперь дают разные BT-72 effects и разные result texts
- shared `TryConsumeBestTankServiceKit(...)` выбирает лучший доступный kit под текущее состояние танка, так что authored `tank_service` не тратит неподходящий набор
- smoke-check покрывает engine/suspension/sensor/turret service flow, missing-kit fallback и no-damage guidance

Следующее отсюда:

- держать service kit catalog и BT-72 subsystem effects синхронно, если будут добавляться новые типы сервисных наборов
