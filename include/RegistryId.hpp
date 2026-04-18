#pragma once

#include <string>
#include <string_view>

namespace bunker {

enum class RegistryEntityType {
    User,
    Character,
    Monster,
    Boss,
    Item,
    Structure,
    Transport,
    Unknown
};

class RegistryId {
public:
    static constexpr RegistryEntityType GetType(std::string_view id) {
        if (id.empty()) {
            return RegistryEntityType::Unknown;
        }

        if (id.front() == '[') {
            if (id.size() >= 2 && id.substr(0, 2) == "[%") {
                return RegistryEntityType::Structure;
            }
            if (id.size() >= 4 && id.substr(0, 4) == "[#tr") {
                return RegistryEntityType::Transport;
            }
        }

        if (id.front() == '@') {
            return RegistryEntityType::Character;
        }

        if (id.size() >= 2 && id.substr(0, 2) == "#%") {
            const std::string_view sub = id.substr(2);
            if (sub.rfind("midB_", 0) == 0) {
                return RegistryEntityType::Boss;
            }
            if (sub.rfind("mid_", 0) == 0) {
                return RegistryEntityType::Monster;
            }
            if (sub.rfind("it_", 0) == 0) {
                return RegistryEntityType::Item;
            }
        }

        if (id.front() == '#') {
            return RegistryEntityType::User;
        }

        return RegistryEntityType::Unknown;
    }

    static constexpr bool IsValid(std::string_view id) {
        return GetType(id) != RegistryEntityType::Unknown;
    }

    static constexpr const char* ToString(RegistryEntityType type) {
        switch (type) {
            case RegistryEntityType::User:
                return "User";
            case RegistryEntityType::Character:
                return "Character";
            case RegistryEntityType::Monster:
                return "Monster";
            case RegistryEntityType::Boss:
                return "Boss";
            case RegistryEntityType::Item:
                return "Item";
            case RegistryEntityType::Structure:
                return "Structure";
            case RegistryEntityType::Transport:
                return "Transport";
            case RegistryEntityType::Unknown:
                return "Unknown";
        }
        return "Unknown";
    }

    static std::string MakeUserId(int numericId) {
        return "#" + std::to_string(numericId);
    }

    static std::string MakeCharacterId(int numericId) {
        return "@" + std::to_string(numericId);
    }
};

}  // namespace bunker
