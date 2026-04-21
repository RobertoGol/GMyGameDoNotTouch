#include "LauncherSupport.hpp"

#include <algorithm>
#include <array>
#include <cstdio>

#include <GLFW/glfw3.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include "imgui.h"

#include "../../include/AppPaths.hpp"
#include "../../include/AtomicPersistence.hpp"
#include "../../include/BuildAnnouncement.hpp"
#include "../../include/LanlineLobbyLogic.hpp"
#include "../../include/LanlineServices.hpp"
#include "../../include/SessionFlow.hpp"

namespace launcher_support {

int ClampIndex(int value, int itemCount) {
    if (itemCount <= 0) {
        return 0;
    }
    return std::clamp(value, 0, itemCount - 1);
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
    bool ready) {
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

    roster.push_back("Host [Pending Link]");
    roster.push_back(std::string(selectedCharacterLabel) + " [Joining]");
    return roster;
}

std::string BuildLanlineSessionId(const LauncherState& launcherState, const std::filesystem::path& selectedWorld) {
    return std::string(launcherState.login) + "::" +
        bunker::NormalizeWorldReference(selectedWorld.string()) + "::" +
        std::string(launcherState.lanHost) + ":" + launcherState.lanPort;
}

const char* JoinabilityLabel(const bunker::LanlineSessionState& session) {
    if (session.mode != "LAN Host") {
        return "non-host";
    }
    if (!session.pendingPeer.empty()) {
        return "pending";
    }
    if (!session.connectedPeer.empty()) {
        return "linked";
    }
    if (bunker::AvailableLanlineSessionSlots(session) <= 0) {
        return "full";
    }
    return "joinable";
}

std::string JoinabilityReason(const bunker::LanlineSessionState& session) {
    if (session.mode != "LAN Host") {
        return "Only LAN Host sessions can accept clients.";
    }
    if (!session.pendingPeer.empty()) {
        return "Host already has a pending peer waiting for approval.";
    }
    if (!session.connectedPeer.empty()) {
        return "Host already has an active linked client.";
    }
    if (bunker::AvailableLanlineSessionSlots(session) <= 0) {
        return "Lobby has no open Lanline seats.";
    }
    return "Ready for client join.";
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
    return static_cast<int>(std::count_if(knownLanlineSessions.begin(), knownLanlineSessions.end(),
        [](const bunker::LanlineSessionState& session) {
            return bunker::IsJoinableLanlineSession(session);
        }));
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
    std::snprintf(launcherState.lanHost, sizeof(launcherState.lanHost), "%s", snapshot.hostEndpoint.c_str());
    if (const auto separator = snapshot.hostEndpoint.find(':'); separator != std::string::npos) {
        std::snprintf(launcherState.lanHost, sizeof(launcherState.lanHost), "%s", snapshot.hostEndpoint.substr(0, separator).c_str());
        std::snprintf(launcherState.lanPort, sizeof(launcherState.lanPort), "%s", snapshot.hostEndpoint.substr(separator + 1).c_str());
    }
    for (std::size_t index = 0; index < worlds.size(); ++index) {
        if (bunker::NormalizeWorldReference(worlds[index].string()) == bunker::NormalizeWorldReference(snapshot.worldName)) {
            launcherState.selectedWorldIndex = static_cast<int>(index);
            break;
        }
    }
}

void SaveLanlineSnapshotAndMaybeActive(const bunker::LanlineSessionState& snapshot) {
    bunker::SaveLanlineSessionState(snapshot, bunker::LanlineSessionSnapshotPath(snapshot.sessionId));
    bunker::SaveLanlineSessionState(snapshot);
}

bool AcceptPendingLanlinePeer(bunker::LanlineSessionState& session) {
    if (session.pendingPeer.empty()) {
        return false;
    }
    session.connectedPeer = session.pendingPeer;
    session.pendingPeer.clear();
    session.lifecycleStage = "HostClientAccepted";
    AcceptLanlineLobbySlot(session, session.connectedPeer);
    session.eventLog.push_back("Launcher accepted pending peer " + session.connectedPeer + ".");
    TrimLanlineEventLog(session, 12);
    SaveLanlineSnapshotAndMaybeActive(session);
    return true;
}

bool ClearLanlinePeerLink(bunker::LanlineSessionState& session) {
    if (session.connectedPeer.empty() && session.pendingPeer.empty()) {
        return false;
    }
    const std::string previousPeer = !session.connectedPeer.empty() ? session.connectedPeer : session.pendingPeer;
    ClearLanlineLobbyPeerSlots(session, previousPeer);
    session.connectedPeer.clear();
    session.pendingPeer.clear();
    session.lifecycleStage = session.mode == "LAN Host" ? "HostAwaitingPeer" : "Idle";
    session.eventLog.push_back("Launcher cleared peer link for " + previousPeer + ".");
    TrimLanlineEventLog(session, 12);
    SaveLanlineSnapshotAndMaybeActive(session);
    return true;
}

bool ReserveLanlinePeerSlot(bunker::LanlineSessionState& session, const std::string& peerName) {
    if (peerName.empty() || !session.pendingPeer.empty() || !session.connectedPeer.empty()) {
        return false;
    }
    session.pendingPeer = peerName;
    session.lifecycleStage = "HostPendingPeer";
    ReserveLanlineLobbySlot(session, peerName);
    session.eventLog.push_back("Launcher reserved a Lanline seat for " + peerName + ".");
    TrimLanlineEventLog(session, 12);
    SaveLanlineSnapshotAndMaybeActive(session);
    return true;
}

bool ToggleLanlinePlayerReady(bunker::LanlineSessionState& session, const std::string& displayName) {
    for (auto& player : session.players) {
        if (player.displayName == displayName) {
            player.ready = !player.ready;
            session.eventLog.push_back(displayName + (player.ready ? " is ready." : " is no longer ready."));
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
        : "Host returned Lanline session to lobby standby.");
    TrimLanlineEventLog(session, 12);
    SaveLanlineSnapshotAndMaybeActive(session);
    return true;
}

std::vector<std::filesystem::path> DiscoverWorlds() {
    std::vector<std::filesystem::path> worlds;
    bunker::EnsureProjectDirectories();
    for (const auto& entry : std::filesystem::directory_iterator(bunker::WorldDirectory())) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() == ".bwld") {
            worlds.push_back(entry.path().filename());
        }
    }
    if (worlds.empty()) {
        worlds.push_back(bunker::DefaultWorldPath().filename());
    }
    std::sort(worlds.begin(), worlds.end());
    return worlds;
}

