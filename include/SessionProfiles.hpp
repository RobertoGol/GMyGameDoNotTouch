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

struct TapeEntry {
    std::string tapeId;
    std::string title;
    bool played = false;
    bool damaged = false;
    bool reconstructed = false;
};

struct PassiveSkill {
    std::string skillId;
    std::string displayName;
    bool unlocked = false;
    bool equipped = false;
};

struct SpecialistEntry {
    std::string specialistId;
    std::string displayName;
    std::string role;
    std::string assignment = "unassigned";
    bool awakened = false;
};

struct LanlineServicesProfile {
    int relayCredits = 420;
    std::vector<std::string> ownedCosmetics{};
    std::vector<std::string> pendingSupportOrders{};
    bool serviceHubKnown = false;
    bool cosmeticsShopSeen = false;
};

struct LauncherAnnouncementState {
    int lastSeenBuildNumber = 0;
    std::string lastSeenAnnouncementId{};
    std::string lastSeenVersionLabel{};
};

struct WorldFieldState {
    std::string worldName;
    float etherErosion = 0.0f;
    float infrastructureDecay = 0.0f;
    bool towerSyncRecovered = false;
    bool localRelayAvailable = false;
    bool regionalGridOnline = false;
    bool caravanRouteActive = false;
    bool droneStationsActive = false;
    bool tradeNetworkActive = false;
    bool railFreightActive = false;
    bool orbitalUplinkActive = false;
    bool railFortressActive = false;
    bool recoveryFabricatorActive = false;
    bool industrialGateUnlocked = false;
    float routeContamination = 0.0f;
    bool routeOverrun = false;
    int caravanRunsCompleted = 0;
    int droneRunsCompleted = 0;
    int tradeCyclesCompleted = 0;
    int purgeCycles = 0;
    int railRunsCompleted = 0;
    int orbitalScansCompleted = 0;
    int railFortressDeployments = 0;
    int fabricatorCyclesCompleted = 0;
    int recoveryMilestonesClaimed = 0;
    int campFortificationLevel = 0;
    bool industrialSurveyActive = false;
    int surveyRunsCompleted = 0;
    bool industrialOutpostActive = false;
    int outpostSupplyRuns = 0;
    bool assemblyCellActive = false;
    int assemblyCyclesCompleted = 0;
    bool foundryLineActive = false;
    int foundryCyclesCompleted = 0;
    bool reactorYardActive = false;
    int reactorCyclesCompleted = 0;
    bool capacitorBankActive = false;
    int capacitorDischargeCycles = 0;
    bool relaySubstationActive = false;
    int relaySyncCycles = 0;
    bool serviceBayActive = false;
    int serviceCyclesCompleted = 0;
    bool waterReclaimerActive = false;
    int waterCyclesCompleted = 0;
    bool feyRingIntercityUnlocked = false;
    bool feyRingInterserverUnlocked = false;
    int relayCreditsEarned = 0;
    int relayCreditsSpent = 0;
    std::string activeRouteEventType{};
    float routeEventTimeRemaining = 0.0f;
    float routeEventCooldown = 0.0f;
    float routeEventOfferTimeRemaining = 0.0f;
    int routeEventProgress = 0;
    int routeEventStage = 0;
    int routeEventSerial = 0;
    int routeEventsResolved = 0;
    int routeEventsFailed = 0;
    int routeEventsExpired = 0;
    std::string lastRouteEventType{};
    std::string lastRouteEventOutcome{};
};

struct SkillAwakeningProgress {
    int archiveSyncs = 0;
    int footKills = 0;
    int tankActions = 0;
    int stressSurvivals = 0;
    int heavyCarryDrills = 0;
    int fieldServiceUses = 0;
};

