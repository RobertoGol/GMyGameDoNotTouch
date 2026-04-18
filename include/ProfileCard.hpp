#pragma once

#include <string>

#include "SessionProfiles.hpp"

namespace bunker {

enum class AccessTier {
    Resident,
    Mechanic,
    Command,
};

struct ProfileCard {
    std::string cardId;
    std::string holderName;
    std::string linkedAccountId;
    std::string linkedCharacterId;
    AccessTier accessTier = AccessTier::Resident;
    bool active = true;
};

inline const char* ToString(AccessTier tier) {
    switch (tier) {
        case AccessTier::Resident:
            return "Resident";
        case AccessTier::Mechanic:
            return "Mechanic";
        case AccessTier::Command:
            return "Command";
    }
    return "Unknown";
}

inline ProfileCard MakeProfileCard(const SessionProfile& profile) {
    ProfileCard card;
    card.cardId = "CARD-" + profile.account.accountId.substr(1) + "-" + profile.character.characterId.substr(1);
    card.holderName = profile.character.displayName;
    card.linkedAccountId = profile.account.accountId;
    card.linkedCharacterId = profile.character.characterId;
    card.accessTier = profile.partnerTank.bondedToPilot ? AccessTier::Mechanic : AccessTier::Resident;
    return card;
}

}  // namespace bunker
