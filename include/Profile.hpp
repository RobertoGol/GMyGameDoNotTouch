#pragma once

#include <algorithm>
#include <cctype>
#include <ctime>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "AppPaths.hpp"
#include "RegistryId.hpp"

namespace bunker {

struct SpecialStats {
    int strength = 5;
    int perception = 5;
    int endurance = 5;
    int charisma = 5;
    int intelligence = 5;
    int agility = 5;
    int luck = 5;
};

struct InventoryEntry {
    std::string itemId;
    int count = 0;
    float unitWeight = 0.0f;
};

struct AccountProfile {
    std::string accountId = "#10001";
    std::string username = "wanderer";
    std::string email = "local@bunker";
    std::time_t registerDate = 0;
    int totalPlayTimeMinutes = 0;
    std::vector<std::string> linkedCharacters = {"@20001", "@20002", "@20003"};
};

struct CharacterProfile {
    std::string characterId = "@20001";
    std::string displayName = "Scout";
    int level = 1;
    float hp = 100.0f;
    float maxHp = 100.0f;
    float mp = 75.0f;
    float maxMp = 75.0f;
    float carryWeight = 0.0f;
    SpecialStats special{};
    std::vector<InventoryEntry> inventory{};

    int StatValue(char statCode) const {
        switch (static_cast<char>(std::toupper(static_cast<unsigned char>(statCode)))) {
            case 'S':
                return special.strength;
            case 'P':
                return special.perception;
            case 'E':
                return special.endurance;
            case 'C':
                return special.charisma;
            case 'I':
                return special.intelligence;
            case 'A':
                return special.agility;
            case 'L':
                return special.luck;
            default:
                return 0;
        }
    }
};

struct SessionProfile {
    AccountProfile account{};
    CharacterProfile character{};
    std::string selectedWorld = "start_zone.bwld";
    std::string sessionMode = "Solo";
};

inline SessionProfile MakeDefaultSessionProfile() {
    SessionProfile profile;
    profile.account.registerDate = std::time(nullptr);
    profile.character.inventory.push_back({"#%it_field_ration", 2, 0.3f});
    profile.character.inventory.push_back({"#%it_ptrs_ammo", 8, 0.7f});
    profile.character.carryWeight = 6.2f;
    return profile;
}

inline void NormalizeSessionProfile(SessionProfile& profile) {
    if (profile.account.accountId.empty() || !RegistryId::IsValid(profile.account.accountId)) {
        profile.account.accountId = "#10001";
    }
    if (profile.character.characterId.empty() || !RegistryId::IsValid(profile.character.characterId)) {
        profile.character.characterId = "@20001";
    }
    if (profile.account.username.empty()) {
        profile.account.username = "wanderer";
    }
    if (profile.character.displayName.empty()) {
        profile.character.displayName = "Scout";
    }
    if (profile.account.registerDate == 0) {
        profile.account.registerDate = std::time(nullptr);
    }
    while (profile.account.linkedCharacters.size() < 3) {
        profile.account.linkedCharacters.push_back(RegistryId::MakeCharacterId(20001 + static_cast<int>(profile.account.linkedCharacters.size())));
    }
    if (profile.character.inventory.empty()) {
        profile.character.inventory.push_back({"#%it_field_ration", 1, 0.3f});
    }
}

inline bool SaveSessionProfile(const SessionProfile& profile, const fs::path& filePath) {
    std::ofstream out(filePath);
    if (!out.is_open()) {
        return false;
    }

    out << "account_id=" << profile.account.accountId << '\n';
    out << "username=" << profile.account.username << '\n';
    out << "email=" << profile.account.email << '\n';
    out << "register_date=" << static_cast<long long>(profile.account.registerDate) << '\n';
    out << "playtime_minutes=" << profile.account.totalPlayTimeMinutes << '\n';
    out << "character_id=" << profile.character.characterId << '\n';
    out << "character_name=" << profile.character.displayName << '\n';
    out << "character_level=" << profile.character.level << '\n';
    out << "hp=" << profile.character.hp << '\n';
    out << "max_hp=" << profile.character.maxHp << '\n';
    out << "mp=" << profile.character.mp << '\n';
    out << "max_mp=" << profile.character.maxMp << '\n';
    out << "carry_weight=" << profile.character.carryWeight << '\n';
    out << "special=S:" << profile.character.special.strength
        << ",P:" << profile.character.special.perception
        << ",E:" << profile.character.special.endurance
        << ",C:" << profile.character.special.charisma
        << ",I:" << profile.character.special.intelligence
        << ",A:" << profile.character.special.agility
        << ",L:" << profile.character.special.luck << '\n';
    out << "session_mode=" << profile.sessionMode << '\n';
    out << "selected_world=" << profile.selectedWorld << '\n';

    for (const auto& id : profile.account.linkedCharacters) {
        out << "linked_character=" << id << '\n';
    }
    for (const auto& item : profile.character.inventory) {
        out << "inventory=" << item.itemId << ',' << item.count << ',' << item.unitWeight << '\n';
    }

    return true;
}

inline bool LoadSessionProfile(const fs::path& filePath, SessionProfile& outProfile) {
    std::ifstream in(filePath);
    if (!in.is_open()) {
        return false;
    }

    outProfile = MakeDefaultSessionProfile();
    outProfile.account.linkedCharacters.clear();
    outProfile.character.inventory.clear();

    std::string line;
    while (std::getline(in, line)) {
        const auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, pos);
        const std::string value = line.substr(pos + 1);

        if (key == "account_id") outProfile.account.accountId = value;
        else if (key == "username") outProfile.account.username = value;
        else if (key == "email") outProfile.account.email = value;
        else if (key == "register_date") outProfile.account.registerDate = static_cast<std::time_t>(std::stoll(value));
        else if (key == "playtime_minutes") outProfile.account.totalPlayTimeMinutes = std::stoi(value);
        else if (key == "character_id") outProfile.character.characterId = value;
        else if (key == "character_name") outProfile.character.displayName = value;
        else if (key == "character_level") outProfile.character.level = std::stoi(value);
        else if (key == "hp") outProfile.character.hp = std::stof(value);
        else if (key == "max_hp") outProfile.character.maxHp = std::stof(value);
        else if (key == "mp") outProfile.character.mp = std::stof(value);
        else if (key == "max_mp") outProfile.character.maxMp = std::stof(value);
        else if (key == "carry_weight") outProfile.character.carryWeight = std::stof(value);
        else if (key == "session_mode") outProfile.sessionMode = value;
        else if (key == "selected_world") outProfile.selectedWorld = value;
        else if (key == "linked_character") outProfile.account.linkedCharacters.push_back(value);
        else if (key == "inventory") {
            const auto first = value.find(',');
            const auto second = value.find(',', first == std::string::npos ? first : first + 1);
            if (first != std::string::npos && second != std::string::npos) {
                outProfile.character.inventory.push_back(
                    {value.substr(0, first), std::stoi(value.substr(first + 1, second - first - 1)), std::stof(value.substr(second + 1))});
            }
        } else if (key == "special") {
            std::size_t start = 0;
            while (start < value.size()) {
                const std::size_t next = value.find(',', start);
                const std::string token = value.substr(start, next == std::string::npos ? std::string::npos : next - start);
                if (token.size() >= 3 && token[1] == ':') {
                    const int statValue = std::stoi(token.substr(2));
                    switch (token[0]) {
                        case 'S': outProfile.character.special.strength = statValue; break;
                        case 'P': outProfile.character.special.perception = statValue; break;
                        case 'E': outProfile.character.special.endurance = statValue; break;
                        case 'C': outProfile.character.special.charisma = statValue; break;
                        case 'I': outProfile.character.special.intelligence = statValue; break;
                        case 'A': outProfile.character.special.agility = statValue; break;
                        case 'L': outProfile.character.special.luck = statValue; break;
                    }
                }
                if (next == std::string::npos) {
                    break;
                }
                start = next + 1;
            }
        }
    }

    NormalizeSessionProfile(outProfile);

    return true;
}

}  // namespace bunker
