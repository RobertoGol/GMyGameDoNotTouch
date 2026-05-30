#include "LauncherSupport.hpp"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <system_error>
#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include <filesystem>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_dx11.h"

#include "../../include/AppPaths.hpp"
#include "../../include/AtomicPersistence.hpp"
#include "../../include/BuildAnnouncement.hpp"
#include "../../include/LanlineLobbyLogic.hpp"
#include "../../include/LanlineServices.hpp"
#include "../../include/SessionFlow.hpp"
#include "../../include/StoryRoute.hpp"

// Наследуем оригинальные структуры и функции из пространства LauncherSupport
namespace {
    struct LauncherState {
        bool loggedIn = false;
        bool authFailed = false;
        int selectedCharacter = 0;
        int sessionModeIndex = 0;
        int previousSessionModeIndex = 0;
        int selectedWorldIndex = 0;
        int selectedLanlineSnapshot = -1;
        char login[32] = "wanderer";
        char password[32] = "prototype";
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

    using launcher_support::RefreshLauncherData;
    using launcher_support::PrepareSelectedCharacter;
    using launcher_support::SaveLanlineRosterState;
    using launcher_support::SaveLanlineSnapshotAndMaybeActive;
    using launcher_support::TryLaunchSiblingExecutable;
    using launcher_support::SelectedWorldPath;
}

// Структура для математических лепестков сакуры со скриншота Elder Tales
struct SakuraPetal {
    float x, y;
    float speedY;
    float speedX;
    float baseScale;
    float currentScale;
    float rotationAngle;
};

// Глобальные COM-объекты DirectX 11 для лаунчера
IDXGISwapChain*           g_pSwapChain = nullptr;
ID3D11Device*             g_pd3dDevice = nullptr;
ID3D11DeviceContext*      g_pd3dDeviceContext = nullptr;
ID3D11RenderTargetView*   g_mainRenderTargetView = nullptr;
std::vector<SakuraPetal>  g_SakuraBlossoms;

// Инициализация 60 лепестков сакуры для старых ПК без видеопамяти
void InitMathematicalSakura(int screenWidth) {
    g_SakuraBlossoms.resize(60);
    for (int i = 0; i < 60; ++i) {
        g_SakuraBlossoms[i].x = static_cast<float>(rand() % screenWidth);
        g_SakuraBlossoms[i].y = static_cast<float>(-(rand() % 800));
        g_SakuraBlossoms[i].speedY = 50.0f + (rand() % 40);
        g_SakuraBlossoms[i].speedX = -15.0f + (rand() % 30);
        g_SakuraBlossoms[i].baseScale = 4.0f + static_cast<float>(rand() % 6);
        g_SakuraBlossoms[i].currentScale = g_SakuraBlossoms[i].baseScale;
        g_SakuraBlossoms[i].rotationAngle = static_cast<float>(rand() % 360);
    }
}

// Математический расчет падения, ветра и динамического масштабирования сакуры
void UpdateAndRenderSakuraAnimation(ImDrawList* drawList, float dt, float screenWidth, float screenHeight, float sessionTime) {
    uint32_t petalColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.45f, 0.61f, 0.65f)); // Розовый с альфой

    for (auto& petal : g_SakuraBlossoms) {
        petal.y += petal.speedY * dt;
        petal.x += (petal.speedX + std::sin(sessionTime * 1.2f) * 12.0f) * dt;
        petal.rotationAngle += 1.0f * dt;
        petal.currentScale = petal.baseScale + std::sin(sessionTime * 2.5f + petal.x) * 1.2f;

        if (petal.y > screenHeight || petal.x < 0 || petal.x > screenWidth) {
            petal.y = -20.0f;
            petal.x = static_cast<float>(rand() % static_cast<int>(screenWidth));
            petal.currentScale = petal.baseScale;
        }

        ImVec2 center(petal.x, petal.y);
        float r = petal.currentScale;
        
        // Рисуем форму лепестка без тяжелых текстур (мизерная нагрузка на Intel HD)
        ImVec2 p1(center.x + std::sin(petal.rotationAngle) * r, center.y + std::cos(petal.rotationAngle) * r);
        ImVec2 p2(center.x + std::sin(petal.rotationAngle + 1.5f) * (r * 0.6f), center.y + std::cos(petal.rotationAngle + 1.5f) * (r * 0.6f));
        ImVec2 p3(center.x - std::sin(petal.rotationAngle) * (r * 0.4f), center.y - std::cos(petal.rotationAngle) * (r * 0.4f));
        ImVec2 p4(center.x - std::sin(petal.rotationAngle + 1.5f) * (r * 0.6f), center.y - std::cos(petal.rotationAngle + 1.5f) * (r * 0.6f));

        drawList->AddQuadFilled(p1, p2, p3, p4, petalColor);
    }
}

