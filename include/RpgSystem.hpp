#pragma once

#include <algorithm>

#include "SessionProfiles.hpp"

namespace bunker {

inline int FalloutStyleExperienceRequiredForLevel(int level) {
    return 75 + (std::max(1, level) * 25);
}

inline int ApplyIntelligenceExperienceBonus(const SessionProfile& profile, int baseExperience) {
    const int intelligence = std::max(1, profile.character.special.intelligence);
    const float multiplier = 1.0f + static_cast<float>(intelligence - 1) * 0.03f;
    return std::max(0, static_cast<int>(static_cast<float>(baseExperience) * multiplier));
}

}  // namespace bunker
