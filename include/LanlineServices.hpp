#pragma once

#include <cstdint>
#include <ctime>
#include <filesystem>
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
    bool towerSyncRecovered = false;
    bool firstTowerActivated = false;
    bool localRelayAvailable = false;
    bool relaySubstationActive = false;
    bool serviceBayActive = false;
    bool waterReclaimerActive = false;
    bool backboneStable = false;
    bool feyRingIntercityUnlocked = false;
    bool feyRingInterserverUnlocked = false;
    bool intercityPortalsUnlocked = false;
    bool interserverPortalsUnlocked = false;
};

inline bool IsLanlineServicesUnlocked(const ServicesUnlockState& state) {
    return state.towerSyncRecovered && state.tier != ServicesUnlockTier::Locked;
}

inline bool IsTankServiceUnlocked(const ServicesUnlockState& state) {
    return state.serviceBayActive || state.tier >= ServicesUnlockTier::BackboneStable;
}

inline bool IsMedicalSupportUnlocked(const ServicesUnlockState& state) {
    return state.localRelayAvailable || state.tier >= ServicesUnlockTier::TowerLinked;
}

inline bool IsIntercityPortalScheduleUnlocked(const ServicesUnlockState& state) {
    return state.feyRingIntercityUnlocked || state.intercityPortalsUnlocked;
}

inline bool IsInterserverPortalScheduleUnlocked(const ServicesUnlockState& state) {
    return state.feyRingInterserverUnlocked || state.interserverPortalsUnlocked;
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
    Delivered,
    Claimed
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
    StoreCurrency paymentCurrency = StoreCurrency::InGame;
    int priceCredits = 0;
    std::int64_t createdAtUnix = 0;
    std::int64_t etaUnix = 0;
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
    std::vector<std::string> ownedCosmetics{};
};

struct LanlineServicesSave {
    int relayCredits = 420;
    VoiceSettings voice{};
    std::vector<SupportOrder> supportOrders{};
    std::vector<std::string> ownedCosmetics{};
};

ServicesUnlockState BuildServicesUnlockState(const SessionProfile& profile, const WorldFieldState* worldState);
bool IsAllowedSupportItem(const SupportCatalogItem& item);
std::vector<SupportCatalogItem> MakeDefaultSupportCatalog();
std::vector<FeyGateCycle> MakeDefaultFeyGateCycles(std::int64_t nowUnix);
LanlineServicesState MakeDefaultLanlineServicesState(std::int64_t nowUnix);
LanlineServicesState MakeLanlineServicesStateFromSave(const LanlineServicesSave& save, std::int64_t nowUnix);
LanlineServicesSave BuildLanlineServicesSave(const LanlineServicesState& state);
void ApplyLanlineServicesProfileSnapshot(LanlineServicesState& state, const LanlineServicesProfile& profile);
void SyncLanlineServicesProfileSnapshot(LanlineServicesProfile& profile, const LanlineServicesState& state);
void SyncLanlineServicesSessionProfile(SessionProfile& profile, const LanlineServicesState& state);
void AdvanceLanlineSupportOrders(LanlineServicesState& state, std::int64_t nowUnix);
int CountSupportOrdersInState(const LanlineServicesState& state, SupportOrderState orderState);
int ClaimDeliveredSupportOrders(LanlineServicesState& state, SessionProfile& profile, std::string* summary = nullptr);
ServiceHubMode ResolveLanlineServicesMode(const ServicesUnlockState& unlockState, const LanlineSessionState* sessionState);
void SyncLanlineServicesPresence(LanlineServicesState& state,
    const LanlineSessionState* sessionState,
    const ServicesUnlockState& unlockState);
FeyGateState ComputeFeyGateState(const FeyGateCycle& gate, std::int64_t nowUnix);
std::string FormatCountdown(std::int64_t secondsRemaining);
std::filesystem::path DefaultLanlineServicesSavePath();
bool SaveLanlineServicesSave(const LanlineServicesSave& save, const std::filesystem::path& path);
bool LoadLanlineServicesSave(const std::filesystem::path& path, LanlineServicesSave& outSave);
void DrawLanlineServicesPanel(LanlineServicesState& state,
    const ServicesUnlockState& unlockState,
    std::int64_t nowUnix);

}  // namespace bunker
