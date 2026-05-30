#pragma once
#include <string>
#include <vector>
#include <map>
#include "Registry_ID.hpp"

/**
 * PROJECT: Bunker Protocol
 * MODULE: Avatar_State
 * DESCRIPTION: Состояние персонажа (@XXXXXXX): статы S.P.E.C.I.A.L., HP/MP и инвентарь.
 */

namespace BunkerProtocol {

    struct SpecialStats {
        int strength = 5;
        int perception = 5;
        int endurance = 5;
        int charisma = 5;
        int intelligence = 5;
        int agility = 5;
        int luck = 5;
    };

    struct InventoryItem {
        std::string itemID; // #%it_XXXXXXX
        int count;
        float weight;
    };

    class AvatarState {
    private:
        std::string characterID; // @XXXXXXX
        
        // Основные показатели
        float hp, maxHp;
        float mp, maxMp; // Мана / Энергия для скиллов
        int level = 1;
        
        SpecialStats stats;
        
        // Инвентарь и переносимый вес
        std::vector<InventoryItem> inventory;
        float currentWeight = 0.0f;

    public:
        AvatarState(const std::string& id) : characterID(id) {
            // Начальные статы
            maxHp = 100.0f + (stats.endurance * 10.0f);
            hp = maxHp;
            maxMp = 50.0f + (stats.intelligence * 15.0f);
            mp = maxMp;
        }

        /**
         * Добавление ресурсов (добыча из World_Resources)
         */
        void AddItem(const std::string& id, int count, float unitWeight) {
            inventory.push_back({ id, count, unitWeight });
            currentWeight += (unitWeight * count);
            
            // Проверка перегруза (Fallout style)
            float maxWeight = stats.strength * 10.0f;
            if (currentWeight > maxWeight) {
                // Здесь будет триггер на замедление скорости в Camera_Avatar или Input_Handler
            }
        }

        /**
         * Применение урона или затрат маны
         */
        void ModifyHP(float delta) { hp = std::clamp(hp + delta, 0.0f, maxHp); }
        void ModifyMP(float delta) { mp = std::clamp(mp + delta, 0.0f, maxMp); }

        // Геттеры для UI
        float GetHPPercent() const { return (hp / maxHp) * 100.0f; }
        float GetMPPercent() const { return (mp / maxMp) * 100.0f; }
        const SpecialStats& GetSpecial() const { return stats; }
        float GetCurrentWeight() const { return currentWeight; }
    };
}
