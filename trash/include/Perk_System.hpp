#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include "Avatar_State.hpp"
#include "Skill_Base.hpp"

namespace BunkerProtocol {

    struct PerkCard {
        std::string name;
        char specialStat; // 'S','P','E','C','I','A','L'
        int cost;         // Сколько очков занимает
        SkillRarity rarity;
        bool isEquipped = false;
        std::string description;
    };

    class PerkSystem {
    private:
        std::vector<PerkCard> playerDeck;

    public:
        PerkSystem() = default;

        /**
         * Проверка: влезет ли карточка в текущий лимит S.P.E.C.I.A.L.
         */
        bool CanEquip(const PerkCard& card, const AvatarState& avatar) {
            int used = 0;
            for (const auto& p : playerDeck) {
                if (p.isEquipped && p.specialStat == card.specialStat) used += p.cost;
            }
            // Проверяем, не превышает ли сумма очков значение из Avatar_State
            return (used + card.cost <= avatar.GetStatValue(card.specialStat));
        }

        /**
         * Экипировка/Снятие карточки (Fallout 76 Style)
         */
        void TogglePerk(const std::string& perkName, AvatarState& avatar) {
            auto it = std::find_if(playerDeck.begin(), playerDeck.end(), 
                      [&](const PerkCard& p) { return p.name == perkName; });

            if (it != playerDeck.end()) {
                if (it->isEquipped) it->isEquipped = false;
                else if (CanEquip(*it, avatar)) it->isEquipped = true;
            }
        }

        // Добавление новой SSR/UR карты, созданной через действия (Log Horizon Style)
        void AddCreatedPerk(PerkCard newPerk) {
            playerDeck.push_back(newPerk);
        }
    };
}
