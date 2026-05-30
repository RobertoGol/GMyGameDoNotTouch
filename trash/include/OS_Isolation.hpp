#pragma once
#include <string>
#include <filesystem>
#include <iostream>
#include <chrono>
#include <ctime>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/sysinfo.h>
    #include <sys/utsname.h>
#endif

namespace BunkerProtocol {

    namespace fs = std::filesystem;

    struct SystemSpecs {
        std::string cpuName;
        uint64_t ramTotalMB;
        std::string osVersion;
        int graphicsPreset; // 0 - Low, 1 - Medium, 2 - High
    };

    class OSIsolation {
    private:
        fs::path appDataPath;
        fs::path gameRootPath;
        SystemSpecs specs;

    public:
        OSIsolation() {
            gameRootPath = fs::current_path();
            InitAppDataPath();
            CollectHardwareInfo();
        }

        /**
         * Инициализация путей: AppData для логов/сейвов и проверка папки world.
         */
        void InitAppDataPath() {
#ifdef _WIN32
            const char* localAppData = std::getenv("LOCALAPPDATA");
            if (localAppData) appDataPath = fs::path(localAppData) / "BunkerProtocol";
            else appDataPath = gameRootPath / "userdata";
#else
            const char* home = std::getenv("HOME");
            if (home) appDataPath = fs::path(home) / ".local/share/BunkerProtocol";
            else appDataPath = gameRootPath / "userdata";
#endif
            // Создаем структуру папок, если её нет
            fs::create_directories(appDataPath / "logs");
            fs::create_directories(appDataPath / "saves");
            
            // Проверяем наличие папки world в корне проекта
            if (!fs::exists(gameRootPath / "world")) {
                fs::create_directories(gameRootPath / "world");
            }
        }

        void CollectHardwareInfo() {
#ifdef _WIN32
            specs.osVersion = "Windows";
            MEMORYSTATUSEX status;
            status.dwLength = sizeof(status);
            GlobalMemoryStatusEx(&status);
            specs.ramTotalMB = status.ullTotalPhys / (1024 * 1024);
#else
            struct utsname name;
            uname(&name);
            specs.osVersion = std::string(name.sysname) + " " + name.release;
            struct sysinfo si;
            sysinfo(&si);
            specs.ramTotalMB = (si.totalram * si.mem_unit) / (1024 * 1024);
#endif
            if (specs.ramTotalMB < 4096) specs.graphicsPreset = 0;
            else if (specs.ramTotalMB < 8192) specs.graphicsPreset = 1;
            else specs.graphicsPreset = 2;

            specs.cpuName = "Generic x86_64 CPU"; 
        }

        /**
         * Проверка: не пытается ли код выйти за пределы папки игры или AppData.
         */
        bool IsPathSafe(const fs::path& target) const {
            std::string targetStr = fs::absolute(target).string();
            return (targetStr.find(gameRootPath.string()) == 0 || 
                    targetStr.find(appDataPath.string()) == 0);
        }

        /**
         * Синхронизация времени ПК для Hardware Monitor.
         */
        static std::string GetSystemTimeSync() {
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            char buf[10];
            std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now_c));
            return std::string(buf);
        }

        // Геттеры путей
        fs::path GetWorldPath() const { return gameRootPath / "world"; }
        fs::path GetAssetsPath() const { return gameRootPath / "assets"; }
        std::string GetTelemetryString() const {
            return "OS: " + specs.osVersion + " | RAM: " + std::to_string(specs.ramTotalMB) + "MB";
        }
        const SystemSpecs& GetSpecs() const { return specs; }
    };
}
