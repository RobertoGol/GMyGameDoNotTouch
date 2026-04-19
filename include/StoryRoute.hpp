#pragma once

#include <string>
#include <vector>

#include "SessionProfiles.hpp"
#include "StaticEraser.hpp"

namespace bunker {

struct StoryRouteEntry {
    std::string text;
    bool completed = false;
};

std::string CurrentStoryObjective(const SessionProfile& profile, const StaticEraser& staticEraser);
std::vector<StoryRouteEntry> BuildStarterRoute(const SessionProfile& profile, const StaticEraser& staticEraser);
bool HasLanlineServicesObjective(const SessionProfile& profile);
bool HasFeyRingIntercityObjective(const SessionProfile& profile);
bool HasFeyRingInterserverObjective(const SessionProfile& profile);

}  // namespace bunker
