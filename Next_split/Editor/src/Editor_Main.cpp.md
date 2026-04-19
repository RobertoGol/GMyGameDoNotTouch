# Editor/src/Editor_Main.cpp

Из `Next.md` сюда относятся service/fey presets и authoring validation:

Новые пресеты:

- `lanline_service_hub`
- `fey_ring`
- `medical_support`
- `tank_service`

Ожидаемый смысл:

- authoring этих узлов прямо через editor
- заполнение `scriptTag`
- заполнение `linkTarget`
- корректные `interaction/category`

Дополнительно:

- проверить валидацию `scriptTag/linkTarget`
- не переделывать editor целиком, а просто расширить уже существующий preset-flow
