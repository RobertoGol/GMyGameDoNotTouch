# include/HangarSystem.hpp

Из `Next.md` сюда вынесен один конкретный gameplay hook:

- `ConsumeTankServiceKit(...)`

Ожидаемое поведение:

- списать нужный service item из инвентаря
- применить ремонт к нужной подсистеме танка
- вернуть понятный текст результата

Смысл этого шага: связать `Lanline Services` и `tank_service` не только через UI, но через реальный gameplay effect.
