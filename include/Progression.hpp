#pragma once

#include <string>

#include "SessionProfiles.hpp"

namespace bunker {

int ExperienceRequiredForLevel(int level);
bool AwardExperience(SessionProfile& profile, int amount, std::string* eventText = nullptr);

}  // namespace bunker