struct FirstPlayableRouteProgress {
    bool introSeen = false;
    bool emergencyMeleeRecovered = false;
    bool earlyVerminEncounterResolved = false;
    bool accessCardRecovered = false;
    int prePipPadClueCount = 0;
    bool bt72HullInspected = false;
    bool bt72CoreRecovered = false;
    bool bt72ServiceNotesRecovered = false;
    bool bt72Restored = false;
    bool clearanceBlueprintRecovered = false;
    bool clearanceMaterialsRecovered = false;
    bool clearanceModuleInstalled = false;
    bool surfaceArrivalReached = false;
    bool firstTankCombatResolved = false;
    bool firstServicePerformed = false;
    bool firstRecoveryNodeActivated = false;
    bool debriefSummaryViewed = false;
};

struct StoryProgress {
    bool awakenedFromCryo = false;
    bool pipPadRecovered = false;
    bool archiveRecovered = false;
    bool tankLinked = false;
    bool bucketRecovered = false;
    bool exitedBunker = false;
    bool outerRoadCleared = false;
    bool relayRecovered = false;
    bool returnedToBase = false;
};

enum class VehicleType {
    Motorcycle,
    Car,
    Truck,
    Helicopter,
    Boat,
    Utility
};

enum class TankClass {
    Vanguard,
    Destroyer,
    Tactician
};

enum class ShelterDoctrine {
    Balanced,
    Industry,
    Defense,
    Medical
};

enum class TankModuleSlotType {
    Chassis,
    Turret,
    PrimaryWeapon,
    SecondaryWeapon,
    Utility,
    Sensor,
    PowerCore,
    Bucket
};

struct TankModuleSlot {
    TankModuleSlotType type = TankModuleSlotType::Utility;
    std::string moduleId;
    std::string displayName;
    bool installed = false;
    float durability = 100.0f;
};

struct TankDamageState {
    float hull = 100.0f;
    float turret = 100.0f;
    float bucket = 100.0f;
    float sensors = 100.0f;
    float cockpit = 100.0f;
    float powerCore = 100.0f;
};

struct TankLoadout {
    std::string chassisId = "[#tr9300]";
    std::string hullName = "BT-Partner Hull";
    std::string turretId = "turret_field_mk1";
    std::string turretName = "Field Turret Mk.I";
    std::vector<std::string> primaryWeapons = {"ptrs_41_mount"};
    std::vector<std::string> secondaryWeapons = {"coax_mg"};
    std::vector<TankModuleSlot> modules = {
        {TankModuleSlotType::Chassis, "chassis_bt_partner", "Partner Chassis", true, 100.0f},
        {TankModuleSlotType::Turret, "turret_field_mk1", "Field Turret Mk.I", true, 100.0f},
        {TankModuleSlotType::PrimaryWeapon, "ptrs_41_mount", "PTRS-41 Mount", true, 100.0f},
        {TankModuleSlotType::SecondaryWeapon, "coax_mg", "Coax MG", true, 100.0f},
        {TankModuleSlotType::Sensor, "nerv_sensor_mk1", "Nerv Sensor Mk.I", true, 100.0f},
        {TankModuleSlotType::PowerCore, "tier2_core", "Tier-2 Ether Core", true, 100.0f},
        {TankModuleSlotType::Bucket, "bucket_shield_a", "Bucket Shield A", true, 100.0f},
    };
};

struct PartnerTankProfile {
    std::string partnerTankId = "[#tr9300]";
    std::string callSign = "BT-72";
    TankClass tankClass = TankClass::Vanguard;
    bool bondedToPilot = true;
    bool deployed = false;
    bool evacRequested = false;
    bool inRepair = false;
    bool worldPositionKnown = false;
    float trustLink = 0.86f;
    float energyReserve = 78.0f;
    float ammoReserve = 62.0f;
    float worldX = 0.0f;
    float worldY = 0.0f;
    int syncRamActions = 0;
    int syncShotActions = 0;
    bool secondSeatUnlocked = false;
    bool gunnerDrillSeen = false;
    std::string secondSeatPolicy = "pilot_only";
    std::string trustedGunnerHandle{};
    std::string assignedGunnerHandle{};
    TankDamageState damage{};
    TankLoadout loadout{};
};

