struct MapObject {
    char objectID;
    InteractionType interactType;
    float x, y, z;
    float health;
    
    // Новые поля для сундуков
    bool isManualLoot;      // true - лут из редактора, false - автозаполнение
    char manualLootIDs[5];  // Список конкретных ID предметов (до 5 штук)
};
