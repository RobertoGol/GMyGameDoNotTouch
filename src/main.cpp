#include "../Dx11Renderer.hpp"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // Захват HWND для DirectX 11 контекста
#include <d3d11.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_dx11.h" // Исправлено: строго DX11 бэкенд вместо OpenGL3

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

// Объявления внешних функций из модуля EscMenuSystem.cpp
namespace bunker {
    void HandleEscKeyPress(GameState& gameState);
    void DrawImGuiEscMenuSystem(GameState& gameState, SessionProfile& profile);
}

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
            state.eventLog.erase(state.eventLog.begin(), state.eventLog.begin() + 
                static_cast<std::vector<std::string>::difference_type>(state.eventLog.size() - maxEntries));
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
        const std::string actorName = sessionProfile.character.displayName.empty() ? "Operator" : 
        sessionProfile.character.displayName;
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
        bunker::LanlineSessionState state; if (!bunker::LoadLanlineSessionState(state)) 
        {return;} std::string previousSeatAssignment;
        bool currentReady = false;
        for (const auto& entry : state.players) { if (entry.displayName == actorName) 
        { previousSeatAssignment = entry.seatAssignment; currentReady = entry.ready; break; 
        }} state.activeActor = actorName; state.worldName = sessionProfile.selectedWorld;
        state.bt72SecondSeatUnlocked = bunker::Bt72SecondSeatUnlocked(sessionProfile);
        state.bt72SecondSeatPolicy = 
        bunker::NormalizeBt72SecondSeatPolicy(sessionProfile.partnerTank.secondSeatPolicy);
        state.bt72TrustedGunnerHandle = sessionProfile.partnerTank.trustedGunnerHandle;
        state.bt72AssignedGunnerHandle = seatAssignment == "gunner" ? actorName : 
        sessionProfile.partnerTank.assignedGunnerHandle;
        UpsertLanlinePlayer(state, actorName, actorRole, true, currentReady, seatAssignment);
        if (previousSeatAssignment != seatAssignment) { state.eventLog.push_back(actorName + " "
        "shifted to " + std::string(bunker::Bt72SeatAssignmentLabel(seatAssignment)) + 
        "."); } TrimLanlineEventLog(state, 12);
        bunker::SaveLanlineSessionState(state);
        bunker::SaveLanlineSessionState(state, bunker::LanlineSessionSnapshotPath(state.sessionId));
        previousSignature = signature;
        lastSyncTime = now;
        }
} // закрытие анонимного namespace / пространства функций логики присутствия