struct VehicleProfile {
    std::string vehicleId;
    std::string displayName;
    VehicleType type = VehicleType::Utility;
    bool available = true;
    bool deployed = false;
    float durability = 100.0f;
    float fuelOrCharge = 100.0f;
    std::vector<std::string> modules{};
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
    // Группа уровней и опыта
    int level = 1;
    int experience = 0;    
    int unusedPoints = 0;  
    float hp = 100.0f;
    float maxHp = 100.0f;
    float mp = 75.0f;
    float maxMp = 75.0f;
    float carryWeight = 13000.0f;
    SpecialStats special{};
    std::vector<InventoryEntry> inventory{};
    std::vector<TapeEntry> collectedTapes{};
    std::vector<PassiveSkill> passiveSkills{};
    SkillAwakeningProgress awakening{};
    int activeTapeIndex = -1;
    int StatValue(char statCode) const {
        switch (static_cast<char>(std::toupper(static_cast<unsigned char>(statCode)))) {
            case 'S': return special.strength;
            case 'P': return special.perception;
            case 'E': return special.endurance;
            case 'C': return special.charisma;
            case 'I': return special.intelligence;
            case 'A': return special.agility;
            case 'L': return special.luck;
            default: return 0;
        }
    }
};

inline bool HasActiveTape(const CharacterProfile& character) {
    return character.activeTapeIndex >= 0 &&
        character.activeTapeIndex < static_cast<int>(character.collectedTapes.size());
}

inline const TapeEntry* ActiveTape(const CharacterProfile& character) {
    if (!HasActiveTape(character)) {
        return nullptr;
    }
    return &character.collectedTapes[static_cast<std::size_t>(character.activeTapeIndex)];
}

inline float TapeMovementMultiplier(const CharacterProfile& character, bool insideTank) {
    const TapeEntry* tape = ActiveTape(character);
    if (tape == nullptr) {
        return 1.0f;
    }
    if (tape->tapeId.rfind("music_", 0) == 0) {
        return insideTank ? 1.03f : 1.08f;
    }
    return 1.0f;
}

inline std::string ActiveTapeBonusLabel(const CharacterProfile& character, bool insideTank) {
    const TapeEntry* tape = ActiveTape(character);
    if (tape == nullptr) {
        return "No active tape bonus";
    }
    if (tape->tapeId.rfind("music_", 0) == 0) {
        return insideTank ? "Music bonus: +3% movement" : "Music bonus: +8% movement";
    }
    return "Archive tape active";
}

inline std::string NormalizeBt72SecondSeatPolicy(std::string_view policy) {
    if (policy == "trusted_only" || policy == "trusted") {
        return "trusted_only";
    }
    if (policy == "open_crew" || policy == "open") {
        return "open_crew";
    }
    return "pilot_only";
}

inline std::string NormalizeBt72SeatAssignment(std::string_view seatAssignment) {
    if (seatAssignment == "gunner" || seatAssignment == "support_gunner") {
        return "gunner";
    }
    if (seatAssignment == "pilot" || seatAssignment == "driver") {
        return "pilot";
    }
    return "on_foot";
}

inline const char* Bt72SecondSeatPolicyLabel(std::string_view policy) {
    const std::string normalized = NormalizeBt72SecondSeatPolicy(policy);
    if (normalized == "trusted_only") {
        return "Trusted Gunner";
    }
    if (normalized == "open_crew") {
        return "Open Crew";
    }
    return "Pilot Only";
}

inline const char* Bt72SeatAssignmentLabel(std::string_view seatAssignment) {
    const std::string normalized = NormalizeBt72SeatAssignment(seatAssignment);
    if (normalized == "gunner") {
        return "BT-72 Gunner";
    }
    if (normalized == "pilot") {
        return "BT-72 Pilot";
    }
    return "On Foot";
}


