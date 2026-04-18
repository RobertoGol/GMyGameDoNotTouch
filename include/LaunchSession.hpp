#pragma once

#include <string>

namespace bunker {

struct LaunchTicketInfo {
    std::string accountId;
    std::string sessionMode;
    std::string characterName;
    std::string selectedWorld;
    std::string lanlineSessionId;
    std::string hostEndpoint;
};

bool IssueLaunchTicket(const LaunchTicketInfo& ticketInfo);
bool ConsumeLaunchTicket(LaunchTicketInfo& ticketInfo, std::string& failureReason);

}  // namespace bunker