int main() {
    bunker::EnsureProjectDirectories();
    bunker::LaunchTicketInfo launchTicket;
    std::string launchFailureReason;
    
    if (!bunker::ConsumeLaunchTicket(launchTicket, launchFailureReason)) {
        if (!glfwInit()) {
            return -1;
        }
        
        // Изменение под DirectX 11 API для окна блокировки
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* deniedWindow = glfwCreateWindow(760, 220, "BunkerGame Launch Gate", nullptr, nullptr);
        if (deniedWindow == nullptr) {
            glfwTerminate();
            return -1;
        }
        
        // Получаем нативный хэндл HWND для инициализации вспомогательного DirectX 11 контекста
        HWND deniedHwnd = glfwGetWin32Window(deniedWindow);
        
        // Создание минимального DX11 контекста для окна ошибки
        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = deniedHwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        
        IDXGISwapChain* pDeniedSwapChain = nullptr;
        ID3D11Device* pDeniedDevice = nullptr;
        ID3D11DeviceContext* pDeniedContext = nullptr;
        ID3D11RenderTargetView* pDeniedRTV = nullptr;
        
        D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, 
            D3D11_SDK_VERSION, &sd, &pDeniedSwapChain, &pDeniedDevice, nullptr, &pDeniedContext
        );
        
        ID3D11Texture2D* pBackBuffer = nullptr;
        pDeniedSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        pDeniedDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pDeniedRTV);
        pBackBuffer->Release();
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOther(deniedWindow, true);
        ImGui_ImplDX11_Init(dx11Renderer.GetDevice(), dx11Renderer.GetDeviceContext());

        
        // Исправлено: строго DX11 бэкенд
        const float restricted_clear_color[4] = { 0.07f, 0.05f, 0.05f, 1.0f };
        
        while (!glfwWindowShouldClose(deniedWindow)) {
            glfwPollEvents();
            
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
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
            pDeniedContext->OMSetRenderTargets(1, &pDeniedRTV, nullptr);
            pDeniedContext->ClearRenderTargetView(pDeniedRTV, restricted_clear_color);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            pDeniedSwapChain->Present(1, 0);
        }
        
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        pDeniedRTV->Release();
        pDeniedSwapChain->Release();
        pDeniedContext->Release();
        pDeniedDevice->Release();
        glfwDestroyWindow(deniedWindow);
        glfwTerminate();
        return 0;
    }
    
    bunker::SessionProfile sessionProfile;
    const auto profilePath = bunker::DefaultSessionProfilePath();
    
    if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
        sessionProfile = bunker::MakeDefaultSessionProfile();
        const auto initialProfileSave = bunker::SaveProfileAtomically(sessionProfile, profilePath);
        ReportRuntimeSaveOutcome("Initial profile bootstrap", BuildRuntimeSaveOutcome(nullptr, &initialProfileSave, sessionProfile.selectedWorld, false));
    }
    
    bunker::NormalizeSessionProfile(sessionProfile);
    
    if (launchTicket.sessionMode == "LAN Client") {
        sessionProfile.partnerTank.secondSeatUnlocked = sessionProfile.partnerTank.secondSeatUnlocked || launchTicket.bt72SeatRole == "gunner";
        sessionProfile.partnerTank.secondSeatPolicy = bunker::NormalizeBt72SecondSeatPolicy(launchTicket.bt72SecondSeatPolicy);
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
    
    // Инициализируем бэкенд ImGui под DirectX 11 контекст основного движка игры
    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplDX11_Init(pDeniedDevice, pDeniedContext);

    
    bunker::GameState gameState;
    gameState.phase = bunker::RuntimeGamePhase::WORLD_LOADING;
    
    bunker::World world;
    const auto worldPath = bunker::ResolveWorldPath(sessionProfile.selectedWorld);
    
    if (!world.Load(worldPath.string())) {
        world.GeneratePrototypeZone();
        const auto initialWorldSave = bunker::SaveWorldAtomically(world, worldPath);
        ReportRuntimeSaveOutcome("Initial world bootstrap", BuildRuntimeSaveOutcome(&initialWorldSave, nullptr, sessionProfile.selectedWorld, false));
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
        player.bt72GunnerSeat = player.insideTank && bunker::NormalizeBt72SeatAssignment(launchTicket.bt72SeatRole) == "gunner";
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
        gameState.lastEvent = "Runtime sealed " + std::to_string(executionContext.adoptedAnchorCount) + " semantic anchor(s) before activation.";
    }
    
    double lastTime = glfwGetTime();
    bool g_EscKeyPressedLastFrame = false; // Переменная контроля дребезга Esc
    
    // ГЛАВНЫЙ ИГРОВОЙ ЦИКЛ ОБНОВЛЕНИЯ И РЕНДЕРА ПОД DIRECTX 11
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - lastTime);
        lastTime = now;
        
        // --- ФИЗИЧЕСКИЙ ОПРОС КЛАВИАТУРЫ НА НАЖАТИЕ КЛАВИШИ ESCAPE ---
        bool isEscPressedNow = (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
        if (isEscPressedNow && !g_EscKeyPressedLastFrame) {
            bunker::HandleEscKeyPress(gameState); // Переключение стейтов гибридного меню
        }
        g_EscKeyPressedLastFrame = isEscPressedNow;
        
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
        
        const bool atCriticalHp = sessionProfile.character.hp > 0.0f && sessionProfile.character.hp <= std::max(18.0f, sessionProfile.character.maxHp * 0.24f);
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
        
        if (atCriticalHp && !gameState.secondWindTriggered && bunker::HasEquippedPassiveSkill(sessionProfile, "skill_second_wind")) {
            const float hpBoost = player.insideTank ? 12.0f : 18.0f;
            const float mpCost = 10.0f;
            if (sessionProfile.character.mp >= mpCost) {
                sessionProfile.character.mp = std::max(0.0f, sessionProfile.character.mp - mpCost);
                sessionProfile.character.hp = std::min(sessionProfile.character.maxHp, sessionProfile.character.hp + hpBoost);
                gameState.secondWindTriggered = true;
                gameState.lastEvent = "Second Wind engaged. Operator stabilized under stress.";
            }
        }
        
        if (gameState.attackCooldown > 0.0f) gameState.attackCooldown -= dt;
        if (gameState.specialAttackCooldown > 0.0f) gameState.specialAttackCooldown -= dt;
        if (gameState.fieldWorkbenchCooldown > 0.0f) gameState.fieldWorkbenchCooldown = std::max(0.0f, gameState.fieldWorkbenchCooldown - dt);
        if (gameState.workshopServiceCooldown > 0.0f) gameState.workshopServiceCooldown = std::max(0.0f, gameState.workshopServiceCooldown - dt);
        if (gameState.damageFlashTimer > 0.0f) gameState.damageFlashTimer -= dt;
        
        player.recoilOffset = std::max(0.0f, player.recoilOffset - (dt * 1.7f));
        player.muzzleFlashTimer = std::max(0.0f, player.muzzleFlashTimer - dt * 2.6f);
        player.shockWaveTimer = std::max(0.0f, player.shockWaveTimer - dt);
        
        if (player.muzzleFlashTimer <= 0.0f) player.muzzleFlashStrength = 0.0f;
        if (player.shockWaveTimer <= 0.0f) {
            player.shockWaveDuration = 0.0f;
            player.shockWaveStrength = 0.0f;
        }
        
        // --- СИСТЕМА ОБНОВЛЕНИЯ ФИЗИКИ И ТЕПЛОВОЙ НАГРУЗКИ ---
        float moveX = 0.0f;
        float moveY = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveY += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveY -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveX += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveX -= 1.0f;
        
        const float moveLength = std::sqrt((moveX * moveX) + (moveY * moveY));
        
                   const float moveLength = std::sqrt((moveX * moveX) + (moveY * moveY));

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
            }
            else if (gameState.weather == bunker::WeatherAnomaly::EtherFog) {
                thermalRise += dt * 0.25f;
            }

            
            const bool cooledByGrid = bunker::HasActiveFieldCheckpoint(sessionProfile) && 
                (std::abs(player.x - sessionProfile.fieldCheckpointX) <= 4.0f) && 
                (std::abs(player.y - sessionProfile.fieldCheckpointY) <= 4.0f);
            
            if (cooledByGrid) {
                thermalRise -= dt * 1.4f;
            }
            
            gameState.tankThermalLoad = std::clamp(gameState.tankThermalLoad + thermalRise, 0.0f, 100.0f);
        }
        else {
            gameState.tankThermalLoad = std::max(0.0f, gameState.tankThermalLoad - dt * 0.35f);
        }

        // --- ТЯЖЕЛАЯ АТЛЕТИКА (HEAVY CARRY DRILL) ---
        // Считаем moveLength на основе нажатых клавиш движения, чтобы переменная гарантированно существовала
        float moveX = 0.0f;
        float moveY = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveY += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveY -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveX += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveX -= 1.0f;
        const float moveLength = std::sqrt((moveX * moveX) + (moveY * moveY));

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

        // --- СКОРОСТЬ ПОВОРОТА И ВВОД КАМЕРЫ ---
        const float rSpeed = 1.8f + (bunker::EffectiveStatValue(sessionProfile, gameState, 'A') * 0.08f);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) player.facingRadians += dt * rSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) player.facingRadians -= dt * rSpeed;

        const bool cycleViewNow = (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS);
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
            bunker::TryToggleActivePipDevice(gameState, sessionProfile);
        }
        gameState.uiPressed = toggleUiNow;

        // --- DIRECTX 11 IMGUI RENDER PASS ---
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bunker::DrawImGuiEscMenuSystem(gameState, sessionProfile, world);


        ImGui::Render();

        dx11Renderer.BeginFrame();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        dx11Renderer.EndFrame();
    }

    // --- CLEANUP AND TERMINATION ---
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    dx11Renderer.Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}