struct SessionProfile {
    AccountProfile account{};
    CharacterProfile character{};
    PartnerTankProfile partnerTank{};
    std::vector<VehicleProfile> ownedVehicles{};
    std::vector<SpecialistEntry> rescuedSpecialists{};
    std::vector<WorldFieldState> worldFieldStates{};
    LanlineServicesProfile lanlineServices{};
    LauncherAnnouncementState launcherAnnouncements{};
    ShelterDoctrine doctrine = ShelterDoctrine::Balanced;
    std::string selectedWorld = "start_zone.bwld";
    std::string sessionMode = "Solo";
    bool fieldCheckpointKnown = false;
    float fieldCheckpointX = 0.0f;
    float fieldCheckpointY = 0.0f;
    std::string fieldCheckpointWorld{};
    std::string fieldCheckpointLabel{};
    int scavengerRunsCompleted = 0;
    bool continuityAnchorSeeded = false;
    float continuityAnchorVariance = 0.0f;
    std::string activePipDeviceId{};
    bool pipDeviceReselectPending = false;
    bool pipPadExpansionCoverPresent = true;
    bool blueLinkModuleRecovered = false;
    bool blueLinkModuleInstalled = false;
    bool hangarPowerRestored = false;
    bool bt72CraneControlOnline = false;
    bool bt72CranePathClear = false;
    bool bt72HullAttachedToCrane = false;
    bool bt72HullMovedToServiceLift = false;
    bool bt72HullLockedInRestorationCradle = false;
    FirstPlayableRouteProgress firstPlayableRoute{};
    StoryProgress story{};
};

// Legacy/internal alias: SoulLine / Линейка Сознания.
inline bool SoulLineSeeded(const SessionProfile& profile) {
    return profile.continuityAnchorSeeded;
}

inline float SoulLineVariance(const SessionProfile& profile) {
    return profile.continuityAnchorVariance;
}

inline bool HasActiveFieldCheckpoint(const SessionProfile& profile) {
    return profile.fieldCheckpointKnown && !profile.fieldCheckpointWorld.empty() &&
        profile.fieldCheckpointWorld == profile.selectedWorld;
}

inline bool HasRescuedSpecialist(const SessionProfile& profile, const std::string& specialistId) {
    for (const auto& specialist : profile.rescuedSpecialists) {
        if (specialist.specialistId == specialistId && specialist.awakened) {
            return true;
        }
    }
    return false;
}

inline bool HasAwakenedSpecialistRole(const SessionProfile& profile, const std::string& role) {
    for (const auto& specialist : profile.rescuedSpecialists) {
        if (specialist.role == role && specialist.awakened) {
            return true;
        }
    }
    return false;
}

inline bool HasAssignedSpecialistRole(const SessionProfile& profile, const std::string& role, const std::string& assignment) {
    for (const auto& specialist : profile.rescuedSpecialists) {
        if (specialist.role == role && specialist.assignment == assignment && specialist.awakened) {
            return true;
        }
    }
    return false;
}

inline WorldFieldState* FindWorldFieldState(SessionProfile& profile, const std::string& worldName, bool createIfMissing = false) {
    const std::string normalizedWorldName = NormalizeWorldReference(worldName);
    for (auto& worldState : profile.worldFieldStates) {
        if (worldState.worldName == normalizedWorldName) {
            return &worldState;
        }
    }
    if (!createIfMissing || normalizedWorldName.empty()) {
        return nullptr;
    }
    WorldFieldState state{};
    state.worldName = normalizedWorldName;
    profile.worldFieldStates.push_back(state);
    return &profile.worldFieldStates.back();
}

inline const WorldFieldState* FindWorldFieldState(const SessionProfile& profile, const std::string& worldName) {
    const std::string normalizedWorldName = NormalizeWorldReference(worldName);
    for (const auto& worldState : profile.worldFieldStates) {
        if (worldState.worldName == normalizedWorldName) {
            return &worldState;
        }
    }
    return nullptr;
}

