#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace bunker {

struct LanlinePlayerEntry {
    std::string displayName;
    std::string role;
    bool online = true;
    bool ready = false;
};

struct LanlineSessionState {
    std::string sessionId = "lanline_local";
    std::string mode = "Solo";
    std::string lifecycleStage = "Idle";
    std::string worldName = "start_zone.bwld";
    std::string hostEndpoint = "127.0.0.1:27015";
    std::string updatedAt = "unknown";
    std::string activeActor = "Operator";
    std::string pendingPeer;
    std::string connectedPeer;
    std::vector<LanlinePlayerEntry> players{};
    std::vector<std::string> eventLog{};
};

struct LanlineDiagnostics {
    bool hostReachable = false;
    bool worldMatch = false;
    bool snapshotFresh = false;
    int pingMs = -1;
    int onlinePlayers = 0;
    int totalPlayers = 0;
    std::string normalizedWorldName = "start_zone.bwld";
    std::string lastError{};
};

bool SaveLanlineSessionState(const LanlineSessionState& state);
bool LoadLanlineSessionState(LanlineSessionState& state);
bool SaveLanlineSessionState(const LanlineSessionState& state, const std::filesystem::path& path);
bool LoadLanlineSessionState(const std::filesystem::path& path, LanlineSessionState& state);
std::vector<LanlineSessionState> DiscoverLanlineSessionSnapshots();
LanlineDiagnostics ProbeLanlineHost(const LanlineSessionState& session, std::string_view runtimeWorldName);

}  // namespace bunker
