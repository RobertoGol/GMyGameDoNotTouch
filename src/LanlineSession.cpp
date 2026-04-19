#include "../include/LanlineSession.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string_view>

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

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

bool TryParseTimestamp(std::string_view timestamp, std::tm& outTime) {
    if (timestamp.size() != 19) {
        return false;
    }
    outTime = {};
    try {
        outTime.tm_year = std::stoi(std::string(timestamp.substr(0, 4))) - 1900;
        outTime.tm_mon = std::stoi(std::string(timestamp.substr(5, 2))) - 1;
        outTime.tm_mday = std::stoi(std::string(timestamp.substr(8, 2)));
        outTime.tm_hour = std::stoi(std::string(timestamp.substr(11, 2)));
        outTime.tm_min = std::stoi(std::string(timestamp.substr(14, 2)));
        outTime.tm_sec = std::stoi(std::string(timestamp.substr(17, 2)));
        outTime.tm_isdst = -1;
        return true;
    } catch (...) {
        return false;
    }
}

bool IsFreshTimestamp(std::string_view timestamp, int maxAgeSeconds) {
    std::tm parsedTime{};
    if (!TryParseTimestamp(timestamp, parsedTime)) {
        return false;
    }
    const std::time_t sessionTime = std::mktime(&parsedTime);
    if (sessionTime <= 0) {
        return false;
    }
    const std::time_t now = std::time(nullptr);
    return now >= sessionTime && (now - sessionTime) <= maxAgeSeconds;
}

void TrimRelayMessages(LanlineSessionState& state, std::size_t maxMessages) {
    if (state.relayMessages.size() > maxMessages) {
        state.relayMessages.erase(
            state.relayMessages.begin(),
            state.relayMessages.begin() + static_cast<std::vector<LanlineRelayMessage>::difference_type>(state.relayMessages.size() - maxMessages));
    }
}

void TrimVoicePresence(LanlineSessionState& state, std::size_t maxEntries) {
    if (state.voicePresence.size() > maxEntries) {
        state.voicePresence.erase(
            state.voicePresence.begin(),
            state.voicePresence.begin() + static_cast<std::vector<LanlineVoicePresence>::difference_type>(state.voicePresence.size() - maxEntries));
    }
}

struct WinsockSession {
    bool initialized = false;

    WinsockSession() {
        WSADATA data{};
        initialized = WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }

    ~WinsockSession() {
        if (initialized) {
            WSACleanup();
        }
    }
};

}  // namespace

bool SaveLanlineSessionState(const LanlineSessionState& state) {
    return SaveLanlineSessionState(state, LanlineSessionPath());
}

bool SaveLanlineSessionState(const LanlineSessionState& state, const std::filesystem::path& path) {
    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }

    const std::string normalizedWorldName = NormalizeWorldReference(state.worldName);

    out << "session_id=" << state.sessionId << '\n';
    out << "mode=" << state.mode << '\n';
    out << "lifecycle_stage=" << state.lifecycleStage << '\n';
    out << "world=" << normalizedWorldName << '\n';
    out << "host=" << state.hostEndpoint << '\n';
    out << "updated_at=" << (state.updatedAt.empty() ? CurrentTimestampString() : state.updatedAt) << '\n';
    out << "active_actor=" << state.activeActor << '\n';
    out << "pending_peer=" << state.pendingPeer << '\n';
    out << "connected_peer=" << state.connectedPeer << '\n';
    for (const auto& player : state.players) {
        out << "player=" << player.displayName << "|" << player.role << "|" << (player.online ? 1 : 0) << "|" << (player.ready ? 1 : 0) << '\n';
    }
    for (const auto& relayMessage : state.relayMessages) {
        out << "relay_message="
            << relayMessage.channelId << "|"
            << relayMessage.author << "|"
            << relayMessage.timeLabel << "|"
            << relayMessage.body << '\n';
    }
    for (const auto& voicePresence : state.voicePresence) {
        out << "voice_presence="
            << voicePresence.handle << "|"
            << (voicePresence.voiceEnabled ? 1 : 0) << "|"
            << (voicePresence.pushToTalk ? 1 : 0) << "|"
            << (voicePresence.speaking ? 1 : 0) << "|"
            << voicePresence.peakLevel << "|"
            << voicePresence.timeLabel << '\n';
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
        } else if (key == "lifecycle_stage") {
            state.lifecycleStage = value;
        } else if (key == "world") {
            state.worldName = NormalizeWorldReference(value);
        } else if (key == "host") {
            state.hostEndpoint = value;
        } else if (key == "updated_at") {
            state.updatedAt = value;
        } else if (key == "active_actor") {
            state.activeActor = value;
        } else if (key == "pending_peer") {
            state.pendingPeer = value;
        } else if (key == "connected_peer") {
            state.connectedPeer = value;
        } else if (key == "player") {
            const auto first = value.find('|');
            const auto second = value.find('|', first == std::string::npos ? first : first + 1);
            const auto third = value.find('|', second == std::string::npos ? second : second + 1);
            if (first != std::string::npos && second != std::string::npos) {
                state.players.push_back({
                    value.substr(0, first),
                    value.substr(first + 1, second - first - 1),
                    third == std::string::npos ? (value.substr(second + 1) != "0") : (value.substr(second + 1, third - second - 1) != "0"),
                    third == std::string::npos ? false : (value.substr(third + 1) != "0")});
            }
        } else if (key == "relay_message") {
            const auto first = value.find('|');
            const auto second = value.find('|', first == std::string::npos ? first : first + 1);
            const auto third = value.find('|', second == std::string::npos ? second : second + 1);
            if (first != std::string::npos && second != std::string::npos && third != std::string::npos) {
                state.relayMessages.push_back({
                    value.substr(0, first),
                    value.substr(first + 1, second - first - 1),
                    value.substr(second + 1, third - second - 1),
                    value.substr(third + 1)});
            }
        } else if (key == "voice_presence") {
            const auto first = value.find('|');
            const auto second = value.find('|', first == std::string::npos ? first : first + 1);
            const auto third = value.find('|', second == std::string::npos ? second : second + 1);
            const auto fourth = value.find('|', third == std::string::npos ? third : third + 1);
            const auto fifth = value.find('|', fourth == std::string::npos ? fourth : fourth + 1);
            if (first != std::string::npos &&
                second != std::string::npos &&
                third != std::string::npos &&
                fourth != std::string::npos &&
                fifth != std::string::npos) {
                state.voicePresence.push_back({
                    value.substr(0, first),
                    value.substr(first + 1, second - first - 1) != "0",
                    value.substr(second + 1, third - second - 1) != "0",
                    value.substr(third + 1, fourth - third - 1) != "0",
                    std::stof(value.substr(fourth + 1, fifth - fourth - 1)),
                    value.substr(fifth + 1)});
            }
        } else if (key == "event") {
            state.eventLog.push_back(value);
        }
    }

    TrimRelayMessages(state, 32);
    TrimVoicePresence(state, 16);

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

    std::sort(sessions.begin(), sessions.end(), [](const LanlineSessionState& left, const LanlineSessionState& right) {
        if (left.updatedAt != right.updatedAt) {
            return left.updatedAt > right.updatedAt;
        }
        return left.sessionId < right.sessionId;
    });

    return sessions;
}

