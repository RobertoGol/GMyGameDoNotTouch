#include <GLFW/glfw3.h>

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../../include/AppPaths.hpp"
#include "../../include/LanlineSession.hpp"
#include "../../include/LaunchSession.hpp"
#include "../../include/SessionFlow.hpp"
#include "../../include/SessionProfiles.hpp"
namespace {

struct LauncherState {
    bool loggedIn = false;
    bool authFailed = false;
    int selectedCharacter = 0;
    int sessionModeIndex = 0;
    int selectedWorldIndex = 0;
    int selectedLanlineSnapshot = -1;
    char login[64] = "wanderer";
    char password[64] = "prototype";
    char lanHost[64] = "127.0.0.1";
    char lanPort[16] = "27015";
    std::string statusText = "System ready. Authorize to continue.";
};

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

void SaveLanlineRosterState(const LauncherState& launcherState, const char* selectedCharacterLabel, const std::filesystem::path& selectedWorld) {
    const std::string worldReference = bunker::NormalizeWorldReference(selectedWorld.string());
    bunker::LanlineSessionState state;
    state.sessionId = BuildLanlineSessionId(launcherState, selectedWorld);
    state.mode = launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::Solo)
        ? "Solo"
        : (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::LanHost) ? "LAN Host" : "LAN Client");
    state.worldName = worldReference;
    state.hostEndpoint = std::string(launcherState.lanHost) + ":" + launcherState.lanPort;
    state.updatedAt.clear();

    if (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::Solo)) {
        state.players.push_back({selectedCharacterLabel, "Local Operator", true});
        state.eventLog.push_back(std::string(selectedCharacterLabel) + " entered solo runtime.");
    } else if (launcherState.sessionModeIndex == static_cast<int>(bunker::SessionMode::LanHost)) {
        state.players.push_back({selectedCharacterLabel, "Host", true});
        state.players.push_back({"LAN slot 2", "Awaiting", false});
        state.players.push_back({"LAN slot 3", "Awaiting", false});
        state.players.push_back({"LAN slot 4", "Awaiting", false});
        state.eventLog.push_back(std::string(selectedCharacterLabel) + " opened a Lanline - optime host session.");
        state.eventLog.push_back(std::string("Host endpoint ready at ") + state.hostEndpoint + ".");
    } else {
        state.players.push_back({selectedCharacterLabel, "Client", true});
        state.players.push_back({std::string("Host @ ") + launcherState.lanHost, "Host", true});
        state.players.push_back({"LAN roster sync", "Pending", false});
        state.eventLog.push_back(std::string(selectedCharacterLabel) + " is attempting to join a Lanline - optime session.");
        state.eventLog.push_back(std::string("Client targeting host ") + state.hostEndpoint + ".");
    }

    bunker::SaveLanlineSessionState(state);
    bunker::SaveLanlineSessionState(state, bunker::LanlineSessionSnapshotPath(state.sessionId));
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
    return worlds;
}

bool TryLaunchSiblingExecutable(const char* executableName, std::string& statusText) {
    const auto candidate = std::filesystem::current_path() / executableName;
    if (!std::filesystem::exists(candidate)) {
        statusText = std::string("Executable not found near launcher: ") + candidate.string();
        return false;
    }

    const std::string command = "\"" + candidate.string() + "\"";
    statusText = "Launching " + candidate.filename().string();
    std::system(command.c_str());
    return true;
}

