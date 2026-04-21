#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "LauncherSupport.hpp"

#include "../../include/AppPaths.hpp"
#include "../../include/AtomicPersistence.hpp"
#include "../../include/LanlineLobbyLogic.hpp"
#include "../../include/LanlineServices.hpp"
#include "../../include/LanlineSession.hpp"
#include "../../include/LaunchSession.hpp"
#include "../../include/SessionFlow.hpp"
#include "../../include/SessionProfiles.hpp"
namespace {

#if 0

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
    std::string statusText = "System ready. Authorize to continue.";
};

struct LauncherDataCache {
    std::vector<std::filesystem::path> worlds;
    std::vector<std::string> worldLabelStorage;
    std::vector<const char*> worldLabels;
    std::vector<bunker::LanlineSessionState> knownLanlineSessions;
    double lastRefreshTime = -10.0;
};

void SaveLanlineSnapshotAndMaybeActive(const bunker::LanlineSessionState& snapshot);
std::vector<std::filesystem::path> DiscoverWorlds();

int ClampIndex(int value, int itemCount) {
    if (itemCount <= 0) {
        return 0;
    }
    return std::clamp(value, 0, itemCount - 1);
}

template <std::size_t N>
int ClampArrayIndex(int value, const char* const (&)[N]) {
    static_assert(N > 0, "Array must not be empty.");
    return ClampIndex(value, static_cast<int>(N));
}

void TrimLanlineEventLog(bunker::LanlineSessionState& state, std::size_t maxEntries) {
    if (state.eventLog.size() > maxEntries) {
        state.eventLog.erase(
            state.eventLog.begin(),
            state.eventLog.begin() + static_cast<std::vector<std::string>::difference_type>(state.eventLog.size() - maxEntries));
    }
}

void UpsertLanlinePlayer(bunker::LanlineSessionState& state,
    const std::string& displayName,
    const std::string& role,
    bool online,
    bool ready = false) {
    auto playerIt = std::find_if(state.players.begin(), state.players.end(),
        [&](const bunker::LanlinePlayerEntry& entry) {
            return entry.displayName == displayName;
        });
    if (playerIt == state.players.end()) {
        state.players.push_back({displayName, role, online, ready});
        return;
    }
    playerIt->role = role;
    playerIt->online = online;
    playerIt->ready = ready;
}

void ReserveLanlineLobbySlot(bunker::LanlineSessionState& state, const std::string& peerName) {
    for (auto& player : state.players) {
        if (player.displayName == peerName && (bunker::IsLanlineReservedSlot(player) || bunker::IsLanlinePendingSlot(player))) {
            return;
        }
    }
    const int slotIndex = bunker::FindFirstAwaitingSlotIndex(state);
    if (slotIndex < 0) {
        return;
    }
    auto& slot = state.players[static_cast<std::size_t>(slotIndex)];
    slot.displayName = peerName;
    slot.role = "Reserved Client";
    slot.online = false;
    slot.ready = false;
}

void PromoteReservedLanlineLobbySlot(bunker::LanlineSessionState& state, const std::string& peerName) {
    for (auto& player : state.players) {
        if (player.displayName == peerName) {
            if (bunker::IsLanlineReservedSlot(player) || bunker::IsLanlinePendingSlot(player)) {
                player.role = "Pending Client";
                player.online = false;
                player.ready = false;
            }
            return;
        }
    }
    ReserveLanlineLobbySlot(state, peerName);
    for (auto& player : state.players) {
        if (player.displayName == peerName && bunker::IsLanlineReservedSlot(player)) {
            player.role = "Pending Client";
            player.online = false;
            player.ready = false;
            return;
        }
    }
}

void AcceptLanlineLobbySlot(bunker::LanlineSessionState& state, const std::string& peerName) {
    for (auto& player : state.players) {
        if (player.displayName == peerName) {
            player.role = "Client";
            player.online = true;
            player.ready = false;
            return;
        }
    }
    ReserveLanlineLobbySlot(state, peerName);
    for (auto& player : state.players) {
        if (player.displayName == peerName) {
            player.role = "Client";
            player.online = true;
            player.ready = false;
            return;
        }
    }
    UpsertLanlinePlayer(state, peerName, "Client", true, false);
}

void ClearLanlineLobbyPeerSlots(bunker::LanlineSessionState& state, const std::string& peerName) {
    int awaitingOrdinal = 2;
    for (auto& player : state.players) {
        if ((player.displayName == peerName && (player.role == "Reserved Client" || player.role == "Pending Client" || player.role == "Client")) ||
            player.role == "Reserved Client" || player.role == "Pending Client") {
            player.displayName = "LAN slot " + std::to_string(awaitingOrdinal);
            player.role = "Awaiting";
            player.online = false;
            player.ready = false;
        }
        if (player.role == "Awaiting") {
            player.displayName = "LAN slot " + std::to_string(awaitingOrdinal);
            awaitingOrdinal += 1;
        }
    }
}

std::filesystem::path LauncherExecutableDirectory() {
    std::array<char, MAX_PATH> buffer{};
    const DWORD length = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(std::string(buffer.data(), length)).parent_path();
}

