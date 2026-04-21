#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "../../include/LanlineSession.hpp"
#include "../../include/SessionProfiles.hpp"

namespace launcher_support {

struct LauncherState {
    bool loggedIn = false;
    bool authFailed = false;
    int selectedCharacter = 0;
    int sessionModeIndex = 0;
    int previousSessionModeIndex = 0;
    int selectedWorldIndex = 0;
    int selectedLanlineSnapshot = -1;
    char login[64] = "wanderer";
    char password[64] = "prototype";
    char lanHost[64] = "127.0.0.1";
    char lanPort[16] = "27015";
    bool announcementDetailsOpen = false;
    std::string activeAnnouncementId{};
    std::string statusText = "System ready. Authorize to continue.";
};

struct LauncherDataCache {
    std::vector<std::filesystem::path> worlds;
    std::vector<std::string> worldLabelStorage;
    std::vector<const char*> worldLabels;
    std::vector<bunker::LanlineSessionState> knownLanlineSessions;
    double lastRefreshTime = -10.0;
};

int ClampIndex(int value, int itemCount);

template <std::size_t N>
int ClampArrayIndex(int value, const char* const (&)[N]) {
    static_assert(N > 0, "Array must not be empty.");
    return ClampIndex(value, static_cast<int>(N));
}

void TrimLanlineEventLog(bunker::LanlineSessionState& state, std::size_t maxEntries);
void UpsertLanlinePlayer(bunker::LanlineSessionState& state, const std::string& displayName, const std::string& role, bool online, bool ready = false);
void ReserveLanlineLobbySlot(bunker::LanlineSessionState& state, const std::string& peerName);
void PromoteReservedLanlineLobbySlot(bunker::LanlineSessionState& state, const std::string& peerName);
void AcceptLanlineLobbySlot(bunker::LanlineSessionState& state, const std::string& peerName);
void ClearLanlineLobbyPeerSlots(bunker::LanlineSessionState& state, const std::string& peerName);
std::filesystem::path LauncherExecutableDirectory();
std::filesystem::path FindSiblingExecutable(const char* executableName);
std::filesystem::path SelectedWorldPath(const std::vector<std::filesystem::path>& worlds, int selectedWorldIndex);
void RefreshWorldSelectionFromProfile(const bunker::SessionProfile& sessionProfile, LauncherState& launcherState, const std::vector<std::filesystem::path>& worlds);
void RebuildWorldLabels(LauncherDataCache& cache);
void RefreshLauncherData(LauncherDataCache& cache, LauncherState& launcherState, const bunker::SessionProfile& sessionProfile);
std::vector<std::string> BuildLanlineRoster(const LauncherState& launcherState, const char* selectedCharacterLabel);
std::string BuildLanlineSessionId(const LauncherState& launcherState, const std::filesystem::path& selectedWorld);
const char* JoinabilityLabel(const bunker::LanlineSessionState& session);
std::string JoinabilityReason(const bunker::LanlineSessionState& session);
int FindFirstJoinableSessionIndex(const std::vector<bunker::LanlineSessionState>& knownLanlineSessions);
int CountJoinableSessions(const std::vector<bunker::LanlineSessionState>& knownLanlineSessions);
const bunker::LanlineSessionState* SelectedJoinTarget(const LauncherState& launcherState, const std::vector<bunker::LanlineSessionState>& knownLanlineSessions);
bunker::LanlineSessionState SaveLanlineRosterState(const LauncherState& launcherState, const char* selectedCharacterLabel, const std::filesystem::path& selectedWorld, const bunker::LanlineSessionState* joinTarget);
void ApplyLanlineSnapshotToLauncher(const bunker::LanlineSessionState& snapshot, LauncherState& launcherState, const std::vector<std::filesystem::path>& worlds);
void SaveLanlineSnapshotAndMaybeActive(const bunker::LanlineSessionState& snapshot);
bool AcceptPendingLanlinePeer(bunker::LanlineSessionState& session);
bool ClearLanlinePeerLink(bunker::LanlineSessionState& session);
bool ReserveLanlinePeerSlot(bunker::LanlineSessionState& session, const std::string& peerName);
bool ToggleLanlinePlayerReady(bunker::LanlineSessionState& session, const std::string& displayName);
bool SetLanlineMatchStartArmed(bunker::LanlineSessionState& session, bool armed);
std::vector<std::filesystem::path> DiscoverWorlds();
bool TryLaunchSiblingExecutable(const char* executableName, std::string& statusText);
void PrepareSelectedCharacter(bunker::SessionProfile& sessionProfile, const LauncherState& launcherState, const char* const* characters, int characterCount);
std::string BuildLauncherObjectivePreview(const bunker::SessionProfile& sessionProfile, const bunker::WorldFieldState* worldState);
const bunker::LanlineDiagnostics& CachedLanlineDiagnostics(const bunker::LanlineSessionState& session, std::string_view runtimeWorldName);
float DrawLauncherAnnouncementWidget(
    bunker::SessionProfile& sessionProfile,
    const std::filesystem::path& profilePath,
    LauncherState& launcherState);
void DrawSessionSummary(const bunker::SessionProfile& sessionProfile, const char* selectedCharacterLabel, const char* selectedModeLabel, const std::filesystem::path& selectedWorld);

}  // namespace launcher_support
