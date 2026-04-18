#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace bunker {

struct LanlinePlayerEntry {
    std::string displayName;
    std::string role;
    bool online = true;
};

struct LanlineSessionState {
    std::string sessionId = "lanline_local";
    std::string mode = "Solo";
    std::string worldName = "start_zone.bwld";
    std::string hostEndpoint = "127.0.0.1:27015";
    std::string updatedAt = "unknown";
    std::vector<LanlinePlayerEntry> players{};
    std::vector<std::string> eventLog{};
};

bool SaveLanlineSessionState(const LanlineSessionState& state);
bool LoadLanlineSessionState(LanlineSessionState& state);
bool SaveLanlineSessionState(const LanlineSessionState& state, const std::filesystem::path& path);
bool LoadLanlineSessionState(const std::filesystem::path& path, LanlineSessionState& state);
std::vector<LanlineSessionState> DiscoverLanlineSessionSnapshots();

}  // namespace bunker
