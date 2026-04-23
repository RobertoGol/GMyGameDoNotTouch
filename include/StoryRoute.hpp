#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "SessionProfiles.hpp"
#include "StaticEraser.hpp"

namespace bunker {

struct StoryRouteEntry {
    std::string text;
    bool completed = false;
};

std::string CurrentStoryCheckpointLabel(const SessionProfile& profile);
std::string CurrentStoryObjectivePreview(const SessionProfile& profile);
std::string CurrentStoryObjectivePreview(const SessionProfile& profile, std::string_view worldReference);
std::string CurrentStoryObjective(const SessionProfile& profile, const StaticEraser& staticEraser);
std::string CurrentRecoveryHandoffSummary(const SessionProfile& profile);
std::string CurrentRecoveryHandoffSummary(const SessionProfile& profile, std::string_view worldReference);
std::string ActiveRouteEventSummary(const SessionProfile& profile);
std::string ActiveRouteEventSummary(const SessionProfile& profile, std::string_view worldReference);
std::vector<StoryRouteEntry> BuildBt72RestorationRoute(const SessionProfile& profile);
std::vector<StoryRouteEntry> BuildFirstPlayableRouteSlice(const SessionProfile& profile);
std::vector<StoryRouteEntry> BuildStarterRoute(const SessionProfile& profile, const StaticEraser& staticEraser);
bool HasLanlineServicesObjective(const SessionProfile& profile);
bool HasFeyRingIntercityObjective(const SessionProfile& profile);
bool HasFeyRingInterserverObjective(const SessionProfile& profile);

}  // namespace bunker
 
