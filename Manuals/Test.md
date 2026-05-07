Сделай финальную ручную проверку UX для Loot / Links, без больших переделок.

Проверь в редакторе:

1. Открыть placed container -> Reference Properties -> Loot / Links.
2. При пустом lootEntries таблица показывает минимум 20 строк.
3. Выбрать пустую строку, например row 10.
4. Pick From Object Window Selection должен создать lootEntries до row 10 и заполнить именно row 10.
5. Drag/drop item на существующую заполненную строку должен заменить эту строку.
6. Drag/drop item на пустую virtual row, например row 20, должен расширить vector до этой строки и заполнить её.
7. Drag/drop в нижнюю область списка должен append в конец.
8. После append должна появиться следующая пустая строка.
9. Save/reload не должен сохранять пустые virtual rows.
10. После reload снова видно минимум 20 строк.
11. Remove Selected удаляет только реальную selected entry.
12. Clear Selected чистит только реальную selected entry.
13. Clear All очищает lootEntries и manualLootIds.
14. Undo/redo должен откатывать изменения lootEntries.

Также проверь actual source вокруг syncReferenceLootBuffers / lootDisplayRowCountForCurrentObject, чтобы не осталось странных старых веток clamp к реальному lootEntries.size().
Если всё нормально:
- git diff --check
- cmake --build build_finish_msvc --config Debug
- ctest --test-dir build_finish_msvc -C Debug --output-on-failure

Не коммитить.
Photo/, Test.md и временные файлы не трогать.