std::filesystem::path FindSiblingExecutable(const char* executableName) {
    const std::array<std::filesystem::path, 2> candidateRoots = {
        LauncherExecutableDirectory(),
        std::filesystem::current_path(),
    };
    for (const auto& root : candidateRoots) {
        const auto candidate = root / executableName;
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

std::filesystem::path SelectedWorldPath(const std::vector<std::filesystem::path>& worlds, int selectedWorldIndex) {
    if (worlds.empty()) {
        return bunker::DefaultWorldPath().filename();
    }
    return worlds[static_cast<std::size_t>(ClampIndex(selectedWorldIndex, static_cast<int>(worlds.size())))];
}

void RefreshWorldSelectionFromProfile(const bunker::SessionProfile& sessionProfile,
    LauncherState& launcherState,
    const std::vector<std::filesystem::path>& worlds) {
    if (worlds.empty()) {
        launcherState.selectedWorldIndex = 0;
        return;
    }
    launcherState.selectedWorldIndex = ClampIndex(launcherState.selectedWorldIndex, static_cast<int>(worlds.size()));
    for (std::size_t index = 0; index < worlds.size(); ++index) {
        if (bunker::NormalizeWorldReference(worlds[index].string()) == sessionProfile.selectedWorld) {
            launcherState.selectedWorldIndex = static_cast<int>(index);
            return;
        }
    }
}

void RebuildWorldLabels(LauncherDataCache& cache) {
    cache.worldLabelStorage.clear();
    cache.worldLabels.clear();
    cache.worldLabelStorage.reserve(cache.worlds.size());
    cache.worldLabels.reserve(cache.worlds.size());
    for (const auto& world : cache.worlds) {
        cache.worldLabelStorage.push_back(world.string());
    }
    for (const auto& worldLabel : cache.worldLabelStorage) {
        cache.worldLabels.push_back(worldLabel.c_str());
    }
}

void RefreshLauncherData(LauncherDataCache& cache,
    LauncherState& launcherState,
    const bunker::SessionProfile& sessionProfile) {
    cache.worlds = DiscoverWorlds();
    cache.knownLanlineSessions = bunker::DiscoverLanlineSessionSnapshots();
    RebuildWorldLabels(cache);
    RefreshWorldSelectionFromProfile(sessionProfile, launcherState, cache.worlds);
    launcherState.selectedLanlineSnapshot = cache.knownLanlineSessions.empty()
        ? -1
        : ClampIndex(launcherState.selectedLanlineSnapshot, static_cast<int>(cache.knownLanlineSessions.size()));
    cache.lastRefreshTime = glfwGetTime();
}

std::vector<std::string> BuildLanlineRoster(const LauncherState& launcherState, const char* selectedCharacterLabel) {
    std::vector<std::string> roster;
    if (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::Solo)) {
        roster.push_back(std::string(selectedCharacterLabel) + " [Local Operator]");
        return roster;
    }

    if (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::LanHost)) {
        roster.push_back(std::string(selectedCharacterLabel) + " [Host]");
        roster.push_back("LAN slot 2 [Awaiting]");
        roster.push_back("LAN slot 3 [Awaiting]");
        roster.push_back("LAN slot 4 [Awaiting]");
        return roster;
    }

    roster.push_back(std::string(selectedCharacterLabel) + " [Client]");
    roster.push_back(std::string("Host @ ") + launcherState.lanHost + ":" + launcherState.lanPort);
    roster.push_back("LAN roster sync pending");
    return roster;
}

std::string BuildLanlineSessionId(const LauncherState& launcherState, const std::filesystem::path& selectedWorld) {
    return std::string("lanline_") + selectedWorld.stem().string() + "_" +
        (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::Solo) ? "solo" :
            (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::LanHost) ? "host" : "client"));
}

const char* JoinabilityLabel(const bunker::LanlineSessionState& session) {
    if (session.mode != "LAN Host") {
        return "non-host";
    }
    if (!session.connectedPeer.empty()) {
        return "linked";
    }
    if (!session.pendingPeer.empty()) {
        return "pending";
    }
    if (session.lifecycleStage == "HostLobbyOpen" || session.lifecycleStage == "HostRuntimeActive") {
        return "joinable";
    }
    return "locked";
}

std::string JoinabilityReason(const bunker::LanlineSessionState& session) {
    if (session.mode != "LAN Host") {
        return "Only host snapshots can be used as a join target.";
    }
    if (bunker::AvailableLanlineSessionSlots(session) <= 0) {
        return "This host snapshot has no remaining open slots.";
    }
    if (!session.connectedPeer.empty()) {
        return "This host snapshot already has an active connected peer: " + session.connectedPeer + ".";
    }
    if (!session.pendingPeer.empty()) {
        return "This host snapshot is already processing a pending peer: " + session.pendingPeer + ".";
    }
    if (session.lifecycleStage == "HostLobbyOpen" || session.lifecycleStage == "HostRuntimeActive") {
        return "Host snapshot is ready to accept a client join target.";
    }
    return "Host snapshot is not in a join-ready lifecycle stage yet.";
}

int FindFirstJoinableSessionIndex(const std::vector<bunker::LanlineSessionState>& knownLanlineSessions) {
    for (int index = 0; index < static_cast<int>(knownLanlineSessions.size()); ++index) {
        if (bunker::IsJoinableLanlineSession(knownLanlineSessions[static_cast<std::size_t>(index)])) {
            return index;
        }
    }
    return -1;
}

int CountJoinableSessions(const std::vector<bunker::LanlineSessionState>& knownLanlineSessions) {
    int count = 0;
    for (const auto& session : knownLanlineSessions) {
        if (bunker::IsJoinableLanlineSession(session)) {
            count += 1;
        }
    }
    return count;
}

const bunker::LanlineSessionState* SelectedJoinTarget(const LauncherState& launcherState,
    const std::vector<bunker::LanlineSessionState>& knownLanlineSessions) {
    if (launcherState.sessionModeIndex != static_cast<int>(bunker::SessionMode::LanClient)) {
        return nullptr;
    }
    if (launcherState.selectedLanlineSnapshot < 0 ||
        launcherState.selectedLanlineSnapshot >= static_cast<int>(knownLanlineSessions.size())) {
        return nullptr;
    }
    const auto& candidate = knownLanlineSessions[static_cast<std::size_t>(launcherState.selectedLanlineSnapshot)];
    return bunker::IsJoinableLanlineSession(candidate) ? &candidate : nullptr;
}

bunker::LanlineSessionState SaveLanlineRosterState(const LauncherState& launcherState,
    const char* selectedCharacterLabel,
    const std::filesystem::path& selectedWorld,
    const bunker::LanlineSessionState* joinTarget) {
    const std::string worldReference = joinTarget != nullptr
        ? bunker::NormalizeWorldReference(joinTarget->worldName)
        : bunker::NormalizeWorldReference(selectedWorld.string());
    bunker::LanlineSessionState state;
    state.sessionId = joinTarget != nullptr ? joinTarget->sessionId : BuildLanlineSessionId(launcherState, selectedWorld);
    state.mode = launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::Solo)
        ? "Solo"
        : (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::LanHost) ? "LAN Host" : "LAN Client");
    state.activeActor = selectedCharacterLabel;
    state.worldName = worldReference;
    state.hostEndpoint = joinTarget != nullptr
        ? joinTarget->hostEndpoint
        : (std::string(launcherState.lanHost) + ":" + launcherState.lanPort);
    state.updatedAt.clear();
    state.pendingPeer.clear();
    state.connectedPeer.clear();

    if (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::Solo)) {
        state.lifecycleStage = "LauncherSeeded";
        state.players.push_back({selectedCharacterLabel, "Local Operator", true, false});
        state.eventLog.push_back(std::string(selectedCharacterLabel) + " entered solo runtime.");
    } else if (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::LanHost)) {
        state.lifecycleStage = "HostLobbyOpen";
        state.players.push_back({selectedCharacterLabel, "Host", true, false});
        state.players.push_back({"LAN slot 2", "Awaiting", false, false});
        state.players.push_back({"LAN slot 3", "Awaiting", false, false});
        state.players.push_back({"LAN slot 4", "Awaiting", false, false});
        state.eventLog.push_back(std::string(selectedCharacterLabel) + " opened a Lanline - optime host session.");
        state.eventLog.push_back(std::string("Host endpoint ready at ") + state.hostEndpoint + ".");
    } else {
        state.lifecycleStage = joinTarget != nullptr ? "ClientJoinTargetLocked" : "ClientJoinRequested";
        state.pendingPeer = selectedCharacterLabel;
        state.connectedPeer = joinTarget != nullptr ? joinTarget->activeActor : std::string{};
        state.players.push_back({selectedCharacterLabel, "Client", true, false});
        state.players.push_back({std::string("Host @ ") + launcherState.lanHost, "Host", true, false});
        state.players.push_back({"LAN roster sync", "Pending", false, false});
        state.eventLog.push_back(std::string(selectedCharacterLabel) + " is attempting to join a Lanline - optime session.");
        state.eventLog.push_back(std::string("Client targeting host ") + state.hostEndpoint + ".");
        if (joinTarget != nullptr) {
            state.eventLog.push_back("Join target locked to session " + joinTarget->sessionId + ".");
            bunker::LanlineSessionState hostSession = *joinTarget;
            hostSession.pendingPeer = selectedCharacterLabel;
            hostSession.lifecycleStage = "HostJoinPending";
            PromoteReservedLanlineLobbySlot(hostSession, selectedCharacterLabel);
            hostSession.eventLog.push_back(std::string(selectedCharacterLabel) + " requested join through launcher targeting " + hostSession.sessionId + ".");
            TrimLanlineEventLog(hostSession, 12);
            SaveLanlineSnapshotAndMaybeActive(hostSession);
        }
    }

    bunker::SaveLanlineSessionState(state);
    bunker::SaveLanlineSessionState(state, bunker::LanlineSessionSnapshotPath(state.sessionId));
    return state;
}

