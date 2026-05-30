#pragma once

#include <cstdint>
#include <cstddef>
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
    bool merchantWindowActive = false;
    int routeEventsResolved = 0;
    int routeEventsFailed = 0;
    int routeEventsExpired = 0;
    std::string backboneStage{};
    std::string backboneStatus{};
    std::string backbonePayoff{};
    std::string routeEventSummary{};
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

struct NetworkWeaponVisual {
    std::uint32_t weaponRegistryIdHash = 0;
    std::uint8_t receiverId = 0;
    std::uint8_t barrelId = 0;
    std::uint8_t magazineId = 0;
    std::uint8_t muzzleId = 0;
    std::uint8_t paintJobId = 0;
    std::uint8_t wearLevel = 0;
    std::uint8_t metallicGloss = 0;
};

struct NetworkApparelVisual {
    std::uint8_t undergarmentId = 0;
    std::uint8_t armorPlatesId = 0;
    std::uint8_t decalId = 0;
    std::uint8_t decalPosition = 0;
    std::uint32_t customColorHEX = 0;
};

struct NetworkPlayerVisualSnapshot {
    std::uint64_t networkPlayerId = 0;
    char characterName[32]{};
    NetworkWeaponVisual equippedWeapon{};
    NetworkApparelVisual apparel{};
    float positionX = 0.0f;
    float positionZ = 0.0f;
    float rotationY = 0.0f;
};

using LanlineVisualSnapshotBroadcast = bool (*)(const std::uint8_t* data, std::size_t size);

extern std::vector<NetworkPlayerVisualSnapshot> g_ConnectedTeammates;

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
void HandleIncomingPlayerSnapshot(const std::uint8_t* rawNetworkBuffer, std::size_t bufferSize);
void InjectOfflineDebugBot();
void InjectOfflineDebugBot(const SessionProfile& profile);
void SetLanlineVisualSnapshotBroadcast(LanlineVisualSnapshotBroadcast broadcast);
bool SendMyVisualSnapshotToNetwork(const SessionProfile& profile, float currentX, float currentZ, float rotation);
void DrawLanlineServicesPanel(LanlineServicesState& state,
    const ServicesUnlockState& unlockState,
    std::int64_t nowUnix);

}  // namespace bunker
