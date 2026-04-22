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

bool IssueLaunchTicket(const LaunchTicketInfo& ticketInfo) {
    std::ofstream out(LaunchTicketPath(), std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "issued_at=" << CurrentUnixSeconds() << '\n';
    out << "account_id=" << ticketInfo.accountId << '\n';
    out << "session_mode=" << ticketInfo.sessionMode << '\n';
    out << "character_name=" << ticketInfo.characterName << '\n';
    out << "selected_world=" << ticketInfo.selectedWorld << '\n';
    out << "lanline_session_id=" << ticketInfo.lanlineSessionId << '\n';
    out << "host_endpoint=" << ticketInfo.hostEndpoint << '\n';
    out << "bt72_seat_role=" << ticketInfo.bt72SeatRole << '\n';
    out << "bt72_second_seat_policy=" << ticketInfo.bt72SecondSeatPolicy << '\n';
    out << "bt72_trusted_gunner=" << ticketInfo.bt72TrustedGunnerHandle << '\n';
    return static_cast<bool>(out);
}

bool ConsumeLaunchTicket(LaunchTicketInfo& ticketInfo, std::string& failureReason) {
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
        } else if (key == "account_id") {
            ticketInfo.accountId = value;
        } else if (key == "session_mode") {
            ticketInfo.sessionMode = value;
        } else if (key == "character_name") {
            ticketInfo.characterName = value;
        } else if (key == "selected_world") {
            ticketInfo.selectedWorld = value;
        } else if (key == "lanline_session_id") {
            ticketInfo.lanlineSessionId = value;
        } else if (key == "host_endpoint") {
            ticketInfo.hostEndpoint = value;
        } else if (key == "bt72_seat_role") {
            ticketInfo.bt72SeatRole = value;
        } else if (key == "bt72_second_seat_policy") {
            ticketInfo.bt72SecondSeatPolicy = value;
        } else if (key == "bt72_trusted_gunner") {
            ticketInfo.bt72TrustedGunnerHandle = value;
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
