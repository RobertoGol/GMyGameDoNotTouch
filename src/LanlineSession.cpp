#include "../include/LanlineSession.hpp"

#include <ctime>
#include <fstream>
#include <sstream>

#include "../include/AppPaths.hpp"

namespace bunker {

namespace {

std::string CurrentTimestampString() {
    const std::time_t now = std::time(nullptr);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &now);
#else
    localTime = *std::localtime(&now);
#endif
    std::ostringstream out;
    out << (localTime.tm_year + 1900) << '-';
    if (localTime.tm_mon + 1 < 10) out << '0';
    out << (localTime.tm_mon + 1) << '-';
    if (localTime.tm_mday < 10) out << '0';
    out << localTime.tm_mday << ' ';
    if (localTime.tm_hour < 10) out << '0';
    out << localTime.tm_hour << ':';
    if (localTime.tm_min < 10) out << '0';
    out << localTime.tm_min << ':';
    if (localTime.tm_sec < 10) out << '0';
    out << localTime.tm_sec;
    return out.str();
}

}  // namespace

bool SaveLanlineSessionState(const LanlineSessionState& state) {
    return SaveLanlineSessionState(state, LanlineSessionPath());
}

bool SaveLanlineSessionState(const LanlineSessionState& state, const std::filesystem::path& path) {
    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }

    out << "session_id=" << state.sessionId << '\n';
    out << "mode=" << state.mode << '\n';
    out << "world=" << state.worldName << '\n';
    out << "host=" << state.hostEndpoint << '\n';
    out << "updated_at=" << (state.updatedAt.empty() ? CurrentTimestampString() : state.updatedAt) << '\n';
    for (const auto& player : state.players) {
        out << "player=" << player.displayName << "|" << player.role << "|" << (player.online ? 1 : 0) << '\n';
    }
    for (const auto& eventLine : state.eventLog) {
        out << "event=" << eventLine << '\n';
    }
    return true;
}

bool LoadLanlineSessionState(LanlineSessionState& state) {
    return LoadLanlineSessionState(LanlineSessionPath(), state);
}

bool LoadLanlineSessionState(const std::filesystem::path& path, LanlineSessionState& state) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    state = LanlineSessionState{};
    std::string line;
    while (std::getline(in, line)) {
        const auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, pos);
        const std::string value = line.substr(pos + 1);
        if (key == "session_id") {
            state.sessionId = value;
        } else if (key == "mode") {
            state.mode = value;
        } else if (key == "world") {
            state.worldName = value;
        } else if (key == "host") {
            state.hostEndpoint = value;
        } else if (key == "updated_at") {
            state.updatedAt = value;
        } else if (key == "player") {
            const auto first = value.find('|');
            const auto second = value.find('|', first == std::string::npos ? first : first + 1);
            if (first != std::string::npos && second != std::string::npos) {
                state.players.push_back({
                    value.substr(0, first),
                    value.substr(first + 1, second - first - 1),
                    value.substr(second + 1) != "0"});
            }
        } else if (key == "event") {
            state.eventLog.push_back(value);
        }
    }

    return true;
}

std::vector<LanlineSessionState> DiscoverLanlineSessionSnapshots() {
    std::vector<LanlineSessionState> sessions;
    const auto sessionsDir = LanlineSessionsDirectory();
    if (!std::filesystem::exists(sessionsDir)) {
        return sessions;
    }

    for (const auto& entry : std::filesystem::directory_iterator(sessionsDir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".state") {
            continue;
        }

        LanlineSessionState state;
        if (LoadLanlineSessionState(entry.path(), state)) {
            sessions.push_back(state);
        }
    }

    return sessions;
}

}  // namespace bunker