LanlineDiagnostics ProbeLanlineHost(const LanlineSessionState& session, std::string_view runtimeWorldName) {
    LanlineDiagnostics out;
    out.normalizedWorldName = NormalizeWorldReference(session.worldName);
    out.worldMatch = out.normalizedWorldName == NormalizeWorldReference(runtimeWorldName);
    out.totalPlayers = static_cast<int>(session.players.size());
    for (const auto& player : session.players) {
        if (player.online) {
            out.onlinePlayers += 1;
        }
    }
    out.snapshotFresh = IsFreshTimestamp(session.updatedAt, 15);

    const auto colonPos = session.hostEndpoint.find(':');
    const std::string host = colonPos == std::string::npos ? session.hostEndpoint : session.hostEndpoint.substr(0, colonPos);
    const std::string port = colonPos == std::string::npos ? "27015" : session.hostEndpoint.substr(colonPos + 1);
    if (host.empty() || port.empty()) {
        out.lastError = "Host endpoint is incomplete.";
        return out;
    }

    WinsockSession winsock;
    if (!winsock.initialized) {
        out.lastError = "Winsock initialization failed.";
        return out;
    }

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &result) != 0 || result == nullptr) {
        out.lastError = "Unable to resolve Lanline host.";
        if (result != nullptr) {
            freeaddrinfo(result);
        }
        return out;
    }

    for (addrinfo* address = result; address != nullptr; address = address->ai_next) {
        SOCKET socketHandle = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (socketHandle == INVALID_SOCKET) {
            continue;
        }

        u_long nonBlocking = 1;
        ioctlsocket(socketHandle, FIONBIO, &nonBlocking);
        const auto start = std::chrono::steady_clock::now();
        const int connectResult = connect(socketHandle, address->ai_addr, static_cast<int>(address->ai_addrlen));
        if (connectResult == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS && error != WSAEINVAL) {
                closesocket(socketHandle);
                continue;
            }
        }

        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(socketHandle, &writeSet);
        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = 180000;
        const int selectResult = select(0, nullptr, &writeSet, nullptr, &timeout);
        if (selectResult > 0 && FD_ISSET(socketHandle, &writeSet)) {
            int socketError = 0;
            int socketErrorSize = sizeof(socketError);
            getsockopt(socketHandle, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&socketError), &socketErrorSize);
            if (socketError == 0) {
                const auto end = std::chrono::steady_clock::now();
                out.hostReachable = true;
                out.pingMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
                closesocket(socketHandle);
                break;
            }
        }
        closesocket(socketHandle);
    }

    freeaddrinfo(result);
    if (!out.hostReachable && out.lastError.empty()) {
        out.lastError = "Host did not answer within probe window.";
    }
    return out;
}

}  // namespace bunker
