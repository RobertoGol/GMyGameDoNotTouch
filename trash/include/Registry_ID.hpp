#pragma once
#include <string>
#include <string_view>

/**
 * PROJECT: Bunker Protocol
 * MODULE: Registry_ID
 * DESCRIPTION: Единая система идентификации сущностей (#, @, [%, [#)
 */

namespace BunkerProtocol {

    enum class EntityType {
        User,        // #12345
        Character,   // @12345
        Monster,     // #%mid_12345
        Boss,        // #%midB_12345
        Item,        // #%it_12345
        Structure,   // [% 12345 ]
        Transport,   // [#tr12345 ]
        Unknown
    };

    class RegistryID {
    public:
        // Конструктор по умолчанию
        RegistryID() = default;

        /**
         * Определяет тип сущности на основе префикса строки.
         * Оптимизировано для работы через Proton на слабых машинах.
         */
        static constexpr EntityType GetTypeFromID(std::string_view id) {
            if (id.empty()) return EntityType::Unknown;

            // 1. Проверка на строения и транспорт (квадратные скобки)
            if (id.front() == '[') {
                if (id.size() > 3 && id.substr(0, 2) == "[%") return EntityType::Structure;
                if (id.size() > 4 && id.substr(0, 4) == "[#tr") return EntityType::Transport;
            }

            // 2. Проверка на персонажа
            if (id.front() == '@') return EntityType::Character;

            // 3. Проверка на системные префиксы (#%)
            if (id.size() > 2 && id.substr(0, 2) == "#%") {
                std::string_view sub = id.substr(2);
                if (sub.starts_with("midB_")) return EntityType::Boss;
                if (sub.starts_with("mid_"))  return EntityType::Monster;
                if (sub.starts_with("it_"))   return EntityType::Item;
            }

            // 4. Проверка на базовый ID аккаунта
            if (id.front() == '#') return EntityType::User;

            return EntityType::Unknown;
        }

        /**
         * Валидация ID: проверяет, соответствует ли строка формату Bunker Protocol
         */
        static bool IsValid(std::string_view id) {
            return GetTypeFromID(id) != EntityType::Unknown;
        }

        // Вспомогательная функция для отладки и вывода в лог чата
        static std::string_view TypeToString(EntityType type) {
            switch (type) {
                case EntityType::User:      return "USER";
                case EntityType::Character: return "CHAR";
                case EntityType::Monster:   return "MONSTER";
                case EntityType::Boss:      return "BOSS";
                case EntityType::Item:      return "ITEM";
                case EntityType::Structure: return "STRUCT";
                case EntityType::Transport: return "TRANS";
                default:                    return "UNKNOWN";
            }
        }
    };
}
