# src/World.cpp

Парный список к `include/World.hpp` из старого `Next.md`:

- реализовать поиск объекта по `scriptTag`
- реализовать поиск объекта по `linkTarget`
- сделать bool-helper-ы `HasScriptTag(...)` и `HasLinkTarget(...)`

Контекст из `Next.md`:

- мир уже хранит `scriptTag/linkTarget`
- `BWL2` уже используется
- значит следующий шаг не новый формат мира, а нормальный runtime/editor glue поверх существующей authored семантики
