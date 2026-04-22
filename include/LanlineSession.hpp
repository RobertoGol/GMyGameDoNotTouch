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
    std::string seatAssignment = "on_foot";
};

struct LanlineRelayMessage {
    std::string channelId = "session";
    std::string author = "Operator";
    std::string timeLabel = "now";
    std::string body{};
};

struct LanlineVoicePresence {
    std::string handle = "Operator";
    bool voiceEnabled = true;
    bool pushToTalk = true;
    bool speaking = false;
    float peakLevel = 0.0f;
    std::string timeLabel = "now";
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
    bool bt72SecondSeatUnlocked = false;
    std::string bt72SecondSeatPolicy = "pilot_only";
    std::string bt72TrustedGunnerHandle{};
    std::string bt72AssignedGunnerHandle{};
    std::vector<LanlinePlayerEntry> players{};
    std::vector<LanlineRelayMessage> relayMessages{};
    std::vector<LanlineVoicePresence> voicePresence{};
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