void ApplyLanlineSnapshotToLauncher(const bunker::LanlineSessionState& snapshot,
    LauncherState& launcherState,
    const std::vector<std::filesystem::path>& worlds) {
    const std::string snapshotWorld = bunker::NormalizeWorldReference(snapshot.worldName);
    if (snapshot.mode == "LAN Host") {
        launcherState.sessionModeIndex = static_cast<int>(bunker::SessionMode::LanHost);
    } else if (snapshot.mode == "LAN Client") {
        launcherState.sessionModeIndex = static_cast<int>(bunker::SessionMode::LanClient);
    } else {
        launcherState.sessionModeIndex = static_cast<int>(bunker::SessionMode::Solo);
    }

    const auto colonPos = snapshot.hostEndpoint.find(':');
    const std::string host = colonPos == std::string::npos ? snapshot.hostEndpoint : snapshot.hostEndpoint.substr(0, colonPos);
    const std::string port = colonPos == std::string::npos ? "27015" : snapshot.hostEndpoint.substr(colonPos + 1);
    std::snprintf(launcherState.lanHost, sizeof(launcherState.lanHost), "%s", host.c_str());
    std::snprintf(launcherState.lanPort, sizeof(launcherState.lanPort), "%s", port.c_str());

    for (std::size_t index = 0; index < worlds.size(); ++index) {
        if (bunker::NormalizeWorldReference(worlds[index].string()) == snapshotWorld) {
            launcherState.selectedWorldIndex = static_cast<int>(index);
            break;
        }
    }
}

void SaveLanlineSnapshotAndMaybeActive(const bunker::LanlineSessionState& snapshot) {
    bunker::SaveLanlineSessionState(snapshot, bunker::LanlineSessionSnapshotPath(snapshot.sessionId));

    bunker::LanlineSessionState activeSession;
    if (bunker::LoadLanlineSessionState(activeSession) && activeSession.sessionId == snapshot.sessionId) {
        bunker::SaveLanlineSessionState(snapshot);
    }
}

bool AcceptPendingLanlinePeer(bunker::LanlineSessionState& session) {
    if (session.pendingPeer.empty() && session.connectedPeer.empty()) {
        return false;
    }
    session.connectedPeer = session.pendingPeer.empty() ? session.connectedPeer : session.pendingPeer;
    session.pendingPeer.clear();
    session.lifecycleStage = "HostClientAccepted";
    AcceptLanlineLobbySlot(session, session.connectedPeer);
    session.eventLog.push_back("Launcher accepted pending Lanline peer " + session.connectedPeer + ".");
    TrimLanlineEventLog(session, 12);
    SaveLanlineSnapshotAndMaybeActive(session);
    return true;
}

bool ClearLanlinePeerLink(bunker::LanlineSessionState& session) {
    if (session.pendingPeer.empty() && session.connectedPeer.empty()) {
        return false;
    }

    const std::string clearedPeer = !session.connectedPeer.empty() ? session.connectedPeer : session.pendingPeer;
    session.pendingPeer.clear();
    session.connectedPeer.clear();
    if (!clearedPeer.empty()) {
        ClearLanlineLobbyPeerSlots(session, clearedPeer);
    }
    if (session.mode == "LAN Host") {
        session.lifecycleStage = "HostLobbyOpen";
    } else if (session.mode == "LAN Client") {
        session.lifecycleStage = "ClientJoinRequested";
    } else {
        session.lifecycleStage = "LauncherSeeded";
    }
    session.eventLog.push_back("Launcher cleared Lanline peer link for " + clearedPeer + ".");
    TrimLanlineEventLog(session, 12);
    SaveLanlineSnapshotAndMaybeActive(session);
    return true;
}

bool ReserveLanlinePeerSlot(bunker::LanlineSessionState& session, const std::string& peerName) {
    if (peerName.empty()) {
        return false;
    }
    ReserveLanlineLobbySlot(session, peerName);
    session.lifecycleStage = "HostSeatReserved";
    session.eventLog.push_back("Launcher reserved a Lanline slot for " + peerName + ".");
    TrimLanlineEventLog(session, 12);
    SaveLanlineSnapshotAndMaybeActive(session);
    return true;
}

bool ToggleLanlinePlayerReady(bunker::LanlineSessionState& session, const std::string& displayName) {
    for (auto& player : session.players) {
        if (player.displayName == displayName && bunker::IsLanlineReadyEligibleSlot(player)) {
            player.ready = !player.ready;
            session.eventLog.push_back(displayName + std::string(player.ready ? " is now ready." : " is no longer ready."));
            TrimLanlineEventLog(session, 12);
            SaveLanlineSnapshotAndMaybeActive(session);
            return true;
        }
    }
    return false;
}

bool SetLanlineMatchStartArmed(bunker::LanlineSessionState& session, bool armed) {
    session.lifecycleStage = armed ? "MatchStartReady" : "HostLobbyOpen";
    session.eventLog.push_back(armed
        ? "Host armed Lanline match start."
        : "Host returned Lanline session to lobby state.");
    TrimLanlineEventLog(session, 12);
    SaveLanlineSnapshotAndMaybeActive(session);
    return true;
}

