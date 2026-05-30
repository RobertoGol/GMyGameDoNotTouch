#pragma once
#include <string>
#include <vector>
#include <map>
#include "Avatar_State.hpp"

namespace BunkerProtocol {

    // Система рангов в стиле гача/JRPG
    enum class SkillRarity {
        R,    // Rare (Базовые)
        SR,   // Super Rare (Улучшенные)
        SSR,  // Specially Super Rare (Редкие техники)
        UR    // Ultra Rare (Уникальные авторские навыки)
    };

    struct SkillData {
        std::string name;
        SkillRarity rarity = SkillRarity::R;
        float cooldown;
        float currentCD = 0;
        float manaCost;
        float powerMultiplier; // Зависит от ранга
        
        bool isCustom = false; // Создан ли игроком вручную
    };

    class SkillBase {
    private:
        std::vector<SkillData> learnedSkills;
        
        // Счетчик повторений действий для создания новых скиллов
        // Ключ - тип действия, Значение - сколько раз совершено
        std::map<std::string, int> actionTracker;

    public:
        SkillBase() = default;

        /**
         * Метод отслеживания действий игрока.
         * Если действие "Jump_Attack" повторено 1000 раз - рождается новый скилл.
         */
        void RecordAction(const std::string& actionID) {
            actionTracker[actionID]++;
            
            if (actionTracker[actionID] == 1000) { 
                CreateUniqueSkill(actionID);
            }
        }

        /**
         * Генерация авторского скилла (Log Horizon Style)
         */
        void CreateUniqueSkill(const std::string& basedOn) {
            SkillData uniqueSkill;
            uniqueSkill.name = "Custom_" + basedOn; // Игрок потом сможет переименовать
            uniqueSkill.rarity = SkillRarity::SSR;   // Авторские техники всегда высокие по рангу
            uniqueSkill.cooldown = 15.0f;
            uniqueSkill.manaCost = 40.0f;
            uniqueSkill.isCustom = true;
            
            learnedSkills.push_back(uniqueSkill);
            std::cout << "[System] New SSR Skill Created through persistence: " << uniqueSkill.name << std::endl;
        }

        bool CastSkill(int index, AvatarState& caster) {
            if (index >= learnedSkills.size()) return false;
            auto& s = learnedSkills[index];

            if (s.currentCD <= 0 && caster.GetMPPercent() > 10.0f) {
                // Логика ранга: SSR и UR дают бонус от Удачи (Luck) и Интеллекта
                float boost = (s.rarity >= SkillRarity::SSR) ? (caster.GetSpecial().luck * 0.1f) : 1.0f;
                
                s.currentCD = s.cooldown;
                caster.ModifyMP(-s.manaCost);
                return true;
            }
            return false;
        }

        void Update(float dt) {
            for (auto& s : learnedSkills) {
                if (s.currentCD > 0) s.currentCD -= dt;
            }
        }
    };
}
