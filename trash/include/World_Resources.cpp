#include "../include/World_Resources.hpp"
#include <iostream>
#include <fstream>
#include <vector>

/**
 * PROJECT: Bunker Protocol
 * MODULE: World_Resources
 * DESCRIPTION: Реализация загрузки .wld файлов из папки /world/
 */

namespace BunkerProtocol {

    /**
     * Загрузка мира: Читает бинарные данные из папки /world/ в корне проекта.
     * Использует массовое чтение данных (data()) для оптимизации под слабые ПК.
     */
    bool WorldResources::LoadWorld(const std::string& worldName, const OSIsolation& sys) {
        // Формируем путь: корень/world/имя.wld
        std::string fullPath = sys.GetWorldPath().string() + "/" + worldName + ".wld";
        
        // Проверка безопасности через забор изоляции
        if (!sys.IsPathSafe(fullPath)) {
            std::cerr << "[Security] Denied access to world file: " << fullPath << std::endl;
            return false;
        }

        std::ifstream file(fullPath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[IO] Error: .wld file not found in /world/ folder." << std::endl;
            return false;
        }

        // 1. ПРОВЕРКА МАГИЧЕСКОГО ЧИСЛА (Magic Number)
        char magic[4];
        file.read(magic, 4);
        if (std::string(magic, 4) != "BWLD") {
            std::cerr << "[Format] Error: File is not a valid Bunker World (.wld)!" << std::endl;
            return false;
        }

        // 2. МЕТАДАННЫЕ ЛОКАЦИИ (Название для UI Overlay)
        char areaBuf[64];
        file.read(areaBuf, 64);
        currentLevel.areaName = areaBuf;

        // 3. СЛОЙ СТАТИКИ (Здания [% ], Стены, Окружение)
        uint32_t staticCount;
        file.read(reinterpret_cast<char*>(&staticCount), sizeof(staticCount));
        currentLevel.statics.resize(staticCount);
        file.read(reinterpret_cast<char*>(currentLevel.statics.data()), sizeof(MapObject) * staticCount);

        // 4. СЛОЙ ЛУТА И СУНДУКОВ (#%it_ )
        uint32_t lootCount;
        file.read(reinterpret_cast<char*>(&lootCount), sizeof(lootCount));
        currentLevel.resources.resize(lootCount);
        file.read(reinterpret_cast<char*>(currentLevel.resources.data()), sizeof(MapObject) * lootCount);

        // 5. СЛОЙ СУЩНОСТЕЙ (Спавн-точки мобов #%mid_ )
        uint32_t npcCount;
        file.read(reinterpret_cast<char*>(&npcCount), sizeof(npcCount));
        currentLevel.spawners.resize(npcCount);
        file.read(reinterpret_cast<char*>(currentLevel.spawners.data()), sizeof(MapObject) * npcCount);

        file.close();
        isLoaded = true;

        // Системный лог в стиле Log Horizon
        std::cout << "[System] World Initialized: " << currentLevel.areaName << std::endl;
        std::cout << "[System] Nodes: Statics[" << staticCount << "] Loot[" << lootCount << "] Entities[" << npcCount << "]" << std::endl;
        
        return true;
    }

    /**
     * Освобождение памяти при переходе между зонами
     */
    void WorldResources::UnloadWorld() {
        currentLevel.statics.clear();
        currentLevel.resources.clear();
        currentLevel.spawners.clear();
        isLoaded = false;
        std::cout << "[System] Memory cleared for next zone." << std::endl;
    }
}