std::vector<std::filesystem::path> DiscoverWorlds() {
    std::vector<std::filesystem::path> worlds;
    const auto worldDir = bunker::WorldDirectory();
    if (std::filesystem::exists(worldDir)) {
        for (const auto& entry : std::filesystem::directory_iterator(worldDir)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            if (entry.path().extension() == ".bwld") {
                worlds.push_back(entry.path().filename());
            }
        }
    }
    if (worlds.empty()) {
        worlds.push_back(bunker::DefaultWorldPath().filename());
    }
    std::sort(worlds.begin(), worlds.end());
    return worlds;
}

bool TryLaunchSiblingExecutable(const char* executableName, std::string& statusText) {
    const auto candidate = FindSiblingExecutable(executableName);
    if (!std::filesystem::exists(candidate)) {
        statusText = std::string("Executable not found near launcher binary: ") + executableName;
        return false;
    }

    STARTUPINFOA startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo{};
    std::string commandLine = "\"" + candidate.string() + "\"";
    std::string workingDirectory = candidate.parent_path().string();
    if (!CreateProcessA(
            candidate.string().c_str(),
            commandLine.data(),
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            workingDirectory.empty() ? nullptr : workingDirectory.c_str(),
            &startupInfo,
            &processInfo)) {
        statusText = "Failed to launch " + candidate.filename().string();
        return false;
    }

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    statusText = "Launching " + candidate.filename().string();
    return true;
}

void PrepareSelectedCharacter(bunker::SessionProfile& sessionProfile,
    const LauncherState& launcherState,
    const char* const* characters,
    std::size_t characterCount) {
    if (characterCount == 0) {
        sessionProfile.character.displayName.clear();
        sessionProfile.character.characterId = "@fallback_character";
        sessionProfile.account.username = launcherState.login;
        return;
    }

    const int safeIndex = std::clamp(
        launcherState.selectedCharacter,
        0,
        static_cast<int>(characterCount - 1));
    const std::size_t idx = static_cast<std::size_t>(safeIndex);
    sessionProfile.character.displayName = characters[idx];
    if (idx < sessionProfile.account.linkedCharacters.size()) {
        sessionProfile.character.characterId = sessionProfile.account.linkedCharacters[idx];
    } else if (!sessionProfile.account.linkedCharacters.empty()) {
        sessionProfile.character.characterId = sessionProfile.account.linkedCharacters.front();
    } else {
        sessionProfile.character.characterId = "@fallback_character";
    }
    sessionProfile.account.username = launcherState.login;
}

std::string BuildLauncherObjectivePreview(const bunker::SessionProfile& sessionProfile, const bunker::WorldFieldState* worldState) {
    if (!sessionProfile.story.awakenedFromCryo) {
        return "Wake from the cryo capsule and stabilize the recovery route.";
    }
    if (!sessionProfile.story.pipPadRecovered) {
        return "Recover the missing Pip-Pad from the locker bay.";
    }
    if (!sessionProfile.story.archiveRecovered) {
        return "Read the archive terminal and reconstruct what happened in Shelter 17.";
    }
    if (!sessionProfile.story.tankLinked) {
        return "Reach the garage and establish the BT-72 tank link.";
    }
    if (!sessionProfile.story.bucketRecovered) {
        return "Recover the bucket plow rack before opening the outer route.";
    }
    if (!sessionProfile.story.exitedBunker) {
        return "Open the outer bulkhead and move into the recovery corridor.";
    }
    if (!sessionProfile.story.outerRoadCleared) {
        return "Raise the bucket and clear the outer debris barrier.";
    }
    if (!sessionProfile.story.relayRecovered) {
        return "Sync the relay terminal and recover the reconstruction schematics.";
    }
    if (!sessionProfile.story.returnedToBase) {
        return "Return to the debrief console inside Shelter 17.";
    }
    if (worldState == nullptr) {
        return "Recovery buildout active. Runtime world state not loaded yet.";
    }
    if (!bunker::IsRailFreightOperational(sessionProfile, *worldState)) {
        return "Restore the industrial rail depot and bring heavy freight back to Shelter 17.";
    }
    if (!bunker::IsOrbitalUplinkOperational(sessionProfile, *worldState)) {
        return "Align the orbital uplink to extend long-range recovery scans.";
    }
    if (!bunker::IsRailFortressOperational(sessionProfile, *worldState)) {
        return "Deploy the Rail Fortress to secure the restored industrial spur.";
    }
    if (!bunker::IsRecoveryFabricatorOperational(sessionProfile, *worldState)) {
        return "Prime the Recovery Fabricator to turn salvage into operational supplies.";
    }
    if (!worldState->industrialGateUnlocked) {
        return "Unlock the industrial gate and push Shelter 17 into the inner spur.";
    }
    if (!bunker::IsIndustrialSurveyOperational(*worldState)) {
        return "Align the industrial survey beacon and start mapping the inner spur.";
    }
    if (!bunker::IsIndustrialOutpostOperational(*worldState)) {
        return "Establish the inner spur outpost and secure a forward foothold beyond the gate.";
    }
    if (!bunker::IsAssemblyCellOperational(*worldState)) {
        return "Bring the inner spur assembly cell online for local industrial recovery.";
    }
    if (!bunker::IsFoundryLineOperational(*worldState)) {
        return "Restart the inner spur foundry line to resume heavy plate fabrication.";
    }
    if (!bunker::IsReactorYardOperational(*worldState)) {
        return "Bring the inner spur reactor yard online to stabilize deeper industrial energy flow.";
    }
    if (!bunker::IsCapacitorBankOperational(*worldState)) {
        return "Charge the inner spur capacitor bank to buffer and stabilize the heavy grid.";
    }
    if (!bunker::IsRelaySubstationOperational(*worldState)) {
        return "Sync the relay substation and route inner spur power back into Shelter 17.";
    }
    if (!bunker::IsServiceBayOperational(*worldState)) {
        return "Bring the inner spur service bay online to push BT-72 repairs deeper into the factory belt.";
    }
    if (!bunker::IsWaterReclaimerOperational(*worldState)) {
        return "Bring the inner spur water reclaimer online to stabilize long-range recovery and camp support.";
    }
    return "Water reclaimer online. Shelter 17 now has a stable recovery backbone; expand deeper into the inner spur and wider factory belt.";
}

