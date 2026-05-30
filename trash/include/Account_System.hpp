#pragma once
#include <string>
#include <vector>
#include <ctime>
#include "Registry_ID.hpp"

/**
 * PROJECT: Bunker Protocol
 * MODULE: Account_System
 * DESCRIPTION: Управление учетной записью пользователя (#ID) и списком персонажей (@ID).
 */

namespace BunkerProtocol {

    struct AccountInfo {
        std::string accountID;     // Основной #12345
        std::string username;
        std::string email;
        std::time_t registerDate;
        int totalPlayTimeMinutes;
    };

    class AccountSystem {
    private:
        AccountInfo activeAccount;
        std::vector<std::string> linkedCharacters; // Список @ID персонажей
        bool isAccountLoaded = false;

    public:
        AccountSystem() = default;

        /**
         * Инициализация данных аккаунта после успешного Login в Auth_Secure.
         */
        void LoadAccount(const std::string& id, const std::string& name) {
            activeAccount.accountID = id;
            activeAccount.username = name;
            activeAccount.registerDate = std::time(nullptr);
            activeAccount.totalPlayTimeMinutes = 0;
            isAccountLoaded = true;
            
            std::cout << "[Account] Profile loaded: " << name << " [" << id << "]" << std::endl;
        }

        /**
         * Привязка нового персонажа к аккаунту.
         */
        void LinkCharacter(const std::string& charID) {
            if (RegistryID::GetTypeFromID(charID) == EntityType::Character) {
                linkedCharacters.push_back(charID);
            }
        }

        /**
         * Получение всех доступных героев для меню выбора (Ancient Tales style).
         */
        const std::vector<std::string>& GetCharacterList() const {
            return linkedCharacters;
        }

        const AccountInfo& GetInfo() const { return activeAccount; }
        bool IsReady() const { return isAccountLoaded; }

        /**
         * Сохранение прогресса аккаунта через OS_Isolation (вызывается при выходе).
         */
        void UpdatePlayTime(int additionalMinutes) {
            activeAccount.totalPlayTimeMinutes += additionalMinutes;
        }
    };
}
