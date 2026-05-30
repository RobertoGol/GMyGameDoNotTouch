#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

// Подключаем все наши модули
#include "include/Engine_Core.hpp"
#include "include/OS_Isolation.hpp"
#include "include/Auth_Secure.hpp"
#include "include/Account_System.hpp"
#include "include/World_Resources.hpp"
#include "include/UI_Overlay.hpp"
#include "include/UI_Ping.hpp"

using namespace BunkerProtocol;

int main() {
    // 1. ИНИЦИАЛИЗАЦИЯ СИСТЕМЫ ИЗОЛЯЦИИ
    auto sys = std::make_unique<OSIsolation>();
    std::cout << "[System] " << sys->GetTelemetryString() << std::endl;
    std::cout << "[System] Time Synced: " << OSIsolation::GetSystemTimeSync() << std::endl;

    // 2. ЗАПУСК ЯДРА (Engine Core)
    auto core = std::make_unique<EngineCore>();
    if (!core->Initialize()) {
        std::cerr << "[Critical] Engine failed to start. Security breach or missing folders!" << std::endl;
        return -1;
    }

    // 3. АВТОРИЗАЦИЯ И ЛАУНЧЕР (Style: Ancient Tales)
    AuthSecure auth;
    AccountSystem account;
    
    std::cout << "\n--- Welcome to Bunker Protocol Launcher ---" << std::endl;
    
    // Имитируем процесс логина (здесь будет ввод из UI)
    if (auth.Login("Survivor_01", "secure_pass", *sys)) {
        account.LoadAccount(auth.GetSession().accountID, "Survivor_01");
        core->SetState(GameState::WorldLoading);
    } else {
        return -1;
    }

    // 4. ЗАГРУЗКА МИРА ИЗ ПАПКИ /world/
    WorldResources world;
    if (core->GetCurrentState() == GameState::WorldLoading) {
        // Пробуем загрузить тестовый файл (убедись, что он есть в папке world)
        if (world.LoadWorld("start_zone", *sys)) {
            core->SetState(GameState::ActivePlay);
        } else {
            std::cout << "[Warning] Could not load 'start_zone.wld'. Create it in /world/ folder!" << std::endl;
        }
    }

    // 5. ИГРОВОЙ ЦИКЛ (HUD и Статистика)
    UIPing ping;
    UIOverlay overlay;
    ping.SetPing(42); // Имитируем стабильный линк

    std::cout << "\n--- Bunker Protocol Active Play Mode ---" << std::endl;
    std::cout << "[HUD] Active ID: " << auth.GetSession().accountID << std::endl;
    std::cout << "[HUD] Connection: " << ping.GetPingString() << std::endl;

    // Имитируем работу системы (1 шаг цикла)
    if (core->GetCurrentState() == GameState::ActivePlay) {
        std::cout << "[Game] Player is now in: " << world.GetCurrentLocation().areaName << std::endl;
        // Здесь начинается работа Camera_Avatar и Input_Direct
    }

    std::cout << "\n[System] Shutdown initiated. Saving account data..." << std::endl;
    account.UpdatePlayTime(1); // Добавим минуту прогресса
    
    return 0;
}