inline void MergeWorldFieldState(WorldFieldState& target, const WorldFieldState& source) {
    target.etherErosion = std::max(target.etherErosion, source.etherErosion);
    target.infrastructureDecay = std::max(target.infrastructureDecay, source.infrastructureDecay);
    target.towerSyncRecovered = target.towerSyncRecovered || source.towerSyncRecovered;
    target.localRelayAvailable = target.localRelayAvailable || source.localRelayAvailable;
    target.regionalGridOnline = target.regionalGridOnline || source.regionalGridOnline;
    target.caravanRouteActive = target.caravanRouteActive || source.caravanRouteActive;
    target.droneStationsActive = target.droneStationsActive || source.droneStationsActive;
    target.tradeNetworkActive = target.tradeNetworkActive || source.tradeNetworkActive;
    target.railFreightActive = target.railFreightActive || source.railFreightActive;
    target.orbitalUplinkActive = target.orbitalUplinkActive || source.orbitalUplinkActive;
    target.railFortressActive = target.railFortressActive || source.railFortressActive;
    target.recoveryFabricatorActive = target.recoveryFabricatorActive || source.recoveryFabricatorActive;
    target.industrialGateUnlocked = target.industrialGateUnlocked || source.industrialGateUnlocked;
    target.routeContamination = std::max(target.routeContamination, source.routeContamination);
    target.routeOverrun = target.routeOverrun || source.routeOverrun;
    target.caravanRunsCompleted = std::max(target.caravanRunsCompleted, source.caravanRunsCompleted);
    target.droneRunsCompleted = std::max(target.droneRunsCompleted, source.droneRunsCompleted);
    target.tradeCyclesCompleted = std::max(target.tradeCyclesCompleted, source.tradeCyclesCompleted);
    target.purgeCycles = std::max(target.purgeCycles, source.purgeCycles);
    target.railRunsCompleted = std::max(target.railRunsCompleted, source.railRunsCompleted);
    target.orbitalScansCompleted = std::max(target.orbitalScansCompleted, source.orbitalScansCompleted);
    target.railFortressDeployments = std::max(target.railFortressDeployments, source.railFortressDeployments);
    target.fabricatorCyclesCompleted = std::max(target.fabricatorCyclesCompleted, source.fabricatorCyclesCompleted);
    target.recoveryMilestonesClaimed = std::max(target.recoveryMilestonesClaimed, source.recoveryMilestonesClaimed);
    target.campFortificationLevel = std::max(target.campFortificationLevel, source.campFortificationLevel);
    target.industrialSurveyActive = target.industrialSurveyActive || source.industrialSurveyActive;
    target.surveyRunsCompleted = std::max(target.surveyRunsCompleted, source.surveyRunsCompleted);
    target.industrialOutpostActive = target.industrialOutpostActive || source.industrialOutpostActive;
    target.outpostSupplyRuns = std::max(target.outpostSupplyRuns, source.outpostSupplyRuns);
    target.assemblyCellActive = target.assemblyCellActive || source.assemblyCellActive;
    target.assemblyCyclesCompleted = std::max(target.assemblyCyclesCompleted, source.assemblyCyclesCompleted);
    target.foundryLineActive = target.foundryLineActive || source.foundryLineActive;
    target.foundryCyclesCompleted = std::max(target.foundryCyclesCompleted, source.foundryCyclesCompleted);
    target.reactorYardActive = target.reactorYardActive || source.reactorYardActive;
    target.reactorCyclesCompleted = std::max(target.reactorCyclesCompleted, source.reactorCyclesCompleted);
    target.capacitorBankActive = target.capacitorBankActive || source.capacitorBankActive;
    target.capacitorDischargeCycles = std::max(target.capacitorDischargeCycles, source.capacitorDischargeCycles);
    target.relaySubstationActive = target.relaySubstationActive || source.relaySubstationActive;
    target.relaySyncCycles = std::max(target.relaySyncCycles, source.relaySyncCycles);
    target.serviceBayActive = target.serviceBayActive || source.serviceBayActive;
    target.serviceCyclesCompleted = std::max(target.serviceCyclesCompleted, source.serviceCyclesCompleted);
    target.waterReclaimerActive = target.waterReclaimerActive || source.waterReclaimerActive;
    target.waterCyclesCompleted = std::max(target.waterCyclesCompleted, source.waterCyclesCompleted);
    target.feyRingIntercityUnlocked = target.feyRingIntercityUnlocked || source.feyRingIntercityUnlocked;
    target.feyRingInterserverUnlocked = target.feyRingInterserverUnlocked || source.feyRingInterserverUnlocked;
    target.relayCreditsEarned = std::max(target.relayCreditsEarned, source.relayCreditsEarned);
    target.relayCreditsSpent = std::max(target.relayCreditsSpent, source.relayCreditsSpent);
    if (target.activeRouteEventType.empty() || source.routeEventTimeRemaining > target.routeEventTimeRemaining) {
        target.activeRouteEventType = source.activeRouteEventType;
        target.routeEventTimeRemaining = source.routeEventTimeRemaining;
        target.routeEventOfferTimeRemaining = source.routeEventOfferTimeRemaining;
        target.routeEventProgress = source.routeEventProgress;
        target.routeEventStage = source.routeEventStage;
    }
    target.routeEventCooldown = std::max(target.routeEventCooldown, source.routeEventCooldown);
    target.routeEventSerial = std::max(target.routeEventSerial, source.routeEventSerial);
    target.routeEventsResolved = std::max(target.routeEventsResolved, source.routeEventsResolved);
    target.routeEventsFailed = std::max(target.routeEventsFailed, source.routeEventsFailed);
    target.routeEventsExpired = std::max(target.routeEventsExpired, source.routeEventsExpired);
    if (target.lastRouteEventOutcome.empty() || source.routeEventCooldown > target.routeEventCooldown) {
        target.lastRouteEventType = source.lastRouteEventType;
        target.lastRouteEventOutcome = source.lastRouteEventOutcome;
    }
}

