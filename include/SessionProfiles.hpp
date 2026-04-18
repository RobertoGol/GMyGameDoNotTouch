#pragma once

#include <algorithm>
#include <cctype>
#include <ctime>
#include <fstream>
#include <string>
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

struct WorldFieldState {
    std::string worldName;
    float etherErosion = 0.0f;
    float infrastructureDecay = 0.0f;
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
};

struct SkillAwakeningProgress {
    int archiveSyncs = 0;
    int footKills = 0;
    int tankActions = 0;
    int stressSurvivals = 0;
    int heavyCarryDrills = 0;
    int fieldServiceUses = 0;
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


struct SessionProfile {
    AccountProfile account{};
    CharacterProfile character{};
    PartnerTankProfile partnerTank{};
    std::vector<VehicleProfile> ownedVehicles{};
    std::vector<SpecialistEntry> rescuedSpecialists{};
    std::vector<WorldFieldState> worldFieldStates{};
    ShelterDoctrine doctrine = ShelterDoctrine::Balanced;
    std::string selectedWorld = "start_zone.bwld";
    std::string sessionMode = "Solo";
    bool fieldCheckpointKnown = false;
    float fieldCheckpointX = 0.0f;
    float fieldCheckpointY = 0.0f;
    std::string fieldCheckpointWorld{};
    std::string fieldCheckpointLabel{};
    int scavengerRunsCompleted = 0;
    StoryProgress story{};
};

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
}

inline bool HasCollectedTapeId(const SessionProfile& profile, const std::string& tapeId) {
    for (const auto& tape : profile.character.collectedTapes) {
        if (tape.tapeId == tapeId) {
            return true;
        }
    }
    return false;
}

