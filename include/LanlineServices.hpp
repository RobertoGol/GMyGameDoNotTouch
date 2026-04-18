#pragma once

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

#include "LanlineSession.hpp"
#include "SessionProfiles.hpp"

namespace bunker {

enum class ServiceHubMode {
    OfflineLocal,
    LanlineLocal,
    RelayOnline
};

enum class ServicesUnlockTier {
    Locked,
    TowerLinked,
    BackboneStable,
    RelayExpanded
};

struct ServicesUnlockState {
    ServicesUnlockTier tier = ServicesUnlockTier::Locked;
    bool firstTowerActivated = false;
    bool localRelayAvailable = false;
    bool backboneStable = false;
    bool intercityPortalsUnlocked = false;
    bool interserverPortalsUnlocked = false;
};

inline bool IsLanlineServicesUnlocked(const ServicesUnlockState& state) {
    return state.firstTowerActivated && state.tier != ServicesUnlockTier::Locked;
}

inline bool IsTankServiceUnlocked(const ServicesUnlockState& state) {
    return state.tier >= ServicesUnlockTier::BackboneStable;
}

inline bool IsMedicalSupportUnlocked(const ServicesUnlockState& state) {
    return state.tier >= ServicesUnlockTier::TowerLinked;
}

inline bool IsIntercityPortalScheduleUnlocked(const ServicesUnlockState& state) {
    return state.tier >= ServicesUnlockTier::BackboneStable;
}

inline bool IsInterserverPortalScheduleUnlocked(const ServicesUnlockState& state) {
    return state.tier >= ServicesUnlockTier::RelayExpanded;
}

enum class SupportCategory {
    Materials,
    Skins,
    Cosmetics,
    Utility,
    TankService,
    Medical
};

enum class StoreCurrency {
    InGame,
    SymbolicSupport
};

enum class TankSubsystem {
    None,
    Hull,
    Suspension,
    Turret,
    Engine,
    Sensors,
    PowerCore
};

enum class SupportOrderState {
    Draft,
    Queued,
    Routed,
    Delivered
};

enum class FeyGateState {
    Closed,
    Open,
    Transit
};

struct FriendEntry {
    std::string handle;
    std::string nodeLabel;
    std::string statusText;
    bool online = false;
    bool inCurrentSession = false;
    bool voiceMuted = false;
    float voiceVolume = 1.0f;
};

struct ChatMessageEntry {
    std::string author;
    std::string body;
    std::string timeLabel;
};

struct ChatChannel {
    std::string id;
    std::string label;
    std::vector<ChatMessageEntry> messages{};
};

struct VoiceSettings {
    bool enabled = true;
    bool pushToTalk = true;
    char pushToTalkKey[16] = "V";
    float inputSensitivity = 0.55f;
    float inputGain = 1.0f;
    float outputGain = 1.0f;
    int selectedInputDevice = 0;
    int selectedOutputDevice = 0;
};

struct SupportCatalogItem {
    std::string id;
    std::string label;
    std::string description;
    SupportCategory category = SupportCategory::Materials;
    StoreCurrency currency = StoreCurrency::InGame;
    int priceCredits = 0;
    std::string supportLabel;
    bool grantsWeapon = false;
    bool grantsCompletedTank = false;
    bool combatAdvantage = false;
    TankSubsystem tankSubsystem = TankSubsystem::None;
    std::vector<std::string> contents{};
};

struct SupportOrder {
    std::string orderId;
    std::string itemId;
    std::string itemLabel;
    std::string destinationNode = "Shelter 17";
    SupportOrderState state = SupportOrderState::Draft;
};

struct FeyGateCycle {
    std::string id;
    std::string originLabel;
    std::string destinationLabel;
    std::string routeType;
    FeyGateState state = FeyGateState::Closed;
    std::int64_t opensAtUnix = 0;
    std::int64_t closesAtUnix = 0;
    std::int64_t nextWindowUnix = 0;
    int queueSize = 0;
    int capacity = 0;
    bool interServer = false;
    bool unstable = false;
};

struct LanlineServicesState {
    ServiceHubMode mode = ServiceHubMode::OfflineLocal;
    int relayCredits = 420;
    std::vector<FriendEntry> friends{};
    std::vector<ChatChannel> chatChannels{};
    VoiceSettings voice{};
    std::vector<SupportCatalogItem> supportCatalog{};
    std::vector<SupportOrder> supportOrders{};
    std::vector<FeyGateCycle> feyGateCycles{};
};

ServicesUnlockState BuildServicesUnlockState(const SessionProfile& profile, const WorldFieldState* worldState);
bool IsAllowedSupportItem(const SupportCatalogItem& item);
std::vector<SupportCatalogItem> MakeDefaultSupportCatalog();
std::vector<FeyGateCycle> MakeDefaultFeyGateCycles(std::int64_t nowUnix);
LanlineServicesState MakeDefaultLanlineServicesState(std::int64_t nowUnix);
ServiceHubMode ResolveLanlineServicesMode(const ServicesUnlockState& unlockState, const LanlineSessionState* sessionState);
void SyncLanlineServicesPresence(LanlineServicesState& state,
    const LanlineSessionState* sessionState,
    const ServicesUnlockState& unlockState);
FeyGateState ComputeFeyGateState(const FeyGateCycle& gate, std::int64_t nowUnix);
std::string FormatCountdown(std::int64_t secondsRemaining);
void DrawLanlineServicesPanel(LanlineServicesState& state,
    const ServicesUnlockState& unlockState,
    std::int64_t nowUnix);

}  // namespace bunker