const bunker::LanlineDiagnostics& CachedLanlineDiagnostics(const bunker::LanlineSessionState& session, std::string_view runtimeWorldName) {
    static std::string cachedSessionKey;
    static std::string cachedWorldName;
    static double lastProbeTime = -10.0;
    static bunker::LanlineDiagnostics cachedDiagnostics;

    const std::string probeKey = session.sessionId + "|" + session.updatedAt + "|" + session.hostEndpoint;
    const std::string normalizedWorldName = bunker::NormalizeWorldReference(runtimeWorldName);
    const double now = glfwGetTime();
    if (probeKey != cachedSessionKey || normalizedWorldName != cachedWorldName || (now - lastProbeTime) >= 1.5) {
        cachedDiagnostics = bunker::ProbeLanlineHost(session, normalizedWorldName);
        cachedSessionKey = probeKey;
        cachedWorldName = normalizedWorldName;
        lastProbeTime = now;
    }
    return cachedDiagnostics;
}

void DrawSessionSummary(const bunker::SessionProfile& sessionProfile, const char* selectedCharacterLabel, const char* selectedModeLabel, const std::filesystem::path& selectedWorld) {
    const std::string worldReference = bunker::NormalizeWorldReference(selectedWorld.string());
    const auto* worldState = bunker::FindWorldFieldState(sessionProfile, worldReference);
    const bool stableBackbone = worldState != nullptr &&
        bunker::IsStableRecoveryBackbone(sessionProfile, *worldState);
    const bool tradeOperational = worldState != nullptr &&
        bunker::IsTradeNetworkOperational(sessionProfile, *worldState);
    const bool railOperational = worldState != nullptr &&
        bunker::IsRailFreightOperational(sessionProfile, *worldState);
    const bool orbitalOperational = worldState != nullptr &&
        bunker::IsOrbitalUplinkOperational(sessionProfile, *worldState);
    const bool fabricatorOperational = worldState != nullptr &&
        bunker::IsRecoveryFabricatorOperational(sessionProfile, *worldState);
    const std::string objective = BuildLauncherObjectivePreview(sessionProfile, worldState);
    const char* recoveryStatus = stableBackbone
        ? "Stable backbone"
        : (sessionProfile.story.returnedToBase ? "Recovery buildout active" : "Starter route");

    ImGui::Text("Startup Summary");
    ImGui::BulletText("Account: %s", sessionProfile.account.accountId.c_str());
    ImGui::BulletText("Character: %s", selectedCharacterLabel);
    ImGui::BulletText("Session: %s", selectedModeLabel);
    ImGui::BulletText("World: %s", worldReference.c_str());
    ImGui::BulletText("Partner tank: %s [%s]", sessionProfile.partnerTank.callSign.c_str(), sessionProfile.partnerTank.partnerTankId.c_str());
    ImGui::BulletText("Tank class: %s", bunker::ToString(sessionProfile.partnerTank.tankClass));
    ImGui::BulletText("Story checkpoint: cryo %d | tank %d | relay %d | debrief %d",
        sessionProfile.story.awakenedFromCryo ? 1 : 0,
        sessionProfile.story.tankLinked ? 1 : 0,
        sessionProfile.story.relayRecovered ? 1 : 0,
        sessionProfile.story.returnedToBase ? 1 : 0);
    ImGui::BulletText("Recovery status: %s", recoveryStatus);
    ImGui::BulletText("Trade loop: %s", tradeOperational ? "operational" : "not operational");
    ImGui::BulletText("Rail loop: %s", railOperational ? "operational" : "not operational");
    ImGui::BulletText("Orbital support: %s", orbitalOperational ? "operational" : "not operational");
    ImGui::BulletText("Fabricator: %s", fabricatorOperational ? "operational" : "not operational");
    ImGui::TextWrapped("Objective: %s", objective.c_str());
}

#endif

using launcher_support::AcceptPendingLanlinePeer;
using launcher_support::ApplyLanlineSnapshotToLauncher;
using launcher_support::BuildLanlineRoster;
using launcher_support::BuildLanlineSessionId;
using launcher_support::CachedLanlineDiagnostics;
using launcher_support::ClampArrayIndex;
using launcher_support::ClampIndex;
using launcher_support::ClearLanlinePeerLink;
using launcher_support::CountJoinableSessions;
using launcher_support::DiscoverWorlds;
using launcher_support::DrawSessionSummary;
using launcher_support::FindFirstJoinableSessionIndex;
using launcher_support::JoinabilityLabel;
using launcher_support::JoinabilityReason;
using launcher_support::LauncherDataCache;
using launcher_support::LauncherState;
using launcher_support::PrepareSelectedCharacter;
using launcher_support::RefreshLauncherData;
using launcher_support::ReserveLanlinePeerSlot;
using launcher_support::SaveLanlineRosterState;
using launcher_support::SelectedWorldPath;
using launcher_support::SelectedJoinTarget;
using launcher_support::SetLanlineMatchStartArmed;
using launcher_support::ToggleLanlinePlayerReady;
using launcher_support::TryLaunchSiblingExecutable;

}  // namespace

