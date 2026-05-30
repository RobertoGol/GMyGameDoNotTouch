#include "../Dx11Renderer.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <filesystem>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../include/AppPaths.hpp"
#include "../include/AtomicPersistence.hpp"
#include "../include/GameExecution.hpp"
#include "../include/GameRuntime.hpp"
#include "../include/LanlineLobbyLogic.hpp"
#include "../include/LanlineServices.hpp"
#include "../include/LanlineSession.hpp"
#include "../include/LaunchSession.hpp"
#include "../include/WorldEvents.hpp"
#include "../include/WorldSemanticAuthoring.hpp"



namespace {

struct RuntimeSaveOutcome {
    bool worldSaved = false;
    bool profileSaved = false;
    bool eraserSaved = false;
    std::string message;
};

RuntimeSaveOutcome BuildRuntimeSaveOutcome(
    const bunker::SaveStatus* worldSave,
    const bunker::SaveStatus* profileSave,
    std::string_view worldName,
    bool saveEraser) {
    RuntimeSaveOutcome outcome;

    outcome.worldSaved = (worldSave == nullptr) ? true : worldSave->ok;
    outcome.profileSaved = (profileSave == nullptr) ? true : profileSave->ok;

    if (saveEraser) {
        const auto eraserPath = bunker::StaticEraserPath(worldName);
        outcome.eraserSaved = std::filesystem::exists(eraserPath);
    } else {
        outcome.eraserSaved = true;
    }

    if (!outcome.worldSaved && worldSave != nullptr) {
        outcome.message = "world save failed: " + worldSave->message;
    } else if (!outcome.profileSaved && profileSave != nullptr) {
        outcome.message = "profile save failed: " + profileSave->message;
    } else if (!outcome.eraserSaved) {
        outcome.message = "static eraser save could not be confirmed on disk";
    } else {
        outcome.message = "world/profile save flow completed successfully";
    }

    return outcome;
}

void ReportRuntimeSaveOutcome(const char* label, const RuntimeSaveOutcome& outcome) {
    std::fprintf(stderr,
        "%s: world=%s profile=%s eraser=%s :: %s\n",
        label,
        outcome.worldSaved ? "ok" : "failed",
        outcome.profileSaved ? "ok" : "failed",
        outcome.eraserSaved ? "ok" : "failed",
        outcome.message.c_str());
}

void TrimLanlineEventLog(bunker::LanlineSessionState& state, std::size_t maxEntries) {
    if (state.eventLog.size() > maxEntries) {
        state.eventLog.erase(state.eventLog.begin(), state.eventLog.begin() + static_cast<std::vector<std::string>::difference_type>(state.eventLog.size() - maxEntries));
    }
}

void UpsertLanlinePlayer(bunker::LanlineSessionState& state,
    const std::string& displayName,
    const std::string& role,
    bool online,
    bool ready = false,
    std::string seatAssignment = "on_foot") {
    auto playerIt = std::find_if(state.players.begin(), state.players.end(),
        [&](const bunker::LanlinePlayerEntry& entry) {
            return entry.displayName == displayName;
        });
    if (playerIt == state.players.end()) {
        state.players.push_back({displayName, role, online, ready, std::move(seatAssignment)});
        return;
    }
    playerIt->role = role;
    playerIt->online = online;
    playerIt->ready = ready;
    playerIt->seatAssignment = std::move(seatAssignment);
}

void AcceptLanlineLobbySlot(bunker::LanlineSessionState& state, const std::string& peerName) {
    for (auto& player : state.players) {
        if (player.displayName == peerName) {
            player.role = "Client";
            player.online = true;
            player.ready = false;
            player.seatAssignment = "on_foot";
            return;
        }
    }
    const int slotIndex = bunker::FindFirstAwaitingSlotIndex(state);
    if (slotIndex >= 0) {
        auto& slot = state.players[static_cast<std::size_t>(slotIndex)];
        slot.displayName = peerName;
        slot.role = "Client";
        slot.online = true;
        slot.ready = false;
        slot.seatAssignment = "on_foot";
        return;
    }
    UpsertLanlinePlayer(state, peerName, "Client", true, false);
}

std::string CurrentRuntimeSeatAssignment(const bunker::PlayerState& player) {
    if (!player.insideTank) {
        return "on_foot";
    }
    return player.bt72GunnerSeat ? "gunner" : "pilot";
}

void SyncLanlineRuntimeLaunchState(const bunker::LaunchTicketInfo& launchTicket, const bunker::SessionProfile& sessionProfile) {
    bunker::LanlineSessionState state;
    if (!bunker::LoadLanlineSessionState(state)) {
        state = bunker::LanlineSessionState{};
    }

    if (!launchTicket.lanlineSessionId.empty()) {
        state.sessionId = launchTicket.lanlineSessionId;
    }
    if (!launchTicket.sessionMode.empty()) {
        if (launchTicket.sessionMode == "LAN Host") {
            state.mode = "LAN Host";
        } else if (launchTicket.sessionMode == "LAN Client") {
            state.mode = "LAN Client";
        } else {
            state.mode = "Solo";
        }
    }
    if (!sessionProfile.selectedWorld.empty()) {
        state.worldName = sessionProfile.selectedWorld;
    } else if (!launchTicket.selectedWorld.empty()) {
        state.worldName = launchTicket.selectedWorld;
    }
    if (!launchTicket.hostEndpoint.empty()) {
        state.hostEndpoint = launchTicket.hostEndpoint;
    }
    state.bt72SecondSeatUnlocked = bunker::Bt72SecondSeatUnlocked(sessionProfile) || launchTicket.bt72SeatRole == "gunner";
    state.bt72SecondSeatPolicy = bunker::NormalizeBt72SecondSeatPolicy(
        launchTicket.bt72SecondSeatPolicy.empty() ? sessionProfile.partnerTank.secondSeatPolicy : launchTicket.bt72SecondSeatPolicy);
    state.bt72TrustedGunnerHandle = launchTicket.bt72TrustedGunnerHandle.empty()
        ? sessionProfile.partnerTank.trustedGunnerHandle
        : launchTicket.bt72TrustedGunnerHandle;
    state.bt72AssignedGunnerHandle = sessionProfile.partnerTank.assignedGunnerHandle;

    const std::string actorName = sessionProfile.character.displayName.empty()
        ? (launchTicket.characterName.empty() ? "Operator" : launchTicket.characterName)
        : sessionProfile.character.displayName;
    const std::string actorRole = state.mode == "LAN Host"
        ? "Host"
        : (state.mode == "LAN Client" ? "Client" : "Local Operator");
    state.activeActor = actorName;
    if (state.mode == "LAN Host") {
        if (!state.pendingPeer.empty()) {
            state.connectedPeer = state.pendingPeer;
            state.pendingPeer.clear();
            state.lifecycleStage = "HostClientAccepted";
            AcceptLanlineLobbySlot(state, state.connectedPeer);
            state.eventLog.push_back("Host accepted Lanline peer " + state.connectedPeer + ".");
        } else {
            state.lifecycleStage = "HostRuntimeActive";
        }
    } else if (state.mode == "LAN Client") {
        if (state.pendingPeer.empty()) {
            state.pendingPeer = actorName;
        }
        if (state.connectedPeer.empty()) {
            state.connectedPeer = "Host";
        }
        for (auto& player : state.players) {
        if (player.displayName == actorName && bunker::IsLanlineReservedSlot(player)) {
            player.role = "Pending Client";
            break;
        }
        }
        state.lifecycleStage = "ClientRuntimeJoined";
    } else {
        state.pendingPeer.clear();
        state.connectedPeer.clear();
        state.lifecycleStage = "RuntimeActive";
    }
    bool currentReady = false;
    for (const auto& player : state.players) {
        if (player.displayName == actorName) {
            currentReady = player.ready;
            break;
        }
    }
    const std::string seatAssignment = sessionProfile.partnerTank.deployed
        ? bunker::NormalizeBt72SeatAssignment(launchTicket.bt72SeatRole)
        : std::string("on_foot");
    if (seatAssignment == "gunner") {
        state.bt72AssignedGunnerHandle = actorName;
    }
    UpsertLanlinePlayer(state, actorName, actorRole, true, currentReady, seatAssignment);

    state.eventLog.push_back(actorName + " entered BunkerGame runtime via launcher ticket.");
    state.eventLog.push_back("Lanline lifecycle advanced to " + state.lifecycleStage + ".");
    if (!state.worldName.empty()) {
        state.eventLog.push_back("Runtime world confirmed at " + state.worldName + ".");
    }
    TrimLanlineEventLog(state, 12);
    bunker::SaveLanlineSessionState(state);
    bunker::SaveLanlineSessionState(state, bunker::LanlineSessionSnapshotPath(state.sessionId));
}

void SyncLanlineRuntimePresence(const bunker::PlayerState& player, const bunker::SessionProfile& sessionProfile) {
    static std::string previousSignature;
    static double lastSyncTime = -10.0;

    const double now = glfwGetTime();
    const std::string seatAssignment = CurrentRuntimeSeatAssignment(player);
    const std::string actorName = sessionProfile.character.displayName.empty() ? "Operator" : sessionProfile.character.displayName;
    const std::string actorRole = sessionProfile.sessionMode == "LAN Host"
        ? "Host"
        : (sessionProfile.sessionMode == "LAN Client" ? "Client" : "Local Operator");
    const std::string signature =
        sessionProfile.sessionMode + "|" +
        sessionProfile.selectedWorld + "|" +
        actorName + "|" +
        actorRole + "|" +
        seatAssignment + "|" +
        sessionProfile.partnerTank.secondSeatPolicy + "|" +
        sessionProfile.partnerTank.trustedGunnerHandle + "|" +
        sessionProfile.partnerTank.assignedGunnerHandle;
    if (signature == previousSignature && (now - lastSyncTime) < 1.0) {
        return;
    }

    bunker::LanlineSessionState state;
    if (!bunker::LoadLanlineSessionState(state)) {
        return;
    }

    std::string previousSeatAssignment;
    bool currentReady = false;
    for (const auto& entry : state.players) {
        if (entry.displayName == actorName) {
            previousSeatAssignment = entry.seatAssignment;
            currentReady = entry.ready;
            break;
        }
    }

    state.activeActor = actorName;
    state.worldName = sessionProfile.selectedWorld;
    state.bt72SecondSeatUnlocked = bunker::Bt72SecondSeatUnlocked(sessionProfile);
    state.bt72SecondSeatPolicy = bunker::NormalizeBt72SecondSeatPolicy(sessionProfile.partnerTank.secondSeatPolicy);
    state.bt72TrustedGunnerHandle = sessionProfile.partnerTank.trustedGunnerHandle;
    state.bt72AssignedGunnerHandle = seatAssignment == "gunner"
        ? actorName
        : sessionProfile.partnerTank.assignedGunnerHandle;
    UpsertLanlinePlayer(state, actorName, actorRole, true, currentReady, seatAssignment);

    if (previousSeatAssignment != seatAssignment) {
        state.eventLog.push_back(actorName + " shifted to " +
            std::string(bunker::Bt72SeatAssignmentLabel(seatAssignment)) + ".");
    }
    TrimLanlineEventLog(state, 12);
    bunker::SaveLanlineSessionState(state);
    bunker::SaveLanlineSessionState(state, bunker::LanlineSessionSnapshotPath(state.sessionId));
    previousSignature = signature;
    lastSyncTime = now;
}

}  // namespace