void PrepareSelectedCharacter(bunker::SessionProfile& sessionProfile, const LauncherState& launcherState, const char* const* characters) {
    const std::size_t idx = static_cast<std::size_t>(launcherState.selectedCharacter);
    sessionProfile.character.displayName = characters[idx];
    sessionProfile.character.characterId = sessionProfile.account.linkedCharacters[idx];
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
        bunker::SaveSessionProfile(sessionProfile, profilePath);
    }
    bunker::NormalizeSessionProfile(sessionProfile);

    LauncherState launcherState;
    std::snprintf(launcherState.login, sizeof(launcherState.login), "%s", sessionProfile.account.username.c_str());
    const char* characters[] = {"Scout", "Mechanic", "Commander"};
    const char* sessionModes[] = {"Solo", "LAN Host", "LAN Client"};
    auto worlds = DiscoverWorlds();
    std::vector<const char*> worldLabels;
    worldLabels.reserve(worlds.size());
    for (const auto& world : worlds) {
        worldLabels.push_back(world.string().c_str());
    }

    for (std::size_t index = 0; index < worlds.size(); ++index) {
        if (bunker::NormalizeWorldReference(worlds[index].string()) == sessionProfile.selectedWorld) {
            launcherState.selectedWorldIndex = static_cast<int>(index);
            break;
        }
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        const auto knownLanlineSessions = bunker::DiscoverLanlineSessionSnapshots();

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
        ImGui::Combo("World", &launcherState.selectedWorldIndex, worldLabels.data(), static_cast<int>(worldLabels.size()));
        ImGui::Text("Profile: %s", profilePath.filename().string().c_str());
        ImGui::Text("Current flow: %s", bunker::ToString(bunker::AppFlowState::Launcher));

        if (launcherState.sessionModeIndex != static_cast<int>(bunker::SessionMode::Solo)) {
            ImGui::Separator();
            ImGui::Text("LAN Session");
            ImGui::InputText("Host / IP", launcherState.lanHost, IM_ARRAYSIZE(launcherState.lanHost));
            ImGui::InputText("Port", launcherState.lanPort, IM_ARRAYSIZE(launcherState.lanPort));
            ImGui::TextDisabled("Discovery/ping sync is architectural for now.");
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
            worlds[static_cast<std::size_t>(launcherState.selectedWorldIndex)]);

        ImGui::Separator();
        ImGui::Text("Launch Actions");
        ImGui::BeginDisabled(!launcherState.loggedIn);
        if (ImGui::Button("Play BunkerGame", ImVec2(250.0f, 48.0f))) {
            PrepareSelectedCharacter(sessionProfile, launcherState, characters);
            sessionProfile.sessionMode = sessionModes[launcherState.sessionModeIndex];
            sessionProfile.selectedWorld = bunker::NormalizeWorldReference(worlds[static_cast<std::size_t>(launcherState.selectedWorldIndex)].string());
            bunker::SaveSessionProfile(sessionProfile, profilePath);
            SaveLanlineRosterState(launcherState, characters[launcherState.selectedCharacter], worlds[static_cast<std::size_t>(launcherState.selectedWorldIndex)]);
            if (!bunker::IssueLaunchTicket(sessionProfile.account.accountId, sessionProfile.sessionMode)) {
                launcherState.statusText = "Failed to create launcher ticket.";
            } else {
                TryLaunchSiblingExecutable("BunkerGame.exe", launcherState.statusText);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Open BunkerEditor", ImVec2(250.0f, 48.0f))) {
            PrepareSelectedCharacter(sessionProfile, launcherState, characters);
            sessionProfile.sessionMode = sessionModes[launcherState.sessionModeIndex];
            sessionProfile.selectedWorld = bunker::NormalizeWorldReference(worlds[static_cast<std::size_t>(launcherState.selectedWorldIndex)].string());
            bunker::SaveSessionProfile(sessionProfile, profilePath);
            SaveLanlineRosterState(launcherState, characters[launcherState.selectedCharacter], worlds[static_cast<std::size_t>(launcherState.selectedWorldIndex)]);
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
        ImGui::Text("Session ID: %s", BuildLanlineSessionId(launcherState, worlds[static_cast<std::size_t>(launcherState.selectedWorldIndex)]).c_str());
        ImGui::TextWrapped("Goal: players should not lose each other inside a LAN session even before full discovery/chat/ping is finished.");
        ImGui::Separator();
        ImGui::Text("Known Lanline Sessions");
        if (knownLanlineSessions.empty()) {
            ImGui::TextDisabled("No session snapshots discovered yet.");
        } else {
            for (int index = 0; index < static_cast<int>(knownLanlineSessions.size()); ++index) {
                const auto& session = knownLanlineSessions[static_cast<std::size_t>(index)];
                const bool selected = launcherState.selectedLanlineSnapshot == index;
                if (ImGui::Selectable((session.sessionId + "##lanline_snapshot").c_str(), selected)) {
                    launcherState.selectedLanlineSnapshot = index;
                }
                ImGui::TextDisabled("%s | %s | %s", session.mode.c_str(), session.worldName.c_str(), session.hostEndpoint.c_str());
            }
            if (launcherState.selectedLanlineSnapshot >= 0 &&
                launcherState.selectedLanlineSnapshot < static_cast<int>(knownLanlineSessions.size())) {
                const auto& selectedSnapshot = knownLanlineSessions[static_cast<std::size_t>(launcherState.selectedLanlineSnapshot)];
                if (ImGui::Button("Use Selected Lanline Session", ImVec2(-1.0f, 32.0f))) {
                    ApplyLanlineSnapshotToLauncher(selectedSnapshot, launcherState, worlds);
                    launcherState.statusText = "Applied Lanline session snapshot: " + selectedSnapshot.sessionId;
                }
            }
        }
        ImGui::Separator();
        ImGui::Text("World Assets");
        for (const auto& world : worlds) {
            ImGui::BulletText("%s", world.string().c_str());
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    bunker::SaveSessionProfile(sessionProfile, profilePath);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