inline bool HasActiveRouteEvent(const WorldFieldState& worldState) {
    return !worldState.activeRouteEventType.empty() && worldState.routeEventTimeRemaining > 0.0f;
}

inline bool HasCollectedTapeId(const SessionProfile& profile, const std::string& tapeId) {
    for (const auto& tape : profile.character.collectedTapes) {
        if (tape.tapeId == tapeId) {
            return true;
        }
    }
    return false;
}

inline bool Bt72SecondSeatUnlocked(const SessionProfile& profile) {
    return profile.partnerTank.secondSeatUnlocked || profile.story.tankLinked;
}

inline bool Bt72GunnerHandleAllowed(const SessionProfile& profile, std::string_view handle) {
    if (!Bt72SecondSeatUnlocked(profile) || handle.empty()) {
        return false;
    }

    const std::string normalizedPolicy = NormalizeBt72SecondSeatPolicy(profile.partnerTank.secondSeatPolicy);
    if (normalizedPolicy == "open_crew") {
        return true;
    }
    if (normalizedPolicy == "trusted_only") {
        return !profile.partnerTank.trustedGunnerHandle.empty() &&
            profile.partnerTank.trustedGunnerHandle == handle;
    }
    return handle == profile.character.displayName;
}

inline bool HasRegionalGridOnline(const SessionProfile& profile) {
    if (const auto* state = FindWorldFieldState(profile, profile.selectedWorld); state != nullptr) {
        if (state->regionalGridOnline || state->towerSyncRecovered) {
            return true;
        }
    }
    return profile.story.relayRecovered || HasCollectedTapeId(profile, "[%term_0001]_tower");
}