bool TryLaunchSiblingExecutable(const char* executableName, std::string& statusText) {
    const auto executablePath = FindSiblingExecutable(executableName);
    if (executablePath.empty()) {
        statusText = std::string("Unable to locate ") + executableName + ".";
        return false;
    }

    STARTUPINFOA startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo{};
    std::string commandLine = "\"" + executablePath.string() + "\"";
    const std::string workingDirectory = executablePath.parent_path().string();
    const BOOL created = CreateProcessA(
        executablePath.string().c_str(),
        commandLine.data(),
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        workingDirectory.c_str(),
        &startupInfo,
        &processInfo);
    if (!created) {
        statusText = std::string("Launch failed for ") + executableName + ".";
        return false;
    }

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    statusText = std::string("Launch request issued for ") + executableName + ".";
    return true;
}

void PrepareSelectedCharacter(bunker::SessionProfile& sessionProfile,
    const LauncherState& launcherState,
    const char* const* characters,
    int characterCount) {
    if (characterCount <= 0) {
        return;
    }
    const int selectedIndex = ClampIndex(launcherState.selectedCharacter, characterCount);
    sessionProfile.character.displayName = characters[selectedIndex];
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(sessionProfile.account.linkedCharacters.size())) {
        sessionProfile.character.characterId = sessionProfile.account.linkedCharacters[static_cast<std::size_t>(selectedIndex)];
    }
}

std::string BuildLauncherObjectivePreview(const bunker::SessionProfile& sessionProfile, const bunker::WorldFieldState* worldState) {
    std::string preview = sessionProfile.story.exitedBunker
        ? "Secure the recovery backbone and expand beyond Shelter 17."
        : "Launch from cryo, recover the Pip-Pad, and restore the tank link.";
    preview += sessionProfile.story.relayRecovered ? " | Relay recovered" : " | Relay missing";
    if (worldState != nullptr && worldState->industrialGateUnlocked) {
        preview += " | Industrial gate open";
    }
    return preview;
}

