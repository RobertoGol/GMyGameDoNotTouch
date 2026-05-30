/**
 * Логика вскрытия сундука [#%it_ ]
 */
void WorldResources::OpenContainer(MapObject& container, AvatarState& player) {
    if (container.isManualLoot) {
        // ВАРИАНТ А: Лут из редактора (частный случай)
        for (int i = 0; i < 5; ++i) {
            if (container.manualLootIDs[i] != 0) {
                player.AddItem(std::to_string(container.manualLootIDs[i]), 1, 0.5f);
            }
        }
        std::cout << "[Loot] Manual loot set retrieved." << std::endl;
    } 
    else {
        // ВАРИАНТ Б: Автозаполнение (нормальный режим)
        // Генерируем строго от 3 до 5 предметов
        int itemCount = 3 + (rand() % 3); // Результат: 3, 4 или 5
        
        for (int i = 0; i < itemCount; ++i) {
            // Здесь вызываем твой рандомизатор предметов по тирам R/SR
            std::string randomItem = "#%it_junk_" + std::to_string(rand() % 100);
            player.AddItem(randomItem, 1, 0.2f);
        }
        std::cout << "[Loot] Auto-generated " << itemCount << " items." << std::endl;
    }
}
