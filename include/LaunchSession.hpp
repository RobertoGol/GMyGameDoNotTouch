#pragma once

#include <string>

namespace bunker {

bool IssueLaunchTicket(const std::string& accountId, const std::string& sessionMode);
bool ConsumeLaunchTicket(std::string& failureReason);

}  // namespace bunker
