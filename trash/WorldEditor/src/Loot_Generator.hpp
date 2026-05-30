#pragma once
#include <string>
#include <vector>
#include <random>
#include <iostream>
#include "Registry_ID.hpp"

/**
 * PROJECT: Bunker Protocol
 * MODULE: Loot_Generator
 * DESCRIPTION: Генерация предметов. Обычный лут (3-5 шт) vs Рейдовый (Legendary/Mythic).
 */

namespace BunkerProtocol {

    enum class LootTier {
        Common,     // Хлам, базовые ресурсы
        Uncommon,   // Полезные детали
        Rare,       // Редкие компоненты
        SR,         // Super Rare (Высший предел обычных сундуков)
        Legendary,  // ТОЛЬКО РЕЙДЫ
        Mythic      // ТОЛЬКО МАСШТАБНЫЕ РЕЙДЫ
    };

    class LootGenerator {
    public:
        /**
         * Генерация для обычного сундука (3-5 предметов).
         * Легендарки и Мифики здесь выпасть НЕ МОГУТ.
         */
        static std::vector<std::string> GenerateContainerLoot() {
            std::vector<std::string> lootList;
            
            // Строго от 3 до 5 предметов
            int count = 3 + (std::rand() % 3); 

            for (int i = 0; i < count; ++i) {
                int roll = std::rand() % 100;
                LootTier selectedTier;

                if (roll < 60)       selectedTier = LootTier::Common;   // 60%
                else if (roll < 85)  selectedTier = LootTier::Uncommon; // 25%
                else if (roll < 97)  selectedTier = LootTier::Rare;     // 12%
                else                 selectedTier = LootTier::SR;       // 3%

                lootList.push_back(GetRandomItemID(selectedTier));
            }

            return lootList;
        }

        /**
         * ГЕНЕРАЦИЯ ДЛЯ РЕЙДОВ.
         * Единственное место, где падают легендарные и мифические вещи.
         */
        static std::vector<std::string> GenerateRaidLoot(bool isMassiveRaid) {
            std::vector<std::string> lootList;
            int roll = std::rand() % 100;

            if (isMassiveRaid) {
                if (roll < 5)        lootList.push_back(GetRandomItemID(LootTier::Mythic));    // 5% шанс
                else if (roll < 25)  lootList.push_back(GetRandomItemID(LootTier::Legendary)); // 20% шанс
                else                 lootList.push_back(GetRandomItemID(LootTier::SR));
            } else {
                // Обычный рейд - максимум Легендарка с малым шансом
                if (roll < 10)       lootList.push_back(GetRandomItemID(LootTier::Legendary));
                else                 lootList.push_back(GetRandomItemID(LootTier::SR));
            }

            return lootList;
        }

    private:
        /**
         * Возвращает ID предмета в формате #%it_ из реестра
         */
        static std::string GetRandomItemID(LootTier tier) {
            std::string prefix = "#%it_";
            switch (tier) {
                case LootTier::Common:    return prefix + "std_" + std::to_string(100 + std::rand() % 99);
                case LootTier::Uncommon:  return prefix + "unc_" + std::to_string(200 + std::rand() % 99);
                case LootTier::Rare:      return prefix + "rare_" + std::to_string(300 + std::rand() % 99);
                case LootTier::SR:        return prefix + "sr_" + std::to_string(400 + std::rand() % 99);
                case LootTier::Legendary: return prefix + "LEG_" + std::to_string(777 + std::rand() % 10);
                case LootTier::Mythic:    return prefix + "MYTH_" + std::to_string(999);
                default:                  return prefix + "error";
            }
        }
    };
}
