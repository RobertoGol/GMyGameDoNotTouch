#include "../include/Progression.hpp"

#include <algorithm>

namespace bunker {

int ExperienceRequiredForLevel(int level) {
    const int safeLevel = std::max(1, level);
    return 100 + ((safeLevel - 1) * 75);
}

bool AwardExperience(SessionProfile& profile, int amount, std::string* eventText) {
    if (amount <= 0) {
        return false;
    }

    profile.character.experience += amount;
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

}  // namespace bunker