// Инициализация девайса DirectX 11 поверх окна GLFW
bool InitializeDX11Graphics(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, 
                                             featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, 
                                             &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext))) {
        return false;
    }

    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
    return true;
}

void CleanupDX11Graphics() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

int main(int argc, char* argv[]) {
    // 1. Загрузка оффлайн профилей и путей сессии (Из твоих оригинальных мануалов)
    bunker::EnsureProjectDirectories();
    bunker::SessionProfile sessionProfile;
    const auto profilePath = bunker::DefaultSessionProfilePath();

    if (!bunker::LoadSessionProfile(profilePath, sessionProfile)) {
        sessionProfile = bunker::MakeDefaultSessionProfile();
        bunker::SaveProfileAtomically(sessionProfile, profilePath);
    }
    bunker::NormalizeSessionProfile(sessionProfile);

    // 2. Старт окна GLFW под контекст DX11
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Исключаем старый OpenGL
    
    GLFWwindow* window = glfwCreateWindow(1280, 720, "BUNKER PROTOCOL LAUNCHER [v1.32_DX11]", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }

    HWND hwnd = glfwGetWin32Window(window);
    if (!InitializeDX11Graphics(hwnd)) { CleanupDX11Graphics(); glfwTerminate(); return -1; }

    // 3. Старт бэкендов ImGui под DirectX 11
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    LauncherState launcherState;
    std::snprintf(launcherState.login, sizeof(launcherState.login), "%s", sessionProfile.account.username.c_str());
    
    const char* characters[] = { "Scout", "Mechanic", "Commander" };
    const char* sessionModes[] = { "Solo", "LAN Host", "LAN Client" };
    LauncherDataCache launcherData;
    RefreshLauncherData(launcherData, launcherState, sessionProfile);

    int w, h;
    glfwGetWindowSize(window, &w, &h);
    InitMathematicalSakura(w);

    float lastTime = static_cast<float>(glfwGetTime());
    bool triggerGameStart = false;

    // 4. Главный цикл красивого графического экрана Elder Tales
    while (!glfwWindowShouldClose(window) && !triggerGameStart) {
        glfwPollEvents();

        float currentTime = static_cast<float>(glfwGetTime());
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Захватываем задний фон ImGui для вывода падающих лепестков сакуры
        ImDrawList* backgroundDrawList = ImGui::GetBackgroundDrawList();
        
        // Запускаем синусоидальную анимацию сакуры поверх экрана (0 нагрузки на встроенные GPU)
        int currentWidth, currentHeight;
        glfwGetWindowSize(window, &currentWidth, &currentHeight);
        UpdateAndRenderSakuraAnimation(backgroundDrawList, dt, static_cast<float>(currentWidth), static_cast<float>(currentHeight), currentTime);

        // ОТРИСОВКА ИНТЕРФЕЙСА АВТОРИЗАЦИИ ПО ЦЕНТРУ (Как на твоем скриншоте)
        ImGui::SetNextWindowPos(ImVec2(currentWidth * 0.38f, currentHeight * 0.38f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(340.0f, 200.0f));
        ImGui::Begin("##ElderTalesAuth", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
        
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ELDER TALES MMO NETWORK");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::InputText("Логин", launcherState.login, sizeof(launcherState.login));
ImGui::InputText("Пароль", launcherState.password, sizeof(launcherState.password), ImGuiInputTextFlags_Password);ImGui::Spacing();
// ТВОЕ ИСПРАВЛЕНИЕ: Маленькая кнопка входа (Зеленый круг со скриншота)
if (ImGui::Button("Вход в систему (Authorize)", ImVec2(-1.0f, 40.0f))) {triggerGameStart = true;
     // Запускаем бесшовный переход в игру
     }ImGui::End();
     // ТВОЕ ИСПРАВЛЕНИЕ: КНОПКА ВЫХОДА В ЛЕВОМ НИЖНЕМ УГЛУ
     ImGui::SetNextWindowPos(ImVec2(20.0f, currentHeight - 60.0f), ImGuiCond_Always);
     ImGui::Begin("##ExitPanel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
     if (ImGui::Button("Выход", ImVec2(100, 35))) {glfwSetWindowShouldClose(window, true);
    }ImGui::End();
     // ТВОЕ ИСПРАВЛЕНИЕ: Рендеринг кадра через DirectX 11 (Исправлен синтаксис инициализации массива)
     ImGui::Render();const float clear_color = { 0.05f, 0.02f, 0.08f, 1.0f };
     // Глубокий фиолетовый цвет ночного неба со скрина
     g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
     g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
     ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
     g_pSwapChain->Present(1, 0);
      // V-Sync включен для плавности
}
      // Очистка ImGui окон
      ImGui_ImplDX11_Shutdown();
      ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();
      // 5. ФИНАЛЬНЫЙ ВЫЛЕТ В ЯДРО ИГРЫ: Если нажали вход, пакуем билет сессии и стартуем 
        BunkerGame.exe if (triggerGameStart) {
        // ТВОИ ИСПРАВЛЕНИЯ: Точки с запятой расставлены, вызовы массивов через квадратные скобки
        PrepareSelectedCharacter(sessionProfile, launcherState, characters, 3);
        sessionProfile.sessionMode = sessionModes(launcherState.sessionModeIndex);
        const auto selectedWorld = SelectedWorldPath(launcherData.worlds, launcherState.selectedWorldIndex);
        if (!launcherData.worlds.empty()) {sessionProfile.selectedWorld = bunker::NormalizeWorldReference(selectedWorld.string());
        }bunker::SaveProfileAtomically(sessionProfile, profilePath);
        auto launchSession = SaveLanlineRosterState(launcherState, characters(launcherState.selectedCharacter), selectedWorld, nullptr);
        launchSession.bt72SecondSeatUnlocked = bunker::Bt72SecondSeatUnlocked(sessionProfile);
        launchSession.bt72SecondSeatPolicy = bunker::NormalizeBt72SecondSeatPolicy(sessionProfile.partnerTank.secondSeatPolicy);
        launchSession.bt72TrustedGunnerHandle = sessionProfile.partnerTank.trustedGunnerHandle;
        launchSession.bt72AssignedGunnerHandle = sessionProfile.partnerTank.assignedGunnerHandle;
        SaveLanlineSnapshotAndMaybeActive(launchSession);bunker::LaunchTicketInfo launchTicket;
        launchTicket.accountId = sessionProfile.account.accountId;launchTicket.sessionMode = sessionProfile.sessionMode;
        launchTicket.characterName = sessionProfile.character.displayName;launchTicket.selectedWorld = sessionProfile.selectedWorld;
        launchTicket.lanlineSessionId = launchSession.sessionId;launchTicket.hostEndpoint = launchSession.hostEndpoint;
        launchTicket.bt72SeatRole = "pilot";launchTicket.bt72SecondSeatPolicy = launchSession.bt72SecondSeatPolicy;
        launchTicket.bt72TrustedGunnerHandle = launchSession.bt72TrustedGunnerHandle;launchTicket.launcherRole = "player";
        bunker::IssueLaunchTicket(launchTicket);std::string launchStatus;TryLaunchSiblingExecutable("BunkerGame.exe", launchStatus);
    }CleanupDX11Graphics();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