int main() {
    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1280, 820, "BunkerLauncher", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    bunker::EnsureProjectDirectories();
    bunker::SessionProfile sessionProfile;
    const auto profilePath = bunker::DefaultSessionProfilePath();
    if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
        sessionProfile = bunker::MakeDefaultSessionProfile();
        bunker::SaveProfileAtomically(sessionProfile, profilePath);
    }
    bunker::NormalizeSessionProfile(sessionProfile);

    LauncherState launcherState;
    bunker::LanlineServicesState lanlineServices = bunker::MakeDefaultLanlineServicesState(std::time(nullptr));
    bunker::LanlineServicesSave lanlineSave{};
    if (bunker::LoadLanlineServicesSave(bunker::DefaultLanlineServicesSavePath(), lanlineSave)) {
        lanlineServices = bunker::MakeLanlineServicesStateFromSave(lanlineSave, static_cast<std::int64_t>(std::time(nullptr)));
    }
    bunker::ApplyLanlineServicesProfileSnapshot(lanlineServices, sessionProfile.lanlineServices);
    std::snprintf(launcherState.login, sizeof(launcherState.login), "%s", sessionProfile.account.username.c_str());
    const char* characters[] = {"Scout", "Mechanic", "Commander"};
    const char* sessionModes[] = {"Solo", "LAN Host", "LAN Client"};
    LauncherDataCache launcherData;
    RefreshLauncherData(launcherData, launcherState, sessionProfile);
    launcherState.previousSessionModeIndex = launcherState.sessionModeIndex;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        const double now = glfwGetTime();
        if ((now - launcherData.lastRefreshTime) >= 2.0) {
            RefreshLauncherData(launcherData, launcherState, sessionProfile);
        }
        launcherState.selectedCharacter = ClampArrayIndex(launcherState.selectedCharacter, characters);
        launcherState.sessionModeIndex = ClampArrayIndex(launcherState.sessionModeIndex, sessionModes);
        launcherState.selectedWorldIndex = ClampIndex(launcherState.selectedWorldIndex, static_cast<int>(launcherData.worlds.size()));
        launcherState.selectedLanlineSnapshot = launcherData.knownLanlineSessions.empty()
            ? -1
            : ClampIndex(launcherState.selectedLanlineSnapshot, static_cast<int>(launcherData.knownLanlineSessions.size()));
        const bool enteredLanClientMode =
            launcherState.previousSessionModeIndex != static_cast<int>(bunker::SessionMode::LanClient) &&
            launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::LanClient);
        if (enteredLanClientMode) {
            const int firstJoinableIndex = FindFirstJoinableSessionIndex(launcherData.knownLanlineSessions);
            if (firstJoinableIndex >= 0) {
                launcherState.selectedLanlineSnapshot = firstJoinableIndex;
                launcherState.statusText = "Focused first joinable Lanline host snapshot.";
            } else {
                launcherState.statusText = "No joinable Lanline host snapshots found yet.";
            }
        }
        launcherState.previousSessionModeIndex = launcherState.sessionModeIndex;
        bunker::LanlineSessionState activeLanlineSession;
        const bool hasActiveLanlineSession = bunker::LoadLanlineSessionState(activeLanlineSession);
        const bunker::LanlineSessionState* servicesSession = hasActiveLanlineSession ? &activeLanlineSession : nullptr;
        if (launcherState.selectedLanlineSnapshot >= 0 &&
            launcherState.selectedLanlineSnapshot < static_cast<int>(launcherData.knownLanlineSessions.size())) {
            servicesSession = &launcherData.knownLanlineSessions[static_cast<std::size_t>(launcherState.selectedLanlineSnapshot)];
        }
        const auto* servicesWorldState = bunker::FindWorldFieldState(sessionProfile, sessionProfile.selectedWorld);
        const auto servicesUnlockState = bunker::BuildServicesUnlockState(sessionProfile, servicesWorldState);
        bunker::SyncLanlineServicesPresence(lanlineServices, servicesSession, servicesUnlockState);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glViewport(0, 0, 1280, 820);
        glClearColor(0.05f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::SetNextWindowPos(ImVec2(18.0f, 18.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(420.0f, 784.0f), ImGuiCond_Always);
        ImGui::Begin("Access Console", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Bunker Protocol Launcher");
        ImGui::TextDisabled("Required entry point for BunkerGame");
        ImGui::Separator();

        ImGui::InputText("Login", launcherState.login, IM_ARRAYSIZE(launcherState.login));
        ImGui::InputText("Password", launcherState.password, IM_ARRAYSIZE(launcherState.password), ImGuiInputTextFlags_Password);
        if (ImGui::Button("Authorize", ImVec2(-1.0f, 0.0f))) {
            launcherState.loggedIn = std::string(launcherState.login).size() >= 3 && std::string(launcherState.password).size() >= 3;
            launcherState.authFailed = !launcherState.loggedIn;
            launcherState.statusText = launcherState.loggedIn ? "Authorization accepted." : "Authorization failed.";
        }
        if (launcherState.authFailed) {
            ImGui::TextColored(ImVec4(0.92f, 0.28f, 0.24f, 1.0f), "Prototype auth requires 3+ chars.");
        }

        ImGui::Separator();
        ImGui::Text("Session Setup");
        ImGui::Combo("Character", &launcherState.selectedCharacter, characters, IM_ARRAYSIZE(characters));
        ImGui::Combo("Mode", &launcherState.sessionModeIndex, sessionModes, IM_ARRAYSIZE(sessionModes));
        ImGui::BeginDisabled(launcherData.worldLabels.empty());
        ImGui::Combo("World", &launcherState.selectedWorldIndex, launcherData.worldLabels.data(), static_cast<int>(launcherData.worldLabels.size()));
        ImGui::EndDisabled();
        const auto* joinTarget = SelectedJoinTarget(launcherState, launcherData.knownLanlineSessions);
        ImGui::Text("Profile: %s", profilePath.filename().string().c_str());
        ImGui::Text("Current flow: %s", bunker::ToString(bunker::AppFlowState::Launcher));
        if (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::LanClient)) {
            const int joinableCount = CountJoinableSessions(launcherData.knownLanlineSessions);
            ImGui::Text("Joinable hosts: %d", joinableCount);
        }
        if (joinTarget != nullptr) {
            ImGui::TextWrapped("Join target: %s | %s | %s",
                joinTarget->sessionId.c_str(),
                joinTarget->activeActor.empty() ? "host unresolved" : joinTarget->activeActor.c_str(),
                joinTarget->hostEndpoint.c_str());
        } else if (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::LanClient) &&
            launcherState.selectedLanlineSnapshot >= 0 &&
            launcherState.selectedLanlineSnapshot < static_cast<int>(launcherData.knownLanlineSessions.size())) {
            const auto& selectedSnapshot = launcherData.knownLanlineSessions[static_cast<std::size_t>(launcherState.selectedLanlineSnapshot)];
            ImGui::TextWrapped("Selected snapshot is not joinable: %s", JoinabilityReason(selectedSnapshot).c_str());
            const int firstJoinableIndex = FindFirstJoinableSessionIndex(launcherData.knownLanlineSessions);
            if (firstJoinableIndex >= 0 && ImGui::Button("Select First Joinable Host", ImVec2(-1.0f, 0.0f))) {
                launcherState.selectedLanlineSnapshot = firstJoinableIndex;
                launcherState.statusText = "Focused first joinable Lanline host snapshot.";
            }
        }

        if (launcherState.sessionModeIndex != static_cast<int>(bunker::SessionMode::Solo)) {
            ImGui::Separator();
            ImGui::Text("LAN Session");
            ImGui::BeginDisabled(joinTarget != nullptr);
            ImGui::InputText("Host / IP", launcherState.lanHost, IM_ARRAYSIZE(launcherState.lanHost));
            ImGui::InputText("Port", launcherState.lanPort, IM_ARRAYSIZE(launcherState.lanPort));
            ImGui::EndDisabled();
            ImGui::TextDisabled("Lanline - optime diagnostics are now live against session snapshots.");
        }
        if (ImGui::Button("Refresh Worlds / Lanline", ImVec2(-1.0f, 28.0f))) {
            RefreshLauncherData(launcherData, launcherState, sessionProfile);
            launcherState.statusText = "Launcher data refreshed.";
        }

        ImGui::Separator();
        ImGui::TextWrapped("%s", launcherState.statusText.c_str());
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(456.0f, 18.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(806.0f, 784.0f), ImGuiCond_Always);
        ImGui::Begin("Operations Deck", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Project Pillars");
        ImGui::BulletText("Launcher -> Game runtime");
        ImGui::BulletText("Editor remains separate and optional for player builds");
        ImGui::BulletText("LAN is prepared in the launcher, not fully shipped yet");
        ImGui::BulletText("Lanline - optime: LAN-first co-op without forced platform auth");
        ImGui::BulletText("Game target: bunker survival, heavy machines, recovery loops");
        ImGui::Separator();

        DrawSessionSummary(
            sessionProfile,
            characters[launcherState.selectedCharacter],
            sessionModes[launcherState.sessionModeIndex],
            SelectedWorldPath(launcherData.worlds, launcherState.selectedWorldIndex));

        ImGui::Separator();
        ImGui::Text("Launch Actions");
        ImGui::BeginDisabled(!launcherState.loggedIn || launcherData.worlds.empty());
        if (ImGui::Button("Play BunkerGame", ImVec2(250.0f, 48.0f))) {
            const auto selectedWorld = SelectedWorldPath(launcherData.worlds, launcherState.selectedWorldIndex);
            PrepareSelectedCharacter(sessionProfile, launcherState, characters, IM_ARRAYSIZE(characters));
            sessionProfile.sessionMode = sessionModes[launcherState.sessionModeIndex];
            sessionProfile.selectedWorld = joinTarget != nullptr
                ? bunker::NormalizeWorldReference(joinTarget->worldName)
                : bunker::NormalizeWorldReference(selectedWorld.string());
            bunker::SaveProfileAtomically(sessionProfile, profilePath);
            const auto launchSession = SaveLanlineRosterState(
                launcherState,
                characters[launcherState.selectedCharacter],
                selectedWorld,
                joinTarget);
            bunker::LaunchTicketInfo launchTicket;
            launchTicket.accountId = sessionProfile.account.accountId;
            launchTicket.sessionMode = sessionProfile.sessionMode;
            launchTicket.characterName = sessionProfile.character.displayName;
            launchTicket.selectedWorld = sessionProfile.selectedWorld;
            launchTicket.lanlineSessionId = launchSession.sessionId;
            launchTicket.hostEndpoint = launchSession.hostEndpoint;
            if (!bunker::IssueLaunchTicket(launchTicket)) {
                launcherState.statusText = "Failed to create launcher ticket.";
            } else {
                TryLaunchSiblingExecutable("BunkerGame.exe", launcherState.statusText);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Open BunkerEditor", ImVec2(250.0f, 48.0f))) {
            const auto selectedWorld = SelectedWorldPath(launcherData.worlds, launcherState.selectedWorldIndex);
            PrepareSelectedCharacter(sessionProfile, launcherState, characters, IM_ARRAYSIZE(characters));
            sessionProfile.sessionMode = sessionModes[launcherState.sessionModeIndex];
            sessionProfile.selectedWorld = bunker::NormalizeWorldReference(selectedWorld.string());
            bunker::SaveProfileAtomically(sessionProfile, profilePath);
            SaveLanlineRosterState(launcherState, characters[launcherState.selectedCharacter], selectedWorld, nullptr);
            TryLaunchSiblingExecutable("BunkerEditor.exe", launcherState.statusText);
        }
        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::TextWrapped("Launcher remains the required path into BunkerGame. BunkerEditor is a separate production tool and not required for players.");
        ImGui::Separator();
        ImGui::Text("Owned Vehicles");
        for (const auto& vehicle : sessionProfile.ownedVehicles) {
            ImGui::BulletText("%s | %s | dur %.0f | charge %.0f",
                vehicle.displayName.c_str(),
                bunker::ToString(vehicle.type),
                vehicle.durability,
                vehicle.fuelOrCharge);
        }
        ImGui::Separator();
        ImGui::Text("Lanline - optime");
        ImGui::TextDisabled("LAN-first session roster without Steam/Xbox auth");
        for (const auto& rosterEntry : BuildLanlineRoster(launcherState, characters[launcherState.selectedCharacter])) {
            ImGui::BulletText("%s", rosterEntry.c_str());
        }
        ImGui::Text("Session ID: %s", BuildLanlineSessionId(launcherState, SelectedWorldPath(launcherData.worlds, launcherState.selectedWorldIndex)).c_str());
        ImGui::TextWrapped("Goal: players should not lose each other inside a LAN session even before full discovery/chat/ping is finished.");
        ImGui::Separator();
        ImGui::Text("Known Lanline Sessions");
        if (launcherData.knownLanlineSessions.empty()) {
            ImGui::TextDisabled("No session snapshots discovered yet.");
        } else {
            for (int index = 0; index < static_cast<int>(launcherData.knownLanlineSessions.size()); ++index) {
                const auto& session = launcherData.knownLanlineSessions[static_cast<std::size_t>(index)];
                const bool selected = launcherState.selectedLanlineSnapshot == index;
                if (ImGui::Selectable((session.sessionId + "##lanline_snapshot").c_str(), selected)) {
                    launcherState.selectedLanlineSnapshot = index;
                }
                ImGui::TextDisabled("%s | %s | %s | slots %d/%d | %s | %s",
                    session.mode.c_str(),
                    JoinabilityLabel(session),
                    session.lifecycleStage.c_str(),
                    bunker::OccupiedLanlineSessionSlots(session),
                    bunker::MaxLanlineSessionSlots(session),
                    session.worldName.c_str(),
                    session.hostEndpoint.c_str());
            }
            if (launcherState.selectedLanlineSnapshot >= 0 &&
                launcherState.selectedLanlineSnapshot < static_cast<int>(launcherData.knownLanlineSessions.size())) {
                const auto& selectedSnapshot = launcherData.knownLanlineSessions[static_cast<std::size_t>(launcherState.selectedLanlineSnapshot)];
                const auto& diagnostics = CachedLanlineDiagnostics(
                    selectedSnapshot,
                    bunker::NormalizeWorldReference(SelectedWorldPath(launcherData.worlds, launcherState.selectedWorldIndex).string()));
                ImGui::Separator();
                ImGui::Text("Selected Session Diagnostics");
                ImGui::BulletText("Lifecycle: %s", selectedSnapshot.lifecycleStage.c_str());
                ImGui::BulletText("Joinability: %s", JoinabilityLabel(selectedSnapshot));
                ImGui::BulletText("Slots: %d / %d occupied",
                    bunker::OccupiedLanlineSessionSlots(selectedSnapshot),
                    bunker::MaxLanlineSessionSlots(selectedSnapshot));
                ImGui::BulletText("Open slots: %d", bunker::AvailableLanlineSessionSlots(selectedSnapshot));
                ImGui::BulletText("Reserved slots: %d", bunker::ReservedLanlineSessionSlots(selectedSnapshot));
                ImGui::BulletText("Pending slots: %d", bunker::PendingLanlineSessionSlots(selectedSnapshot));
                ImGui::BulletText("Accepted client slots: %d", bunker::AcceptedLanlineSessionSlots(selectedSnapshot));
                ImGui::BulletText("Ready seats: %d", bunker::ReadyLanlineSessionSlots(selectedSnapshot));
                ImGui::BulletText("Active actor: %s", selectedSnapshot.activeActor.c_str());
                ImGui::BulletText("Pending peer: %s", selectedSnapshot.pendingPeer.empty() ? "none" : selectedSnapshot.pendingPeer.c_str());
                ImGui::BulletText("Connected peer: %s", selectedSnapshot.connectedPeer.empty() ? "none" : selectedSnapshot.connectedPeer.c_str());
                ImGui::BulletText("Host: %s", diagnostics.hostReachable ? "reachable" : "offline/unreachable");
                ImGui::BulletText("Ping: %s",
                    diagnostics.pingMs >= 0 ? (std::to_string(diagnostics.pingMs) + " ms").c_str() : "n/a");
                ImGui::BulletText("World match: %s", diagnostics.worldMatch ? "yes" : "no");
                ImGui::BulletText("Snapshot freshness: %s", diagnostics.snapshotFresh ? "fresh" : "stale");
                ImGui::BulletText("Presence: %d / %d online", diagnostics.onlinePlayers, diagnostics.totalPlayers);
                if (!diagnostics.lastError.empty()) {
                    ImGui::TextDisabled("Details: %s", diagnostics.lastError.c_str());
                }
                ImGui::TextWrapped("%s", JoinabilityReason(selectedSnapshot).c_str());
                ImGui::Separator();
                ImGui::Text("Lobby Seats");
                for (std::size_t seatIndex = 0; seatIndex < selectedSnapshot.players.size(); ++seatIndex) {
                    const auto& playerEntry = selectedSnapshot.players[seatIndex];
                    ImGui::PushID(static_cast<int>(seatIndex));
                    ImGui::Text("%s | %s | %s | %s",
                        playerEntry.displayName.c_str(),
                        playerEntry.role.c_str(),
                        bunker::LanlineSlotStateLabel(playerEntry),
                        bunker::LanlineReadyLabel(playerEntry));
                    if (bunker::IsLanlineReadyEligibleSlot(playerEntry)) {
                        ImGui::SameLine();
                        bunker::LanlineSessionState readySnapshot = selectedSnapshot;
                        if (ImGui::SmallButton(playerEntry.ready ? "Mark Not Ready" : "Mark Ready")) {
                            if (ToggleLanlinePlayerReady(readySnapshot, playerEntry.displayName)) {
                                launcherState.statusText = "Updated Lanline ready state for " + playerEntry.displayName;
                            }
                        }
                    }
                    ImGui::PopID();
                }
                if (ImGui::Button("Use Selected Lanline Session", ImVec2(-1.0f, 32.0f))) {
                    ApplyLanlineSnapshotToLauncher(selectedSnapshot, launcherState, launcherData.worlds);
                    launcherState.statusText = "Applied Lanline session snapshot: " + selectedSnapshot.sessionId;
                }
                if (bunker::IsJoinableLanlineSession(selectedSnapshot)) {
                    if (ImGui::Button("Join Selected Lanline Session", ImVec2(-1.0f, 32.0f))) {
                        bunker::LanlineSessionState reservedSnapshot = selectedSnapshot;
                        ReserveLanlinePeerSlot(reservedSnapshot, characters[launcherState.selectedCharacter]);
                        ApplyLanlineSnapshotToLauncher(selectedSnapshot, launcherState, launcherData.worlds);
                        launcherState.sessionModeIndex = static_cast<int>(bunker::SessionMode::LanClient);
                        launcherState.statusText = "Locked launcher to join target: " + selectedSnapshot.sessionId;
                    }
                }
                bunker::LanlineSessionState armedSnapshot = selectedSnapshot;
                if (selectedSnapshot.mode == "LAN Host") {
                    if (bunker::IsLanlineMatchStartReady(selectedSnapshot)) {
                        if (ImGui::Button("Arm Session Start", ImVec2(-1.0f, 32.0f))) {
                            if (SetLanlineMatchStartArmed(armedSnapshot, true)) {
                                launcherState.statusText = "Lanline match start armed for session: " + selectedSnapshot.sessionId;
                            }
                        }
                    } else if (selectedSnapshot.lifecycleStage == "MatchStartReady") {
                        if (ImGui::Button("Return Session To Lobby", ImVec2(-1.0f, 32.0f))) {
                            if (SetLanlineMatchStartArmed(armedSnapshot, false)) {
                                launcherState.statusText = "Lanline session returned to lobby state.";
                            }
                        }
                    }
                }
                bunker::LanlineSessionState editableSnapshot = selectedSnapshot;
                if (selectedSnapshot.mode == "LAN Host" && !selectedSnapshot.pendingPeer.empty()) {
                    if (ImGui::Button("Accept Pending Peer", ImVec2(-1.0f, 32.0f))) {
                        if (AcceptPendingLanlinePeer(editableSnapshot)) {
                            launcherState.statusText = "Accepted Lanline peer for session: " + editableSnapshot.sessionId;
                        }
                    }
                }
                if (!selectedSnapshot.pendingPeer.empty() || !selectedSnapshot.connectedPeer.empty()) {
                    if (ImGui::Button("Clear Peer Link", ImVec2(-1.0f, 32.0f))) {
                        if (ClearLanlinePeerLink(editableSnapshot)) {
                            launcherState.statusText = "Cleared Lanline peer link for session: " + editableSnapshot.sessionId;
                        }
                    }
                }
            }
        }
        ImGui::Separator();
        ImGui::Text("World Assets");
        for (const auto& world : launcherData.worlds) {
            ImGui::BulletText("%s", world.string().c_str());
        }
        ImGui::Separator();
        ImGui::Text("Lanline Services");
        bunker::SessionProfile previewProfile = sessionProfile;
        previewProfile.selectedWorld = bunker::NormalizeWorldReference(SelectedWorldPath(launcherData.worlds, launcherState.selectedWorldIndex).string());
        const auto* previewWorldState = bunker::FindWorldFieldState(previewProfile, previewProfile.selectedWorld);
        const auto servicesUnlock = bunker::BuildServicesUnlockState(previewProfile, previewWorldState);
        bunker::DrawLanlineServicesPanel(lanlineServices, servicesUnlock, static_cast<std::int64_t>(std::time(nullptr)));
        bunker::SyncLanlineServicesProfileSnapshot(sessionProfile.lanlineServices, lanlineServices);
        bunker::SaveLanlineServicesSave(
            bunker::BuildLanlineServicesSave(lanlineServices),
            bunker::DefaultLanlineServicesSavePath());
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    sessionProfile.selectedWorld = bunker::NormalizeWorldReference(SelectedWorldPath(launcherData.worlds, launcherState.selectedWorldIndex).string());
    bunker::SaveProfileAtomically(sessionProfile, profilePath);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
