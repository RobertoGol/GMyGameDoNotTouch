#include "../include/Progression.hpp"
#include "../include/RpgSystem.hpp" // Каноничный слой формул Fallout 4

#include <algorithm>

namespace bunker {

// 1. Расчет порога уровня (Использует каноничную формулу 75 + level * 25)
int ExperienceRequiredForLevel(int level) {
    return FalloutStyleExperienceRequiredForLevel(level);
}

// 2. Начисление опыта с учетом Интеллекта SPECIAL (+3% за каждое очко)
bool AwardExperience(SessionProfile& profile, int amount, std::string* eventText) {
    if (amount <= 0) {
        return false;
    }

    // Твой ИИ красиво применил бонус, не ломая внутренний цикл!
    profile.character.experience += ApplyIntelligenceExperienceBonus(profile, amount);
    bool leveledUp = false;

    while (profile.character.experience >= ExperienceRequiredForLevel(profile.character.level)) {
        profile.character.experience -= ExperienceRequiredForLevel(profile.character.level);
        profile.character.level += 1;
        profile.character.unusedPoints += 2;
        profile.character.maxHp += 8.0f;
        profile.character.maxMp += 5.0f;
        profile.character.hp = profile.character.maxHp;
        profile.character.mp = profile.character.maxMp;
        leveledUp = true;
    }

    if (eventText != nullptr) {
        if (leveledUp) {
            *eventText = "Level up achieved. Recovery stats refreshed and attribute points awarded.";
        } else {
            *eventText = "Experience gained.";
        }
    }

    return leveledUp;
}

// 3. СЛЕДУЮЩИЙ КОД БЫЛ СРЕЗАН, НО МЫ ЕГО ВОЗВРАЩАЕМ (Валюта LANLINE сети)
int CurrentRelayCredits(const SessionProfile& profile) {
    return std::max(0, profile.lanlineServices.relayCredits);
}

bool AwardRelayCredits(SessionProfile& profile, int amount, std::string* eventText) {
    if (amount <= 0) {
        return false;
    }
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    profile.lanlineServices.relayCredits += amount;
    if (worldState != nullptr) {
        worldState->relayCreditsEarned += amount;
    }
    if (eventText != nullptr) {
        *eventText = "Relay credits banked.";
    }
    return true;
}

bool SpendRelayCredits(SessionProfile& profile, int amount, std::string* eventText) {
    if (amount <= 0) {
        return false;
    }
    if (profile.lanlineServices.relayCredits < amount) {
        if (eventText != nullptr) {
            *eventText = "Not enough relay credits.";
        }
        return false;
    }
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    profile.lanlineServices.relayCredits -= amount;
    if (worldState != nullptr) {
        worldState->relayCreditsSpent += amount;
    }
    if (eventText != nullptr) {
        *eventText = "Relay credits spent.";
    }
    return true;
}

}  // namespace bunker
