#pragma once

#include <string>

#include "SessionProfiles.hpp"

namespace bunker {

bool HasEquippedPassiveSkill(const SessionProfile& profile, const std::string& skillId);
void RegisterArchiveSync(SessionProfile& profile, std::string* eventText = nullptr);
void RegisterFootKill(SessionProfile& profile, std::string* eventText = nullptr);
void RegisterTankAction(SessionProfile& profile, std::string* eventText = nullptr);
void RegisterStressSurvival(SessionProfile& profile, std::string* eventText = nullptr);
void RegisterHeavyCarryDrill(SessionProfile& profile, std::string* eventText = nullptr);
void RegisterFieldServiceUse(SessionProfile& profile, std::string* eventText = nullptr);

}  // namespace bunker
