#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "Registry_ID.hpp"
#include "OS_Isolation.hpp"

/**
 * PROJECT: Bunker Protocol
 * MODULE: World_Resources
 * DESCRIPTION: Загрузка .map файлов и менеджмент ресурсов (добыча, статика).
 */

namespace BunkerProtocol {

    // Типы взаимодействия с объектами
    enum class InteractionType {
        Static,      // Просто декорация
        Harvest,     // Можно добыть (руда, запчасти, дерево)
        Container,   // Можно обыскать (ящик, сейф)
        Transition   // Переход между локациями (лифт, дверь)
    };

    struct MapObject {
        char objectID[16];      // Регистрационный ID (#%it_..., [% ...)
        InteractionType interactType;
        float x, y, z;
        float health;           // "Прочность" узла добычи (если 0 - объект исчезает/ломается)
        int resourceYield;      // Сколько ресурсов дает за один цикл добычи
    };

    struct LocationData {
        std::string areaName;
        std::vector<MapObject> statics;   // Стены, окружение
        std::vector<MapObject> resources; // Месторождения, обломки для разбора
        std::vector<MapObject> portals;   // Точки перехода
    };

    class WorldResources {
    private:
        LocationData currentLevel;
        bool isLoaded = false;

    public:
        WorldResources() = default;

        /**
         * Метод добычи ресурса (вызывается при ударе/взаимодействии)
         * targetID - ID объекта из Registry
         */
        bool TryHarvest(const std::string& targetID, int toolPower) {
            for (auto& res : currentLevel.resources) {
                if (res.objectID == targetID && res.health > 0) {
                    res.health -= (float)toolPower; // Уменьшаем прочность объекта
                    
                    if (res.health <= 0) {
                        std::cout << "[World] Resource node depleted: " << targetID << std::endl;
                        // Здесь логика выдачи предмета в инвентарь
                    }
                    return true;
                }
            }
            return false;
        }

        /**
         * Загрузка бинарного файла локации
         */
        bool LoadMap(const std::string& mapName, const OSIsolation& sys) {
            std::string fullPath = sys.GetAssetsPath().string() + "/maps/" + mapName + ".map";
            if (!sys.IsPathSafe(fullPath)) return false;

            std::ifstream file(fullPath, std::ios::binary);
            if (!file.is_open()) return false;

            // Логика чтения заголовка и объектов...
            // (Разделяем объекты по типам: statics, resources, portals)
            
            isLoaded = true;
            return true;
        }

        const LocationData& GetCurrentLocation() const { return currentLevel; }
    };
}
