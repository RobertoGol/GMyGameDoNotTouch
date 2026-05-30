#pragma once
#include <string>
#include <vector>
#include "Registry_ID.hpp"
#include "OS_Isolation.hpp"

/**
 * PROJECT: Bunker Protocol
 * MODULE: Auth_Secure
 * DESCRIPTION: Система авторизации (лаунчер) и связь с серверной статистикой.
 */

namespace BunkerProtocol {

    enum class AuthStatus {
        LoggedOut,
        Authenticating,
        Authorized,
        Denied,
        ConnectionError
    };

    struct UserSession {
        std::string accountID; // Формат #XXXXXXX
        std::string sessionToken;
        AuthStatus status = AuthStatus::LoggedOut;
    };

    class AuthSecure {
    private:
        UserSession currentSession;
        std::string lastErrorMessage;

    public:
        AuthSecure() = default;

        /**
         * Попытка входа (стиль лаунчера Ancient Tales)
         * Передает логин, пароль и данные о железе для аналитики.
         */
        bool Login(const std::string& login, const std::string& password, const OSIsolation& sys) {
            currentSession.status = AuthStatus::Authenticating;

            // 1. Формируем пакет данных о системе
            std::string hardwareLog = sys.GetTelemetryString();
            
            // 2. Логика проверки (в будущем - запрос к серверу)
            // Имитируем успех для разработки
            if (!login.empty() && !password.empty()) {
                currentSession.accountID = "#" + std::to_string(10000 + rand() % 90000);
                currentSession.status = AuthStatus::Authorized;
                
                std::cout << "[Auth] User " << login << " logged in as " 
                          << currentSession.accountID << " on " << hardwareLog << std::endl;
                return true;
            }

            currentSession.status = AuthStatus::Denied;
            lastErrorMessage = "Invalid credentials";
            return false;
        }

        /**
         * Регистрация нового аккаунта
         */
        void Register(const std::string& login, const std::string& email, const std::string& password) {
            // Здесь будет POST запрос к базе данных
            std::cout << "[Auth] Registration request for: " << login << std::endl;
        }

        // Геттеры
        const UserSession& GetSession() const { return currentSession; }
        std::string GetLastError() const { return lastErrorMessage; }
        
        bool IsLoggedIn() const { 
            return currentSession.status == AuthStatus::Authorized; 
        }

        /**
         * Проверка безопасности: валидация токена перед загрузкой мира
         */
        bool ValidateSession() {
            return !currentSession.sessionToken.empty();
        }
    };
}