inline int CountRestoredPylonsInProfile(const SessionProfile& profile) {
    int restored = 0;
    for (const auto& tape : profile.character.collectedTapes) {
        if (tape.tapeId.size() >= 6 && tape.tapeId.find("_pylon") != std::string::npos) {
            restored += 1;
        }
    }
    return restored;
}

inline bool IsCaravanOperational(const SessionProfile& profile, const WorldFieldState& worldState) {
    return worldState.caravanRouteActive &&
        HasActiveFieldCheckpoint(profile) &&
        profile.story.outerRoadCleared &&
        HasRegionalGridOnline(profile);
}

inline bool IsDroneStationOperational(const SessionProfile& profile, const WorldFieldState& worldState) {
    return worldState.droneStationsActive && HasRegionalGridOnline(profile);
}

inline bool IsTradeNetworkOperational(const SessionProfile& profile, const WorldFieldState& worldState) {
    return worldState.tradeNetworkActive &&
        HasActiveFieldCheckpoint(profile) &&
        HasRegionalGridOnline(profile) &&
        (worldState.caravanRouteActive || worldState.droneStationsActive);
}

inline bool IsRailFreightOperational(const SessionProfile& profile, const WorldFieldState& worldState) {
    return worldState.railFreightActive &&
        HasActiveFieldCheckpoint(profile) &&
        profile.story.outerRoadCleared &&
        HasRegionalGridOnline(profile) &&
        CountRestoredPylonsInProfile(profile) >= 1 &&
        (worldState.caravanRouteActive || worldState.droneStationsActive);
}

inline bool IsOrbitalUplinkOperational(const SessionProfile& profile, const WorldFieldState& worldState) {
    return worldState.orbitalUplinkActive &&
        HasRegionalGridOnline(profile) &&
        worldState.railFreightActive &&
        CountRestoredPylonsInProfile(profile) >= 2 &&
        (worldState.tradeNetworkActive || worldState.droneStationsActive);
}

inline bool IsRailFortressOperational(const SessionProfile& profile, const WorldFieldState& worldState) {
    return worldState.railFortressActive &&
        worldState.railFreightActive &&
        worldState.orbitalUplinkActive &&
        HasRegionalGridOnline(profile) &&
        CountRestoredPylonsInProfile(profile) >= 2;
}

inline bool IsRecoveryFabricatorOperational(const SessionProfile& profile, const WorldFieldState& worldState) {
    return worldState.recoveryFabricatorActive &&
        HasRegionalGridOnline(profile) &&
        (worldState.railFreightActive || worldState.droneStationsActive) &&
        (worldState.tradeNetworkActive || worldState.orbitalUplinkActive);
}

inline bool IsIndustrialSurveyOperational(const WorldFieldState& worldState) {
    return worldState.industrialSurveyActive &&
        worldState.industrialGateUnlocked &&
        worldState.orbitalUplinkActive &&
        worldState.tradeNetworkActive;
}

inline bool IsIndustrialOutpostOperational(const WorldFieldState& worldState) {
    return worldState.industrialOutpostActive &&
        worldState.industrialGateUnlocked &&
        worldState.industrialSurveyActive &&
        worldState.tradeNetworkActive;
}

inline bool IsAssemblyCellOperational(const WorldFieldState& worldState) {
    return worldState.assemblyCellActive &&
        worldState.industrialOutpostActive &&
        worldState.industrialSurveyActive &&
        worldState.recoveryFabricatorActive;
}

inline bool IsFoundryLineOperational(const WorldFieldState& worldState) {
    return worldState.foundryLineActive &&
        worldState.assemblyCellActive &&
        worldState.industrialOutpostActive &&
        worldState.railFortressActive;
}

