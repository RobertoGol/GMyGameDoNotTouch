#include "../include/World_Resources.hpp"
#include <iostream>
#include <vector>

namespace BunkerProtocol {

    /**
     * Загрузка .wld файла: Карта + Лут-лист + Мобы
     */
    bool WorldResources::LoadWorld(const std::string& worldName, const OSIsolation& sys) {
        std::string fullPath = sys.GetAssetsPath().string() + "/worlds/" + worldName + ".wld";
        
        if (!sys.IsPathSafe(fullPath)) return false;

        std::ifstream file(fullPath, std::ios::binary);
        if (!file.is_open()) return false;

        // 1. HEADER (Заголовок)
        // [4 байта: "BWLD"] [4 байта: версия]
        char magic[4];
        file.read(magic, 4);
        if (std::string(magic, 4) != "BWLD") return false;

        // 2. META DATA (Название локации)
        char areaBuf[64];
        file.read(areaBuf, 64);
        currentLevel.areaName = areaBuf;

        // 3. LAYER: STATIC (Геометрия, здания [% ])
        uint32_t staticCount;
        file.read(reinterpret_cast<char*>(&staticCount), sizeof(staticCount));
        currentLevel.statics.resize(staticCount);
        file.read(reinterpret_cast<char*>(currentLevel.statics.data()), sizeof(MapObject) * staticCount);

        // 4. LAYER: LOOT & CONTAINERS (Сундуки, ресурсы [#%it_ ])
        // Здесь хранятся ящики и точки добычи ресурсов
        uint32_t lootCount;
        file.read(reinterpret_cast<char*>(&lootCount), sizeof(lootCount));
        currentLevel.resources.resize(lootCount);
        file.read(reinterpret_cast<char*>(currentLevel.resources.data()), sizeof(MapObject) * lootCount);

        // 5. LAYER: ENTITIES (Мобы, NPC, Боссы #%mid_ )
        // Только точки спавна и типы врагов
        uint32_t npcCount;
        file.read(reinterpret_cast<char*>(&npcCount), sizeof(npcCount));
        currentLevel.spawners.resize(npcCount);
        file.read(reinterpret_cast<char*>(currentLevel.spawners.data()), sizeof(MapObject) * npcCount);

        file.close();
        
        // Системный лог (Log Horizon Style)
        std::cout << "[System] World Layer Initialized: " << currentLevel.areaName << std::endl;
        std::cout << "[System] Entities loaded: " << npcCount << " | Loot nodes: " << lootCount << std::endl;
        
        return true;
    }
}
