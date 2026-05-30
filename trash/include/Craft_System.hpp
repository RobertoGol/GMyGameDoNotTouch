#pragma once
#include <map>
#include <vector>
#include "Avatar_State.hpp"
#include "Registry_ID.hpp"

namespace BunkerProtocol {

    struct CraftRecipe {
        std::string resultID; // #%it_...
        std::map<std::string, int> ingredients; // ID ресурса -> кол-во
        int requiredInt;      // Минимальный интеллект (Log Horizon Style)
        std::string requiredPerk; // Нужен ли спец. перк (Fallout style)
    };

    class CraftSystem {
    public:
        CraftSystem() = default;

        /**
         * РАЗБОР: Превращает предмет в базовые компоненты.
         */
        void ScrapItem(const std::string& itemID, AvatarState& avatar) {
            // Если ID содержит "scraps" или "junk", выдаем базовое железо и медь
            if (itemID.find("it_") != std::string::npos) {
                avatar.AddItem("#%it_iron_scrap", 2, 0.4f);
                std::cout << "[System] " << itemID << " scrapped successfully." << std::endl;
            }
        }

        /**
         * КРАФТ: Проверяет ресурсы, перки и интеллект.
         */
        bool Craft(const CraftRecipe& recipe, AvatarState& avatar) {
            // 1. Проверка интеллекта
            if (avatar.GetStatValue('I') < recipe.minIntelligence) return false;

            // 2. Проверка ингредиентов (в реальной реализации - цикл по map)
            // if (!avatar.HasResources(recipe.ingredients)) return false;

            // 3. Создание предмета
            avatar.AddItem(recipe.resultID, 1, 1.0f);
            return true;
        }
    };
}
