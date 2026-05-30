#pragma once
#include <memory>
#include <vector>
#include <iostream>
#include "OS_Isolation.hpp"
#include "Registry_ID.hpp"

/**
 * PROJECT: Bunker Protocol
 * MODULE: Engine_Core
 * DESCRIPTION: Связующее звено. Управляет состояниями и передает данные железа в UI.
 */

namespace BunkerProtocol {

    enum class GameState {
        Boot,           // Инициализация
        Launcher,       // Авторизация и сбор статистики
        WorldLoading,   // Загрузка локаций (.map)
        ActivePlay,     // Игровой процесс
        Paused
    };

    class EngineCore {
    private:
        GameState currentState = GameState::Boot;
        std::unique_ptr<OSIsolation> sysIsolation;
        
        std::string startTimeSync;
        int activeGraphicsPreset = 1; // По умолчанию Medium

    public:
        EngineCore() {
            // 1. Сразу поднимаем изоляцию и чекаем железо
            sysIsolation = std::make_unique<OSIsolation>();
            startTimeSync = OSIsolation::GetSystemTimeSync();
            
            // 2. Получаем пресет графики на основе RAM из OS_Isolation
            activeGraphicsPreset = sysIsolation->GetSpecs().graphicsPreset;
        }

        /**
         * Инициализация с проверкой безопасности
         */
        bool Initialize() {
            std::cout << "[Core] Initializing Bunker Protocol..." << std::endl;
            
            // Проверка путей
            if (!sysIsolation->IsPathSafe(sysIsolation->GetAssetsPath())) {
                std::cerr << "[Core] Critical Error: Assets path is outside sandbox!" << std::endl;
                return false; 
            }

            // Вывод данных о системе в лог (как в Log Horizon)
            std::cout << "[Core] Hardware detected: " << sysIsolation->GetTelemetryString() << std::endl;
            std::cout << "[Core] Time Synced: " << startTimeSync << std::endl;

            SetState(GameState::Launcher);
            return true;
        }

        /**
         * Переключение состояний
         */
        void SetState(GameState newState) {
            currentState = newState;
            // Здесь будет триггер для UI_Manager (например, показать форму логина)
        }

        /**
         * Данные для HUD (Тройное время + Статы железа)
         */
        struct CoreUIData {
            std::string localTime;
            std::string cpuInfo;
            std::string ramInfo;
            int gpuPreset;
        };

        CoreUIData GetInfoForUI() const {
            const auto& specs = sysIsolation->GetSpecs();
            return {
                startTimeSync,
                specs.cpuName,
                std::to_string(specs.ramTotalMB) + " MB",
                specs.graphicsPreset
            };
        }

        // Геттеры для других систем
        GameState GetCurrentState() const { return currentState; }
        OSIsolation& GetSystem() { return *sysIsolation; }
    };
}