const bunker::LanlineDiagnostics& CachedLanlineDiagnostics(const bunker::LanlineSessionState& session, std::string_view runtimeWorldName) {
    static std::vector<std::pair<std::string, bunker::LanlineDiagnostics>> cache;
    const std::string cacheKey = session.sessionId + "::" + std::string(runtimeWorldName);
    auto cached = std::find_if(cache.begin(), cache.end(), [&](const auto& entry) { return entry.first == cacheKey; });
    if (cached == cache.end()) {
        cache.push_back({cacheKey, bunker::ProbeLanlineHost(session, runtimeWorldName)});
        return cache.back().second;
    }
    cached->second = bunker::ProbeLanlineHost(session, runtimeWorldName);
    return cached->second;
}

float DrawLauncherAnnouncementWidget(
    bunker::SessionProfile& sessionProfile,
    const std::filesystem::path& profilePath,
    LauncherState& launcherState) {
    const auto& announcement = bunker::CurrentBuildAnnouncement();
    const bool shouldShow = bunker::ShouldShowBuildAnnouncement(
        sessionProfile.launcherAnnouncements.lastSeenBuildNumber,
        sessionProfile.launcherAnnouncements.lastSeenAnnouncementId);
    if (!shouldShow) {
        launcherState.activeAnnouncementId.clear();
        launcherState.announcementDetailsOpen = false;
        return 0.0f;
    }

    if (launcherState.activeAnnouncementId != announcement.announcementId) {
        launcherState.activeAnnouncementId = announcement.announcementId;
        launcherState.announcementDetailsOpen = false;
    }

    ImGui::SetNextWindowPos(ImVec2(18.0f, 18.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.96f);
    ImGui::Begin(
        "Node Notice",
        nullptr,
        ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextColored(ImVec4(0.72f, 0.86f, 0.52f, 1.0f), "%s", announcement.title);
    ImGui::TextWrapped("%s", announcement.summary);
    ImGui::TextDisabled("Version %s | %s | %s",
        announcement.versionLabel,
        announcement.dateLabel,
        announcement.buildId);
    if (announcement.details[0] != '\0') {
        if (ImGui::Button(launcherState.announcementDetailsOpen ? "Hide Details" : "Details")) {
            launcherState.announcementDetailsOpen = !launcherState.announcementDetailsOpen;
        }
        ImGui::SameLine();
    }
    if (ImGui::Button("Dismiss")) {
        sessionProfile.launcherAnnouncements.lastSeenBuildNumber = announcement.buildNumber;
        sessionProfile.launcherAnnouncements.lastSeenAnnouncementId = announcement.announcementId;
        sessionProfile.launcherAnnouncements.lastSeenVersionLabel = announcement.versionLabel;
        const auto saveStatus = bunker::SaveProfileAtomically(sessionProfile, profilePath);
        launcherState.statusText = saveStatus.ok
            ? "Launcher notice dismissed."
            : "Failed to persist launcher notice state: " + saveStatus.message;
        launcherState.announcementDetailsOpen = false;
    }
    if (launcherState.announcementDetailsOpen && announcement.details[0] != '\0') {
        ImGui::Separator();
        ImGui::TextWrapped("%s", announcement.details);
    }
    const float heightWithGap = ImGui::GetWindowSize().y + 12.0f;
    ImGui::End();
    return heightWithGap;
}

void DrawSessionSummary(const bunker::SessionProfile& sessionProfile, const char* selectedCharacterLabel, const char* selectedModeLabel, const std::filesystem::path& selectedWorld) {
    const auto* worldState = bunker::FindWorldFieldState(sessionProfile, sessionProfile.selectedWorld);
    ImGui::Text("Session Summary");
    ImGui::BulletText("Operator: %s", selectedCharacterLabel);
    ImGui::BulletText("Mode: %s", selectedModeLabel);
    ImGui::BulletText("World: %s", selectedWorld.string().c_str());
    ImGui::BulletText("Level %d | XP %d", sessionProfile.character.level, sessionProfile.character.experience);
    ImGui::BulletText("Relay Credits: %d", sessionProfile.lanlineServices.relayCredits);
    const std::string objective = BuildLauncherObjectivePreview(sessionProfile, worldState);
    ImGui::TextWrapped("Objective: %s", objective.c_str());
}

}  // namespace launcher_support
