# include/World.hpp

Из `Next.md` сюда относятся helper-методы для authored semantics:

- `FindObjectByScriptTag(...) const`
- `FindObjectByScriptTag(...)`
- `FindObjectByLinkTarget(...) const`
- `FindObjectByLinkTarget(...)`
- `HasScriptTag(...)`
- `HasLinkTarget(...)`

Задача этих helper-ов: дать runtime и editor простой доступ к `scriptTag/linkTarget`, не копируя поисковую логику по коду.
