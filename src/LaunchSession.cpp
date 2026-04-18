#include "../include/LaunchSession.hpp"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>

#include "../include/AppPaths.hpp"

namespace bunker {

namespace {

long long CurrentUnixSeconds() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

}  // namespace

bool IssueLaunchTicket(const std::string& accountId, const std::string& sessionMode) {
    std::ofstream out(LaunchTicketPath(), std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "issued_at=" << CurrentUnixSeconds() << '\n';
    out << "account_id=" << accountId << '\n';
    out << "session_mode=" << sessionMode << '\n';
    return static_cast<bool>(out);
}

bool ConsumeLaunchTicket(std::string& failureReason) {
    const auto ticketPath = LaunchTicketPath();
    std::ifstream in(ticketPath);
    if (!in.is_open()) {
        failureReason = "Launcher ticket not found. Start the game from BunkerLauncher.";
        return false;
    }

    long long issuedAt = 0;
    std::string line;
    while (std::getline(in, line)) {
        const auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, pos);
        const std::string value = line.substr(pos + 1);
        if (key == "issued_at") {
            issuedAt = std::atoll(value.c_str());
        }
    }
    in.close();
    std::remove(ticketPath.string().c_str());

    if (issuedAt == 0) {
        failureReason = "Launcher ticket is invalid. Relaunch through BunkerLauncher.";
        return false;
    }

    const long long now = CurrentUnixSeconds();
    if ((now - issuedAt) > 60) {
        failureReason = "Launcher ticket expired. Relaunch through BunkerLauncher.";
        return false;
    }

    return true;
}

}  // namespace bunker