inline bool HasRegionalGridOnline(const SessionProfile& profile) {
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

inline SessionProfile MakeDefaultSessionProfile() {
    SessionProfile profile;
    profile.account.registerDate = std::time(nullptr);
    profile.character.inventory.push_back({"#%it_field_ration", 2, 0.3f});
    profile.character.inventory.push_back({"#%it_ptrs_ammo", 8, 0.7f});
    profile.character.collectedTapes.push_back({"tape_intro_001", "Cryo Wing Log", false, false, false});
    profile.character.collectedTapes.push_back({"music_recovery_001", "Recovery Station Mixtape", false, false, false});
    profile.character.passiveSkills.push_back({"skill_field_reflex", "Field Reflex", false, false});
    profile.character.passiveSkills.push_back({"skill_pilot_sync", "Pilot Sync", false, false});
    profile.character.passiveSkills.push_back({"skill_data_miner", "Data Miner", false, false});
    profile.character.passiveSkills.push_back({"skill_second_wind", "Second Wind", false, false});
    profile.character.passiveSkills.push_back({"skill_muscle_memory", "Muscle Memory", false, false});
    profile.ownedVehicles.push_back({"[#tr2001]", "Dust Runner Bike", VehicleType::Motorcycle, true, false, 92.0f, 74.0f, {"cargo_rack_light"}});
    profile.ownedVehicles.push_back({"[#tr2002]", "Wallbreaker Utility", VehicleType::Truck, true, false, 88.0f, 61.0f, {"salvage_bed", "field_crane"}});
    return profile;
}

inline void NormalizeSessionProfile(SessionProfile& profile) {
    profile.selectedWorld = NormalizeWorldReference(profile.selectedWorld);
    profile.fieldCheckpointWorld = profile.fieldCheckpointWorld.empty()
        ? std::string()
        : NormalizeWorldReference(profile.fieldCheckpointWorld);

    std::vector<WorldFieldState> normalizedWorldStates;
    normalizedWorldStates.reserve(profile.worldFieldStates.size());
    for (const auto& worldState : profile.worldFieldStates) {
        WorldFieldState normalizedState = worldState;
        normalizedState.worldName = NormalizeWorldReference(worldState.worldName);
        auto existing = std::find_if(normalizedWorldStates.begin(), normalizedWorldStates.end(),
            [&](const WorldFieldState& state) { return state.worldName == normalizedState.worldName; });
        if (existing != normalizedWorldStates.end()) {
            MergeWorldFieldState(*existing, normalizedState);
        } else {
            normalizedWorldStates.push_back(normalizedState);
        }
    }
    profile.worldFieldStates = std::move(normalizedWorldStates);

    if (profile.account.accountId.empty() || !RegistryId::IsValid(profile.account.accountId)) profile.account.accountId = "#10001";
    if (profile.character.characterId.empty() || !RegistryId::IsValid(profile.character.characterId)) profile.character.characterId = "@20001";
    if (profile.account.username.empty()) profile.account.username = "wanderer";
    if (profile.character.displayName.empty()) profile.character.displayName = "Scout";
    if (profile.account.registerDate == 0) profile.account.registerDate = std::time(nullptr);
    while (profile.account.linkedCharacters.size() < 3) {
        profile.account.linkedCharacters.push_back(RegistryId::MakeCharacterId(20001 + static_cast<int>(profile.account.linkedCharacters.size())));
    }
    if (profile.character.inventory.empty()) profile.character.inventory.push_back({"#%it_field_ration", 1, 0.3f});
    if (profile.character.collectedTapes.empty()) {
        profile.character.collectedTapes.push_back({"tape_intro_001", "Cryo Wing Log", false, false, false});
        profile.character.collectedTapes.push_back({"music_recovery_001", "Recovery Station Mixtape", false, false, false});
    }
    if (profile.character.passiveSkills.empty()) {
        profile.character.passiveSkills.push_back({"skill_field_reflex", "Field Reflex", false, false});
        profile.character.passiveSkills.push_back({"skill_pilot_sync", "Pilot Sync", false, false});
        profile.character.passiveSkills.push_back({"skill_data_miner", "Data Miner", false, false});
        profile.character.passiveSkills.push_back({"skill_second_wind", "Second Wind", false, false});
        profile.character.passiveSkills.push_back({"skill_muscle_memory", "Muscle Memory", false, false});
    }
    if (profile.partnerTank.callSign.empty()) profile.partnerTank.callSign = "BT-72";
    if (profile.ownedVehicles.empty()) profile.ownedVehicles.push_back({"[#tr2001]", "Dust Runner Bike", VehicleType::Motorcycle, true, false, 92.0f, 74.0f, {"cargo_rack_light"}});
    if (profile.fieldCheckpointKnown && profile.fieldCheckpointWorld.empty()) {
        profile.fieldCheckpointWorld = profile.selectedWorld;
    }
    (void)FindWorldFieldState(profile, profile.selectedWorld, true);
}

inline bool SaveSessionProfile(const SessionProfile& profile, const fs::path& filePath) {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;

    out << "account_id=" << profile.account.accountId << '\n';
    out << "username=" << profile.account.username << '\n';
    out << "email=" << profile.account.email << '\n';
    out << "register_date=" << static_cast<long long>(profile.account.registerDate) << '\n';
    out << "playtime_minutes=" << profile.account.totalPlayTimeMinutes << '\n';
    out << "character_id=" << profile.character.characterId << '\n';
    out << "character_name=" << profile.character.displayName << '\n';
    out << "character_level=" << profile.character.level << '\n';
    // Внутри SaveSessionProfile (примерно после character_level)
    out << "character_exp=" << profile.character.experience << '\n';
    out << "unused_points=" << profile.character.unusedPoints << '\n';
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
    out << "shelter_doctrine=" << static_cast<int>(profile.doctrine) << '\n';
    out << "field_checkpoint_known=" << (profile.fieldCheckpointKnown ? 1 : 0) << '\n';
    out << "field_checkpoint_x=" << profile.fieldCheckpointX << '\n';
    out << "field_checkpoint_y=" << profile.fieldCheckpointY << '\n';
    out << "field_checkpoint_world=" << profile.fieldCheckpointWorld << '\n';
    out << "field_checkpoint_label=" << profile.fieldCheckpointLabel << '\n';
    out << "scavenger_runs_completed=" << profile.scavengerRunsCompleted << '\n';
    out << "partner_tank_id=" << profile.partnerTank.partnerTankId << '\n';
    out << "partner_tank_callsign=" << profile.partnerTank.callSign << '\n';
    out << "partner_tank_class=" << static_cast<int>(profile.partnerTank.tankClass) << '\n';
    out << "partner_tank_link=" << profile.partnerTank.trustLink << '\n';
    out << "partner_tank_energy=" << profile.partnerTank.energyReserve << '\n';
    out << "partner_tank_ammo=" << profile.partnerTank.ammoReserve << '\n';
    out << "partner_tank_deployed=" << (profile.partnerTank.deployed ? 1 : 0) << '\n';
    out << "partner_tank_repair=" << (profile.partnerTank.inRepair ? 1 : 0) << '\n';
    out << "partner_tank_world_known=" << (profile.partnerTank.worldPositionKnown ? 1 : 0) << '\n';
    out << "partner_tank_x=" << profile.partnerTank.worldX << '\n';
    out << "partner_tank_y=" << profile.partnerTank.worldY << '\n';
    out << "partner_tank_sync_ram=" << profile.partnerTank.syncRamActions << '\n';
    out << "partner_tank_sync_shot=" << profile.partnerTank.syncShotActions << '\n';
    out << "tank_hull=" << profile.partnerTank.damage.hull << '\n';
    out << "tank_turret=" << profile.partnerTank.damage.turret << '\n';
    out << "tank_bucket=" << profile.partnerTank.damage.bucket << '\n';
    out << "tank_sensors=" << profile.partnerTank.damage.sensors << '\n';
    out << "tank_cockpit=" << profile.partnerTank.damage.cockpit << '\n';
    out << "tank_powercore=" << profile.partnerTank.damage.powerCore << '\n';
    out << "story_awakened=" << (profile.story.awakenedFromCryo ? 1 : 0) << '\n';
    out << "story_pippad=" << (profile.story.pipPadRecovered ? 1 : 0) << '\n';
    out << "story_archive=" << (profile.story.archiveRecovered ? 1 : 0) << '\n';
    out << "story_tank=" << (profile.story.tankLinked ? 1 : 0) << '\n';
    out << "story_bucket=" << (profile.story.bucketRecovered ? 1 : 0) << '\n';
    out << "story_exit=" << (profile.story.exitedBunker ? 1 : 0) << '\n';
    out << "story_debris=" << (profile.story.outerRoadCleared ? 1 : 0) << '\n';
    out << "story_relay=" << (profile.story.relayRecovered ? 1 : 0) << '\n';
    out << "story_return=" << (profile.story.returnedToBase ? 1 : 0) << '\n';
    out << "awakening_archive=" << profile.character.awakening.archiveSyncs << '\n';
    out << "awakening_foot=" << profile.character.awakening.footKills << '\n';
    out << "awakening_tank=" << profile.character.awakening.tankActions << '\n';
    out << "awakening_stress=" << profile.character.awakening.stressSurvivals << '\n';
    out << "awakening_heavy_carry=" << profile.character.awakening.heavyCarryDrills << '\n';
    out << "awakening_field_service=" << profile.character.awakening.fieldServiceUses << '\n';

    for (const auto& id : profile.account.linkedCharacters) out << "linked_character=" << id << '\n';
    for (const auto& item : profile.character.inventory) out << "inventory=" << item.itemId << ',' << item.count << ',' << item.unitWeight << '\n';
    for (const auto& tape : profile.character.collectedTapes) {
        out << "tape=" << tape.tapeId << ',' << tape.title << ',' << (tape.played ? 1 : 0)
            << ',' << (tape.damaged ? 1 : 0) << ',' << (tape.reconstructed ? 1 : 0) << '\n';
    }
    for (const auto& skill : profile.character.passiveSkills) {
        out << "passive_skill=" << skill.skillId << ',' << skill.displayName << ',' << (skill.unlocked ? 1 : 0) << ',' << (skill.equipped ? 1 : 0) << '\n';
    }
    for (const auto& specialist : profile.rescuedSpecialists) {
        out << "specialist=" << specialist.specialistId << ',' << specialist.displayName << ',' << specialist.role << ',' << specialist.assignment << ',' << (specialist.awakened ? 1 : 0) << '\n';
    }
    for (const auto& worldState : profile.worldFieldStates) {
        out << "world_field=" << worldState.worldName << ','
            << worldState.etherErosion << ','
            << worldState.infrastructureDecay << ','
            << (worldState.caravanRouteActive ? 1 : 0) << ','
            << (worldState.droneStationsActive ? 1 : 0) << ','
            << (worldState.tradeNetworkActive ? 1 : 0) << ','
            << (worldState.railFreightActive ? 1 : 0) << ','
            << (worldState.orbitalUplinkActive ? 1 : 0) << ','
            << (worldState.railFortressActive ? 1 : 0) << ','
            << (worldState.recoveryFabricatorActive ? 1 : 0) << ','
            << (worldState.industrialGateUnlocked ? 1 : 0) << ','
            << worldState.routeContamination << ','
            << (worldState.routeOverrun ? 1 : 0) << ','
            << worldState.caravanRunsCompleted << ','
            << worldState.droneRunsCompleted << ','
            << worldState.tradeCyclesCompleted << ','
            << worldState.purgeCycles << ','
            << worldState.railRunsCompleted << ','
            << worldState.orbitalScansCompleted << ','
            << worldState.railFortressDeployments << ','
            << worldState.fabricatorCyclesCompleted << ','
            << worldState.recoveryMilestonesClaimed << ','
            << worldState.campFortificationLevel << ','
            << (worldState.industrialSurveyActive ? 1 : 0) << ','
            << worldState.surveyRunsCompleted << ','
            << (worldState.industrialOutpostActive ? 1 : 0) << ','
            << worldState.outpostSupplyRuns << ','
            << (worldState.assemblyCellActive ? 1 : 0) << ','
            << worldState.assemblyCyclesCompleted << ','
            << (worldState.foundryLineActive ? 1 : 0) << ','
            << worldState.foundryCyclesCompleted << ','
            << (worldState.reactorYardActive ? 1 : 0) << ','
            << worldState.reactorCyclesCompleted << ','
            << (worldState.capacitorBankActive ? 1 : 0) << ','
            << worldState.capacitorDischargeCycles << ','
            << (worldState.relaySubstationActive ? 1 : 0) << ','
            << worldState.relaySyncCycles << ','
            << (worldState.serviceBayActive ? 1 : 0) << ','
            << worldState.serviceCyclesCompleted << ','
            << (worldState.waterReclaimerActive ? 1 : 0) << ','
            << worldState.waterCyclesCompleted << '\n';
    }
    out << "active_tape_index=" << profile.character.activeTapeIndex << '\n';
    for (const auto& vehicle : profile.ownedVehicles) {
        out << "vehicle=" << vehicle.vehicleId << ',' << vehicle.displayName << ',' << static_cast<int>(vehicle.type) << ','
            << (vehicle.available ? 1 : 0) << ',' << (vehicle.deployed ? 1 : 0) << ',' << vehicle.durability << ','
            << vehicle.fuelOrCharge << '\n';
    }

    return true;
}

inline bool LoadSessionProfile(const fs::path& filePath, SessionProfile& outProfile) {
    std::ifstream in(filePath);
    if (!in.is_open()) return false;

    outProfile = MakeDefaultSessionProfile();
    outProfile.account.linkedCharacters.clear();
    outProfile.character.inventory.clear();
    outProfile.character.collectedTapes.clear();
    outProfile.rescuedSpecialists.clear();
    outProfile.worldFieldStates.clear();
    outProfile.ownedVehicles.clear();

    std::string line;
    while (std::getline(in, line)) {
        const auto pos = line.find('=');
        if (pos == std::string::npos) continue;
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
        // Внутри LoadSessionProfile (в блоке if/else if ключей)
        else if (key == "character_exp") outProfile.character.experience = std::stoi(value);
        else if (key == "unused_points") outProfile.character.unusedPoints = std::stoi(value);
        else if (key == "hp") outProfile.character.hp = std::stof(value);
        else if (key == "max_hp") outProfile.character.maxHp = std::stof(value);
        else if (key == "mp") outProfile.character.mp = std::stof(value);
        else if (key == "max_mp") outProfile.character.maxMp = std::stof(value);
        else if (key == "carry_weight") outProfile.character.carryWeight = std::stof(value);
        else if (key == "session_mode") outProfile.sessionMode = value;
        else if (key == "selected_world") outProfile.selectedWorld = value;
        else if (key == "shelter_doctrine") outProfile.doctrine = static_cast<ShelterDoctrine>(std::stoi(value));
        else if (key == "field_checkpoint_known") outProfile.fieldCheckpointKnown = (std::stoi(value) != 0);
        else if (key == "field_checkpoint_x") outProfile.fieldCheckpointX = std::stof(value);
        else if (key == "field_checkpoint_y") outProfile.fieldCheckpointY = std::stof(value);
        else if (key == "field_checkpoint_world") outProfile.fieldCheckpointWorld = value;
        else if (key == "field_checkpoint_label") outProfile.fieldCheckpointLabel = value;
        else if (key == "scavenger_runs_completed") outProfile.scavengerRunsCompleted = std::stoi(value);
        else if (key == "partner_tank_id") outProfile.partnerTank.partnerTankId = value;
        else if (key == "partner_tank_callsign") outProfile.partnerTank.callSign = value;
        else if (key == "partner_tank_class") outProfile.partnerTank.tankClass = static_cast<TankClass>(std::stoi(value));
        else if (key == "partner_tank_link") outProfile.partnerTank.trustLink = std::stof(value);
        else if (key == "partner_tank_energy") outProfile.partnerTank.energyReserve = std::stof(value);
        else if (key == "partner_tank_ammo") outProfile.partnerTank.ammoReserve = std::stof(value);
        else if (key == "partner_tank_deployed") outProfile.partnerTank.deployed = (std::stoi(value) != 0);
        else if (key == "partner_tank_repair") outProfile.partnerTank.inRepair = (std::stoi(value) != 0);
        else if (key == "partner_tank_world_known") outProfile.partnerTank.worldPositionKnown = (std::stoi(value) != 0);
        else if (key == "partner_tank_x") outProfile.partnerTank.worldX = std::stof(value);
        else if (key == "partner_tank_y") outProfile.partnerTank.worldY = std::stof(value);
        else if (key == "partner_tank_sync_ram") outProfile.partnerTank.syncRamActions = std::stoi(value);
        else if (key == "partner_tank_sync_shot") outProfile.partnerTank.syncShotActions = std::stoi(value);
        else if (key == "tank_hull") outProfile.partnerTank.damage.hull = std::stof(value);
        else if (key == "tank_turret") outProfile.partnerTank.damage.turret = std::stof(value);
        else if (key == "tank_bucket") outProfile.partnerTank.damage.bucket = std::stof(value);
        else if (key == "tank_sensors") outProfile.partnerTank.damage.sensors = std::stof(value);
        else if (key == "tank_cockpit") outProfile.partnerTank.damage.cockpit = std::stof(value);
        else if (key == "tank_powercore") outProfile.partnerTank.damage.powerCore = std::stof(value);
        else if (key == "story_awakened") outProfile.story.awakenedFromCryo = (std::stoi(value) != 0);
        else if (key == "story_pippad") outProfile.story.pipPadRecovered = (std::stoi(value) != 0);
        else if (key == "story_archive") outProfile.story.archiveRecovered = (std::stoi(value) != 0);
        else if (key == "story_tank") outProfile.story.tankLinked = (std::stoi(value) != 0);
        else if (key == "story_bucket") outProfile.story.bucketRecovered = (std::stoi(value) != 0);
        else if (key == "story_exit") outProfile.story.exitedBunker = (std::stoi(value) != 0);
        else if (key == "story_debris") outProfile.story.outerRoadCleared = (std::stoi(value) != 0);
        else if (key == "story_relay") outProfile.story.relayRecovered = (std::stoi(value) != 0);
        else if (key == "story_return") outProfile.story.returnedToBase = (std::stoi(value) != 0);
        else if (key == "awakening_archive") outProfile.character.awakening.archiveSyncs = std::stoi(value);
        else if (key == "awakening_foot") outProfile.character.awakening.footKills = std::stoi(value);
        else if (key == "awakening_tank") outProfile.character.awakening.tankActions = std::stoi(value);
        else if (key == "awakening_stress") outProfile.character.awakening.stressSurvivals = std::stoi(value);
        else if (key == "awakening_heavy_carry") outProfile.character.awakening.heavyCarryDrills = std::stoi(value);
        else if (key == "awakening_field_service") outProfile.character.awakening.fieldServiceUses = std::stoi(value);
        else if (key == "linked_character") outProfile.account.linkedCharacters.push_back(value);
        else if (key == "inventory") {
            const auto first = value.find(',');
            const auto second = value.find(',', first == std::string::npos ? first : first + 1);
            if (first != std::string::npos && second != std::string::npos) {
                outProfile.character.inventory.push_back({value.substr(0, first), std::stoi(value.substr(first + 1, second - first - 1)), std::stof(value.substr(second + 1))});
            }
        } else if (key == "tape") {
            const auto first = value.find(',');
            const auto second = value.find(',', first == std::string::npos ? first : first + 1);
            const auto third = value.find(',', second == std::string::npos ? second : second + 1);
            const auto fourth = value.find(',', third == std::string::npos ? third : third + 1);
            if (first != std::string::npos && second != std::string::npos) {
                TapeEntry tape{};
                tape.tapeId = value.substr(0, first);
                tape.title = value.substr(first + 1, second - first - 1);
                if (third == std::string::npos) {
                    tape.played = std::stoi(value.substr(second + 1)) != 0;
                } else {
                    tape.played = std::stoi(value.substr(second + 1, third - second - 1)) != 0;
                    if (fourth == std::string::npos) {
                        tape.damaged = std::stoi(value.substr(third + 1)) != 0;
                    } else {
                        tape.damaged = std::stoi(value.substr(third + 1, fourth - third - 1)) != 0;
                        tape.reconstructed = std::stoi(value.substr(fourth + 1)) != 0;
                    }
                }
                outProfile.character.collectedTapes.push_back(tape);
            }
        } else if (key == "active_tape_index") {
            outProfile.character.activeTapeIndex = std::stoi(value);
        } else if (key == "passive_skill") {
            const auto first = value.find(',');
            const auto second = value.find(',', first == std::string::npos ? first : first + 1);
            const auto third = value.find(',', second == std::string::npos ? second : second + 1);
            if (first != std::string::npos && second != std::string::npos && third != std::string::npos) {
                outProfile.character.passiveSkills.push_back({
                    value.substr(0, first),
                    value.substr(first + 1, second - first - 1),
                    std::stoi(value.substr(second + 1, third - second - 1)) != 0,
                    std::stoi(value.substr(third + 1)) != 0});
            }
        } else if (key == "specialist") {
            const auto first = value.find(',');
            const auto second = value.find(',', first == std::string::npos ? first : first + 1);
            const auto third = value.find(',', second == std::string::npos ? second : second + 1);
            const auto fourth = value.find(',', third == std::string::npos ? third : third + 1);
            if (first != std::string::npos && second != std::string::npos && third != std::string::npos) {
                SpecialistEntry specialist{};
                specialist.specialistId = value.substr(0, first);
                specialist.displayName = value.substr(first + 1, second - first - 1);
                specialist.role = value.substr(second + 1, third - second - 1);
                if (fourth == std::string::npos) {
                    specialist.awakened = std::stoi(value.substr(third + 1)) != 0;
                    specialist.assignment = specialist.role == "engineer" ? "workshop" : "field";
                } else {
                    specialist.assignment = value.substr(third + 1, fourth - third - 1);
                    specialist.awakened = std::stoi(value.substr(fourth + 1)) != 0;
                }
                outProfile.rescuedSpecialists.push_back(specialist);
            }
        } else if (key == "world_field") {
            const auto first = value.find(',');
            const auto second = value.find(',', first == std::string::npos ? first : first + 1);
            const auto third = value.find(',', second == std::string::npos ? second : second + 1);
            if (first != std::string::npos && second != std::string::npos) {
                const auto fourth = value.find(',', third == std::string::npos ? third : third + 1);
                const auto fifth = value.find(',', fourth == std::string::npos ? fourth : fourth + 1);
                const auto sixth = value.find(',', fifth == std::string::npos ? fifth : fifth + 1);
                const auto seventh = value.find(',', sixth == std::string::npos ? sixth : sixth + 1);
                const auto eighth = value.find(',', seventh == std::string::npos ? seventh : seventh + 1);
                if (third == std::string::npos) {
                    outProfile.worldFieldStates.push_back({
                        value.substr(0, first),
                        std::stof(value.substr(first + 1, second - first - 1)),
                        0.0f,
                        false,
                        false,
                        false,
                        false,
                        false,
                        false,
                        false,
                        0.0f,
                        false,
                        0,
                        0,
                        0,
                        std::stoi(value.substr(second + 1)),
                        0,
                        0,
                        0,
                        0});
                } else if (fourth == std::string::npos) {
                    outProfile.worldFieldStates.push_back({
                        value.substr(0, first),
                        std::stof(value.substr(first + 1, second - first - 1)),
                        std::stof(value.substr(second + 1, third - second - 1)),
                        false,
                        false,
                        false,
                        false,
                        false,
                        false,
                        false,
                        0.0f,
                        false,
                        0,
                        0,
                        0,
                        std::stoi(value.substr(third + 1)),
                        0,
                        0,
                        0,
                        0});
                } else {
                    WorldFieldState state{};
                    state.worldName = value.substr(0, first);
                    state.etherErosion = std::stof(value.substr(first + 1, second - first - 1));
                    state.infrastructureDecay = std::stof(value.substr(second + 1, third - second - 1));
                    if (eighth == std::string::npos) {
                        state.caravanRouteActive = std::stoi(value.substr(third + 1, fourth - third - 1)) != 0;
                        state.caravanRunsCompleted = std::stoi(value.substr(fourth + 1, fifth - fourth - 1));
                        state.droneRunsCompleted = std::stoi(value.substr(fifth + 1, sixth - fifth - 1));
                        state.purgeCycles = std::stoi(value.substr(sixth + 1));
                    } else {
                        std::vector<std::string> parts;
                        std::size_t partStart = 0;
                        while (partStart <= value.size()) {
                            const auto partEnd = value.find(',', partStart);
                            parts.push_back(value.substr(partStart, partEnd == std::string::npos ? std::string::npos : partEnd - partStart));
                            if (partEnd == std::string::npos) break;
                            partStart = partEnd + 1;
                        }

                        if (parts.size() >= 8) {
                            state.caravanRouteActive = std::stoi(parts[3]) != 0;
                            state.droneStationsActive = std::stoi(parts[4]) != 0;
                            state.tradeNetworkActive = std::stoi(parts[5]) != 0;

                            if (parts.size() >= 20) {
                                state.railFreightActive = std::stoi(parts[6]) != 0;
                                state.orbitalUplinkActive = std::stoi(parts[7]) != 0;
                                state.railFortressActive = std::stoi(parts[8]) != 0;
                                state.recoveryFabricatorActive = std::stoi(parts[9]) != 0;
                                const bool hasGateToken = parts.size() >= 21;
                                state.industrialGateUnlocked = hasGateToken ? (std::stoi(parts[10]) != 0) : false;
                                const std::size_t baseIndex = hasGateToken ? 11 : 10;
                                state.routeContamination = std::stof(parts[baseIndex]);
                                state.routeOverrun = std::stoi(parts[baseIndex + 1]) != 0;
                                state.caravanRunsCompleted = std::stoi(parts[baseIndex + 2]);
                                state.droneRunsCompleted = std::stoi(parts[baseIndex + 3]);
                                state.tradeCyclesCompleted = std::stoi(parts[baseIndex + 4]);
                                state.purgeCycles = std::stoi(parts[baseIndex + 5]);
                                state.railRunsCompleted = std::stoi(parts[baseIndex + 6]);
                                state.orbitalScansCompleted = std::stoi(parts[baseIndex + 7]);
                                state.railFortressDeployments = std::stoi(parts[baseIndex + 8]);
                                state.fabricatorCyclesCompleted = std::stoi(parts[baseIndex + 9]);
                                state.recoveryMilestonesClaimed = parts.size() >= baseIndex + 11 ? std::stoi(parts[baseIndex + 10]) : 0;
                                state.campFortificationLevel = parts.size() >= baseIndex + 12 ? std::stoi(parts[baseIndex + 11]) : 0;
                                state.industrialSurveyActive = parts.size() >= baseIndex + 13 ? (std::stoi(parts[baseIndex + 12]) != 0) : false;
                                state.surveyRunsCompleted = parts.size() >= baseIndex + 14 ? std::stoi(parts[baseIndex + 13]) : 0;
                                state.industrialOutpostActive = parts.size() >= baseIndex + 15 ? (std::stoi(parts[baseIndex + 14]) != 0) : false;
                                state.outpostSupplyRuns = parts.size() >= baseIndex + 16 ? std::stoi(parts[baseIndex + 15]) : 0;
                                state.assemblyCellActive = parts.size() >= baseIndex + 17 ? (std::stoi(parts[baseIndex + 16]) != 0) : false;
                                state.assemblyCyclesCompleted = parts.size() >= baseIndex + 18 ? std::stoi(parts[baseIndex + 17]) : 0;
                                state.foundryLineActive = parts.size() >= baseIndex + 19 ? (std::stoi(parts[baseIndex + 18]) != 0) : false;
                                state.foundryCyclesCompleted = parts.size() >= baseIndex + 20 ? std::stoi(parts[baseIndex + 19]) : 0;
                                state.reactorYardActive = parts.size() >= baseIndex + 21 ? (std::stoi(parts[baseIndex + 20]) != 0) : false;
                                state.reactorCyclesCompleted = parts.size() >= baseIndex + 22 ? std::stoi(parts[baseIndex + 21]) : 0;
                                state.capacitorBankActive = parts.size() >= baseIndex + 23 ? (std::stoi(parts[baseIndex + 22]) != 0) : false;
                                state.capacitorDischargeCycles = parts.size() >= baseIndex + 24 ? std::stoi(parts[baseIndex + 23]) : 0;
                                state.relaySubstationActive = parts.size() >= baseIndex + 25 ? (std::stoi(parts[baseIndex + 24]) != 0) : false;
                                state.relaySyncCycles = parts.size() >= baseIndex + 26 ? std::stoi(parts[baseIndex + 25]) : 0;
                                state.serviceBayActive = parts.size() >= baseIndex + 27 ? (std::stoi(parts[baseIndex + 26]) != 0) : false;
                                state.serviceCyclesCompleted = parts.size() >= baseIndex + 28 ? std::stoi(parts[baseIndex + 27]) : 0;
                                state.waterReclaimerActive = parts.size() >= baseIndex + 29 ? (std::stoi(parts[baseIndex + 28]) != 0) : false;
                                state.waterCyclesCompleted = parts.size() >= baseIndex + 30 ? std::stoi(parts[baseIndex + 29]) : 0;
                            } else if (parts.size() >= 18) {
                                state.railFreightActive = std::stoi(parts[6]) != 0;
                                state.orbitalUplinkActive = std::stoi(parts[7]) != 0;
                                state.railFortressActive = std::stoi(parts[8]) != 0;
                                state.industrialGateUnlocked = false;
                                state.routeContamination = std::stof(parts[9]);
                                state.routeOverrun = std::stoi(parts[10]) != 0;
                                state.caravanRunsCompleted = std::stoi(parts[11]);
                                state.droneRunsCompleted = std::stoi(parts[12]);
                                state.tradeCyclesCompleted = std::stoi(parts[13]);
                                state.purgeCycles = std::stoi(parts[14]);
                                state.railRunsCompleted = std::stoi(parts[15]);
                                state.orbitalScansCompleted = std::stoi(parts[16]);
                                state.railFortressDeployments = std::stoi(parts[17]);
                                state.fabricatorCyclesCompleted = 0;
                                state.recoveryMilestonesClaimed = 0;
                                state.campFortificationLevel = 0;
                                state.industrialSurveyActive = false;
                                state.surveyRunsCompleted = 0;
                                state.industrialOutpostActive = false;
                                state.outpostSupplyRuns = 0;
                                state.assemblyCellActive = false;
                                state.assemblyCyclesCompleted = 0;
                                state.foundryLineActive = false;
                                state.foundryCyclesCompleted = 0;
                                state.reactorYardActive = false;
                                state.reactorCyclesCompleted = 0;
                                state.capacitorBankActive = false;
                                state.capacitorDischargeCycles = 0;
                                state.relaySubstationActive = false;
                                state.relaySyncCycles = 0;
                                state.serviceBayActive = false;
                                state.serviceCyclesCompleted = 0;
                                state.waterReclaimerActive = false;
                                state.waterCyclesCompleted = 0;
                            } else if (parts.size() >= 14) {
                                state.railFreightActive = std::stoi(parts[6]) != 0;
                                state.industrialGateUnlocked = false;
                                state.routeContamination = std::stof(parts[7]);
                                state.routeOverrun = std::stoi(parts[8]) != 0;
                                state.caravanRunsCompleted = std::stoi(parts[9]);
                                state.droneRunsCompleted = std::stoi(parts[10]);
                                state.tradeCyclesCompleted = std::stoi(parts[11]);
                                state.purgeCycles = std::stoi(parts[12]);
                                state.railRunsCompleted = std::stoi(parts[13]);
                                state.orbitalScansCompleted = 0;
                                state.railFortressDeployments = 0;
                                state.fabricatorCyclesCompleted = 0;
                                state.recoveryMilestonesClaimed = 0;
                                state.campFortificationLevel = 0;
                                state.industrialSurveyActive = false;
                                state.surveyRunsCompleted = 0;
                                state.industrialOutpostActive = false;
                                state.outpostSupplyRuns = 0;
                                state.assemblyCellActive = false;
                                state.assemblyCyclesCompleted = 0;
                                state.foundryLineActive = false;
                                state.foundryCyclesCompleted = 0;
                                state.reactorYardActive = false;
                                state.reactorCyclesCompleted = 0;
                                state.capacitorBankActive = false;
                                state.capacitorDischargeCycles = 0;
                                state.relaySubstationActive = false;
                                state.relaySyncCycles = 0;
                                state.serviceBayActive = false;
                                state.serviceCyclesCompleted = 0;
                                state.waterReclaimerActive = false;
                                state.waterCyclesCompleted = 0;
                            } else {
                                state.industrialGateUnlocked = false;
                                state.routeContamination = std::stof(parts[6]);
                                state.routeOverrun = std::stoi(parts[7]) != 0;
                                if (parts.size() >= 11) {
                                    state.caravanRunsCompleted = std::stoi(parts[8]);
                                    state.droneRunsCompleted = std::stoi(parts[9]);
                                    state.tradeCyclesCompleted = std::stoi(parts[10]);
                                }
                                if (parts.size() >= 12) {
                                    state.purgeCycles = std::stoi(parts[11]);
                                }
                                state.railRunsCompleted = 0;
                                state.orbitalScansCompleted = 0;
                                state.railFortressDeployments = 0;
                                state.fabricatorCyclesCompleted = 0;
                                state.recoveryMilestonesClaimed = 0;
                                state.campFortificationLevel = 0;
                                state.industrialSurveyActive = false;
                                state.surveyRunsCompleted = 0;
                                state.industrialOutpostActive = false;
                                state.outpostSupplyRuns = 0;
                                state.assemblyCellActive = false;
                                state.assemblyCyclesCompleted = 0;
                                state.foundryLineActive = false;
                                state.foundryCyclesCompleted = 0;
                                state.reactorYardActive = false;
                                state.reactorCyclesCompleted = 0;
                                state.capacitorBankActive = false;
                                state.capacitorDischargeCycles = 0;
                                state.relaySubstationActive = false;
                                state.relaySyncCycles = 0;
                                state.serviceBayActive = false;
                                state.serviceCyclesCompleted = 0;
                                state.waterReclaimerActive = false;
                                state.waterCyclesCompleted = 0;
                            }
                        }
                    }
                    outProfile.worldFieldStates.push_back(state);
                }
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
                if (next == std::string::npos) break;
                start = next + 1;
            }
        } else if (key == "vehicle") {
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (start <= value.size()) {
                const std::size_t next = value.find(',', start);
                parts.push_back(value.substr(start, next == std::string::npos ? std::string::npos : next - start));
                if (next == std::string::npos) break;
                start = next + 1;
            }
            if (parts.size() >= 7) {
                outProfile.ownedVehicles.push_back({parts[0], parts[1], static_cast<VehicleType>(std::stoi(parts[2])), std::stoi(parts[3]) != 0, std::stoi(parts[4]) != 0, std::stof(parts[5]), std::stof(parts[6]), {}});
            }
        }
    }

    NormalizeSessionProfile(outProfile);
    return true;
}

}  // namespace bunker