int main() {
    bunker::EnsureProjectDirectories();

    bunker::LaunchTicketInfo launchTicket;
    std::string launchFailureReason;
    if (!bunker::ConsumeLaunchTicket(launchTicket, launchFailureReason)) {
        if (!glfwInit()) {
            return -1;
        }

        GLFWwindow* deniedWindow = glfwCreateWindow(760, 220, "BunkerGame Launch Gate", nullptr, nullptr);
        if (deniedWindow == nullptr) {
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(deniedWindow);
        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(deniedWindow, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        while (!glfwWindowShouldClose(deniedWindow)) {
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            glViewport(0, 0, 760, 220);
            glClearColor(0.07f, 0.05f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(720.0f, 170.0f), ImGuiCond_Always);
            ImGui::Begin("Launch Restricted", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            ImGui::TextWrapped("%s", launchFailureReason.c_str());
            ImGui::Separator();
            ImGui::TextWrapped("BunkerGame is expected to be started by BunkerLauncher. BunkerEditor remains optional for players.");
                if (ImGui::Button("Close")) {
            glfwSetWindowShouldClose(deniedWindow, GLFW_TRUE);
            }
            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(deniedWindow);
        }

            ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(); 
        glfwDestroyWindow(deniedWindow);
        glfwTerminate();
         
                return 0;
    }

        bunker::SessionProfile sessionProfile;
    const auto profilePath = bunker::DefaultSessionProfilePath();
    if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
    sessionProfile = bunker::MakeDefaultSessionProfile();

    const auto initialProfileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);
    ReportRuntimeSaveOutcome(
        "Initial profile bootstrap",
        BuildRuntimeSaveOutcome(nullptr, &initialProfileSave, sessionProfile.selectedWorld, false));
    }
    
    bunker::NormalizeSessionProfile(sessionProfile);
    if (launchTicket.sessionMode == "LAN Client") {
        sessionProfile.partnerTank.secondSeatUnlocked =
            sessionProfile.partnerTank.secondSeatUnlocked || launchTicket.bt72SeatRole == "gunner";
        sessionProfile.partnerTank.secondSeatPolicy =
            bunker::NormalizeBt72SecondSeatPolicy(launchTicket.bt72SecondSeatPolicy);
        if (!launchTicket.bt72TrustedGunnerHandle.empty()) {
            sessionProfile.partnerTank.trustedGunnerHandle = launchTicket.bt72TrustedGunnerHandle;
        }
    }
    SyncLanlineRuntimeLaunchState(launchTicket, sessionProfile);

    bunker::StaticEraser staticEraser;
    staticEraser.Load(sessionProfile.selectedWorld);

    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1400, 900, "BunkerGame", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    bunker::Dx11Renderer dx11Renderer;
    if (!dx11Renderer.Initialize(window, framebufferWidth, framebufferHeight)) {
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    bunker::GameState gameState;
    gameState.phase = bunker::RuntimeGamePhase::WORLD_LOADING;

    bunker::World world;
    const auto worldPath = bunker::ResolveWorldPath(sessionProfile.selectedWorld);
    if (!world.Load(worldPath.string())) {

        world.GeneratePrototypeZone();
        const auto initialWorldSave = bunker::SaveWorldAtomically(world, worldPath);
        ReportRuntimeSaveOutcome(
            "Initial world bootstrap",
            BuildRuntimeSaveOutcome(&initialWorldSave, nullptr, sessionProfile.selectedWorld, false));
    }
    world.EnsureStarterInfrastructure();
    std::string semanticSealStatus;
    const int adoptedAnchorCount = bunker::AdoptAllAutoCreatedSemanticAnchors(world, semanticSealStatus);
    bunker::ApplyStaticEraser(world, staticEraser);
    bunker::SyncStoryFlagsFromWorld(sessionProfile, staticEraser);
    bunker::UpdateWorldMetadata(world, sessionProfile, staticEraser);
    bunker::WorldExecutionContext executionContext = bunker::BuildWorldExecutionContext(world);
    executionContext.adoptedAnchorCount = std::max(0, adoptedAnchorCount);
    executionContext.adoptionStatus = semanticSealStatus;

    bunker::PlayerState player;
    player.x = world.metadata.playerSpawnX;
    player.y = world.metadata.playerSpawnY;
    player.uiVisible = bunker::PlayerHasActivePipDeviceAccess(sessionProfile) && player.uiVisible;
    if (bunker::HasActiveFieldCheckpoint(sessionProfile)) {
        player.x = sessionProfile.fieldCheckpointX;
        player.y = sessionProfile.fieldCheckpointY;
    }
    if (sessionProfile.story.tankLinked) {
        player.insideTank = sessionProfile.partnerTank.deployed;
        player.bt72GunnerSeat = player.insideTank &&
            bunker::NormalizeBt72SeatAssignment(launchTicket.bt72SeatRole) == "gunner";
        player.viewMode = player.insideTank ? bunker::ViewMode::Cockpit : bunker::ViewMode::ThirdPerson;
    }
    if (sessionProfile.story.bucketRecovered) {
        bool canRaiseBucket = false;
        for (const auto& module : sessionProfile.partnerTank.loadout.modules) {
            if (module.type == bunker::TankModuleSlotType::Bucket && module.moduleId == "bucket_shield_a") {
                canRaiseBucket = true;
                break;
            }
        }
        player.bucketRaised = canRaiseBucket;
    }
    bunker::SyncPartnerTankAnchor(world, player, sessionProfile);
    gameState.phase = bunker::RuntimeGamePhase::ACTIVE_GAME;
    if (sessionProfile.sessionMode != "LAN Host" && sessionProfile.sessionMode != "LAN Client") {
        bunker::InjectOfflineDebugBot(sessionProfile);
    }
    if (executionContext.adoptedAnchorCount > 0) {
        gameState.lastEvent = "Runtime sealed " + std::to_string(executionContext.adoptedAnchorCount) +
            " semantic anchor(s) before activation.";
    }
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - lastTime);
        lastTime = now;
        gameState.sessionTime += dt / 60.0f;

        bunker::SyncStoryFlagsFromWorld(sessionProfile, staticEraser);
        bunker::UpdateWorldMetadata(world, sessionProfile, staticEraser);
        SyncLanlineRuntimePresence(player, sessionProfile);
        bunker::UpdateRadio(gameState, world, sessionProfile, staticEraser, dt);
        bunker::ProcessScriptedWorldEvents(world, player, sessionProfile, gameState);
        bunker::SyncPartnerTankAnchor(world, player, sessionProfile);
        bunker::UpdateWeatherAnomaly(world, player, sessionProfile, gameState, dt);
        bunker::UpdateEtherErosion(world, player, sessionProfile, gameState, dt);
        bunker::UpdateInfrastructureDecay(world, player, sessionProfile, gameState, dt);
        bunker::UpdateRouteContamination(world, sessionProfile, staticEraser, gameState, dt);
        bunker::UpdateAmbientTankCharging(world, sessionProfile, gameState, dt);
        bunker::UpdateScavengerTeams(sessionProfile, gameState, dt);
        bunker::UpdateCaravanRoute(sessionProfile, gameState, dt);
        bunker::UpdateDroneStations(sessionProfile, gameState, dt);
        bunker::UpdateTradeNetwork(sessionProfile, gameState, dt);
        bunker::UpdateRailFreight(sessionProfile, gameState, dt);
        bunker::UpdateOrbitalUplink(sessionProfile, gameState, dt);
        bunker::UpdateRailFortress(sessionProfile, gameState, dt);
        bunker::UpdateRecoveryFabricator(sessionProfile, gameState, dt);
        bunker::UpdateRecoveryMilestones(sessionProfile, gameState);
        bunker::UpdateIndustrialSurvey(sessionProfile, gameState, dt);
        bunker::UpdateIndustrialOutpost(sessionProfile, gameState, dt);
        bunker::UpdateAssemblyCell(sessionProfile, gameState, dt);
        bunker::UpdateFoundryLine(sessionProfile, gameState, dt);
        bunker::UpdateReactorYard(sessionProfile, gameState, dt);
        bunker::UpdateCapacitorBank(sessionProfile, gameState, dt);
        bunker::UpdateRelaySubstation(sessionProfile, gameState, dt);
        bunker::UpdateServiceBay(sessionProfile, gameState, dt);
        bunker::UpdateWaterReclaimer(sessionProfile, gameState, dt);
        bunker::UpdateRouteRandomEvents(sessionProfile, gameState, dt);
        if (gameState.rationEffectTimer > 0.0f) {
            gameState.rationEffectTimer = std::max(0.0f, gameState.rationEffectTimer - dt);
            if (gameState.rationEffectTimer == 0.0f) {
                gameState.lastEvent = "Toxic ration effect faded. SPECIAL baseline restored.";
            }
        }
        const float passiveMpDrain = player.insideTank ? 3.0f : 1.5f;
        sessionProfile.character.mp = std::max(0.0f, sessionProfile.character.mp - (dt * passiveMpDrain));
        if (sessionProfile.character.mp <= 10.0f && sessionProfile.character.hp > 0.0f) {
            gameState.lastEvent = "Operator fatigue rising. Recover MP before pushing further.";
        }
        const bool atCriticalHp = sessionProfile.character.hp > 0.0f &&
            sessionProfile.character.hp <= std::max(18.0f, sessionProfile.character.maxHp * 0.24f);
        if (atCriticalHp && !gameState.stressThresholdTriggered) {
            gameState.stressThresholdTriggered = true;
            std::string skillEvent;
            bunker::RegisterStressSurvival(sessionProfile, &skillEvent);
            if (!skillEvent.empty()) {
                gameState.lastEvent = skillEvent;
            }
        } else if (!atCriticalHp && sessionProfile.character.hp > sessionProfile.character.maxHp * 0.45f) {
            gameState.stressThresholdTriggered = false;
            gameState.secondWindTriggered = false;
        }
        if (atCriticalHp &&
            !gameState.secondWindTriggered &&
            bunker::HasEquippedPassiveSkill(sessionProfile, "skill_second_wind")) {
            const float hpBoost = player.insideTank ? 12.0f : 18.0f;
            const float mpCost = 10.0f;
            if (sessionProfile.character.mp >= mpCost) {
                sessionProfile.character.mp = std::max(0.0f, sessionProfile.character.mp - mpCost);
                sessionProfile.character.hp = std::min(sessionProfile.character.maxHp, sessionProfile.character.hp + hpBoost);
                gameState.secondWindTriggered = true;
                gameState.lastEvent = "Second Wind engaged. Operator stabilized under stress.";
            }
        }

        if (gameState.attackCooldown > 0.0f) {
            gameState.attackCooldown -= dt;
        }
        if (gameState.specialAttackCooldown > 0.0f) {
            gameState.specialAttackCooldown -= dt;
        }
        if (gameState.fieldWorkbenchCooldown > 0.0f) {
            gameState.fieldWorkbenchCooldown = std::max(0.0f, gameState.fieldWorkbenchCooldown - dt);
        }
        if (gameState.workshopServiceCooldown > 0.0f) {
            gameState.workshopServiceCooldown = std::max(0.0f, gameState.workshopServiceCooldown - dt);
        }
        if (gameState.damageFlashTimer > 0.0f) {
            gameState.damageFlashTimer -= dt;
        }
        player.recoilOffset = std::max(0.0f, player.recoilOffset - (dt * 1.7f));
        player.muzzleFlashTimer = std::max(0.0f, player.muzzleFlashTimer - dt * 2.6f);
        player.shockWaveTimer = std::max(0.0f, player.shockWaveTimer - dt);
        if (player.muzzleFlashTimer <= 0.0f) {
            player.muzzleFlashStrength = 0.0f;
        }
        if (player.shockWaveTimer <= 0.0f) {
            player.shockWaveDuration = 0.0f;
            player.shockWaveStrength = 0.0f;
        }

        float moveX = 0.0f;
        float moveY = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveY += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveY -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveX += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveX -= 1.0f;

        const float moveLength = std::sqrt((moveX * moveX) + (moveY * moveY));
        if (moveLength > 0.0f) {
            moveX /= moveLength;
            moveY /= moveLength;
        }
        if (player.insideTank) {
            float thermalRise = moveLength > 0.1f ? dt * 1.2f : -dt * 0.8f;
            bool towCouplerMounted = false;
            for (const auto& module : sessionProfile.partnerTank.loadout.modules) {
                if (module.type == bunker::TankModuleSlotType::Bucket && module.moduleId == "tow_coupler_mk1") {
                    towCouplerMounted = true;
                    break;
                }
            }
            if (towCouplerMounted && moveLength > 0.1f) {
                thermalRise += dt * 0.35f;
            }
            if (gameState.weather == bunker::WeatherAnomaly::AcidRain) {
                thermalRise += dt * 0.9f;
            } else if (gameState.weather == bunker::WeatherAnomaly::EtherFog) {
                thermalRise += dt * 0.25f;
            }
            const bool cooledByGrid = bunker::HasActiveFieldCheckpoint(sessionProfile) &&
                (std::abs(player.x - sessionProfile.fieldCheckpointX) <= 4.0f) &&
                (std::abs(player.y - sessionProfile.fieldCheckpointY) <= 4.0f);
            if (cooledByGrid) {
                thermalRise -= dt * 1.4f;
            }
            gameState.tankThermalLoad = std::clamp(gameState.tankThermalLoad + thermalRise, 0.0f, 100.0f);
        } else {
            gameState.tankThermalLoad = std::max(0.0f, gameState.tankThermalLoad - dt * 0.35f);
        }
        if (!player.insideTank && moveLength > 0.1f && bunker::CurrentInventoryWeight(sessionProfile) >= 5.0f) {
            gameState.heavyCarryTimer += dt;
            if (gameState.heavyCarryTimer >= 30.0f) {
                gameState.heavyCarryTimer = 0.0f;
                std::string skillEvent;
                bunker::RegisterHeavyCarryDrill(sessionProfile, &skillEvent);
                if (!skillEvent.empty()) {
                    gameState.lastEvent = skillEvent;
                }
            }
        } else {
            gameState.heavyCarryTimer = std::max(0.0f, gameState.heavyCarryTimer - dt * 0.5f);
        }

        const float rotationSpeed = 1.8f + (bunker::EffectiveStatValue(sessionProfile, gameState, 'A') * 0.08f);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) player.facingRadians += dt * rotationSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) player.facingRadians -= dt * rotationSpeed;

        const bool cycleViewNow = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
        if (cycleViewNow && !gameState.cycleViewPressed) {
            bunker::AdvanceViewMode(player);
        }
        gameState.cycleViewPressed = cycleViewNow;

        const bool toggleSeatNow = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;
        if (toggleSeatNow && !gameState.seatSwapPressed && player.insideTank) {
            bunker::TryToggleBt72CrewSeat(player, sessionProfile, gameState);
        }
        gameState.seatSwapPressed = toggleSeatNow;

        const bool toggleBucketNow = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;
        if (toggleBucketNow && !gameState.bucketPressed && player.insideTank && sessionProfile.story.bucketRecovered) {
            std::string utilityModuleId = "bucket_shield_a";
            for (const auto& module : sessionProfile.partnerTank.loadout.modules) {
                if (module.type == bunker::TankModuleSlotType::Bucket) {
                    utilityModuleId = module.moduleId;
                    break;
                }
            }
            if (utilityModuleId == "ram_shield_mk1") {
                gameState.lastEvent = "Ram Shield mounted. Swap back to Bucket Rig before using bucket controls.";
            } else if (utilityModuleId == "tow_coupler_mk1") {
                gameState.lastEvent = "Tow Coupler mounted. Swap back to Bucket Rig before using bucket controls.";
            } else if (player.bt72GunnerSeat) {
                gameState.lastEvent = "Return to BT-72 pilot controls before raising the bucket rig.";
            } else {
                player.bucketRaised = !player.bucketRaised;
                gameState.lastEvent = player.bucketRaised ? "Bucket rig raised." : "Bucket rig stowed.";
            }
        }
        gameState.bucketPressed = toggleBucketNow;

        const bool toggleUiNow = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
        if (toggleUiNow && !gameState.uiPressed) {
            bunker::TryToggleActivePipDeviceUi(player, sessionProfile, gameState);
        }
        gameState.uiPressed = toggleUiNow;

        const bool pipThemeNow = glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS;
        if (pipThemeNow && !gameState.pipThemePressed) {
            bunker::ApplyActivePipDeviceCustomizationCommand(sessionProfile, "theme_next", &gameState);
        }
        gameState.pipThemePressed = pipThemeNow;

        const bool pipDisplayNow = glfwGetKey(window, GLFW_KEY_F7) == GLFW_PRESS;
        if (pipDisplayNow && !gameState.pipDisplayPressed) {
            bunker::ApplyActivePipDeviceCustomizationCommand(sessionProfile, "display_next", &gameState);
        }
        gameState.pipDisplayPressed = pipDisplayNow;

        const bool pipCarryNow = glfwGetKey(window, GLFW_KEY_F8) == GLFW_PRESS;
        if (pipCarryNow && !gameState.pipCarryPressed) {
            bunker::ApplyActivePipDeviceCustomizationCommand(sessionProfile, "carry_next", &gameState);
        }
        gameState.pipCarryPressed = pipCarryNow;

        const bool saveNow = glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS;
        if (saveNow && !gameState.savePressed) {
            const auto profileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);
            staticEraser.Save(sessionProfile.selectedWorld);
            const RuntimeSaveOutcome saveOutcome =
                BuildRuntimeSaveOutcome(nullptr, &profileSave, sessionProfile.selectedWorld, true);
            ReportRuntimeSaveOutcome("Runtime F5 save", saveOutcome);
            gameState.lastEvent = saveOutcome.message;
        }
        gameState.savePressed = saveNow;

        const bool healNow = glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS;
        if (healNow && !gameState.healPressed && sessionProfile.character.hp > 0.0f) {
            if (sessionProfile.character.hp >= sessionProfile.character.maxHp) {
                gameState.lastEvent = "HP already stabilized.";
            } else if (bunker::ConsumeInventoryItem(sessionProfile, "cryo_medkit", 1)) {
                sessionProfile.character.hp = std::min(sessionProfile.character.maxHp, sessionProfile.character.hp + 35.0f);
                sessionProfile.character.mp = std::min(sessionProfile.character.maxMp, sessionProfile.character.mp + 20.0f);
                gameState.lastEvent = "Cryo medkit applied.";
            } else {
                gameState.lastEvent = "No cryo medkits available.";
            }
        }
        gameState.savePressed = saveNow;

        const bool reloadNow = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        if (reloadNow && !gameState.reloadPressed && sessionProfile.character.hp > 0.0f) {
            if (player.insideTank) {
                gameState.lastEvent = sessionProfile.partnerTank.ammoReserve > 0.0f
                    ? "BT-72 reload cycle complete."
                    : "BT-72 ammo reserve depleted.";
            } else {
                gameState.lastEvent = bunker::HasInventoryItem(sessionProfile, "#%it_ptrs_ammo")
                    ? "PTRS reload complete."
                    : "No PTRS ammo left to reload.";
            }
        }
        gameState.reloadPressed = reloadNow;

        float moveSpeed = player.insideTank ? 5.2f : 3.8f;
        const bool outdoors = world.IsStarterScenarioWorld() ? (player.x >= 13.0f || sessionProfile.story.exitedBunker) : true;
        const float etherErosion = bunker::CurrentEtherErosion(sessionProfile);
        if (gameState.weather == bunker::WeatherAnomaly::EtherFog) {
            moveSpeed *= player.insideTank ? 0.9f : 0.82f;
        }
        if (gameState.weather == bunker::WeatherAnomaly::AcidRain && !player.insideTank) {
            moveSpeed *= 0.9f;
        }
        if (outdoors && etherErosion > 0.0f) {
            const float erosionPenalty = player.insideTank
                ? std::min(0.16f, etherErosion * 0.0017f)
                : std::min(0.12f, etherErosion * 0.0013f);
            moveSpeed *= (1.0f - erosionPenalty);
            if (player.insideTank && etherErosion >= 35.0f) {
                sessionProfile.partnerTank.energyReserve = std::max(0.0f, sessionProfile.partnerTank.energyReserve - (dt * 0.08f * (etherErosion / 35.0f)));
            }
        }
        if (sessionProfile.character.mp <= 10.0f) {
            moveSpeed *= 0.72f;
        }
        if (player.insideTank && sessionProfile.partnerTank.damage.hull <= 25.0f) {
            moveSpeed *= 0.7f;
        }
        bool towCouplerMounted = false;
        if (player.insideTank) {
            for (const auto& module : sessionProfile.partnerTank.loadout.modules) {
                if (module.type == bunker::TankModuleSlotType::Bucket && module.moduleId == "tow_coupler_mk1") {
                    towCouplerMounted = true;
                    break;
                }
            }
        }
        if (towCouplerMounted) {
            moveSpeed *= 0.88f;
        }
        if (player.insideTank && player.bt72GunnerSeat) {
            moveSpeed *= 0.32f;
        }
        if (player.insideTank && gameState.tankThermalLoad >= 80.0f) {
            moveSpeed *= 0.76f;
            sessionProfile.partnerTank.damage.powerCore = std::max(0.0f, sessionProfile.partnerTank.damage.powerCore - dt * 0.35f);
            if (sessionProfile.partnerTank.energyReserve > 0.0f) {
                sessionProfile.partnerTank.energyReserve = std::max(0.0f, sessionProfile.partnerTank.energyReserve - dt * 0.2f);
            }
        } else if (player.insideTank && gameState.tankThermalLoad >= 55.0f) {
            moveSpeed *= 0.9f;
        }
        moveSpeed *= bunker::TapeMovementMultiplier(sessionProfile.character, player.insideTank);
        if (player.insideTank) {
            const float desiredVelocityX = moveX * moveSpeed;
            const float desiredVelocityY = moveY * moveSpeed;
            const float acceleration = 2.8f;
            const float damping = 1.9f;
            player.velocityX += (desiredVelocityX - player.velocityX) * std::min(1.0f, dt * acceleration);
            player.velocityY += (desiredVelocityY - player.velocityY) * std::min(1.0f, dt * acceleration);
            if (std::abs(moveX) < 0.01f && std::abs(moveY) < 0.01f) {
                player.velocityX *= std::max(0.0f, 1.0f - dt * damping);
                player.velocityY *= std::max(0.0f, 1.0f - dt * damping);
            }
            const float previousX = player.x;
            const float previousY = player.y;
            bunker::SweepMovePlayerAgainstWorld(world, player, player.velocityX * dt, player.velocityY * dt);
            if (std::abs(player.x - previousX) < 0.0001f) {
                player.velocityX = 0.0f;
            }
            if (std::abs(player.y - previousY) < 0.0001f) {
                player.velocityY = 0.0f;
            }
        } else {
            player.velocityX = moveX * moveSpeed;
            player.velocityY = moveY * moveSpeed;
            bunker::SweepMovePlayerAgainstWorld(world, player, player.velocityX * dt, player.velocityY * dt);
        }

        bunker::UpdateHostiles(world, player, sessionProfile, staticEraser, gameState, dt);
        if (player.insideTank && sessionProfile.partnerTank.damage.hull <= 0.0f && !gameState.soulLineTriggered) {
            player.insideTank = false;
            player.bt72GunnerSeat = false;
            player.viewMode = bunker::ViewMode::ThirdPerson;
            player.x += std::cos(player.facingRadians) * 0.8f;
            player.y += std::sin(player.facingRadians) * 0.8f;
            player.velocityX = 0.0f;
            player.velocityY = 0.0f;
            player.recoilOffset = 0.0f;
            player.muzzleFlashTimer = 0.0f;
            player.muzzleFlashStrength = 0.0f;
            player.shockWaveTimer = 0.0f;
            player.shockWaveDuration = 0.0f;
            player.shockWaveStrength = 0.0f;
            sessionProfile.partnerTank.deployed = false;
            sessionProfile.partnerTank.energyReserve = std::max(0.0f, sessionProfile.partnerTank.energyReserve - 18.0f);
            sessionProfile.character.hp = std::max(18.0f, sessionProfile.character.hp - 28.0f);
            sessionProfile.character.mp = std::max(10.0f, sessionProfile.character.mp - 18.0f);
            gameState.soulLineTriggered = true;
            gameState.lastEvent = "SoulLine rupture detected. BT-72 disabled and operator emergency-ejected.";
        } else if (sessionProfile.partnerTank.damage.hull > 20.0f) {
            gameState.soulLineTriggered = false;
        }

        const bool attackNow = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (attackNow && gameState.attackCooldown <= 0.0f && sessionProfile.character.hp > 0.0f) {
            bunker::HandleAttack(world, player, sessionProfile, staticEraser, gameState);
            gameState.attackCooldown = player.insideTank ? 0.9f : 0.45f;
        }

        const bool specialAttackNow = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
        if (specialAttackNow && gameState.specialAttackCooldown <= 0.0f && sessionProfile.character.hp > 0.0f) {
            bunker::HandleSpecialAttack(world, player, sessionProfile, staticEraser, gameState);
            gameState.specialAttackCooldown = player.insideTank ? 1.8f : 1.2f;
        }

        const bunker::MapObject* nearest = world.FindNearestInteractive(player.x, player.y, 2.2f);
        const bool useNow = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
        if (useNow && !gameState.usePressed && bunker::WantsUseKey(nearest)) {
            bunker::HandleInteraction(nearest, world, player, sessionProfile, staticEraser, gameState, &executionContext);
        }
        gameState.usePressed = useNow;

        const bool contextualNow = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
        if (contextualNow && !gameState.contextualPressed && bunker::WantsContextKey(nearest)) {
            bunker::HandleInteraction(nearest, world, player, sessionProfile, staticEraser, gameState, &executionContext);
        }
        gameState.contextualPressed = contextualNow;

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);

        dx11Renderer.Resize(width, height);
        dx11Renderer.BeginFrame(0.08f, 0.09f, 0.12f, 1.0f);
        dx11Renderer.RenderTerrain(world, gameState.weather, gameState.weatherIntensity, gameState.sessionTime);
        bunker::UpdateWindowTitle(window, player, world, sessionProfile);

        if (sessionProfile.character.hp <= 0.0f) {
            ImGui::SetNextWindowPos(ImVec2(40.0f, 80.0f), ImGuiCond_Always);
            ImGui::Begin("Downed State", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::TextWrapped("Operator down. Emergency respawn protocol engaged.");
            if (ImGui::Button("Respawn At Cryo Wing")) {
                if (bunker::HasActiveFieldCheckpoint(sessionProfile)) {
                    player.x = sessionProfile.fieldCheckpointX;
                    player.y = sessionProfile.fieldCheckpointY;
                } else {
                    player.x = world.metadata.playerSpawnX;
                    player.y = world.metadata.playerSpawnY;
                }
                player.insideTank = false;
                player.bt72GunnerSeat = false;
                player.viewMode = bunker::ViewMode::ThirdPerson;
                player.velocityX = 0.0f;
                player.velocityY = 0.0f;
                player.recoilOffset = 0.0f;
                player.muzzleFlashTimer = 0.0f;
                player.muzzleFlashStrength = 0.0f;
                player.shockWaveTimer = 0.0f;
                player.shockWaveDuration = 0.0f;
                player.shockWaveStrength = 0.0f;
                sessionProfile.partnerTank.deployed = false;
                sessionProfile.partnerTank.worldX = player.x;
                sessionProfile.partnerTank.worldY = player.y;
                sessionProfile.partnerTank.worldPositionKnown = true;
                sessionProfile.character.hp = std::max(35.0f, sessionProfile.character.maxHp * 0.5f);
                sessionProfile.character.mp = std::max(20.0f, sessionProfile.character.maxMp * 0.5f);
                sessionProfile.character.experience = std::max(0, sessionProfile.character.experience - 25);
                gameState.stressThresholdTriggered = false;
                gameState.secondWindTriggered = false;
                gameState.lastEvent = bunker::HasActiveFieldCheckpoint(sessionProfile)
                    ? "Emergency respawn complete at field checkpoint. XP penalty applied."
                    : "Emergency respawn complete. XP penalty applied.";
            }
            ImGui::End();
        }

        if (nearest != nullptr && player.uiVisible) {
            bool promptTowCouplerMounted = false;
            if (player.insideTank) {
                for (const auto& module : sessionProfile.partnerTank.loadout.modules) {
                    if (module.type == bunker::TankModuleSlotType::Bucket && module.moduleId == "tow_coupler_mk1") {
                        promptTowCouplerMounted = true;
                        break;
                    }
                }
            }
            ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
            ImGui::Begin("Field Prompt", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Nearest: %s", nearest->displayName.c_str());
            if (nearest->interaction == bunker::InteractionType::Hostile) {
                const std::string hostileReadability = bunker::DescribeHostileReadability(*nearest, gameState);
                ImGui::TextWrapped("%s", hostileReadability.c_str());
                ImGui::Text("SPACE attack | R reload | G special");
            } else if (bunker::WantsContextKey(nearest)) {
                ImGui::Text("Press F to enter or open");
            } else {
                ImGui::Text("Press E to use or interact");
            }
            if (nearest->interaction != bunker::InteractionType::Hostile) {
                const std::string executionOverlay = bunker::DescribeInteractionOverlay(*nearest, executionContext);
                ImGui::Separator();
                ImGui::TextWrapped("%s", executionOverlay.c_str());
            }
            ImGui::Separator();
            ImGui::Text("TAB Pip Device | F6 Theme | F7 Display | F8 Carry | H Medkit | R Reload | F5 Save");
            ImGui::Text("Left/Right rotate | C camera | V crew seat | B bucket");
            if (promptTowCouplerMounted) {
                ImGui::TextDisabled("Tow Coupler active: logistics boost, lower mobility");
            }
            if (player.insideTank) {
                ImGui::TextDisabled("BT-72 seat: %s",
                    player.bt72GunnerSeat ? "gunner station" : "pilot controls");
            }
            ImGui::Text("Weather: %s", bunker::ToString(gameState.weather));
            ImGui::Text("Ether Pressure: %.0f%%", bunker::CurrentEtherErosion(sessionProfile));
            if (gameState.damageFlashTimer > 0.0f) {
                ImGui::TextColored(ImVec4(0.95f, 0.32f, 0.22f, 1.0f), "Under attack");
            }
            if (player.muzzleFlashTimer > 0.0f) {
                ImGui::TextColored(ImVec4(1.0f, 0.76f, 0.24f, 1.0f), "Weapon flash active");
            }
            ImGui::End();
        }

        if (player.insideTank && player.uiVisible) {
            const char* utilityDisplay = "Unknown";
            bool hudTowCouplerMounted = false;
            for (const auto& module : sessionProfile.partnerTank.loadout.modules) {
                if (module.type == bunker::TankModuleSlotType::Bucket) {
                    utilityDisplay = module.displayName.c_str();
                    hudTowCouplerMounted = module.moduleId == "tow_coupler_mk1";
                    break;
                }
            }
            ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width - 320), 20.0f), ImGuiCond_Always);
            ImGui::Begin("Tank HUD", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("BT-72 // %s", bunker::ToString(player.viewMode));
            ImGui::Text("Hull %.0f%% [%s]", sessionProfile.partnerTank.damage.hull, bunker::TankIntegrityBand(sessionProfile.partnerTank.damage.hull));
            ImGui::Text("Turret %.0f%% [%s]", sessionProfile.partnerTank.damage.turret, bunker::TankIntegrityBand(sessionProfile.partnerTank.damage.turret));
            ImGui::Text("Bucket %.0f%% [%s]", sessionProfile.partnerTank.damage.bucket, bunker::TankIntegrityBand(sessionProfile.partnerTank.damage.bucket));
            ImGui::Text("Sensors %.0f%% [%s]", sessionProfile.partnerTank.damage.sensors, bunker::TankIntegrityBand(sessionProfile.partnerTank.damage.sensors));
            ImGui::Text("Cockpit %.0f%% [%s]", sessionProfile.partnerTank.damage.cockpit, bunker::TankIntegrityBand(sessionProfile.partnerTank.damage.cockpit));
            ImGui::Text("Power Core %.0f%% [%s]", sessionProfile.partnerTank.damage.powerCore, bunker::TankIntegrityBand(sessionProfile.partnerTank.damage.powerCore));
            ImGui::Text("Energy %.0f%%", sessionProfile.partnerTank.energyReserve);
            ImGui::Text("Ammo %.0f%%", sessionProfile.partnerTank.ammoReserve);
            ImGui::Text("Sync %s", bunker::CurrentTankSyncMode(sessionProfile.partnerTank).c_str());
            ImGui::Text("Link %.0f%%", sessionProfile.partnerTank.trustLink * 100.0f);
            ImGui::Text("Seat %s", player.bt72GunnerSeat ? "Gunner" : "Pilot");
            ImGui::Text("Weather %s", bunker::ToString(gameState.weather));
            ImGui::Text("Ether Pressure %.0f%%", bunker::CurrentEtherErosion(sessionProfile));
            ImGui::Text("Thermal %.0f%%", gameState.tankThermalLoad);
            ImGui::Text("Utility %s", utilityDisplay);
            ImGui::Text("Momentum %.1f", std::sqrt((player.velocityX * player.velocityX) + (player.velocityY * player.velocityY)));
            ImGui::Text("SoulLine %s", gameState.soulLineTriggered || sessionProfile.partnerTank.damage.hull <= 0.0f ? "tripped" : "standby");
            if (hudTowCouplerMounted) {
                ImGui::TextDisabled("Tow mode: +logistics / -mobility");
            }
            if (player.bt72GunnerSeat) {
                ImGui::TextDisabled("Gunner station: movement reduced, support weapons prioritized.");
            }
            if (sessionProfile.partnerTank.damage.hull <= 25.0f) {
                ImGui::TextColored(ImVec4(0.95f, 0.3f, 0.25f, 1.0f), "Hull critical. Mobility reduced.");
            }
            if (sessionProfile.partnerTank.damage.powerCore <= 30.0f) {
                ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.1f, 1.0f), "Power core unstable. Combat efficiency reduced.");
            }
            if (sessionProfile.partnerTank.ammoReserve <= 20.0f) {
                ImGui::TextColored(ImVec4(0.95f, 0.7f, 0.22f, 1.0f), "Ammo reserves low. Workshop or field service recommended.");
            }
            if (gameState.tankThermalLoad >= 80.0f) {
                ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.1f, 1.0f), "Thermal overload. Mobility and core efficiency reduced.");
            }
            if (gameState.damageFlashTimer > 0.0f) {
                ImGui::TextColored(ImVec4(0.95f, 0.32f, 0.22f, 1.0f), "Impact warning");
            }
            if (player.muzzleFlashTimer > 0.0f) {
                ImGui::TextColored(ImVec4(1.0f, 0.76f, 0.24f, 1.0f), "Heavy muzzle flash");
            }
            if (player.shockWaveTimer > 0.0f) {
                ImGui::TextColored(ImVec4(1.0f, 0.72f, 0.30f, 1.0f), "Shock wave rolling off the hull");
            }
            if (nearest != nullptr && nearest->interaction == bunker::InteractionType::Hostile) {
                const std::string hostileReadability = bunker::DescribeHostileReadability(*nearest, gameState);
                ImGui::Separator();
                ImGui::TextWrapped("Threat: %s", hostileReadability.c_str());
            }
            ImGui::End();
        }

        bunker::DrawPipPad(world, player, sessionProfile, staticEraser, gameState);

        dx11Renderer.EndFrame();
    }

    const auto finalProfileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);
    staticEraser.Save(sessionProfile.selectedWorld);
    ReportRuntimeSaveOutcome(
        "Final runtime profile save",
        BuildRuntimeSaveOutcome(nullptr, &finalProfileSave, sessionProfile.selectedWorld, true));

    dx11Renderer.Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