inline bool IsReactorYardOperational(const WorldFieldState& worldState) {
    return worldState.reactorYardActive &&
        worldState.foundryLineActive &&
        worldState.assemblyCellActive &&
        worldState.orbitalUplinkActive;
}

inline bool IsCapacitorBankOperational(const WorldFieldState& worldState) {
    return worldState.capacitorBankActive &&
        worldState.reactorYardActive &&
        worldState.foundryLineActive &&
        worldState.orbitalUplinkActive;
}

inline bool IsRelaySubstationOperational(const WorldFieldState& worldState) {
    return worldState.relaySubstationActive &&
        worldState.capacitorBankActive &&
        worldState.reactorYardActive &&
        worldState.industrialOutpostActive;
}

inline bool IsServiceBayOperational(const WorldFieldState& worldState) {
    return worldState.serviceBayActive &&
        worldState.relaySubstationActive &&
        worldState.foundryLineActive &&
        worldState.industrialOutpostActive;
}

inline bool IsWaterReclaimerOperational(const WorldFieldState& worldState) {
    return worldState.waterReclaimerActive &&
        worldState.serviceBayActive &&
        worldState.relaySubstationActive &&
        worldState.recoveryFabricatorActive;
}

inline bool IsStableRecoveryBackbone(const SessionProfile& profile, const WorldFieldState& worldState) {
    return IsRelaySubstationOperational(worldState) &&
        IsServiceBayOperational(worldState) &&
        IsWaterReclaimerOperational(worldState) &&
        IsRecoveryFabricatorOperational(profile, worldState);
}

inline float CurrentEtherErosion(const SessionProfile& profile, const std::string& worldName) {
    const auto* worldState = FindWorldFieldState(profile, worldName);
    return worldState != nullptr ? worldState->etherErosion : 0.0f;
}

inline float CurrentEtherErosion(const SessionProfile& profile) {
    return CurrentEtherErosion(profile, profile.selectedWorld);
}

inline const char* ToString(VehicleType type) {
    switch (type) {
        case VehicleType::Motorcycle: return "Motorcycle";
        case VehicleType::Car: return "Car";
        case VehicleType::Truck: return "Truck";
        case VehicleType::Helicopter: return "Helicopter";
        case VehicleType::Boat: return "Boat";
        case VehicleType::Utility: return "Utility";
    }
    return "Unknown";
}

inline const char* ToString(TankClass tankClass) {
    switch (tankClass) {
        case TankClass::Vanguard: return "Vanguard";
        case TankClass::Destroyer: return "Destroyer";
        case TankClass::Tactician: return "Tactician";
    }
    return "Unknown";
}

inline const char* ToString(ShelterDoctrine doctrine) {
    switch (doctrine) {
        case ShelterDoctrine::Balanced: return "Balanced";
        case ShelterDoctrine::Industry: return "Industry";
        case ShelterDoctrine::Defense: return "Defense";
        case ShelterDoctrine::Medical: return "Medical";
    }
    return "Unknown";
}

inline std::string CurrentTankSyncMode(const PartnerTankProfile& tank) {
    if (tank.syncRamActions >= 3 && tank.syncRamActions >= tank.syncShotActions + 1) {
        return "Bulwark Sync";
    }
    if (tank.syncShotActions >= 3 && tank.syncShotActions >= tank.syncRamActions + 1) {
        return "Stabilizer Sync";
    }
    return "Adaptive Sync";
}

SessionProfile MakeDefaultSessionProfile();
void NormalizeWorldFieldState(WorldFieldState& state);
void NormalizeSessionProfile(SessionProfile& profile);
bool SeedContinuityAnchorAfterBunkerAnomaly(SessionProfile& profile, std::string* diagnosticText = nullptr);
std::string ContinuityAnchorDiagnostic(const SessionProfile& profile);
bool SaveSessionProfile(const SessionProfile& profile, const fs::path& filePath);
bool LoadSessionProfile(const fs::path& filePath, SessionProfile& outProfile);

}  // namespace bunker
