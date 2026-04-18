#include "../include/GameRuntime.hpp"
#include "../include/LanlineSession.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "imgui.h"

namespace bunker {

namespace {

bool TankNeedsRepair(const SessionProfile& profile) {
    const auto& damage = profile.partnerTank.damage;
    return damage.hull < 99.0f || damage.turret < 99.0f || damage.bucket < 99.0f ||
        damage.sensors < 99.0f || damage.cockpit < 99.0f || damage.powerCore < 99.0f ||
        profile.partnerTank.energyReserve < 99.0f || profile.partnerTank.ammoReserve < 99.0f;
}

TankModuleSlot* FindTankModule(SessionProfile& profile, TankModuleSlotType type) {
    for (auto& module : profile.partnerTank.loadout.modules) {
        if (module.type == type) {
            return &module;
        }
    }
    return nullptr;
}

const TankModuleSlot* FindTankModule(const SessionProfile& profile, TankModuleSlotType type) {
    for (const auto& module : profile.partnerTank.loadout.modules) {
        if (module.type == type) {
            return &module;
        }
    }
    return nullptr;
}

bool TankUsesRamShield(const SessionProfile& profile) {
    const auto* module = FindTankModule(profile, TankModuleSlotType::Bucket);
    return module != nullptr && module->moduleId == "ram_shield_mk1";
}

bool TankUsesTowCoupler(const SessionProfile& profile) {
    const auto* module = FindTankModule(profile, TankModuleSlotType::Bucket);
    return module != nullptr && module->moduleId == "tow_coupler_mk1";
}

bool TankUsesBucketRig(const SessionProfile& profile) {
    const auto* module = FindTankModule(profile, TankModuleSlotType::Bucket);
    return module != nullptr && module->moduleId == "bucket_shield_a";
}

float TowLogisticsBoost(const SessionProfile& profile) {
    return TankUsesTowCoupler(profile) ? 1.22f : 1.0f;
}

const char* CurrentUtilityModuleLabel(const SessionProfile& profile) {
    if (TankUsesRamShield(profile)) {
        return "Ram Shield Mk.I";
    }
    if (TankUsesTowCoupler(profile)) {
        return "Tow Coupler Mk.I";
    }
    return "Bucket Rig Mk.I";
}

void ToggleTankUtilityModule(SessionProfile& profile, PlayerState& player, GameState& gameState) {
    auto* module = FindTankModule(profile, TankModuleSlotType::Bucket);
    if (module == nullptr) {
        gameState.lastEvent = "No utility hardpoint found on BT-72.";
        return;
    }

    if (module->moduleId == "bucket_shield_a") {
        module->moduleId = "ram_shield_mk1";
        module->displayName = "Ram Shield Mk.I";
        player.bucketRaised = false;
        gameState.lastEvent = "BT-72 utility module swapped to Ram Shield Mk.I.";
    } else if (module->moduleId == "ram_shield_mk1") {
        module->moduleId = "tow_coupler_mk1";
        module->displayName = "Tow Coupler Mk.I";
        player.bucketRaised = false;
        gameState.lastEvent = "BT-72 utility module swapped to Tow Coupler Mk.I.";
    } else {
        module->moduleId = "bucket_shield_a";
        module->displayName = "Bucket Rig Mk.I";
        player.bucketRaised = profile.story.bucketRecovered;
        gameState.lastEvent = "BT-72 utility module swapped to Bucket Rig Mk.I.";
    }
}

bool TankHasBulwarkSync(const SessionProfile& profile) {
    return CurrentTankSyncMode(profile.partnerTank) == "Bulwark Sync";
}

bool TankHasStabilizerSync(const SessionProfile& profile) {
    return CurrentTankSyncMode(profile.partnerTank) == "Stabilizer Sync";
}

float DoctrineWorkshopBoost(const SessionProfile& profile) {
    switch (profile.doctrine) {
        case ShelterDoctrine::Industry: return 1.14f;
        case ShelterDoctrine::Defense: return 1.06f;
        case ShelterDoctrine::Medical:
        case ShelterDoctrine::Balanced:
        default:
            return 1.0f;
    }
}

float DoctrineCampRecoveryBoost(const SessionProfile& profile) {
    switch (profile.doctrine) {
        case ShelterDoctrine::Medical: return 1.2f;
        case ShelterDoctrine::Industry:
        case ShelterDoctrine::Defense:
        case ShelterDoctrine::Balanced:
        default:
            return 1.0f;
    }
}

float WaterRecoveryBoost(const SessionProfile& profile) {
    const auto* worldState = FindWorldFieldState(profile, profile.selectedWorld);
    if (worldState == nullptr || !worldState->waterReclaimerActive) {
        return 1.0f;
    }
    return profile.doctrine == ShelterDoctrine::Medical ? 1.18f : 1.08f;
}

const char* RecoveryStatusLabel(const SessionProfile& profile, const WorldFieldState* worldState) {
    if (worldState != nullptr && IsStableRecoveryBackbone(profile, *worldState)) {
        return "Stable backbone";
    }
    return profile.story.returnedToBase ? "Recovery buildout active" : "Starter route";
}

std::string DescribeTerminalSync(const MapObject& object) {
    if (object.scriptTag == "tower_sync") {
        return "Tower sync complete. Regional grid reach expanded.";
    }
    if (object.scriptTag == "power_pylon") {
        return "Pylon registry mirrored. Grid restoration route updated.";
    }
    if (object.scriptTag == "drone_station") {
        return "Drone station ledger mirrored. Sweep routes registered to Pip-Pad.";
    }
    if (object.scriptTag == "rail_freight") {
        return "Rail freight depot records mirrored. Heavy spur logistics registered.";
    }
    if (object.scriptTag == "orbital_uplink") {
        return "Orbital uplink records mirrored. Long-range scan queue registered.";
    }
    if (object.scriptTag == "rail_fortress") {
        return "Rail Fortress patrol package mirrored. Spur security doctrine updated.";
    }
    if (object.scriptTag == "recovery_fabricator") {
        return "Recovery fabricator recipes mirrored. Shelter supply pipeline updated.";
    }
    if (object.scriptTag == "industrial_gate") {
        return "Industrial gate overrides mirrored. Inner spur access route logged.";
    }
    if (object.scriptTag == "industrial_survey") {
        return "Industrial survey notes mirrored. Inner spur reconnaissance queue updated.";
    }
    if (object.scriptTag == "industrial_outpost") {
        return "Inner spur outpost records mirrored. Forward recovery foothold logged.";
    }
    if (object.scriptTag == "assembly_cell") {
        return "Assembly cell notes mirrored. Local recovery production registered.";
    }
    if (object.scriptTag == "foundry_line") {
        return "Foundry line records mirrored. Heavy fabrication route registered.";
    }
    if (object.scriptTag == "reactor_yard") {
        return "Reactor yard records mirrored. Heavy energy recovery route registered.";
    }
    if (object.scriptTag == "capacitor_bank") {
        return "Capacitor bank records mirrored. Buffered grid discharge route registered.";
    }
    if (object.scriptTag == "relay_substation") {
        return "Relay substation notes mirrored. Backbone return flow updated.";
    }
    if (object.scriptTag == "service_bay") {
        return "Service bay notes mirrored. BT-72 support route updated.";
    }
    if (object.scriptTag == "water_reclaimer") {
        return "Water reclaimer notes mirrored. Frontier recovery reserves updated.";
    }
    if (object.scriptTag == "specialist_cryo") {
        return "Cryo specialist registry mirrored. Shelter staffing ledger updated.";
    }
    if (object.scriptTag == "echo_trace") {
        return "Residual echo trace mirrored to Pip-Pad.";
    }
    if (object.scriptTag == "workshop_service") {
        return "Workshop terminal mirrored. BT-72 service route and repair notes updated.";
    }
    return object.scriptTag.empty()
        ? "Terminal sync complete. Additional archive fragments copied to Pip-Pad."
        : "Terminal sync complete: " + object.scriptTag;
}

int CountRestoredPylons(const SessionProfile& profile);
bool IsRegionalGridOnline(const SessionProfile& profile);

bool CaravanRouteReady(const SessionProfile& profile) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld);
    return worldState != nullptr && IsCaravanOperational(profile, *worldState);
}

bool TradeNetworkReady(const SessionProfile& profile, const WorldFieldState& worldState) {
    return IsTradeNetworkOperational(profile, worldState);
}

bool RailFreightReady(const SessionProfile& profile, const WorldFieldState& worldState) {
    return IsRailFreightOperational(profile, worldState);
}

bool OrbitalUplinkReady(const SessionProfile& profile, const WorldFieldState& worldState) {
    return IsOrbitalUplinkOperational(profile, worldState);
}

bool RailFortressReady(const SessionProfile& profile, const WorldFieldState& worldState) {
    return IsRailFortressOperational(profile, worldState);
}

bool RecoveryFabricatorReady(const SessionProfile& profile, const WorldFieldState& worldState) {
    return IsRecoveryFabricatorOperational(profile, worldState);
}

bool IndustrialSurveyReady(const WorldFieldState& worldState) {
    return IsIndustrialSurveyOperational(worldState);
}

bool IndustrialOutpostReady(const WorldFieldState& worldState) {
    return IsIndustrialOutpostOperational(worldState);
}

bool AssemblyCellReady(const WorldFieldState& worldState) {
    return IsAssemblyCellOperational(worldState);
}

bool FoundryLineReady(const WorldFieldState& worldState) {
    return IsFoundryLineOperational(worldState);
}

bool ReactorYardReady(const WorldFieldState& worldState) {
    return IsReactorYardOperational(worldState);
}

bool CapacitorBankReady(const WorldFieldState& worldState) {
    return IsCapacitorBankOperational(worldState);
}

bool RelaySubstationReady(const WorldFieldState& worldState) {
    return IsRelaySubstationOperational(worldState);
}

bool ServiceBayReady(const WorldFieldState& worldState) {
    return IsServiceBayOperational(worldState);
}

bool WaterReclaimerReady(const WorldFieldState& worldState) {
    return IsWaterReclaimerOperational(worldState);
}

float DoctrineScavengerBoost(const SessionProfile& profile) {
    switch (profile.doctrine) {
        case ShelterDoctrine::Industry: return 1.18f;
        case ShelterDoctrine::Defense: return 1.06f;
        case ShelterDoctrine::Medical:
        case ShelterDoctrine::Balanced:
        default:
            return 1.0f;
    }
}

void RegisterTankSyncStyle(SessionProfile& profile, bool ramStyle) {
    if (ramStyle) {
        profile.partnerTank.syncRamActions += 1;
    } else {
        profile.partnerTank.syncShotActions += 1;
    }
    profile.partnerTank.trustLink = std::min(1.0f, profile.partnerTank.trustLink + 0.006f);
}

bool ConsumeAnyRepairMaterial(SessionProfile& profile) {
    const char* repairItems[] = {"scrap_steel", "hydraulic_seal", "wreck_scrap", "old_plate", "copper_wire", "repair_patch"};
    for (const char* itemId : repairItems) {
        if (ConsumeInventoryItem(profile, itemId, 1)) {
            return true;
        }
    }
    return false;
}

MapObject* FindObjectByRegistryId(World& world, const std::string& registryId) {
    for (auto& object : world.objects) {
        if (object.registryId == registryId) {
            return &object;
        }
    }
    return nullptr;
}

float DistanceSqToTankAnchor(const SessionProfile& profile, const MapObject& object) {
    const float dx = object.x - profile.partnerTank.worldX;
    const float dy = object.y - profile.partnerTank.worldY;
    return (dx * dx) + (dy * dy);
}

bool IsTankNearServicePoint(const SessionProfile& profile, const MapObject& object, float radius) {
    if (!profile.partnerTank.worldPositionKnown) {
        return false;
    }
    return DistanceSqToTankAnchor(profile, object) <= (radius * radius);
}

void ActivateFieldCheckpoint(const MapObject& campObject,
    SessionProfile& profile,
    const std::string& worldName) {
    profile.fieldCheckpointKnown = true;
    profile.fieldCheckpointX = campObject.x;
    profile.fieldCheckpointY = campObject.y;
    profile.fieldCheckpointWorld = NormalizeWorldReference(worldName);
    profile.fieldCheckpointLabel = campObject.displayName;
}

void AddRescuedSpecialist(SessionProfile& profile,
    const std::string& specialistId,
    const std::string& displayName,
    const std::string& role) {
    for (auto& specialist : profile.rescuedSpecialists) {
        if (specialist.specialistId == specialistId) {
            specialist.awakened = true;
            if (specialist.displayName.empty()) {
                specialist.displayName = displayName;
            }
            if (specialist.role.empty()) {
                specialist.role = role;
            }
            if (specialist.assignment.empty() || specialist.assignment == "unassigned") {
                specialist.assignment = role == "engineer" ? "workshop" : "field";
            }
            return;
        }
    }
    profile.rescuedSpecialists.push_back({specialistId, displayName, role, role == "engineer" ? "workshop" : "field", true});
}

bool TryApplyTankEnergySiphon(const MapObject& object,
    PlayerState& player,
    SessionProfile& profile,
    GameState& gameState,
    float radius,
    float burstCharge,
    const char* successMessage,
    const char* fullMessage) {
    if (!player.insideTank || !IsTankNearServicePoint(profile, object, radius)) {
        return false;
    }
    if (profile.partnerTank.energyReserve >= 100.0f) {
        gameState.lastEvent = fullMessage;
        return true;
    }
    profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + burstCharge);
    gameState.lastEvent = successMessage;
    return true;
}

bool IsContextualAction(const MapObject& object) {
    return object.interaction == InteractionType::VehicleAnchor ||
        object.interaction == InteractionType::Transition;
}

bool TryRunFieldWorkbench(PlayerState& player,
    SessionProfile& profile,
    GameState& gameState) {
    if (!player.insideTank) {
        gameState.lastEvent = "Field service requires an active BT-72 cockpit link.";
        return false;
    }
    if (gameState.fieldWorkbenchCooldown > 0.0f) {
        gameState.lastEvent = "Field service rack still cycling. Give it a moment.";
        return false;
    }
    const float momentum = std::sqrt((player.velocityX * player.velocityX) + (player.velocityY * player.velocityY));
    if (momentum > 1.0f) {
        gameState.lastEvent = "BT-72 is moving too fast for field service. Stop the hull first.";
        return false;
    }
    if (!TankNeedsRepair(profile) && profile.partnerTank.energyReserve >= 92.0f && profile.partnerTank.ammoReserve >= 90.0f) {
        gameState.lastEvent = "Field service rack reports BT-72 already in acceptable condition.";
        return false;
    }
    if (!ConsumeAnyRepairMaterial(profile)) {
        gameState.lastEvent = "Field service needs scrap, seals, plates, wire, or a repair patch.";
        return false;
    }

    const bool hasEngineer = HasAssignedSpecialistRole(profile, "engineer", "scavenger_support");
    const float boost = hasEngineer ? 1.12f : 1.0f;
    profile.partnerTank.damage.hull = std::min(100.0f, profile.partnerTank.damage.hull + 8.0f * boost);
    profile.partnerTank.damage.bucket = std::min(100.0f, profile.partnerTank.damage.bucket + 10.0f * boost);
    profile.partnerTank.damage.sensors = std::min(100.0f, profile.partnerTank.damage.sensors + 9.0f * boost);
    profile.partnerTank.damage.turret = std::min(100.0f, profile.partnerTank.damage.turret + 6.0f * boost);
    profile.partnerTank.damage.cockpit = std::min(100.0f, profile.partnerTank.damage.cockpit + 5.0f * boost);
    profile.partnerTank.damage.powerCore = std::min(100.0f, profile.partnerTank.damage.powerCore + 7.0f * boost);
    profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 7.0f);
    profile.partnerTank.ammoReserve = std::min(100.0f, profile.partnerTank.ammoReserve + 4.0f);
    gameState.tankThermalLoad = std::max(0.0f, gameState.tankThermalLoad - 16.0f);
    gameState.fieldWorkbenchCooldown = 45.0f;
    std::string recipeEvent;
    RegisterFieldServiceUse(profile, &recipeEvent);
    gameState.lastEvent = hasEngineer
        ? "Field service rack cycled. Scavenger-side engineer tuning improved the repair pass."
        : "Field service rack cycled. BT-72 patched and partially resupplied in the field.";
    if (!recipeEvent.empty()) {
        gameState.lastEvent += " " + recipeEvent;
    }
    return true;
}

bool HasCollectedTape(const SessionProfile& profile, const std::string& tapeId) {
    return std::any_of(profile.character.collectedTapes.begin(), profile.character.collectedTapes.end(),
        [&](const TapeEntry& tape) { return tape.tapeId == tapeId; });
}

int InventoryCount(const SessionProfile& profile, const std::string& itemId) {
    for (const auto& item : profile.character.inventory) {
        if (item.itemId == itemId) {
            return item.count;
        }
    }
    return 0;
}

void AddCollectedTapeIfMissing(SessionProfile& profile, const std::string& tapeId, const std::string& title) {
    if (!HasCollectedTape(profile, tapeId)) {
        profile.character.collectedTapes.push_back({tapeId, title, false, false, false});
    }
}

int CountRestoredPylons(const SessionProfile& profile) {
    int restored = 0;
    for (const auto& tape : profile.character.collectedTapes) {
        if (tape.tapeId.size() >= 6 && tape.tapeId.find("_pylon") != std::string::npos) {
            restored += 1;
        }
    }
    return restored;
}

float PylonGridBoost(const SessionProfile& profile) {
    return 1.0f + static_cast<float>(CountRestoredPylons(profile)) * 0.08f;
}

float CampFortificationBoost(const SessionProfile& profile) {
    const auto* worldState = FindWorldFieldState(profile, profile.selectedWorld);
    if (worldState == nullptr) {
        return 1.0f;
    }
    return 1.0f + static_cast<float>(worldState->campFortificationLevel) * 0.08f;
}

bool IsRegionalGridOnline(const SessionProfile& profile) {
    return profile.story.relayRecovered || HasCollectedTape(profile, "[%term_0001]_tower");
}

WorldFieldState& EnsureSelectedWorldFieldState(SessionProfile& profile) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr) {
        static WorldFieldState fallback{};
        return fallback;
    }
    return *worldState;
}

float ReduceSelectedWorldEtherErosion(SessionProfile& profile, float amount, bool countPurgeCycle) {
    if (amount <= 0.0f) {
        return 0.0f;
    }
    auto& worldState = EnsureSelectedWorldFieldState(profile);
    const float before = worldState.etherErosion;
    worldState.etherErosion = std::max(0.0f, worldState.etherErosion - amount);
    const float reduced = before - worldState.etherErosion;
    if (countPurgeCycle && reduced > 0.05f) {
        worldState.purgeCycles += 1;
    }
    return reduced;
}

const char* EtherErosionBand(float etherErosion) {
    if (etherErosion >= 70.0f) {
        return "Severe";
    }
    if (etherErosion >= 35.0f) {
        return "Elevated";
    }
    if (etherErosion >= 10.0f) {
        return "Trace";
    }
    return "Stable";
}

const char* InfrastructureDecayBand(float infrastructureDecay) {
    if (infrastructureDecay >= 70.0f) {
        return "Critical";
    }
    if (infrastructureDecay >= 35.0f) {
        return "Strained";
    }
    if (infrastructureDecay >= 10.0f) {
        return "Worn";
    }
    return "Stable";
}

const char* ThermalBand(float thermalLoad) {
    if (thermalLoad >= 80.0f) {
        return "Overheat";
    }
    if (thermalLoad >= 55.0f) {
        return "Hot";
    }
    if (thermalLoad >= 25.0f) {
        return "Warm";
    }
    return "Cool";
}

float ShelterRecoveryIndex(const SessionProfile& profile) {
    const auto* worldState = FindWorldFieldState(profile, profile.selectedWorld);
    if (worldState == nullptr) {
        return 0.0f;
    }

    float score = 0.0f;
    const bool caravanOperational = IsCaravanOperational(profile, *worldState);
    const bool droneOperational = IsDroneStationOperational(profile, *worldState);
    const bool tradeOperational = IsTradeNetworkOperational(profile, *worldState);
    const bool railOperational = IsRailFreightOperational(profile, *worldState);
    const bool orbitalOperational = IsOrbitalUplinkOperational(profile, *worldState);
    const bool fortressOperational = IsRailFortressOperational(profile, *worldState);
    const bool fabricatorOperational = IsRecoveryFabricatorOperational(profile, *worldState);
    const bool surveyOperational = IsIndustrialSurveyOperational(*worldState);
    const bool outpostOperational = IsIndustrialOutpostOperational(*worldState);
    const bool assemblyOperational = IsAssemblyCellOperational(*worldState);
    const bool foundryOperational = IsFoundryLineOperational(*worldState);
    const bool reactorOperational = IsReactorYardOperational(*worldState);
    const bool capacitorOperational = IsCapacitorBankOperational(*worldState);
    const bool relayOperational = IsRelaySubstationOperational(*worldState);
    const bool serviceOperational = IsServiceBayOperational(*worldState);
    const bool waterOperational = IsWaterReclaimerOperational(*worldState);
    if (profile.story.returnedToBase) score += 8.0f;
    if (HasActiveFieldCheckpoint(profile)) score += 8.0f;
    if (IsRegionalGridOnline(profile)) score += 14.0f;
    score += std::min(12.0f, static_cast<float>(CountRestoredPylons(profile)) * 4.0f);
    if (caravanOperational) score += 8.0f;
    if (droneOperational) score += 7.0f;
    if (tradeOperational) score += 8.0f;
    if (railOperational) score += 10.0f;
    if (orbitalOperational) score += 9.0f;
    if (fortressOperational) score += 8.0f;
    if (fabricatorOperational) score += 8.0f;
    if (worldState->industrialGateUnlocked) score += 10.0f;
    if (surveyOperational) score += 7.0f;
    if (outpostOperational) score += 8.0f;
    if (assemblyOperational) score += 8.0f;
    if (foundryOperational) score += 9.0f;
    if (reactorOperational) score += 9.0f;
    if (capacitorOperational) score += 8.0f;
    if (relayOperational) score += 8.0f;
    if (serviceOperational) score += 8.0f;
    if (waterOperational) score += 7.0f;
    score += std::min(9.0f, static_cast<float>(worldState->campFortificationLevel) * 3.0f);
    score += std::min(6.0f, static_cast<float>(worldState->surveyRunsCompleted) * 1.0f);
    score += std::min(6.0f, static_cast<float>(worldState->outpostSupplyRuns) * 1.0f);
    score += std::min(6.0f, static_cast<float>(worldState->assemblyCyclesCompleted) * 1.0f);
    score += std::min(7.0f, static_cast<float>(worldState->foundryCyclesCompleted) * 1.0f);
    score += std::min(7.0f, static_cast<float>(worldState->reactorCyclesCompleted) * 1.0f);
    score += std::min(6.0f, static_cast<float>(worldState->capacitorDischargeCycles) * 1.0f);
    score += std::min(6.0f, static_cast<float>(worldState->relaySyncCycles) * 1.0f);
    score += std::min(6.0f, static_cast<float>(worldState->serviceCyclesCompleted) * 1.0f);
    score += std::min(5.0f, static_cast<float>(worldState->waterCyclesCompleted) * 1.0f);
    score += std::min(8.0f, static_cast<float>(worldState->fabricatorCyclesCompleted) * 1.0f);
    score += std::min(8.0f, static_cast<float>(worldState->tradeCyclesCompleted) * 0.8f);
    score += std::min(8.0f, static_cast<float>(worldState->railRunsCompleted) * 0.8f);
    score += std::min(6.0f, static_cast<float>(worldState->orbitalScansCompleted) * 0.75f);
    score -= std::min(15.0f, worldState->infrastructureDecay * 0.18f);
    score -= std::min(12.0f, worldState->routeContamination * 0.16f);
    score -= std::min(10.0f, worldState->etherErosion * 0.12f);
    if (worldState->routeOverrun) score -= 8.0f;

    return std::clamp(score, 0.0f, 100.0f);
}

const char* ShelterRecoveryBand(float recoveryIndex) {
    if (recoveryIndex >= 85.0f) {
        return "Recovered";
    }
    if (recoveryIndex >= 65.0f) {
        return "Operational";
    }
    if (recoveryIndex >= 40.0f) {
        return "Stabilizing";
    }
    if (recoveryIndex >= 20.0f) {
        return "Fragile";
    }
    return "Critical";
}

int ShelterRecoveryMilestoneTier(float recoveryIndex) {
    if (recoveryIndex >= 75.0f) {
        return 3;
    }
    if (recoveryIndex >= 50.0f) {
        return 2;
    }
    if (recoveryIndex >= 25.0f) {
        return 1;
    }
    return 0;
}

const char* ShelterRecoveryMilestoneLabel(int tier) {
    switch (tier) {
        case 1: return "Checkpoint I";
        case 2: return "Checkpoint II";
        case 3: return "Checkpoint III";
        default: return "No checkpoint";
    }
}

bool HandleScriptTagInteraction(const MapObject* nearest,
    World& world,
    PlayerState& player,
    SessionProfile& profile,
    GameState& gameState) {
    if (nearest == nullptr || nearest->scriptTag.empty()) {
        return false;
    }

    if (nearest->scriptTag == "tower_sync") {
        if (TryApplyTankEnergySiphon(
                *nearest,
                player,
                profile,
                gameState,
                3.8f,
                18.0f,
                "Tower hardline latched. BT-72 batteries fast-charged from the relay spine.",
                "BT-72 batteries already topped off. Tower hardline not needed.")) {
            return true;
        }
        AddCollectedTapeIfMissing(profile, nearest->registryId + "_tower", nearest->displayName + " // Tower Sync");
        std::string progressionEvent;
        AwardExperience(profile, 20, &progressionEvent);
        const std::string routeTarget = nearest->linkTarget.empty() ? "regional_grid" : nearest->linkTarget;
        const int purged = static_cast<int>(std::round(ReduceSelectedWorldEtherErosion(profile, 14.0f, true)));
        gameState.lastEvent = "Tower relay synchronized. Regional scan and power route tagged -> " + routeTarget + ". " + progressionEvent;
        if (purged > 0) {
            gameState.lastEvent += " Ether bloom purged by " + std::to_string(purged) + "%.";
        }
        return true;
    }

    if (nearest->scriptTag == "remote_link") {
        const std::string target = nearest->linkTarget.empty() ? "gate_control" : nearest->linkTarget;
        gameState.lastEvent = player.insideTank
            ? "Remote link established from cockpit. Control target -> " + target + "."
            : "Remote link available. Use from cockpit or field terminal. Control target -> " + target + ".";
        return true;
    }

    if (nearest->scriptTag == "power_pylon") {
        if (!IsRegionalGridOnline(profile)) {
            gameState.lastEvent = "Pylon restoration requires a live relay backbone first.";
            return true;
        }
        const std::string markerId = nearest->registryId + "_pylon";
        if (HasCollectedTape(profile, markerId)) {
            gameState.lastEvent = "Power pylon already restored and feeding the regional line.";
            return true;
        }
        if (!HasInventoryItem(profile, "steel_scrap") || !HasInventoryItem(profile, "copper_wire")) {
            gameState.lastEvent = "Pylon restoration needs steel_scrap and copper_wire.";
            return true;
        }
        ConsumeInventoryItem(profile, "steel_scrap", 1);
        ConsumeInventoryItem(profile, "copper_wire", 1);
        AddCollectedTapeIfMissing(profile, markerId, nearest->displayName + " // Pylon Restored");
        std::string progressionEvent;
        AwardExperience(profile, 25, &progressionEvent);
        const int purged = static_cast<int>(std::round(ReduceSelectedWorldEtherErosion(profile, 6.0f, true)));
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        worldState.infrastructureDecay = std::max(0.0f, worldState.infrastructureDecay - 5.0f);
        gameState.lastEvent = "Power pylon restored. Grid reach extended. " + progressionEvent;
        if (purged > 0) {
            gameState.lastEvent += " Ether bloom reduced by " + std::to_string(purged) + "%.";
        }
        return true;
    }

    if (nearest->scriptTag == "drone_station") {
        if (!IsRegionalGridOnline(profile)) {
            gameState.lastEvent = "Drone station needs a live grid before boot sequence can begin.";
            return true;
        }
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.droneStationsActive) {
            if (!HasInventoryItem(profile, "copper_wire") || !HasInventoryItem(profile, "power_cell")) {
                gameState.lastEvent = "Drone station activation needs copper_wire and power_cell.";
                return true;
            }
            ConsumeInventoryItem(profile, "copper_wire", 1);
            ConsumeInventoryItem(profile, "power_cell", 1);
            worldState.droneStationsActive = true;
            gameState.droneTimer = 210.0f;
            gameState.lastEvent = "Drone station brought online. Automated scavenger drones launched to local sweep routes.";
        } else {
            worldState.droneStationsActive = false;
            gameState.lastEvent = "Drone station returned to standby. Automated sweep routes suspended.";
        }
        return true;
    }

    if (nearest->scriptTag == "rail_depot") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.railFreightActive) {
            if (!IsRegionalGridOnline(profile)) {
                gameState.lastEvent = "Rail freight link needs a live grid before depot motors can wake.";
                return true;
            }
            if (!HasInventoryItem(profile, "steel_scrap") || !HasInventoryItem(profile, "power_cell")) {
                gameState.lastEvent = "Rail depot activation needs steel_scrap and power_cell.";
                return true;
            }
            ConsumeInventoryItem(profile, "steel_scrap", 1);
            ConsumeInventoryItem(profile, "power_cell", 1);
            worldState.railFreightActive = true;
            gameState.railTimer = 320.0f;
            gameState.lastEvent = "Rail freight spur restored. Heavy salvage trains can now service the bunker route.";
        } else {
            worldState.railFreightActive = false;
            gameState.lastEvent = "Rail freight spur placed on standby. Depot traffic suspended.";
        }
        return true;
    }

    if (nearest->scriptTag == "orbital_uplink") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.orbitalUplinkActive) {
            if (!IsRegionalGridOnline(profile) || !worldState.railFreightActive) {
                gameState.lastEvent = "Orbital uplink requires a live grid and active rail freight backbone.";
                return true;
            }
            if (!HasInventoryItem(profile, "power_cell") || !HasInventoryItem(profile, "copper_wire")) {
                gameState.lastEvent = "Orbital uplink boot needs power_cell and copper_wire.";
                return true;
            }
            ConsumeInventoryItem(profile, "power_cell", 1);
            ConsumeInventoryItem(profile, "copper_wire", 1);
            worldState.orbitalUplinkActive = true;
            gameState.orbitalTimer = 420.0f;
            gameState.lastEvent = "Orbital uplink aligned. Low-orbit scan window acquired for Shelter 17.";
        } else {
            worldState.orbitalUplinkActive = false;
            gameState.lastEvent = "Orbital uplink placed on standby. Scan uplink suspended.";
        }
        return true;
    }

    if (nearest->scriptTag == "rail_fortress_hub") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.railFortressActive) {
            if (!worldState.railFreightActive || !worldState.orbitalUplinkActive) {
                gameState.lastEvent = "Rail fortress requires active rail freight and orbital uplink support.";
                return true;
            }
            if (!HasInventoryItem(profile, "old_plate") || !HasInventoryItem(profile, "power_cell")) {
                gameState.lastEvent = "Rail fortress deployment needs old_plate and power_cell.";
                return true;
            }
            ConsumeInventoryItem(profile, "old_plate", 1);
            ConsumeInventoryItem(profile, "power_cell", 1);
            worldState.railFortressActive = true;
            gameState.railFortressTimer = 520.0f;
            gameState.lastEvent = "Rail fortress marshaled. Armored train now anchors heavy recovery across the spur.";
        } else {
            worldState.railFortressActive = false;
            gameState.lastEvent = "Rail fortress returned to depot standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "recovery_fabricator") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.recoveryFabricatorActive) {
            if (!IsRegionalGridOnline(profile) || !worldState.railFreightActive) {
                gameState.lastEvent = "Recovery fabricator requires a live grid and rail freight support.";
                return true;
            }
            if (!HasInventoryItem(profile, "steel_scrap") || !HasInventoryItem(profile, "ether_shard")) {
                gameState.lastEvent = "Recovery fabricator boot needs steel_scrap and ether_shard.";
                return true;
            }
            ConsumeInventoryItem(profile, "steel_scrap", 1);
            ConsumeInventoryItem(profile, "ether_shard", 1);
            worldState.recoveryFabricatorActive = true;
            gameState.fabricatorTimer = 280.0f;
            gameState.lastEvent = "Recovery fabricator primed. Shelter industry can now refine salvage into field supplies.";
        } else {
            worldState.recoveryFabricatorActive = false;
            gameState.lastEvent = "Recovery fabricator returned to standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "industrial_gate") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (worldState.industrialGateUnlocked) {
            gameState.lastEvent = nearest->linkTarget.empty()
                ? "Industrial gate already unlocked. Inner spur route is open."
                : "Industrial gate already unlocked. Route open -> " + nearest->linkTarget + ".";
            return true;
        }
        if (!worldState.recoveryFabricatorActive || !worldState.railFortressActive || !worldState.orbitalUplinkActive) {
            gameState.lastEvent = "Industrial gate requires the full recovery backbone: fabricator, fortress, and uplink.";
            return true;
        }
        if (InventoryCount(profile, "old_plate") < 2 || InventoryCount(profile, "power_cell") < 1 || InventoryCount(profile, "repair_patch") < 1) {
            gameState.lastEvent = "Industrial gate override needs 2 old_plate, 1 power_cell, and 1 repair_patch.";
            return true;
        }
        ConsumeInventoryItem(profile, "old_plate", 2);
        ConsumeInventoryItem(profile, "power_cell", 1);
        ConsumeInventoryItem(profile, "repair_patch", 1);
        worldState.industrialGateUnlocked = true;
        worldState.routeContamination = std::max(0.0f, worldState.routeContamination - 8.0f);
        worldState.infrastructureDecay = std::max(0.0f, worldState.infrastructureDecay - 6.0f);
        std::string progressionEvent;
        AwardExperience(profile, 90, &progressionEvent);
        gameState.lastEvent = nearest->linkTarget.empty()
            ? "Industrial gate override accepted. Inner spur route unlocked for Shelter 17. " + progressionEvent
            : "Industrial gate override accepted. Route open -> " + nearest->linkTarget + ". " + progressionEvent;
        return true;
    }

    if (nearest->scriptTag == "industrial_survey") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.industrialGateUnlocked) {
            gameState.lastEvent = "Industrial survey beacon is outside the current recovery perimeter. Unlock the gate first.";
            return true;
        }
        if (!worldState.industrialSurveyActive) {
            if (!worldState.orbitalUplinkActive || !worldState.tradeNetworkActive) {
                gameState.lastEvent = "Industrial survey needs orbital uplink and trade network support.";
                return true;
            }
            if (InventoryCount(profile, "power_cell") < 1 || InventoryCount(profile, "copper_wire") < 1) {
                gameState.lastEvent = "Industrial survey startup needs 1 power_cell and 1 copper_wire.";
                return true;
            }
            ConsumeInventoryItem(profile, "power_cell", 1);
            ConsumeInventoryItem(profile, "copper_wire", 1);
            worldState.industrialSurveyActive = true;
            gameState.surveyTimer = 360.0f;
            gameState.lastEvent = "Industrial survey beacon aligned. Inner spur reconnaissance is now feeding Shelter 17.";
        } else {
            worldState.industrialSurveyActive = false;
            gameState.lastEvent = "Industrial survey beacon returned to standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "industrial_outpost") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.industrialGateUnlocked || !worldState.industrialSurveyActive) {
            gameState.lastEvent = "Inner spur outpost needs the gate unlocked and active survey coverage first.";
            return true;
        }
        if (!worldState.industrialOutpostActive) {
            if (InventoryCount(profile, "old_plate") < 2 || InventoryCount(profile, "repair_patch") < 1 || InventoryCount(profile, "power_cell") < 1) {
                gameState.lastEvent = "Outpost activation needs 2 old_plate, 1 repair_patch, and 1 power_cell.";
                return true;
            }
            ConsumeInventoryItem(profile, "old_plate", 2);
            ConsumeInventoryItem(profile, "repair_patch", 1);
            ConsumeInventoryItem(profile, "power_cell", 1);
            worldState.industrialOutpostActive = true;
            gameState.outpostTimer = 300.0f;
            gameState.lastEvent = "Inner spur outpost established. Shelter 17 now has a forward foothold beyond the industrial gate.";
        } else {
            worldState.industrialOutpostActive = false;
            gameState.lastEvent = "Inner spur outpost returned to standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "assembly_cell") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.industrialOutpostActive || !worldState.industrialSurveyActive) {
            gameState.lastEvent = "Assembly cell needs a live inner spur outpost and survey coverage first.";
            return true;
        }
        if (!worldState.assemblyCellActive) {
            if (InventoryCount(profile, "steel_scrap") < 2 || InventoryCount(profile, "old_plate") < 1 || InventoryCount(profile, "power_cell") < 1) {
                gameState.lastEvent = "Assembly cell startup needs 2 steel_scrap, 1 old_plate, and 1 power_cell.";
                return true;
            }
            ConsumeInventoryItem(profile, "steel_scrap", 2);
            ConsumeInventoryItem(profile, "old_plate", 1);
            ConsumeInventoryItem(profile, "power_cell", 1);
            worldState.assemblyCellActive = true;
            gameState.assemblyTimer = 340.0f;
            gameState.lastEvent = "Inner spur assembly cell brought online. Local industrial recovery has started past the gate.";
        } else {
            worldState.assemblyCellActive = false;
            gameState.lastEvent = "Inner spur assembly cell returned to standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "foundry_line") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.assemblyCellActive || !worldState.industrialOutpostActive || !worldState.railFortressActive) {
            gameState.lastEvent = "Foundry line needs an active assembly cell, inner spur outpost, and Rail Fortress cover first.";
            return true;
        }
        if (!worldState.foundryLineActive) {
            if (InventoryCount(profile, "steel_scrap") < 3 || InventoryCount(profile, "old_plate") < 2 || InventoryCount(profile, "power_cell") < 1) {
                gameState.lastEvent = "Foundry startup needs 3 steel_scrap, 2 old_plate, and 1 power_cell.";
                return true;
            }
            ConsumeInventoryItem(profile, "steel_scrap", 3);
            ConsumeInventoryItem(profile, "old_plate", 2);
            ConsumeInventoryItem(profile, "power_cell", 1);
            worldState.foundryLineActive = true;
            gameState.foundryTimer = 420.0f;
            gameState.lastEvent = "Inner spur foundry line brought online. Heavy plate and hull-grade fabrication resumed.";
        } else {
            worldState.foundryLineActive = false;
            gameState.lastEvent = "Inner spur foundry line returned to standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "reactor_yard") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.foundryLineActive || !worldState.assemblyCellActive || !worldState.orbitalUplinkActive) {
            gameState.lastEvent = "Reactor yard needs an active foundry line, assembly cell, and orbital uplink first.";
            return true;
        }
        if (!worldState.reactorYardActive) {
            if (InventoryCount(profile, "power_cell") < 2 || InventoryCount(profile, "ether_shard") < 2 || InventoryCount(profile, "copper_wire") < 2) {
                gameState.lastEvent = "Reactor yard startup needs 2 power_cell, 2 ether_shard, and 2 copper_wire.";
                return true;
            }
            ConsumeInventoryItem(profile, "power_cell", 2);
            ConsumeInventoryItem(profile, "ether_shard", 2);
            ConsumeInventoryItem(profile, "copper_wire", 2);
            worldState.reactorYardActive = true;
            gameState.reactorTimer = 460.0f;
            gameState.lastEvent = "Inner spur reactor yard brought online. Heavy energy recovery resumed beyond the gate.";
        } else {
            worldState.reactorYardActive = false;
            gameState.lastEvent = "Inner spur reactor yard returned to standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "capacitor_bank") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.reactorYardActive || !worldState.foundryLineActive || !worldState.orbitalUplinkActive) {
            gameState.lastEvent = "Capacitor bank needs an active reactor yard, foundry line, and orbital uplink first.";
            return true;
        }
        if (!worldState.capacitorBankActive) {
            if (InventoryCount(profile, "power_cell") < 2 || InventoryCount(profile, "copper_wire") < 2 || InventoryCount(profile, "ether_shard") < 1) {
                gameState.lastEvent = "Capacitor bank startup needs 2 power_cell, 2 copper_wire, and 1 ether_shard.";
                return true;
            }
            ConsumeInventoryItem(profile, "power_cell", 2);
            ConsumeInventoryItem(profile, "copper_wire", 2);
            ConsumeInventoryItem(profile, "ether_shard", 1);
            worldState.capacitorBankActive = true;
            gameState.capacitorTimer = 390.0f;
            gameState.lastEvent = "Inner spur capacitor bank charged and tied into the recovery backbone.";
        } else {
            worldState.capacitorBankActive = false;
            gameState.lastEvent = "Inner spur capacitor bank returned to standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "relay_substation") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.capacitorBankActive || !worldState.reactorYardActive || !worldState.industrialOutpostActive) {
            gameState.lastEvent = "Relay substation needs an active capacitor bank, reactor yard, and inner spur outpost first.";
            return true;
        }
        if (!worldState.relaySubstationActive) {
            if (InventoryCount(profile, "copper_wire") < 3 || InventoryCount(profile, "power_cell") < 1 || InventoryCount(profile, "repair_patch") < 1) {
                gameState.lastEvent = "Relay substation sync needs 3 copper_wire, 1 power_cell, and 1 repair_patch.";
                return true;
            }
            ConsumeInventoryItem(profile, "copper_wire", 3);
            ConsumeInventoryItem(profile, "power_cell", 1);
            ConsumeInventoryItem(profile, "repair_patch", 1);
            worldState.relaySubstationActive = true;
            gameState.relaySubstationTimer = 430.0f;
            gameState.lastEvent = nearest->linkTarget.empty()
                ? "Relay substation synchronized. Inner spur power now routes back into Shelter 17."
                : "Relay substation synchronized. Backbone route online -> " + nearest->linkTarget + ".";
        } else {
            worldState.relaySubstationActive = false;
            gameState.lastEvent = "Relay substation returned to standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "service_bay") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.relaySubstationActive || !worldState.foundryLineActive || !worldState.industrialOutpostActive) {
            gameState.lastEvent = "Service bay needs a live relay substation, foundry line, and inner spur outpost first.";
            return true;
        }
        if (!worldState.serviceBayActive) {
            if (InventoryCount(profile, "old_plate") < 2 || InventoryCount(profile, "repair_patch") < 1 || InventoryCount(profile, "power_cell") < 1) {
                gameState.lastEvent = "Service bay startup needs 2 old_plate, 1 repair_patch, and 1 power_cell.";
                return true;
            }
            ConsumeInventoryItem(profile, "old_plate", 2);
            ConsumeInventoryItem(profile, "repair_patch", 1);
            ConsumeInventoryItem(profile, "power_cell", 1);
            worldState.serviceBayActive = true;
            gameState.serviceBayTimer = 300.0f;
            gameState.lastEvent = "Inner spur service bay brought online. BT-72 can now be serviced deeper into the factory belt.";
        } else {
            worldState.serviceBayActive = false;
            gameState.lastEvent = "Inner spur service bay returned to standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "water_reclaimer") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.serviceBayActive || !worldState.relaySubstationActive || !worldState.recoveryFabricatorActive) {
            gameState.lastEvent = "Water reclaimer needs a live service bay, relay substation, and recovery fabricator first.";
            return true;
        }
        if (!worldState.waterReclaimerActive) {
            if (InventoryCount(profile, "copper_wire") < 2 || InventoryCount(profile, "old_plate") < 1 || InventoryCount(profile, "power_cell") < 1) {
                gameState.lastEvent = "Water reclaimer startup needs 2 copper_wire, 1 old_plate, and 1 power_cell.";
                return true;
            }
            ConsumeInventoryItem(profile, "copper_wire", 2);
            ConsumeInventoryItem(profile, "old_plate", 1);
            ConsumeInventoryItem(profile, "power_cell", 1);
            worldState.waterReclaimerActive = true;
            gameState.waterReclaimerTimer = 260.0f;
            gameState.lastEvent = "Inner spur water reclaimer brought online. Long-range recovery now has a stable water source.";
        } else {
            worldState.waterReclaimerActive = false;
            gameState.lastEvent = "Inner spur water reclaimer returned to standby.";
        }
        return true;
    }

    if (nearest->scriptTag == "archive_sync") {
        AddCollectedTapeIfMissing(profile, nearest->registryId, nearest->displayName);
        gameState.lastEvent = "Archive sync complete. Personnel and recovery records mirrored to Pip-Pad.";
        return true;
    }

    if (nearest->scriptTag == "echo_trace") {
        if (!profile.story.pipPadRecovered) {
            gameState.lastEvent = "Pip-Pad AR layer required before echo traces can be reconstructed.";
            return true;
        }
        AddCollectedTapeIfMissing(profile, nearest->registryId + "_echo", nearest->displayName + " // Echo Trace");
        const std::string targetId = nearest->linkTarget;
        if (!targetId.empty()) {
            if (auto* targetObject = FindObjectByRegistryId(world, targetId); targetObject != nullptr) {
                targetObject->discovered = true;
                gameState.lastEvent = "Pip-Pad AR echo resolved. Hidden trace now points to " + targetObject->displayName + ".";
                return true;
            }
            gameState.lastEvent = "Pip-Pad AR echo resolved. Trace marker linked to " + targetId + ".";
            return true;
        }
        gameState.lastEvent = "Pip-Pad AR echo resolved. Residual silhouettes recorded to archive.";
        return true;
    }

    if (nearest->scriptTag == "specialist_cryo") {
        const std::string role = nearest->linkTarget.empty() ? "specialist" : nearest->linkTarget;
        AddRescuedSpecialist(profile, nearest->registryId, nearest->displayName, role);
        AddCollectedTapeIfMissing(profile, nearest->registryId + "_personnel", nearest->displayName + " // Personnel Recovery");
        gameState.lastEvent = nearest->displayName + " recovered from cryostasis. Role assigned: " + role + ".";
        return true;
    }

    if (nearest->scriptTag == "terminal_sync") {
        AddCollectedTapeIfMissing(profile, nearest->registryId, nearest->displayName);
        gameState.lastEvent = "Terminal sync complete. General system notes mirrored to Pip-Pad.";
        return true;
    }

    if (nearest->scriptTag == "workshop_service") {
        return false;
    }

    return false;
}

}  // namespace

void AddInventoryItem(SessionProfile& profile, const std::string& itemId, int count, float weight) {
    if (itemId.empty() || count <= 0) {
        return;
    }

    for (auto& item : profile.character.inventory) {
        if (item.itemId == itemId) {
            item.count += count;
            return;
        }
    }

    profile.character.inventory.push_back({itemId, count, weight});
}

bool HasInventoryItem(const SessionProfile& profile, const std::string& itemId) {
    return std::any_of(profile.character.inventory.begin(), profile.character.inventory.end(), [&](const InventoryEntry& item) {
        return item.itemId == itemId && item.count > 0;
    });
}

bool ConsumeInventoryItem(SessionProfile& profile, const std::string& itemId, int count) {
    if (count <= 0) {
        return false;
    }

    for (auto it = profile.character.inventory.begin(); it != profile.character.inventory.end(); ++it) {
        if (it->itemId != itemId || it->count < count) {
            continue;
        }

        it->count -= count;
        if (it->count == 0) {
            profile.character.inventory.erase(it);
        }
        return true;
    }

    return false;
}

float CurrentInventoryWeight(const SessionProfile& profile) {
    float totalWeight = 0.0f;
    for (const auto& item : profile.character.inventory) {
        totalWeight += item.unitWeight * static_cast<float>(item.count);
    }
    return totalWeight;
}

int EffectiveStatValue(const SessionProfile& profile, const GameState& gameState, char statCode) {
    int value = profile.character.StatValue(statCode);
    if (gameState.rationEffectTimer > 0.0f) {
        switch (static_cast<char>(std::toupper(static_cast<unsigned char>(statCode)))) {
            case 'S':
                value += gameState.rationStrengthBonus;
                break;
            case 'I':
                value -= gameState.rationIntelligencePenalty;
                break;
            default:
                break;
        }
    }
    return std::max(1, value);
}

bool TryConsumeFieldRation(SessionProfile& profile, GameState& gameState) {
    if (!ConsumeInventoryItem(profile, "#%it_field_ration", 1)) {
        return false;
    }
    gameState.rationEffectTimer = 150.0f;
    gameState.rationStrengthBonus = 2;
    gameState.rationIntelligencePenalty = 1;
    gameState.lastEvent = "Old ration consumed. Toxic boost applied: +2 STR, -1 INT for a while.";
    return true;
}

void AdvanceViewMode(PlayerState& player) {
    switch (player.viewMode) {
        case ViewMode::FirstPerson:
            player.viewMode = ViewMode::ThirdPerson;
            break;
        case ViewMode::ThirdPerson:
            player.viewMode = ViewMode::Cockpit;
            break;
        case ViewMode::Cockpit:
            player.viewMode = ViewMode::FirstPerson;
            break;
    }
}

void ApplyStaticEraser(World& world, const StaticEraser& staticEraser) {
    world.objects.erase(
        std::remove_if(world.objects.begin(), world.objects.end(),
            [&](const MapObject& object) { return staticEraser.IsErased(object.registryId); }),
        world.objects.end());
}

bool ShouldUseStarterStoryFlow(const World& world) {
    return world.IsStarterScenarioWorld();
}

void SyncStoryFlagsFromWorld(SessionProfile& profile, const StaticEraser& staticEraser) {
    profile.story.bucketRecovered = profile.story.bucketRecovered || staticEraser.IsErased("#%it_bucket_0001");
    profile.story.outerRoadCleared = profile.story.outerRoadCleared || staticEraser.IsErased("#%res_scrap_0001");
    profile.story.pipPadRecovered = profile.story.pipPadRecovered || HasInventoryItem(profile, "#%it_pippad");
    profile.story.tankLinked = profile.story.tankLinked || profile.partnerTank.deployed;
    profile.story.relayRecovered = profile.story.relayRecovered || HasInventoryItem(profile, "relay_reconstruction_data");
}

std::string CurrentObjective(const SessionProfile& profile, const StaticEraser& staticEraser) {
    return CurrentStoryObjective(profile, staticEraser);
}

void UpdateWorldMetadata(World& world, const SessionProfile& profile, const StaticEraser& staticEraser) {
    if (ShouldUseStarterStoryFlow(world)) {
        world.metadata.objective = CurrentObjective(profile, staticEraser);
    }
}

void UpdateWindowTitle(GLFWwindow* window, const PlayerState& player, const World& world, const SessionProfile& sessionProfile) {
    const auto* worldState = FindWorldFieldState(sessionProfile, sessionProfile.selectedWorld);
    char title[256];
    std::snprintf(
        title,
        sizeof(title),
        "BunkerGame | %s | %s | %s | %s | %s",
        sessionProfile.character.displayName.c_str(),
        player.insideTank ? sessionProfile.partnerTank.callSign.c_str() : "On Foot",
        ToString(player.viewMode),
        world.metadata.name.c_str(),
        RecoveryStatusLabel(sessionProfile, worldState));
    glfwSetWindowTitle(window, title);
}

void UpdateRadio(GameState& gameState, const World& world, const SessionProfile& profile, const StaticEraser& staticEraser, float dt) {
    if (!ShouldUseStarterStoryFlow(world)) {
        return;
    }

    gameState.radioTimer -= dt;
    if (gameState.radioTimer > 0.0f) {
        return;
    }

    if (gameState.radioPhase < gameState.radioMessages.size()) {
        gameState.lastEvent = gameState.radioMessages[gameState.radioPhase++];
    } else {
        gameState.lastEvent = "COMMS: '" + CurrentStoryObjective(profile, staticEraser) + "'";
    }

    gameState.radioTimer = 40.0f;
}

const MapObject* FindNearestHostile(const World& world, float x, float y, float radius) {
    const MapObject* nearest = nullptr;
    float bestDistanceSq = radius * radius;
    for (const auto& object : world.objects) {
        if (object.interaction != InteractionType::Hostile) {
            continue;
        }

        const float dx = object.x - x;
        const float dy = object.y - y;
        const float distanceSq = (dx * dx) + (dy * dy);
        if (distanceSq <= bestDistanceSq) {
            bestDistanceSq = distanceSq;
            nearest = &object;
        }
    }
    return nearest;
}

void SyncPartnerTankAnchor(World& world,
    const PlayerState& player,
    SessionProfile& profile) {
    auto* tankAnchor = FindObjectByRegistryId(world, "[#tr_hull_0001]");
    if (player.insideTank) {
        profile.partnerTank.worldX = player.x;
        profile.partnerTank.worldY = player.y;
        profile.partnerTank.worldPositionKnown = true;
    } else if (!profile.partnerTank.worldPositionKnown && tankAnchor != nullptr) {
        profile.partnerTank.worldX = tankAnchor->x;
        profile.partnerTank.worldY = tankAnchor->y;
        profile.partnerTank.worldPositionKnown = true;
    }

    if (tankAnchor != nullptr && profile.partnerTank.worldPositionKnown) {
        tankAnchor->x = profile.partnerTank.worldX;
        tankAnchor->y = profile.partnerTank.worldY;
    }
}

void UpdateAmbientTankCharging(const World& world,
    SessionProfile& profile,
    GameState& gameState,
    float dt) {
    if (!profile.partnerTank.worldPositionKnown || profile.partnerTank.energyReserve >= 100.0f) {
        return;
    }

    const MapObject* chargeSource = nullptr;
    float bestDistanceSq = 3.6f * 3.6f;
    for (const auto& object : world.objects) {
        const bool isWorkshop = object.interaction == InteractionType::Workshop || object.scriptTag == "workshop_service";
        const bool isTowerGrid = object.scriptTag == "tower_sync";
        if (!isWorkshop && !isTowerGrid) {
            continue;
        }
        if (isWorkshop && !IsRegionalGridOnline(profile)) {
            continue;
        }
        if (isTowerGrid && !HasCollectedTape(profile, object.registryId + "_tower")) {
            continue;
        }

        const float dx = object.x - profile.partnerTank.worldX;
        const float dy = object.y - profile.partnerTank.worldY;
        const float distanceSq = (dx * dx) + (dy * dy);
        if (distanceSq <= bestDistanceSq) {
            bestDistanceSq = distanceSq;
            chargeSource = &object;
        }
    }

    if (chargeSource == nullptr) {
        return;
    }

    const float chargeRate = chargeSource->scriptTag == "tower_sync" ? 1.2f : 2.0f;
    const float before = profile.partnerTank.energyReserve;
    profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + (dt * chargeRate));
    if (static_cast<int>(before) != static_cast<int>(profile.partnerTank.energyReserve) &&
        static_cast<int>(profile.partnerTank.energyReserve) % 5 == 0) {
        gameState.lastEvent = chargeSource->scriptTag == "tower_sync"
            ? "Grid field charging active. Tank batteries replenishing."
            : "Workshop charging cradle active. Tank batteries replenishing.";
    }
}

void UpdateWeatherAnomaly(const World& world,
    PlayerState& player,
    SessionProfile& profile,
    GameState& gameState,
    float dt) {
    const bool outdoors = world.IsStarterScenarioWorld() ? (player.x >= 13.0f || profile.story.exitedBunker) : true;
    if (!outdoors) {
        gameState.weather = WeatherAnomaly::Clear;
        gameState.weatherIntensity = 0.0f;
        gameState.weatherTimer = 70.0f;
        gameState.weatherEventTimer = 0.0f;
        return;
    }

    gameState.weatherTimer -= dt;
    gameState.weatherEventTimer -= dt;
    if (gameState.weatherTimer <= 0.0f) {
        if (gameState.weather == WeatherAnomaly::Clear) {
            gameState.weather = (profile.story.relayRecovered || HasCollectedTape(profile, "[%term_0001]_tower"))
                ? WeatherAnomaly::AcidRain
                : WeatherAnomaly::EtherFog;
            gameState.weatherIntensity = 0.8f;
            gameState.weatherTimer = 60.0f;
        } else if (gameState.weather == WeatherAnomaly::AcidRain) {
            gameState.weather = WeatherAnomaly::EtherFog;
            gameState.weatherIntensity = 0.9f;
            gameState.weatherTimer = 55.0f;
        } else {
            gameState.weather = WeatherAnomaly::Clear;
            gameState.weatherIntensity = 0.0f;
            gameState.weatherTimer = 80.0f;
        }
        gameState.weatherEventTimer = 0.0f;
    }

    if (gameState.weather == WeatherAnomaly::AcidRain) {
        if (player.insideTank) {
            profile.partnerTank.damage.hull = std::max(0.0f, profile.partnerTank.damage.hull - dt * 0.8f);
            profile.partnerTank.damage.powerCore = std::max(0.0f, profile.partnerTank.damage.powerCore - dt * 0.45f);
        } else {
            profile.character.hp = std::max(0.0f, profile.character.hp - dt * 0.9f);
        }
        if (gameState.weatherEventTimer <= 0.0f) {
            gameState.lastEvent = player.insideTank
                ? "Acid rain hammering the hull. Find cover or keep repairs ready."
                : "Acid rain burning through gear. Seek cover or get back to shelter.";
            gameState.weatherEventTimer = 18.0f;
        }
    } else if (gameState.weather == WeatherAnomaly::EtherFog) {
        if (player.insideTank) {
            profile.partnerTank.damage.sensors = std::max(0.0f, profile.partnerTank.damage.sensors - dt * 0.35f);
        } else {
            profile.character.mp = std::max(0.0f, profile.character.mp - dt * 0.45f);
        }
        if (gameState.weatherEventTimer <= 0.0f) {
            gameState.lastEvent = player.insideTank
                ? "Ether fog smearing sensor returns. Cockpit visibility degraded."
                : "Ether fog closing in. Navigation and focus are degrading.";
            gameState.weatherEventTimer = 18.0f;
        }
    }
}

void UpdateEtherErosion(const World& world,
    const PlayerState& player,
    SessionProfile& profile,
    GameState& gameState,
    float dt) {
    auto& worldState = EnsureSelectedWorldFieldState(profile);
    const bool outdoors = world.IsStarterScenarioWorld() ? (player.x >= 13.0f || profile.story.exitedBunker) : true;
    gameState.etherErosionEventTimer = std::max(0.0f, gameState.etherErosionEventTimer - dt);

    if (!outdoors) {
        if (IsRegionalGridOnline(profile) && HasActiveFieldCheckpoint(profile) && worldState.etherErosion > 0.0f) {
            worldState.etherErosion = std::max(0.0f, worldState.etherErosion - dt * 0.03f);
        }
        return;
    }

    if (gameState.weather == WeatherAnomaly::EtherFog) {
        float growthRate = (0.12f + gameState.weatherIntensity * 0.16f) * dt;
        if (!IsRegionalGridOnline(profile)) {
            growthRate *= 1.4f;
        } else {
            growthRate *= 0.72f;
        }
        if (HasActiveFieldCheckpoint(profile)) {
            growthRate *= 0.82f;
        }
        if (profile.story.outerRoadCleared) {
            growthRate *= 0.9f;
        }
        worldState.etherErosion = std::min(100.0f, worldState.etherErosion + growthRate);
    } else if (IsRegionalGridOnline(profile)) {
        const float decayRate = HasActiveFieldCheckpoint(profile) ? 0.08f : 0.04f;
        worldState.etherErosion = std::max(0.0f, worldState.etherErosion - dt * decayRate);
    }

    if (worldState.etherErosion >= 65.0f && gameState.etherErosionEventTimer <= 0.0f) {
        gameState.lastEvent = "Ether erosion severe. Crystal bloom is choking routes and draining system efficiency.";
        gameState.etherErosionEventTimer = 24.0f;
    } else if (gameState.weather == WeatherAnomaly::EtherFog &&
        worldState.etherErosion >= 25.0f &&
        gameState.etherErosionEventTimer <= 0.0f) {
        gameState.lastEvent = "Ether fog is feeding crystal growth across the route. Keep the grid alive or purge the bloom.";
        gameState.etherErosionEventTimer = 24.0f;
    }
}

void UpdateInfrastructureDecay(const World& world,
    const PlayerState& player,
    SessionProfile& profile,
    GameState& gameState,
    float dt) {
    auto& worldState = EnsureSelectedWorldFieldState(profile);
    const bool outdoors = world.IsStarterScenarioWorld() ? (player.x >= 13.0f || profile.story.exitedBunker) : true;

    if (!IsRegionalGridOnline(profile)) {
        float growthRate = outdoors ? 0.06f : 0.03f;
        growthRate *= 1.0f + (worldState.etherErosion / 120.0f);
        growthRate *= std::max(0.7f, 1.0f - static_cast<float>(CountRestoredPylons(profile)) * 0.08f);
        worldState.infrastructureDecay = std::min(100.0f, worldState.infrastructureDecay + dt * growthRate);
    } else {
        float recoveryRate = HasActiveFieldCheckpoint(profile) ? 0.08f : 0.05f;
        if (profile.doctrine == ShelterDoctrine::Industry) {
            recoveryRate *= 1.3f;
        } else if (profile.doctrine == ShelterDoctrine::Medical) {
            recoveryRate *= 1.1f;
        }
        recoveryRate *= PylonGridBoost(profile) * CampFortificationBoost(profile);
        worldState.infrastructureDecay = std::max(0.0f, worldState.infrastructureDecay - dt * recoveryRate);
    }

    if (worldState.infrastructureDecay >= 65.0f && gameState.etherErosionEventTimer <= 0.0f) {
        gameState.lastEvent = "Infrastructure decay critical. Unpowered structures are slipping into collapse and contamination.";
        gameState.etherErosionEventTimer = 22.0f;
    }
}

void UpdateRouteContamination(World& world,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState,
    float dt) {
    if (!world.IsStarterScenarioWorld() || !profile.story.outerRoadCleared) {
        return;
    }

    auto& worldState = EnsureSelectedWorldFieldState(profile);
    if (!IsRegionalGridOnline(profile)) {
        float growthRate = 0.05f * (1.0f + worldState.etherErosion / 100.0f);
        if (worldState.infrastructureDecay >= 25.0f) {
            growthRate *= 1.2f;
        }
        worldState.routeContamination = std::min(100.0f, worldState.routeContamination + dt * growthRate);
    } else {
        const float recoveryRate = 0.08f * PylonGridBoost(profile) * CampFortificationBoost(profile);
        worldState.routeContamination = std::max(0.0f, worldState.routeContamination - dt * recoveryRate);
    }

    if (!worldState.routeOverrun && worldState.routeContamination >= 58.0f) {
        worldState.routeOverrun = true;
        staticEraser.Load(profile.selectedWorld);
        if (staticEraser.IsErased("#%res_scrap_0001")) {
            world.AddObject({
                "#%res_scrap_return_0001",
                "Reformed Ether Barricade",
                InteractionType::Resource,
                ObjectCategory::ResourceNode,
                17.2f,
                1.2f,
                0.0f,
                3.0f,
                2.2f,
                1.2f,
                45.0f,
                true,
                true,
                true,
                {"steel_scrap", "ether_shard", "old_plate", ""}
            });
        }
        if (staticEraser.IsErased("[%enemy_ghoul_0001]")) {
            world.AddObject({
                "[%enemy_nest_0001]",
                "Ghoul Nest",
                InteractionType::Hostile,
                ObjectCategory::Hostile,
                19.2f,
                -0.8f,
                0.0f,
                1.5f,
                1.2f,
                1.7f,
                50.0f,
                true,
                true,
                false,
                {}
            });
        }
        gameState.lastEvent = "Outer route contamination surged. Ether barricades and fresh nests are reforming beyond the bulkhead.";
    } else if (worldState.routeOverrun && worldState.routeContamination <= 18.0f) {
        worldState.routeOverrun = false;
        gameState.lastEvent = "Outer route stabilized again. Contamination pressure has receded.";
    }
}

void UpdateScavengerTeams(SessionProfile& profile, GameState& gameState, float dt) {
    const bool scavengerReady =
        HasActiveFieldCheckpoint(profile) &&
        profile.story.outerRoadCleared &&
        IsRegionalGridOnline(profile) &&
        HasAwakenedSpecialistRole(profile, "engineer");
    if (!scavengerReady) {
        gameState.scavengerTimer = 180.0f;
        return;
    }

    const bool engineerAssignedToScavengers = HasAssignedSpecialistRole(profile, "engineer", "scavenger_support");
    gameState.scavengerTimer -= dt;
    if (gameState.scavengerTimer > 0.0f) {
        return;
    }

    const auto* worldState = FindWorldFieldState(profile, profile.selectedWorld);
    const float infrastructurePenalty = worldState != nullptr
        ? std::max(0.65f, 1.0f - worldState->infrastructureDecay / 180.0f)
        : 1.0f;
    const float doctrineScavengerBoost = DoctrineScavengerBoost(profile) * infrastructurePenalty * PylonGridBoost(profile) * TowLogisticsBoost(profile);
    AddInventoryItem(profile, "steel_scrap",
        static_cast<int>(std::round((engineerAssignedToScavengers ? 3.0f : 2.0f) * doctrineScavengerBoost)),
        0.5f);
    AddInventoryItem(profile, "copper_wire", 1, 0.2f);
    AddInventoryItem(profile, "repair_patch", 1, 0.2f);
    const int erosionCleared = static_cast<int>(std::round(ReduceSelectedWorldEtherErosion(profile, 6.0f, true)));
    if (erosionCleared > 0) {
        AddInventoryItem(profile, "ether_shard", 1, 0.1f);
    }
    profile.scavengerRunsCompleted += 1;
    gameState.scavengerTimer = (engineerAssignedToScavengers ? 150.0f : 180.0f) / doctrineScavengerBoost;
    gameState.lastEvent = engineerAssignedToScavengers
        ? "Scavenger team returned under engineer supervision with boosted salvage intake."
        : (erosionCleared > 0
            ? "Scavenger team returned with salvage and cut back nearby ether bloom."
            : "Scavenger team returned to camp with fresh salvage from cleared routes.");
}

void UpdateCaravanRoute(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->caravanRouteActive) {
        gameState.caravanTimer = 260.0f;
        return;
    }

    const bool caravanReady = CaravanRouteReady(profile);
    if (!caravanReady) {
        gameState.caravanTimer = 260.0f;
        return;
    }

    const float infrastructurePenalty = std::max(0.6f, 1.0f - worldState->infrastructureDecay / 170.0f);
    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.22f :
            (profile.doctrine == ShelterDoctrine::Defense ? 1.08f : 1.0f)) *
        PylonGridBoost(profile) *
        TowLogisticsBoost(profile);
    const bool engineerSupport = HasAssignedSpecialistRole(profile, "engineer", "scavenger_support");

    gameState.caravanTimer -= dt;
    if (gameState.caravanTimer > 0.0f) {
        return;
    }

    const int steelYield = static_cast<int>(std::round((engineerSupport ? 5.0f : 4.0f) * doctrineBoost * infrastructurePenalty));
    const int wireYield = static_cast<int>(std::round(2.0f * doctrineBoost * infrastructurePenalty));
    AddInventoryItem(profile, "steel_scrap", std::max(2, steelYield), 0.5f);
    AddInventoryItem(profile, "copper_wire", std::max(1, wireYield), 0.2f);
    AddInventoryItem(profile, "old_plate", 1, 0.5f);
    if (profile.doctrine == ShelterDoctrine::Defense) {
        AddInventoryItem(profile, "#%it_ptrs_ammo", 2, 0.7f);
    }
    if (worldState->infrastructureDecay >= 12.0f) {
        worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 3.0f);
    }
    worldState->caravanRunsCompleted += 1;
    gameState.caravanTimer = 260.0f / doctrineBoost;
    gameState.lastEvent = engineerSupport
        ? "Autopilot caravan returned under escort support with reinforced cargo intake."
        : "Autopilot caravan returned from the bunker route with bulk salvage.";
}

void UpdateDroneStations(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->droneStationsActive) {
        gameState.droneTimer = 210.0f;
        return;
    }

    if (!IsRegionalGridOnline(profile)) {
        gameState.droneTimer = 210.0f;
        return;
    }

    const float infrastructurePenalty = std::max(0.62f, 1.0f - worldState->infrastructureDecay / 175.0f);
    const float doctrineBoost = (profile.doctrine == ShelterDoctrine::Industry ? 1.28f :
        (profile.doctrine == ShelterDoctrine::Defense ? 1.05f : 1.0f)) *
        TowLogisticsBoost(profile);
    const float pylonBoost = PylonGridBoost(profile);

    gameState.droneTimer -= dt;
    if (gameState.droneTimer > 0.0f) {
        return;
    }

    AddInventoryItem(profile, "steel_scrap", std::max(1, static_cast<int>(std::round(2.0f * doctrineBoost * infrastructurePenalty * pylonBoost))), 0.5f);
    AddInventoryItem(profile, "ether_shard", 1, 0.1f);
    if (profile.doctrine == ShelterDoctrine::Industry) {
        AddInventoryItem(profile, "copper_wire", 1, 0.2f);
    }
    worldState->droneRunsCompleted += 1;
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 1.5f);
    gameState.droneTimer = 210.0f / (doctrineBoost * std::max(1.0f, pylonBoost * 0.95f));
    gameState.lastEvent = "Automated drone sweep completed. Salvage and ether traces transferred to shelter stores.";
}

void UpdateTradeNetwork(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->tradeNetworkActive) {
        gameState.tradeTimer = 240.0f;
        return;
    }

    const bool tradeReady = TradeNetworkReady(profile, *worldState);
    if (!tradeReady) {
        gameState.tradeTimer = 240.0f;
        return;
    }

    const float infrastructurePenalty = std::max(0.65f, 1.0f - worldState->infrastructureDecay / 185.0f);
    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.24f :
            (profile.doctrine == ShelterDoctrine::Medical ? 1.1f : 1.0f)) *
        PylonGridBoost(profile) *
        TowLogisticsBoost(profile);

    gameState.tradeTimer -= dt;
    if (gameState.tradeTimer > 0.0f) {
        return;
    }

    const int vouchers = std::max(1, static_cast<int>(std::round(doctrineBoost * infrastructurePenalty)));
    AddInventoryItem(profile, "trade_voucher", vouchers, 0.0f);
    worldState->tradeCyclesCompleted += 1;
    gameState.tradeTimer = 240.0f / doctrineBoost;
    gameState.lastEvent = "Trade network convoy synchronized. New vouchers and exchange stock entered the camp ledger.";
}

void UpdateRailFreight(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->railFreightActive) {
        gameState.railTimer = 320.0f;
        return;
    }

    const bool railReady = RailFreightReady(profile, *worldState);
    if (!railReady) {
        gameState.railTimer = 320.0f;
        return;
    }

    const float infrastructurePenalty = std::max(0.58f, 1.0f - worldState->infrastructureDecay / 190.0f);
    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.3f :
            (profile.doctrine == ShelterDoctrine::Defense ? 1.08f : 1.0f)) *
        PylonGridBoost(profile) *
        TowLogisticsBoost(profile);
    const bool engineerSupport = HasAssignedSpecialistRole(profile, "engineer", "scavenger_support");

    gameState.railTimer -= dt;
    if (gameState.railTimer > 0.0f) {
        return;
    }

    const int steelYield = std::max(3, static_cast<int>(std::round((engineerSupport ? 8.0f : 6.0f) * doctrineBoost * infrastructurePenalty)));
    const int plateYield = std::max(1, static_cast<int>(std::round(2.0f * doctrineBoost * infrastructurePenalty)));
    AddInventoryItem(profile, "steel_scrap", steelYield, 0.5f);
    AddInventoryItem(profile, "old_plate", plateYield, 0.5f);
    AddInventoryItem(profile, "copper_wire", std::max(1, static_cast<int>(std::round(2.0f * infrastructurePenalty))), 0.2f);
    AddInventoryItem(profile, "trade_voucher", 1, 0.0f);
    if (profile.doctrine == ShelterDoctrine::Industry) {
        AddInventoryItem(profile, "power_cell", 1, 0.3f);
    }
    worldState->railRunsCompleted += 1;
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 2.5f);
    gameState.railTimer = 320.0f / doctrineBoost;
    gameState.lastEvent = engineerSupport
        ? "Rail freight link returned with reinforced salvage under engineer-backed logistics."
        : "Rail freight link delivered heavy salvage from the restored industrial spur.";
}

void UpdateOrbitalUplink(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->orbitalUplinkActive) {
        gameState.orbitalTimer = 420.0f;
        return;
    }

    const bool uplinkReady = OrbitalUplinkReady(profile, *worldState);
    if (!uplinkReady) {
        gameState.orbitalTimer = 420.0f;
        return;
    }

    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.18f :
            (profile.doctrine == ShelterDoctrine::Medical ? 1.08f : 1.0f)) *
        PylonGridBoost(profile);
    const float infrastructurePenalty = std::max(0.64f, 1.0f - worldState->infrastructureDecay / 200.0f);

    gameState.orbitalTimer -= dt;
    if (gameState.orbitalTimer > 0.0f) {
        return;
    }

    const int purged = static_cast<int>(std::round(ReduceSelectedWorldEtherErosion(profile, 10.0f, true)));
    AddInventoryItem(profile, "ether_shard", 1, 0.1f);
    AddInventoryItem(profile, "trade_voucher", 1, 0.0f);
    if (profile.doctrine == ShelterDoctrine::Industry) {
        AddInventoryItem(profile, "power_cell", 1, 0.3f);
    }
    worldState->orbitalScansCompleted += 1;
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - (2.0f * infrastructurePenalty));
    gameState.orbitalTimer = 420.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = purged > 0
        ? "Orbital scan completed. Ether bloom pockets mapped and reduced across the route."
        : "Orbital scan completed. New salvage and relay traces added to the camp network.";
}

void UpdateRailFortress(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->railFortressActive) {
        gameState.railFortressTimer = 520.0f;
        return;
    }

    const bool fortressReady = RailFortressReady(profile, *worldState);
    if (!fortressReady) {
        gameState.railFortressTimer = 520.0f;
        return;
    }

    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Defense ? 1.24f :
            (profile.doctrine == ShelterDoctrine::Industry ? 1.16f : 1.0f)) *
        PylonGridBoost(profile);
    const float infrastructurePenalty = std::max(0.68f, 1.0f - worldState->infrastructureDecay / 210.0f);

    gameState.railFortressTimer -= dt;
    if (gameState.railFortressTimer > 0.0f) {
        return;
    }

    AddInventoryItem(profile, "steel_scrap", std::max(4, static_cast<int>(std::round(6.0f * doctrineBoost * infrastructurePenalty))), 0.5f);
    AddInventoryItem(profile, "old_plate", std::max(1, static_cast<int>(std::round(2.0f * doctrineBoost * infrastructurePenalty))), 0.5f);
    AddInventoryItem(profile, "#%it_ptrs_ammo", 2, 0.7f);
    AddInventoryItem(profile, "trade_voucher", 1, 0.0f);
    worldState->railFortressDeployments += 1;
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 3.0f);
    const int purged = static_cast<int>(std::round(ReduceSelectedWorldEtherErosion(profile, 5.0f, true)));
    gameState.railFortressTimer = 520.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = purged > 0
        ? "Rail fortress patrol returned with heavy salvage and suppressed outer ether pressure."
        : "Rail fortress patrol returned with armored cargo and route security supplies.";
}

void UpdateRecoveryFabricator(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->recoveryFabricatorActive) {
        gameState.fabricatorTimer = 280.0f;
        return;
    }

    const bool fabricatorReady = RecoveryFabricatorReady(profile, *worldState);
    if (!fabricatorReady) {
        gameState.fabricatorTimer = 280.0f;
        return;
    }

    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.28f :
            (profile.doctrine == ShelterDoctrine::Medical ? 1.08f : 1.0f)) *
        PylonGridBoost(profile);
    const float infrastructurePenalty = std::max(0.66f, 1.0f - worldState->infrastructureDecay / 195.0f);

    gameState.fabricatorTimer -= dt;
    if (gameState.fabricatorTimer > 0.0f) {
        return;
    }

    AddInventoryItem(profile, "repair_patch", std::max(1, static_cast<int>(std::round(1.0f * doctrineBoost * infrastructurePenalty))), 0.2f);
    AddInventoryItem(profile, "power_cell", 1, 0.3f);
    if (profile.doctrine == ShelterDoctrine::Industry) {
        AddInventoryItem(profile, "old_plate", 1, 0.5f);
    } else {
        AddInventoryItem(profile, "cryo_medkit", 1, 0.5f);
    }
    worldState->fabricatorCyclesCompleted += 1;
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 1.5f);
    gameState.fabricatorTimer = 280.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = "Recovery fabricator completed a refinement cycle and issued fresh field supplies.";
}

void UpdateRecoveryMilestones(SessionProfile& profile, GameState& gameState) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr) {
        return;
    }

    const int currentTier = ShelterRecoveryMilestoneTier(ShelterRecoveryIndex(profile));
    if (currentTier <= worldState->recoveryMilestonesClaimed) {
        return;
    }

    while (worldState->recoveryMilestonesClaimed < currentTier) {
        const int nextTier = worldState->recoveryMilestonesClaimed + 1;
        std::string xpEvent;
        if (nextTier == 1) {
            AddInventoryItem(profile, "cryo_medkit", 1, 0.5f);
            AddInventoryItem(profile, "steel_scrap", 2, 0.5f);
            AwardExperience(profile, 35, &xpEvent);
            gameState.lastEvent = "Shelter Recovery Checkpoint I secured. Emergency stores unlocked for the crew. " + xpEvent;
        } else if (nextTier == 2) {
            AddInventoryItem(profile, "power_cell", 1, 0.3f);
            AddInventoryItem(profile, "repair_patch", 2, 0.2f);
            AwardExperience(profile, 55, &xpEvent);
            gameState.lastEvent = "Shelter Recovery Checkpoint II secured. Grid and field service reserves expanded. " + xpEvent;
        } else {
            AddInventoryItem(profile, "trade_voucher", 2, 0.0f);
            AddInventoryItem(profile, "old_plate", 2, 0.5f);
            AddInventoryItem(profile, "clean_water", 2, 0.4f);
            AddInventoryItem(profile, "#%it_ptrs_ammo", 2, 0.7f);
            AwardExperience(profile, 75, &xpEvent);
            gameState.lastEvent = "Shelter Recovery Checkpoint III secured. Shelter 17 now has a stable recovery backbone and sustained water reserves. " + xpEvent;
        }
        worldState->recoveryMilestonesClaimed = nextTier;
    }
}

void UpdateIndustrialSurvey(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->industrialSurveyActive) {
        gameState.surveyTimer = 360.0f;
        return;
    }

    const bool surveyReady = IndustrialSurveyReady(*worldState);
    if (!surveyReady) {
        gameState.surveyTimer = 360.0f;
        return;
    }

    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.16f :
            (profile.doctrine == ShelterDoctrine::Defense ? 1.08f : 1.0f)) *
        PylonGridBoost(profile) *
        CampFortificationBoost(profile);
    const float infrastructurePenalty = std::max(0.68f, 1.0f - worldState->infrastructureDecay / 210.0f);

    gameState.surveyTimer -= dt;
    if (gameState.surveyTimer > 0.0f) {
        return;
    }

    AddInventoryItem(profile, "trade_voucher", 1, 0.0f);
    AddInventoryItem(profile, "ether_shard", 1, 0.1f);
    AddInventoryItem(profile, "copper_wire", 1, 0.2f);
    worldState->surveyRunsCompleted += 1;
    worldState->routeContamination = std::max(0.0f, worldState->routeContamination - 2.5f);
    ReduceSelectedWorldEtherErosion(profile, 4.0f, false);
    gameState.surveyTimer = 360.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = "Industrial survey sweep returned with route intel, trace resources, and inner spur markers.";
}

void UpdateIndustrialOutpost(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->industrialOutpostActive) {
        gameState.outpostTimer = 300.0f;
        return;
    }

    const bool outpostReady = IndustrialOutpostReady(*worldState);
    if (!outpostReady) {
        gameState.outpostTimer = 300.0f;
        return;
    }

    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Defense ? 1.18f :
            (profile.doctrine == ShelterDoctrine::Industry ? 1.1f : 1.0f)) *
        CampFortificationBoost(profile) *
        PylonGridBoost(profile);
    const float infrastructurePenalty = std::max(0.7f, 1.0f - worldState->infrastructureDecay / 220.0f);

    gameState.outpostTimer -= dt;
    if (gameState.outpostTimer > 0.0f) {
        return;
    }

    AddInventoryItem(profile, "repair_patch", 1, 0.2f);
    AddInventoryItem(profile, "#%it_ptrs_ammo", 1, 0.7f);
    AddInventoryItem(profile, "trade_voucher", 1, 0.0f);
    worldState->outpostSupplyRuns += 1;
    worldState->routeContamination = std::max(0.0f, worldState->routeContamination - 3.0f);
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 1.5f);
    gameState.outpostTimer = 300.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = "Inner spur outpost forwarded supplies and route hardening support back to Shelter 17.";
}

void UpdateAssemblyCell(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->assemblyCellActive) {
        gameState.assemblyTimer = 340.0f;
        return;
    }

    const bool assemblyReady = AssemblyCellReady(*worldState);
    if (!assemblyReady) {
        gameState.assemblyTimer = 340.0f;
        return;
    }

    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.22f :
            (profile.doctrine == ShelterDoctrine::Defense ? 1.06f : 1.0f)) *
        PylonGridBoost(profile) *
        CampFortificationBoost(profile);
    const float infrastructurePenalty = std::max(0.72f, 1.0f - worldState->infrastructureDecay / 230.0f);

    gameState.assemblyTimer -= dt;
    if (gameState.assemblyTimer > 0.0f) {
        return;
    }

    AddInventoryItem(profile, "repair_patch", 1, 0.2f);
    AddInventoryItem(profile, "old_plate", 1, 0.5f);
    if (profile.doctrine == ShelterDoctrine::Industry) {
        AddInventoryItem(profile, "power_cell", 1, 0.3f);
    } else {
        AddInventoryItem(profile, "#%it_ptrs_ammo", 1, 0.7f);
    }
    worldState->assemblyCyclesCompleted += 1;
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 1.8f);
    ReduceSelectedWorldEtherErosion(profile, 2.5f, false);
    gameState.assemblyTimer = 340.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = "Assembly cell completed a local industrial cycle and shipped finished parts back to the backbone.";
}

void UpdateFoundryLine(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->foundryLineActive) {
        gameState.foundryTimer = 420.0f;
        return;
    }

    const bool foundryReady = FoundryLineReady(*worldState);
    if (!foundryReady) {
        gameState.foundryTimer = 420.0f;
        return;
    }

    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.24f :
            (profile.doctrine == ShelterDoctrine::Defense ? 1.08f : 1.0f)) *
        PylonGridBoost(profile) *
        CampFortificationBoost(profile);
    const float infrastructurePenalty = std::max(0.72f, 1.0f - worldState->infrastructureDecay / 240.0f);

    gameState.foundryTimer -= dt;
    if (gameState.foundryTimer > 0.0f) {
        return;
    }

    AddInventoryItem(profile, "old_plate", 2, 0.5f);
    AddInventoryItem(profile, "repair_patch", 1, 0.2f);
    AddInventoryItem(profile, "steel_scrap", 2, 0.5f);
    if (profile.partnerTank.damage.hull < 98.0f) {
        profile.partnerTank.damage.hull = std::min(100.0f, profile.partnerTank.damage.hull + 2.5f);
    }
    worldState->foundryCyclesCompleted += 1;
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 2.0f);
    ReduceSelectedWorldEtherErosion(profile, 2.0f, false);
    gameState.foundryTimer = 420.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = "Foundry line completed a heavy fabrication cycle and issued fresh plates to the recovery backbone.";
}

void UpdateReactorYard(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->reactorYardActive) {
        gameState.reactorTimer = 460.0f;
        return;
    }

    const bool reactorReady = ReactorYardReady(*worldState);
    if (!reactorReady) {
        gameState.reactorTimer = 460.0f;
        return;
    }

    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.2f :
            (profile.doctrine == ShelterDoctrine::Medical ? 1.08f : 1.0f)) *
        PylonGridBoost(profile) *
        CampFortificationBoost(profile);
    const float infrastructurePenalty = std::max(0.74f, 1.0f - worldState->infrastructureDecay / 250.0f);

    gameState.reactorTimer -= dt;
    if (gameState.reactorTimer > 0.0f) {
        return;
    }

    AddInventoryItem(profile, "power_cell", 2, 0.3f);
    AddInventoryItem(profile, "ether_shard", 1, 0.1f);
    profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 3.0f);
    worldState->reactorCyclesCompleted += 1;
    worldState->etherErosion = std::max(0.0f, worldState->etherErosion - 2.0f);
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 1.2f);
    gameState.reactorTimer = 460.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = "Reactor yard completed a heavy energy cycle and pushed fresh cells back into the recovery backbone.";
}

void UpdateCapacitorBank(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->capacitorBankActive) {
        gameState.capacitorTimer = 390.0f;
        return;
    }

    const bool capacitorReady = CapacitorBankReady(*worldState);
    if (!capacitorReady) {
        gameState.capacitorTimer = 390.0f;
        return;
    }

    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.18f :
            (profile.doctrine == ShelterDoctrine::Defense ? 1.06f : 1.0f)) *
        PylonGridBoost(profile) *
        CampFortificationBoost(profile);
    const float infrastructurePenalty = std::max(0.76f, 1.0f - worldState->infrastructureDecay / 255.0f);

    gameState.capacitorTimer -= dt;
    if (gameState.capacitorTimer > 0.0f) {
        return;
    }

    profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 5.0f);
    gameState.tankThermalLoad = std::max(0.0f, gameState.tankThermalLoad - 3.5f);
    AddInventoryItem(profile, "power_cell", 1, 0.3f);
    worldState->capacitorDischargeCycles += 1;
    worldState->etherErosion = std::max(0.0f, worldState->etherErosion - 1.5f);
    gameState.capacitorTimer = 390.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = "Capacitor bank discharged a buffered surge into the backbone and stabilized BT-72 reserves.";
}

void UpdateRelaySubstation(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->relaySubstationActive) {
        gameState.relaySubstationTimer = 430.0f;
        return;
    }

    const bool substationReady = RelaySubstationReady(*worldState);
    if (!substationReady) {
        gameState.relaySubstationTimer = 430.0f;
        return;
    }

    const float doctrineBoost =
        (profile.doctrine == ShelterDoctrine::Industry ? 1.16f :
            (profile.doctrine == ShelterDoctrine::Defense ? 1.08f : 1.0f)) *
        PylonGridBoost(profile) *
        CampFortificationBoost(profile);
    const float infrastructurePenalty = std::max(0.74f, 1.0f - worldState->infrastructureDecay / 240.0f);

    gameState.relaySubstationTimer -= dt;
    if (gameState.relaySubstationTimer > 0.0f) {
        return;
    }

    AddInventoryItem(profile, "power_cell", 1, 0.3f);
    AddInventoryItem(profile, "trade_voucher", 1, 0.0f);
    profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 4.0f);
    gameState.tankThermalLoad = std::max(0.0f, gameState.tankThermalLoad - 2.0f);
    worldState->relaySyncCycles += 1;
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 1.8f);
    worldState->etherErosion = std::max(0.0f, worldState->etherErosion - 1.2f);
    worldState->routeContamination = std::max(0.0f, worldState->routeContamination - 1.0f);
    gameState.relaySubstationTimer = 430.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = "Relay substation pushed a synchronized load back into Shelter 17 and stabilized the backbone.";
}

void UpdateServiceBay(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->serviceBayActive) {
        gameState.serviceBayTimer = 300.0f;
        return;
    }

    const bool serviceReady = ServiceBayReady(*worldState);
    if (!serviceReady) {
        gameState.serviceBayTimer = 300.0f;
        return;
    }

    const float doctrineBoost =
        DoctrineWorkshopBoost(profile) *
        PylonGridBoost(profile) *
        CampFortificationBoost(profile);
    const float infrastructurePenalty = std::max(0.75f, 1.0f - worldState->infrastructureDecay / 235.0f);

    gameState.serviceBayTimer -= dt;
    if (gameState.serviceBayTimer > 0.0f) {
        return;
    }

    profile.partnerTank.damage.hull = std::min(100.0f, profile.partnerTank.damage.hull + 5.0f);
    profile.partnerTank.damage.turret = std::min(100.0f, profile.partnerTank.damage.turret + 3.5f);
    profile.partnerTank.damage.bucket = std::min(100.0f, profile.partnerTank.damage.bucket + 4.5f);
    profile.partnerTank.damage.sensors = std::min(100.0f, profile.partnerTank.damage.sensors + 4.0f);
    profile.partnerTank.damage.powerCore = std::min(100.0f, profile.partnerTank.damage.powerCore + 3.0f);
    profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 2.0f);
    gameState.tankThermalLoad = std::max(0.0f, gameState.tankThermalLoad - 2.5f);
    worldState->serviceCyclesCompleted += 1;
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 1.0f);
    gameState.serviceBayTimer = 300.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = "Inner spur service bay completed a repair cycle for BT-72 and the backbone fleet.";
}

void UpdateWaterReclaimer(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr || !worldState->waterReclaimerActive) {
        gameState.waterReclaimerTimer = 260.0f;
        return;
    }

    const bool waterReady = WaterReclaimerReady(*worldState);
    if (!waterReady) {
        gameState.waterReclaimerTimer = 260.0f;
        return;
    }

    const float doctrineBoost =
        DoctrineCampRecoveryBoost(profile) *
        WaterRecoveryBoost(profile) *
        PylonGridBoost(profile);
    const float infrastructurePenalty = std::max(0.76f, 1.0f - worldState->infrastructureDecay / 220.0f);

    gameState.waterReclaimerTimer -= dt;
    if (gameState.waterReclaimerTimer > 0.0f) {
        return;
    }

    AddInventoryItem(profile, "clean_water", std::max(1, static_cast<int>(std::round(2.0f * doctrineBoost * infrastructurePenalty))), 0.4f);
    if (profile.doctrine == ShelterDoctrine::Medical) {
        AddInventoryItem(profile, "cryo_medkit", 1, 0.5f);
    }
    worldState->waterCyclesCompleted += 1;
    worldState->infrastructureDecay = std::max(0.0f, worldState->infrastructureDecay - 0.9f);
    worldState->etherErosion = std::max(0.0f, worldState->etherErosion - 0.7f);
    gameState.waterReclaimerTimer = 260.0f / (doctrineBoost * infrastructurePenalty);
    gameState.lastEvent = "Water reclaimer completed a purification cycle and stabilized frontier recovery reserves.";
}

void UpdateHostiles(World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState,
    float dt) {
    if (gameState.damageCooldown > 0.0f) {
        gameState.damageCooldown -= dt;
    }

    for (auto& object : world.objects) {
        if (object.interaction != InteractionType::Hostile) {
            continue;
        }

        const float dx = player.x - object.x;
        const float dy = player.y - object.y;
        const float distanceSq = (dx * dx) + (dy * dy);
        const float distance = std::sqrt(distanceSq);

        if (distance > 0.2f && distance < 8.0f) {
            const float step = dt * (object.registryId == "[%enemy_ghoul_0001]" ? 1.4f : 2.0f);
            object.x += (dx / distance) * step;
            object.y += (dy / distance) * step;
        }

        if (distance < 1.4f && gameState.damageCooldown <= 0.0f && profile.character.hp > 0.0f) {
            if (player.insideTank) {
                float tankDamage = object.registryId == "[%enemy_ghoul_0001]" ? 8.0f : 5.0f;
                if (TankHasBulwarkSync(profile)) {
                    tankDamage *= 0.82f;
                }
                if (TankUsesRamShield(profile)) {
                    tankDamage *= 0.85f;
                }
                profile.partnerTank.damage.hull = std::max(0.0f, profile.partnerTank.damage.hull - tankDamage);
                profile.partnerTank.damage.sensors = std::max(0.0f, profile.partnerTank.damage.sensors - (tankDamage * 0.35f));
                profile.partnerTank.damage.cockpit = std::max(0.0f, profile.partnerTank.damage.cockpit - (tankDamage * 0.2f));
                gameState.lastEvent = TankHasBulwarkSync(profile) || TankUsesRamShield(profile)
                    ? object.displayName + " scraped the tank hull, but Bulwark Sync absorbed part of the impact."
                    : object.displayName + " scraped the tank hull.";
            } else {
                const float incomingDamage = std::max(4.0f, 11.0f - static_cast<float>(EffectiveStatValue(profile, gameState, 'E')) * 0.6f);
                profile.character.hp = std::max(0.0f, profile.character.hp - incomingDamage);
                gameState.lastEvent = object.displayName + " hit the operator.";
            }
            gameState.damageCooldown = 1.2f;
            gameState.damageFlashTimer = 0.45f;
        }
    }

    world.objects.erase(
        std::remove_if(world.objects.begin(), world.objects.end(), [&](const MapObject& object) {
            if (object.interaction == InteractionType::Hostile && object.health <= 0.0f) {
                staticEraser.Erase(object.registryId);
                return true;
            }
            return false;
        }),
        world.objects.end());
}

void HandleAttack(World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState) {
    const MapObject* hostile = FindNearestHostile(world, player.x, player.y, player.insideTank ? 2.6f : 1.8f);
    if (hostile == nullptr) {
        gameState.lastEvent = player.insideTank ? "No hostile target in ram range." : "No hostile target in melee range.";
        return;
    }

    if (auto* mutableObject = const_cast<MapObject*>(hostile); mutableObject != nullptr) {
        const float damage = player.insideTank
            ? ((TankUsesRamShield(profile) ? 36.0f : 28.0f) + static_cast<float>(EffectiveStatValue(profile, gameState, 'P')) * 1.5f)
            : (14.0f + static_cast<float>(EffectiveStatValue(profile, gameState, 'S')) * 0.9f);
        mutableObject->health -= damage;
        if (player.insideTank) {
            RegisterTankSyncStyle(profile, true);
        }
        if (mutableObject->health <= 0.0f) {
            std::string progressionEvent;
            AwardExperience(profile, player.insideTank ? 45 : 30, &progressionEvent);
            std::string skillEvent;
            if (player.insideTank) RegisterTankAction(profile, &skillEvent);
            else RegisterFootKill(profile, &skillEvent);
            AddInventoryItem(profile, player.insideTank ? "wreck_scrap" : "ether_tissue", 1, 0.4f);
            staticEraser.Erase(mutableObject->registryId);
            staticEraser.Save(profile.selectedWorld);
            gameState.lastEvent = mutableObject->displayName + " neutralized. " + progressionEvent;
            if (!skillEvent.empty()) {
                gameState.lastEvent += " " + skillEvent;
            }
        } else {
            gameState.lastEvent = player.insideTank ? "Ram impact landed on hostile target." : "Strike landed on hostile target.";
        }
    }
}

void HandleSpecialAttack(World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState) {
    const float radius = player.insideTank ? 9.0f : 6.5f;
    const MapObject* hostile = FindNearestHostile(world, player.x, player.y, radius);
    if (hostile == nullptr) {
        gameState.lastEvent = player.insideTank ? "No hostile target in cannon range." : "No hostile target in firing range.";
        return;
    }

    if (player.insideTank) {
        if (profile.partnerTank.ammoReserve < 8.0f || profile.partnerTank.energyReserve < 6.0f) {
            gameState.lastEvent = "BT-72 lacks ammo or energy for a cannon strike.";
            return;
        }
        profile.partnerTank.ammoReserve = std::max(0.0f, profile.partnerTank.ammoReserve - 8.0f);
        const float energyCost = TankHasStabilizerSync(profile) ? 5.0f : 6.0f;
        profile.partnerTank.energyReserve = std::max(0.0f, profile.partnerTank.energyReserve - energyCost);
        float recoilPush = TankHasStabilizerSync(profile) ? 0.32f : 0.55f;
        float recoilOffset = TankHasStabilizerSync(profile) ? 0.18f : 0.32f;
        if (HasEquippedPassiveSkill(profile, "skill_muscle_memory")) {
            const float stabilityFactor = std::max(0.72f, 1.0f - static_cast<float>(EffectiveStatValue(profile, gameState, 'S')) * 0.018f);
            recoilPush *= stabilityFactor;
            recoilOffset *= stabilityFactor;
        }
        player.velocityX -= std::cos(player.facingRadians) * recoilPush;
        player.velocityY -= std::sin(player.facingRadians) * recoilPush;
        player.recoilOffset = std::min(0.75f, player.recoilOffset + recoilOffset);
        gameState.tankThermalLoad = std::min(100.0f, gameState.tankThermalLoad + (TankHasStabilizerSync(profile) ? 10.0f : 14.0f));
    } else {
        if (!ConsumeInventoryItem(profile, "#%it_ptrs_ammo", 1)) {
            gameState.lastEvent = "No PTRS ammo available for a ranged shot.";
            return;
        }
        if (profile.character.mp < 8.0f) {
            AddInventoryItem(profile, "#%it_ptrs_ammo", 1, 0.7f);
            gameState.lastEvent = "Not enough MP to stabilize a precision shot.";
            return;
        }
        profile.character.mp = std::max(0.0f, profile.character.mp - 8.0f);
    }

    if (auto* mutableObject = const_cast<MapObject*>(hostile); mutableObject != nullptr) {
        const float damage = player.insideTank
            ? ((TankHasStabilizerSync(profile) ? 50.0f : 45.0f) + static_cast<float>(EffectiveStatValue(profile, gameState, 'P')) * 2.0f)
            : (24.0f + static_cast<float>(EffectiveStatValue(profile, gameState, 'P')) * 1.1f);
        mutableObject->health -= damage;
        if (player.insideTank) {
            RegisterTankSyncStyle(profile, false);
        }
        if (mutableObject->health <= 0.0f) {
            std::string progressionEvent;
            AwardExperience(profile, player.insideTank ? 60 : 40, &progressionEvent);
            std::string skillEvent;
            if (player.insideTank) RegisterTankAction(profile, &skillEvent);
            else RegisterFootKill(profile, &skillEvent);
            AddInventoryItem(profile, player.insideTank ? "wreck_scrap" : "ether_tissue", 1, 0.4f);
            staticEraser.Erase(mutableObject->registryId);
            staticEraser.Save(profile.selectedWorld);
            gameState.lastEvent = mutableObject->displayName + " destroyed by special attack. " + progressionEvent;
            if (!skillEvent.empty()) {
                gameState.lastEvent += " " + skillEvent;
            }
        } else {
            gameState.lastEvent = player.insideTank ? "Cannon strike landed." : "Precision shot landed.";
        }
    }
}

void HandleInteraction(const MapObject* nearest,
    World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState) {
    if (nearest == nullptr) {
        gameState.lastEvent = "No actionable target nearby.";
        return;
    }

    if (HandleScriptTagInteraction(nearest, world, player, profile, gameState) &&
        nearest->scriptTag != "workshop_service") {
        if (nearest->scriptTag == "specialist_cryo") {
            staticEraser.Erase(nearest->registryId);
            staticEraser.Save(profile.selectedWorld);
            world.RemoveObject(nearest->registryId);
            SaveSessionProfile(profile, DefaultSessionProfilePath());
        }
        return;
    }

    if (nearest->registryId == "[%cryo_0001]") {
        profile.story.awakenedFromCryo = true;
        gameState.lastEvent = "Cryostasis terminated. Memory loss remains, but movement is stable.";
        return;
    }

    if (nearest->registryId == "[%pip_0001]") {
        if (profile.story.pipPadRecovered) {
            gameState.lastEvent = "Pip-Pad locker already cleared.";
            return;
        }

        AddInventoryItem(profile, "#%it_pippad", 1, 0.8f);
        AddInventoryItem(profile, "cryo_medkit", 1, 0.5f);
        profile.story.pipPadRecovered = true;
        staticEraser.Erase(nearest->registryId);
        staticEraser.Save(profile.selectedWorld);
        world.RemoveObject(nearest->registryId);
        gameState.lastEvent = "Pip-Pad recovered. Local UI shell restored.";
        return;
    }

    if (nearest->registryId == "[%archive_0001]") {
        if (profile.story.archiveRecovered) {
            gameState.lastEvent = "Archive already mirrored to Pip-Pad. Personnel records remain available in DATA.";
            return;
        }
        profile.story.archiveRecovered = true;
        std::string skillEvent;
        RegisterArchiveSync(profile, &skillEvent);
        if (std::none_of(profile.character.collectedTapes.begin(),
                profile.character.collectedTapes.end(),
                [&](const TapeEntry& tape) { return tape.tapeId == "archive_missing_personnel"; })) {
            profile.character.collectedTapes.push_back({"archive_missing_personnel", "Missing Personnel Log", false, false, false});
        }
        if (std::none_of(profile.character.collectedTapes.begin(),
                profile.character.collectedTapes.end(),
                [&](const TapeEntry& tape) { return tape.tapeId == "damaged_blackbox_001"; })) {
            profile.character.collectedTapes.push_back({"damaged_blackbox_001", "Damaged Black Box Fragment", false, true, false});
        }
        gameState.lastEvent = "Archive sync complete. One reactor core and one body are still missing.";
        if (!skillEvent.empty()) {
            gameState.lastEvent += " " + skillEvent;
        }
        return;
    }

    if (nearest->registryId == "[#tr_hull_0001]") {
        if (!profile.story.pipPadRecovered) {
            gameState.lastEvent = "The tank refuses pairing. Recover the Pip-Pad first.";
            return;
        }
        if (!player.insideTank && profile.partnerTank.damage.hull <= 0.0f) {
            gameState.lastEvent = "BT-72 hull is disabled. Run workshop service before attempting another link.";
            return;
        }

        player.insideTank = !player.insideTank;
        profile.partnerTank.deployed = player.insideTank;
        profile.story.tankLinked = profile.story.tankLinked || player.insideTank;
        player.viewMode = player.insideTank ? ViewMode::Cockpit : ViewMode::ThirdPerson;
        player.velocityX = 0.0f;
        player.velocityY = 0.0f;
        player.recoilOffset = 0.0f;
        if (player.insideTank) {
            if (profile.partnerTank.worldPositionKnown) {
                player.x = profile.partnerTank.worldX;
                player.y = profile.partnerTank.worldY;
            } else {
                profile.partnerTank.worldX = nearest->x;
                profile.partnerTank.worldY = nearest->y;
                profile.partnerTank.worldPositionKnown = true;
            }
        } else {
            profile.partnerTank.worldX = player.x;
            profile.partnerTank.worldY = player.y;
            profile.partnerTank.worldPositionKnown = true;
        }
        gameState.lastEvent = player.insideTank
            ? "BT-72 link established. Cockpit channel synchronized."
            : "BT-72 link suspended. Returning to foot movement.";
        return;
    }

    if (nearest->registryId == "#%it_bucket_0001") {
        if (!profile.story.tankLinked) {
            gameState.lastEvent = "Recovering the bucket now is pointless. Link with BT-72 first.";
            return;
        }

        for (const auto& lootId : nearest->manualLootIds) {
            AddInventoryItem(profile, lootId, 1, 0.4f);
        }
        player.bucketRaised = true;
        profile.story.bucketRecovered = true;
        staticEraser.Erase(nearest->registryId);
        staticEraser.Save(profile.selectedWorld);
        world.RemoveObject(nearest->registryId);
        gameState.lastEvent = "Bucket plow recovered and mounted to the tank frame.";
        return;
    }

    if (nearest->registryId == "[%bulkhead_0001]") {
        if (!profile.story.bucketRecovered) {
            gameState.lastEvent = "The outer bulkhead opens, but the route beyond is still blocked by debris.";
        } else {
            profile.story.exitedBunker = true;
            gameState.lastEvent = "Outer bulkhead cycled. Recovery corridor is now accessible.";
        }
        return;
    }

    if (nearest->registryId == "#%res_scrap_0001") {
        if (!player.insideTank) {
            gameState.lastEvent = "Debris too dense for manual clearing. Link with BT-72 first.";
            return;
        }
        if (!TankUsesBucketRig(profile)) {
            gameState.lastEvent = "Ram Shield mounted. Swap back to Bucket Rig before clearing debris.";
            return;
        }
        if (!player.bucketRaised || !profile.story.bucketRecovered) {
            gameState.lastEvent = "Raise the bucket rig before pushing the debris barrier.";
            return;
        }

        if (auto* mutableObject = const_cast<MapObject*>(nearest); mutableObject != nullptr) {
            mutableObject->health -= 25.0f;
            RegisterTankAction(profile, nullptr);
            const float baseEnergyCost = HasEquippedPassiveSkill(profile, "skill_pilot_sync") ? 2.0f : 3.0f;
            const float tankEnergyCost = std::max(1.0f, baseEnergyCost - static_cast<float>(EffectiveStatValue(profile, gameState, 'I')) * 0.08f);
            profile.partnerTank.energyReserve = std::max(0.0f, profile.partnerTank.energyReserve - tankEnergyCost);
            profile.partnerTank.damage.bucket = std::max(0.0f, profile.partnerTank.damage.bucket - 1.5f);
            if (mutableObject->health <= 0.0f) {
                for (const auto& lootId : mutableObject->manualLootIds) {
                    AddInventoryItem(profile, lootId, 1, 0.5f);
                }
                staticEraser.Erase(mutableObject->registryId);
                staticEraser.Save(profile.selectedWorld);
                world.RemoveObject(mutableObject->registryId);
                profile.story.outerRoadCleared = true;
                std::string progressionEvent;
                AwardExperience(profile, 50, &progressionEvent);
                gameState.lastEvent = "Outer debris barrier cleared. The bunker route is now stable. " + progressionEvent;
            } else {
                gameState.lastEvent = "Debris impact registered. Keep pushing the barrier.";
            }
        }
        return;
    }

    if (nearest->registryId == "#%term_0001") {
        if (!profile.story.outerRoadCleared || !staticEraser.IsErased("[%enemy_ghoul_0001]")) {
            gameState.lastEvent = "Relay sync is unsafe. Secure the outer route first.";
            return;
        }
        if (!profile.story.relayRecovered) {
            profile.story.relayRecovered = true;
            std::string skillEvent;
            RegisterArchiveSync(profile, &skillEvent);
            AddInventoryItem(profile, "relay_reconstruction_data", 1, 0.2f);
            if (std::none_of(profile.character.collectedTapes.begin(),
                    profile.character.collectedTapes.end(),
                    [&](const TapeEntry& tape) { return tape.tapeId == "relay_reconstruction_data"; })) {
                profile.character.collectedTapes.push_back({"relay_reconstruction_data", "Relay Reconstruction Packet", false});
            }
            std::string progressionEvent;
            AwardExperience(profile, HasEquippedPassiveSkill(profile, "skill_data_miner") ? 50 : 40, &progressionEvent);
            gameState.lastEvent = "Relay schematics recovered. Return to Shelter 17 for debrief. " + progressionEvent;
            if (!skillEvent.empty()) {
                gameState.lastEvent += " " + skillEvent;
            }
        } else {
            gameState.lastEvent = "Relay packet already copied to the Pip-Pad.";
        }
        return;
    }

    if (nearest->registryId == "[%debrief_0001]") {
        if (!profile.story.relayRecovered) {
            gameState.lastEvent = "Debrief incomplete. Recover the relay packet first.";
            return;
        }
        if (!profile.story.returnedToBase) {
            profile.story.returnedToBase = true;
            std::string skillEvent;
            RegisterArchiveSync(profile, &skillEvent);
            if (std::none_of(profile.character.collectedTapes.begin(),
                    profile.character.collectedTapes.end(),
                    [&](const TapeEntry& tape) { return tape.tapeId == "debrief_shelter17"; })) {
                profile.character.collectedTapes.push_back({"debrief_shelter17", "Shelter 17 Debrief", false});
            }
            std::string progressionEvent;
            AwardExperience(profile, HasEquippedPassiveSkill(profile, "skill_data_miner") ? 75 : 60, &progressionEvent);
            gameState.lastEvent = "Debrief uploaded. Starter route complete and industrial recovery planning unlocked for Shelter 17. " + progressionEvent;
            if (!skillEvent.empty()) {
                gameState.lastEvent += " " + skillEvent;
            }
        } else {
            gameState.lastEvent = "Debrief log already archived. Shelter 17 recovery planning remains on file.";
        }
        return;
    }

    if (nearest->registryId == "[%camp_0001]") {
        ActivateFieldCheckpoint(*nearest, profile, profile.selectedWorld);
        const bool gridOnline = IsRegionalGridOnline(profile);
        const int erosionCleared = static_cast<int>(std::round(ReduceSelectedWorldEtherErosion(profile, gridOnline ? 10.0f : 3.0f, gridOnline)));
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        const float doctrineCampBoost = DoctrineCampRecoveryBoost(profile) * WaterRecoveryBoost(profile) * PylonGridBoost(profile);
        const float infrastructurePenalty = std::max(0.72f, 1.0f - worldState.infrastructureDecay / 160.0f);
        profile.character.hp = std::min(profile.character.maxHp, profile.character.hp + (gridOnline ? 22.0f : 18.0f) * doctrineCampBoost * infrastructurePenalty);
        profile.character.mp = std::min(profile.character.maxMp, profile.character.mp + (gridOnline ? 42.0f : 30.0f) * doctrineCampBoost * infrastructurePenalty);
        if (player.insideTank && IsTankNearServicePoint(profile, *nearest, 3.8f) && gridOnline) {
            profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 18.0f);
            profile.partnerTank.ammoReserve = std::min(100.0f, profile.partnerTank.ammoReserve + 6.0f);
        }
        staticEraser.Save(profile.selectedWorld);
        SaveSessionProfile(profile, DefaultSessionProfilePath());
        const char* recoveryStatus = RecoveryStatusLabel(profile, &worldState);
        gameState.lastEvent = gridOnline
            ? "Forward camp anchored. Regional grid online: checkpoint, recovery, and recharge cycle complete. Shelter 17 status: " + std::string(recoveryStatus) + "."
            : "Forward camp anchored. Checkpoint and recovery cycle complete, but the regional grid is still unstable. Shelter 17 status: " + std::string(recoveryStatus) + ".";
        if (erosionCleared > 0) {
            gameState.lastEvent += " Ether bloom reduced by " + std::to_string(erosionCleared) + "%.";
        }
        return;
    }

    switch (nearest->interaction) {
        case InteractionType::Hostile:
            gameState.lastEvent = "This target is hostile. Attack instead of interacting.";
            break;
        case InteractionType::Terminal:
            AddCollectedTapeIfMissing(profile, nearest->registryId, nearest->displayName);
            gameState.lastEvent = DescribeTerminalSync(*nearest);
            break;
        case InteractionType::Transition:
            gameState.lastEvent = nearest->linkTarget.empty()
                ? "Transition marker logged."
                : "Transition marker logged: route -> " + nearest->linkTarget;
            break;
        case InteractionType::Workshop: {
            if (!profile.story.tankLinked) {
                gameState.lastEvent = "Workshop recognizes no active vehicle link.";
                break;
            }
            if (!IsRegionalGridOnline(profile)) {
                gameState.lastEvent = "Workshop service grid offline. Sync the relay network first.";
                break;
            }
            if (!profile.partnerTank.worldPositionKnown) {
                gameState.lastEvent = "Workshop cannot locate BT-72 in the local service grid.";
                break;
            }
            if (!IsTankNearServicePoint(profile, *nearest, 3.4f)) {
                gameState.lastEvent = "Bring BT-72 into the workshop service bay first.";
                break;
            }
            if (player.insideTank && profile.partnerTank.energyReserve < 100.0f) {
                profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 14.0f);
                gameState.lastEvent = "Workshop power coupler connected. BT-72 batteries boosted.";
                break;
            }
            if (!TankNeedsRepair(profile)) {
                gameState.lastEvent = "Workshop reports BT-72 at ready status.";
                break;
            }
            if (!ConsumeAnyRepairMaterial(profile)) {
                gameState.lastEvent = "Workshop needs repair materials: scrap, seals, plates or wire.";
                break;
            }
            profile.partnerTank.inRepair = true;
            const float intelligence = static_cast<float>(EffectiveStatValue(profile, gameState, 'I'));
            const bool hasEngineer = HasAssignedSpecialistRole(profile, "engineer", "workshop");
            const float engineerBoost = hasEngineer ? 1.18f : 1.0f;
            const float doctrineBoost = DoctrineWorkshopBoost(profile) * PylonGridBoost(profile);
            const auto* worldState = FindWorldFieldState(profile, profile.selectedWorld);
            const float infrastructurePenalty = worldState != nullptr
                ? std::max(0.68f, 1.0f - worldState->infrastructureDecay / 150.0f)
                : 1.0f;
            const float repairBoost = engineerBoost * doctrineBoost * infrastructurePenalty;
            profile.partnerTank.damage.hull = std::min(100.0f, profile.partnerTank.damage.hull + (18.0f + intelligence * 0.9f) * repairBoost);
            profile.partnerTank.damage.bucket = std::min(100.0f, profile.partnerTank.damage.bucket + (20.0f + intelligence * 0.7f) * repairBoost);
            profile.partnerTank.damage.sensors = std::min(100.0f, profile.partnerTank.damage.sensors + (16.0f + intelligence * 0.8f) * repairBoost);
            profile.partnerTank.damage.turret = std::min(100.0f, profile.partnerTank.damage.turret + (12.0f + intelligence * 0.6f) * repairBoost);
            profile.partnerTank.damage.cockpit = std::min(100.0f, profile.partnerTank.damage.cockpit + (10.0f + intelligence * 0.5f) * repairBoost);
            profile.partnerTank.damage.powerCore = std::min(100.0f, profile.partnerTank.damage.powerCore + (14.0f + intelligence * 0.6f) * repairBoost);
            profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 22.0f);
            profile.partnerTank.ammoReserve = std::min(100.0f, profile.partnerTank.ammoReserve + 12.0f);
            gameState.workshopServiceCooldown = 120.0f;
            {
                std::string progressionEvent;
                AwardExperience(profile, 25, &progressionEvent);
                gameState.lastEvent = hasEngineer
                    ? "Workshop cycle complete. Resident engineer boosted BT-72 repair output. " + progressionEvent
                    : "Workshop cycle complete. BT-72 repaired and resupplied. " + progressionEvent;
                if (profile.doctrine == ShelterDoctrine::Industry) {
                    gameState.lastEvent += " Industry doctrine amplified service throughput.";
                } else if (profile.doctrine == ShelterDoctrine::Medical) {
                    gameState.lastEvent += " Medical doctrine kept service safe but conservative.";
                } else if (profile.doctrine == ShelterDoctrine::Defense) {
                    gameState.lastEvent += " Defense doctrine prioritized combat readiness.";
                }
            }
            break;
        }
        case InteractionType::Container:
            for (const auto& lootId : nearest->manualLootIds) {
                if (lootId.empty()) {
                    continue;
                }
                AddInventoryItem(profile, lootId, 1, 0.4f);
            }
            staticEraser.Erase(nearest->registryId);
            staticEraser.Save(profile.selectedWorld);
            world.RemoveObject(nearest->registryId);
            gameState.lastEvent = "Container secured and contents transferred.";
            break;
        case InteractionType::Resource:
        case InteractionType::VehicleAnchor:
        case InteractionType::Static:
            if (!nearest->scriptTag.empty()) {
                gameState.lastEvent = nearest->displayName + ": " + nearest->scriptTag;
            }
            break;
    }
}

bool WantsUseKey(const MapObject* nearest) {
    return nearest != nullptr && !IsContextualAction(*nearest);
}

bool WantsContextKey(const MapObject* nearest) {
    return nearest != nullptr && IsContextualAction(*nearest);
}

void DrawPipPad(const World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState) {
    static int activeTab = 0;

    if (!player.uiVisible) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(740.0f, 560.0f), ImGuiCond_FirstUseEver);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.03f, 0.06f, 0.04f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.7f, 0.3f, 0.7f));
    ImGui::Begin("PIP-PAD // Recovery Shell", nullptr, ImGuiWindowFlags_NoCollapse);

    const char* tabs[] = {"STAT", "INV", "DATA", "MAP", "QUEST", "NET"};
    for (int index = 0; index < IM_ARRAYSIZE(tabs); ++index) {
        if (index > 0) {
            ImGui::SameLine();
        }
        if (ImGui::Button(tabs[index], ImVec2(132.0f, 34.0f))) {
            activeTab = index;
        }
    }

    ImGui::Separator();
    const int statS = EffectiveStatValue(profile, gameState, 'S');
    const int statP = EffectiveStatValue(profile, gameState, 'P');
    const int statE = EffectiveStatValue(profile, gameState, 'E');
    const int statC = EffectiveStatValue(profile, gameState, 'C');
    const int statI = EffectiveStatValue(profile, gameState, 'I');
    const int statA = EffectiveStatValue(profile, gameState, 'A');
    const int statL = EffectiveStatValue(profile, gameState, 'L');

    if (activeTab == 0) {
        ImGui::Text("Operator: %s", profile.character.displayName.c_str());
        ImGui::Text("Account: %s", profile.account.username.c_str());
        ImGui::Text("Level: %d", profile.character.level);
        ImGui::Text("Experience: %d", profile.character.experience);
        ImGui::Text("HP %.0f / %.0f", profile.character.hp, profile.character.maxHp);
        ImGui::Text("MP %.0f / %.0f", profile.character.mp, profile.character.maxMp);
        ImGui::Text("Status: %s", profile.character.hp > 0.0f ? (profile.character.mp <= 10.0f ? "Fatigued" : "Operational") : "Downed");
        ImGui::Text("Weather: %s", ToString(gameState.weather));
        ImGui::Text("Ether Pressure: %.0f%% (%s)", CurrentEtherErosion(profile), EtherErosionBand(CurrentEtherErosion(profile)));
        ImGui::Text("Current Load: %.1f kg", CurrentInventoryWeight(profile));
        ImGui::Text("Carry Weight Budget: %.0f kg", profile.character.carryWeight);
        ImGui::Text("Field Medkits: %s", HasInventoryItem(profile, "cryo_medkit") ? "Available" : "None");
        if (gameState.rationEffectTimer > 0.0f) {
            ImGui::Text("Toxic Ration: %.0fs", gameState.rationEffectTimer);
        }
        ImGui::Separator();
        ImGui::Text("SPECIAL");
        ImGui::BulletText("S %d", statS);
        ImGui::BulletText("P %d", statP);
        ImGui::BulletText("E %d", statE);
        ImGui::BulletText("C %d", statC);
        ImGui::BulletText("I %d", statI);
        ImGui::BulletText("A %d", statA);
        ImGui::BulletText("L %d", statL);
    } else if (activeTab == 1) {
        ImGui::Text("Inventory Manifest");
        ImGui::Separator();
        ImGui::BeginChild("InventoryList", ImVec2(0.0f, 320.0f), true);
        for (const auto& item : profile.character.inventory) {
            ImGui::BulletText("%s x%d", item.itemId.c_str(), item.count);
        }
        ImGui::EndChild();
        if (ImGui::Button("Consume Field Ration")) {
            if (!TryConsumeFieldRation(profile, gameState)) {
                gameState.lastEvent = "No field ration available.";
            }
        }
        if (HasInventoryItem(profile, "recipe_repair_patch")) {
            if (ImGui::Button("Craft Repair Patch")) {
                if (HasInventoryItem(profile, "steel_scrap") && HasInventoryItem(profile, "copper_wire")) {
                    ConsumeInventoryItem(profile, "steel_scrap", 1);
                    ConsumeInventoryItem(profile, "copper_wire", 1);
                    AddInventoryItem(profile, "repair_patch", 1, 0.2f);
                    gameState.lastEvent = "Repair Patch fabricated from awakened field recipe.";
                } else {
                    gameState.lastEvent = "Crafting requires steel_scrap and copper_wire.";
                }
            }
            ImGui::TextDisabled("Recipe unlocked: steel_scrap + copper_wire -> repair_patch");
        } else {
            const int fieldServiceUses = profile.character.awakening.fieldServiceUses;
            const int cyclesRemaining = std::max(0, 3 - fieldServiceUses);
            ImGui::TextDisabled("Awakened repair recipe dormant: %d/3 field service cycles logged", std::min(fieldServiceUses, 3));
            if (cyclesRemaining > 0) {
                ImGui::TextDisabled("Field service needs %d more cycle%s to surface the recipe.", cyclesRemaining, cyclesRemaining == 1 ? "" : "s");
            }
        }
        auto* worldFieldState = FindWorldFieldState(profile, profile.selectedWorld, true);
        if (worldFieldState != nullptr && worldFieldState->tradeNetworkActive) {
            ImGui::Separator();
            ImGui::Text("Trade Network");
            ImGui::TextDisabled("Trade vouchers: %s", HasInventoryItem(profile, "trade_voucher") ? "available" : "none");
            if (ImGui::Button("Redeem Medkit")) {
                if (ConsumeInventoryItem(profile, "trade_voucher", 1)) {
                    AddInventoryItem(profile, "cryo_medkit", 1, 0.5f);
                    gameState.lastEvent = "Trade voucher exchanged for one cryo_medkit.";
                } else {
                    gameState.lastEvent = "No trade voucher available.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Redeem Power Cell")) {
                if (ConsumeInventoryItem(profile, "trade_voucher", 1)) {
                    AddInventoryItem(profile, "power_cell", 1, 0.3f);
                    gameState.lastEvent = "Trade voucher exchanged for one power_cell.";
                } else {
                    gameState.lastEvent = "No trade voucher available.";
                }
            }
            if (ImGui::Button("Redeem Ammo Bundle")) {
                if (ConsumeInventoryItem(profile, "trade_voucher", 1)) {
                    AddInventoryItem(profile, "#%it_ptrs_ammo", 2, 0.7f);
                    gameState.lastEvent = "Trade voucher exchanged for an ammo bundle.";
                } else {
                    gameState.lastEvent = "No trade voucher available.";
                }
            }
        }
        ImGui::TextWrapped("Field action: press H to use one cryo medkit if available.");
    } else if (activeTab == 2) {
        ImGui::Text("Vehicle Telemetry");
        ImGui::Text("Partner Tank: %s", profile.partnerTank.callSign.c_str());
        ImGui::Text("Class: %s", ToString(profile.partnerTank.tankClass));
        ImGui::Text("Sync Mode: %s", CurrentTankSyncMode(profile.partnerTank).c_str());
        ImGui::Text("Trust Link: %.0f%%", profile.partnerTank.trustLink * 100.0f);
        ImGui::Text("Utility Module: %s", CurrentUtilityModuleLabel(profile));
        if (TankUsesTowCoupler(profile)) {
            ImGui::TextDisabled("Tow Coupler: +logistics, -mobility, +thermal load");
        } else if (TankUsesRamShield(profile)) {
            ImGui::TextDisabled("Ram Shield: +impact, +survivability");
        } else {
            ImGui::TextDisabled("Bucket Rig: debris clearing and field works");
        }
        if (profile.partnerTank.worldPositionKnown) {
            ImGui::Text("Anchor: %.1f %.1f", profile.partnerTank.worldX, profile.partnerTank.worldY);
        } else {
            ImGui::TextDisabled("Anchor: unknown");
        }
        ImGui::ProgressBar(profile.partnerTank.energyReserve / 100.0f, ImVec2(-1.0f, 18.0f), "Energy");
        ImGui::ProgressBar(profile.partnerTank.ammoReserve / 100.0f, ImVec2(-1.0f, 18.0f), "Ammo");
        ImGui::Text("Workshop Log: %s", gameState.workshopServiceCooldown > 0.0f ? "Recent repair cycle complete" : "No recent workshop cycle");
        ImGui::BulletText("Workshop Engineer: %s", HasAssignedSpecialistRole(profile, "engineer", "workshop") ? "assigned" : "standard crew");
        ImGui::BulletText("Field Engineer: %s", HasAssignedSpecialistRole(profile, "engineer", "scavenger_support") ? "assigned" : "not assigned");
        ImGui::BulletText("Repair Recipe: %s", HasInventoryItem(profile, "recipe_repair_patch") ? "awakened" : "still dormant");
        ImGui::Separator();
        ImGui::Text("Tank Integrity");
        ImGui::ProgressBar(profile.partnerTank.damage.hull / 100.0f, ImVec2(-1.0f, 16.0f), "Hull");
        ImGui::ProgressBar(profile.partnerTank.damage.bucket / 100.0f, ImVec2(-1.0f, 16.0f), "Bucket");
        ImGui::ProgressBar(profile.partnerTank.damage.sensors / 100.0f, ImVec2(-1.0f, 16.0f), "Sensors");
        ImGui::Separator();
        ImGui::Text("Passive Skills");
        for (auto& skill : profile.character.passiveSkills) {
            if (!skill.unlocked) {
                ImGui::TextDisabled("%s [LOCKED]", skill.displayName.c_str());
                continue;
            }
            ImGui::Checkbox(skill.displayName.c_str(), &skill.equipped);
        }
        ImGui::Separator();
        if (profile.story.tankLinked && ImGui::Button("Swap Utility Module")) {
            ToggleTankUtilityModule(profile, player, gameState);
        }
        ImGui::SameLine();
        if (ImGui::Button("Field Service")) {
            TryRunFieldWorkbench(player, profile, gameState);
        }
        if (gameState.fieldWorkbenchCooldown > 0.0f) {
            ImGui::TextDisabled("Field Service Cooldown: %.0fs", gameState.fieldWorkbenchCooldown);
        } else {
            ImGui::TextDisabled("Field Service ready when BT-72 is stationary.");
        }
        ImGui::Separator();
        int damagedArchiveCount = 0;
        int reconstructedArchiveCount = 0;
        for (const auto& tape : profile.character.collectedTapes) {
            if (tape.damaged && !tape.reconstructed) {
                damagedArchiveCount += 1;
            }
            if (tape.reconstructed) {
                reconstructedArchiveCount += 1;
            }
        }
        ImGui::Text("DATA / Archive Summary");
        ImGui::BulletText("Archive Sync: %s", profile.story.archiveRecovered ? "mirrored" : "missing");
        ImGui::BulletText("Relay Packet: %s", profile.story.relayRecovered ? "captured" : "missing");
        ImGui::BulletText("Debrief Record: %s", HasCollectedTape(profile, "debrief_shelter17") ? "archived" : "missing");
        ImGui::BulletText("Archive Sync Count: %d", profile.character.awakening.archiveSyncs);
        ImGui::BulletText("Damaged Carriers: %d", damagedArchiveCount);
        ImGui::BulletText("Reconstructed Carriers: %d", reconstructedArchiveCount);
        ImGui::Separator();
        ImGui::Text("Archive Carriers");
        for (std::size_t index = 0; index < profile.character.collectedTapes.size(); ++index) {
            auto& tape = profile.character.collectedTapes[index];
            std::string tapeLabel = tape.title;
            if (tape.damaged && !tape.reconstructed) {
                tapeLabel += " [DAMAGED]";
            } else {
                tapeLabel += tape.played ? " [LISTENED]" : " [NEW]";
            }
            if (ImGui::Selectable(tapeLabel.c_str(), profile.character.activeTapeIndex == static_cast<int>(index))) {
                profile.character.activeTapeIndex = static_cast<int>(index);
                if (!tape.damaged || tape.reconstructed) {
                    tape.played = true;
                    gameState.lastEvent = "Tape opened: " + tape.title;
                } else {
                    gameState.lastEvent = "Tape is damaged. Reconstruction required before playback.";
                }
            }
        }
        if (HasActiveTape(profile.character)) {
            auto& activeTape = profile.character.collectedTapes[static_cast<std::size_t>(profile.character.activeTapeIndex)];
            if (activeTape.damaged && !activeTape.reconstructed) {
                ImGui::Separator();
                ImGui::TextWrapped("Damaged archive detected. Reconstruction will spend 12 MP to restore readable fragments.");
                if (ImGui::Button("Reconstruct Data")) {
                    if (profile.character.mp < 12.0f) {
                        gameState.lastEvent = "Not enough MP to reconstruct damaged data.";
                    } else {
                        profile.character.mp = std::max(0.0f, profile.character.mp - 12.0f);
                        activeTape.reconstructed = true;
                        activeTape.damaged = false;
                        activeTape.played = true;
                        activeTape.title += " // Reconstructed";
                        std::string progressionEvent;
                        AwardExperience(profile, 20, &progressionEvent);
                        gameState.lastEvent = "Data reconstruction complete for " + activeTape.title + ". " + progressionEvent;
                    }
                }
            }
        }
        ImGui::Separator();
        ImGui::Text("Tape Bonus");
        ImGui::TextWrapped("%s", ActiveTapeBonusLabel(profile.character, player.insideTank).c_str());
    } else if (activeTab == 3) {
        ImGui::Text("Local Survey");
        ImGui::Text("Zone: %s", world.metadata.name.c_str());
        ImGui::Text("Objective: %s", world.metadata.objective.c_str());
        ImGui::Separator();
        ImGui::BeginChild("MapList", ImVec2(0.0f, 300.0f), true);
        for (const auto& object : world.objects) {
            ImGui::BulletText("%s | %.1f %.1f | HP %.0f", object.displayName.c_str(), object.x, object.y, object.health);
        }
        ImGui::EndChild();
    } else if (activeTab == 4) {
        const float recoveryIndex = ShelterRecoveryIndex(profile);
        auto* worldFieldState = FindWorldFieldState(profile, profile.selectedWorld, true);
        const bool stableRecoveryBackbone = worldFieldState != nullptr &&
            IsStableRecoveryBackbone(profile, *worldFieldState);
        ImGui::Text("Mission Log");
        ImGui::BulletText("%s", CurrentStoryObjective(profile, staticEraser).c_str());
        ImGui::Separator();
        ImGui::Text("Recovery Support Stock");
        ImGui::BulletText("Trade Vouchers: %d", InventoryCount(profile, "trade_voucher"));
        ImGui::BulletText("Repair Patches: %d", InventoryCount(profile, "repair_patch"));
        ImGui::BulletText("Power Cells: %d", InventoryCount(profile, "power_cell"));
        ImGui::BulletText("Clean Water: %d", InventoryCount(profile, "clean_water"));
        ImGui::Separator();
        ImGui::Text("Shelter Recovery Index");
        ImGui::BulletText("State: %s", ShelterRecoveryBand(recoveryIndex));
        ImGui::ProgressBar(recoveryIndex / 100.0f, ImVec2(-1.0f, 16.0f));
        ImGui::TextWrapped("This index summarizes how stable Shelter 17 is across power, logistics, route control, and industrial recovery.");
        const int recoveryTier = ShelterRecoveryMilestoneTier(recoveryIndex);
        ImGui::BulletText("Milestone: %s", ShelterRecoveryMilestoneLabel(recoveryTier));
        ImGui::BulletText("Claimed checkpoints: %d / 3", worldFieldState != nullptr ? worldFieldState->recoveryMilestonesClaimed : 0);
        ImGui::BulletText("Recovery Backbone: %s", stableRecoveryBackbone ? "stable" : "still assembling");
        ImGui::TextWrapped("Thresholds: 25%%, 50%%, 75%% recovery. Each checkpoint unlocks a one-time shelter reward package.");
        ImGui::Separator();
        if (HasActiveFieldCheckpoint(profile)) {
            ImGui::Text("Field Checkpoint: %s", profile.fieldCheckpointLabel.empty() ? "Active" : profile.fieldCheckpointLabel.c_str());
            ImGui::BulletText("World: %s", profile.fieldCheckpointWorld.c_str());
            ImGui::BulletText("Coords: %.1f %.1f", profile.fieldCheckpointX, profile.fieldCheckpointY);
            if (worldFieldState != nullptr) {
                ImGui::BulletText("Fortification Level: %d / 3", worldFieldState->campFortificationLevel);
                if (worldFieldState->campFortificationLevel < 3) {
                    const int nextLevel = worldFieldState->campFortificationLevel + 1;
                    const int steelCost = nextLevel * 2;
                    const int wireCost = nextLevel;
                    ImGui::TextWrapped("Upgrade cost: %d steel_scrap, %d copper_wire.", steelCost, wireCost);
                    if (ImGui::SmallButton("Upgrade Camp Fortifications")) {
                        if (InventoryCount(profile, "steel_scrap") < steelCost || InventoryCount(profile, "copper_wire") < wireCost) {
                            gameState.lastEvent = "Camp fortification upgrade needs more steel_scrap and copper_wire.";
                        } else if (!ConsumeInventoryItem(profile, "steel_scrap", steelCost) || !ConsumeInventoryItem(profile, "copper_wire", wireCost)) {
                            gameState.lastEvent = "Camp fortification upgrade materials were incomplete.";
                        } else {
                            worldFieldState->campFortificationLevel = nextLevel;
                            worldFieldState->routeContamination = std::max(0.0f, worldFieldState->routeContamination - 4.0f * nextLevel);
                            std::string xpEvent;
                            AwardExperience(profile, 15 * nextLevel, &xpEvent);
                            gameState.lastEvent = "Field checkpoint fortifications upgraded. Route control and shelter resilience improved. " + xpEvent;
                        }
                    }
                } else {
                    ImGui::TextWrapped("Field checkpoint is fully fortified and holding the route with hardened barriers.");
                }
            }
            ImGui::Separator();
        }
        if (!profile.rescuedSpecialists.empty()) {
            ImGui::Text("Cryo Specialists");
            for (auto& specialist : profile.rescuedSpecialists) {
                ImGui::PushID(specialist.specialistId.c_str());
                ImGui::BulletText("%s | %s | %s", specialist.displayName.c_str(), specialist.role.c_str(), specialist.assignment.c_str());
                ImGui::SameLine();
                if (specialist.role == "engineer") {
                    const char* buttonLabel = specialist.assignment == "scavenger_support"
                        ? "Assign Workshop"
                        : "Assign Scavengers";
                    if (ImGui::SmallButton(buttonLabel)) {
                        specialist.assignment = specialist.assignment == "scavenger_support" ? "workshop" : "scavenger_support";
                        gameState.lastEvent = specialist.assignment == "workshop"
                            ? specialist.displayName + " reassigned to workshop support. Heavy service cycles now get the engineer boost."
                            : specialist.displayName + " reassigned to scavenger support. Field service and salvage teams now get the engineer boost.";
                    }
                }
                ImGui::PopID();
            }
            ImGui::Separator();
        }
        ImGui::Text("Shelter Doctrine");
        ImGui::BulletText("Current: %s", ToString(profile.doctrine));
        if (ImGui::SmallButton("Balanced Doctrine")) {
            profile.doctrine = ShelterDoctrine::Balanced;
            gameState.lastEvent = "Shelter doctrine reset to Balanced.";
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Industry Doctrine")) {
            profile.doctrine = ShelterDoctrine::Industry;
            gameState.lastEvent = "Shelter doctrine shifted to Industry.";
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Defense Doctrine")) {
            profile.doctrine = ShelterDoctrine::Defense;
            gameState.lastEvent = "Shelter doctrine shifted to Defense.";
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Medical Doctrine")) {
            profile.doctrine = ShelterDoctrine::Medical;
            gameState.lastEvent = "Shelter doctrine shifted to Medical.";
        }
        ImGui::TextWrapped("Industry boosts workshop and salvage. Defense favors operational tempo. Medical improves camp recovery.");
        ImGui::Separator();
        ImGui::Text("Power Grid");
        ImGui::BulletText("State: %s", IsRegionalGridOnline(profile) ? "online" : "offline");
        ImGui::BulletText("Tower Sync: %s", HasCollectedTape(profile, "[%term_0001]_tower") ? "linked" : "not linked");
        ImGui::BulletText("Relay Packet: %s", profile.story.relayRecovered ? "recovered" : "missing");
        ImGui::BulletText("Restored Pylons: %d", CountRestoredPylons(profile));
        ImGui::Separator();
        ImGui::Text("Ether Erosion");
        ImGui::BulletText("State: %s", EtherErosionBand(CurrentEtherErosion(profile)));
        ImGui::BulletText("Pressure: %.0f%%", CurrentEtherErosion(profile));
        ImGui::BulletText("Purge cycles: %d", worldFieldState != nullptr ? worldFieldState->purgeCycles : 0);
        ImGui::TextWrapped("%s",
            IsRegionalGridOnline(profile)
                ? "Relay and tower sync are suppressing bloom growth. Camp and scavengers can gradually reclaim the route."
                : "Without a stable grid, ether fog slowly crystallizes the route and chokes vehicle movement.");
        ImGui::Separator();
        ImGui::Text("Infrastructure Decay");
        ImGui::BulletText("State: %s", worldFieldState != nullptr ? InfrastructureDecayBand(worldFieldState->infrastructureDecay) : "Stable");
        ImGui::BulletText("Load: %.0f%%", worldFieldState != nullptr ? worldFieldState->infrastructureDecay : 0.0f);
        ImGui::TextWrapped("Offline grids slowly wear down workshop, camps, and route service. Restored power stabilizes structures over time.");
        ImGui::Separator();
        ImGui::Text("Route Contamination");
        ImGui::BulletText("Pressure: %.0f%%", worldFieldState != nullptr ? worldFieldState->routeContamination : 0.0f);
        ImGui::BulletText("Overrun: %s", worldFieldState != nullptr && worldFieldState->routeOverrun ? "yes" : "no");
        ImGui::TextWrapped("If the outer route stays unsecured under heavy ether pressure, barricades and nests can re-form even after an earlier cleanup.");
        ImGui::Separator();
        const bool scavengerReady =
            HasActiveFieldCheckpoint(profile) &&
            profile.story.outerRoadCleared &&
            IsRegionalGridOnline(profile) &&
            HasAwakenedSpecialistRole(profile, "engineer");
        ImGui::Text("Scavenger Teams");
        if (scavengerReady) {
            ImGui::BulletText("Status: active");
            ImGui::BulletText("Next return: %.0fs", gameState.scavengerTimer);
            ImGui::BulletText("Completed runs: %d", profile.scavengerRunsCompleted);
            ImGui::BulletText("Engineer support: %s", HasAssignedSpecialistRole(profile, "engineer", "scavenger_support") ? "scavenger_support" : "workshop");
        } else {
            ImGui::BulletText("Status: unavailable");
            ImGui::TextWrapped("Requires a field checkpoint, a cleared outer route, and an awakened engineer.");
        }
        ImGui::Separator();
        ImGui::Text("Autopilot Caravan");
        if (worldFieldState != nullptr) {
            const bool caravanReady = CaravanRouteReady(profile);
            ImGui::BulletText("Route: %s",
                worldFieldState->caravanRouteActive
                    ? (caravanReady ? "active" : "blocked")
                    : "offline");
            ImGui::BulletText("Completed runs: %d", worldFieldState->caravanRunsCompleted);
            if (worldFieldState->caravanRouteActive) {
                ImGui::BulletText("Next return: %.0fs", gameState.caravanTimer);
            }
            if (ImGui::SmallButton(worldFieldState->caravanRouteActive ? "Disable Caravan Route" : "Enable Caravan Route")) {
                if (!worldFieldState->caravanRouteActive && !caravanReady) {
                    gameState.lastEvent = "Autopilot caravan needs a field checkpoint, a cleared outer route, and a live regional grid.";
                } else {
                    worldFieldState->caravanRouteActive = !worldFieldState->caravanRouteActive;
                    gameState.caravanTimer = 260.0f;
                    gameState.lastEvent = worldFieldState->caravanRouteActive
                        ? "Autopilot caravan route activated between shelter and forward camp."
                        : "Autopilot caravan route suspended.";
                }
            }
            ImGui::TextWrapped("Requires field checkpoint, cleared route, and regional grid. Industry doctrine and engineer support improve throughput.");
            if (worldFieldState->caravanRouteActive && !caravanReady) {
                ImGui::TextDisabled("Caravan route is enabled but waiting on checkpoint, route clearance, or grid recovery.");
            }
        } else {
            ImGui::TextDisabled("No world logistics state available.");
        }
        ImGui::Separator();
          ImGui::Text("Trade Network");
          if (worldFieldState != nullptr) {
              const bool tradeReady = TradeNetworkReady(profile, *worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->tradeNetworkActive
                      ? (tradeReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Completed cycles: %d", worldFieldState->tradeCyclesCompleted);
              ImGui::BulletText("Trade vouchers on hand: %d", InventoryCount(profile, "trade_voucher"));
              if (worldFieldState->tradeNetworkActive) {
                ImGui::BulletText("Next convoy sync: %.0fs", gameState.tradeTimer);
            }
            if (ImGui::SmallButton(worldFieldState->tradeNetworkActive ? "Disable Trade Network" : "Enable Trade Network")) {
                if (!worldFieldState->tradeNetworkActive && !tradeReady) {
                    gameState.lastEvent = "Trade network needs a field checkpoint, a live grid, and either caravan or drone logistics.";
                } else {
                    worldFieldState->tradeNetworkActive = !worldFieldState->tradeNetworkActive;
                    gameState.tradeTimer = 240.0f;
                    gameState.lastEvent = worldFieldState->tradeNetworkActive
                        ? "Trade network activated across connected camps."
                        : "Trade network placed on standby.";
                }
            }
              ImGui::TextWrapped("Requires a field checkpoint, a live grid, and either caravan or drone logistics. Generates trade vouchers for supply exchange.");
              if (worldFieldState->tradeNetworkActive && !tradeReady) {
                  ImGui::TextDisabled("Trade network is enabled but waiting on checkpoint, grid, or upstream logistics.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Rail Freight Link");
          if (worldFieldState != nullptr) {
              const bool railReady = RailFreightReady(profile, *worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->railFreightActive
                      ? (railReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Completed runs: %d", worldFieldState->railRunsCompleted);
              if (worldFieldState->railFreightActive) {
                  ImGui::BulletText("Next freight return: %.0fs", gameState.railTimer);
              }
              if (ImGui::SmallButton(worldFieldState->railFreightActive ? "Disable Rail Freight" : "Enable Rail Freight")) {
                  if (!worldFieldState->railFreightActive && !railReady) {
                      gameState.lastEvent = "Rail freight needs checkpoint coverage, a cleared route, a live grid, one restored pylon, and upstream logistics.";
                  } else {
                      worldFieldState->railFreightActive = !worldFieldState->railFreightActive;
                      gameState.railTimer = 320.0f;
                      gameState.lastEvent = worldFieldState->railFreightActive
                          ? "Rail freight link activated across the restored spur."
                          : "Rail freight link placed on standby.";
                  }
              }
              ImGui::TextWrapped("Requires a live grid, at least one restored pylon, and an active logistics backbone. Tow Coupler and Industry doctrine improve freight throughput.");
              if (worldFieldState->railFreightActive && !railReady) {
                  ImGui::TextDisabled("Rail freight is enabled but waiting on route clearance, grid reach, pylons, or logistics input.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Orbital Uplink");
          if (worldFieldState != nullptr) {
              const bool orbitalReady = OrbitalUplinkReady(profile, *worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->orbitalUplinkActive
                      ? (orbitalReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Completed scans: %d", worldFieldState->orbitalScansCompleted);
              if (worldFieldState->orbitalUplinkActive) {
                  ImGui::BulletText("Next scan: %.0fs", gameState.orbitalTimer);
              }
              if (ImGui::SmallButton(worldFieldState->orbitalUplinkActive ? "Disable Orbital Uplink" : "Enable Orbital Uplink")) {
                  if (!worldFieldState->orbitalUplinkActive && !orbitalReady) {
                      gameState.lastEvent = "Orbital uplink needs a live grid, active rail freight, two restored pylons, and trade or drone support.";
                  } else {
                      worldFieldState->orbitalUplinkActive = !worldFieldState->orbitalUplinkActive;
                      gameState.orbitalTimer = 420.0f;
                      gameState.lastEvent = worldFieldState->orbitalUplinkActive
                          ? "Orbital uplink activated. Low-orbit scan window opened."
                          : "Orbital uplink placed on standby.";
                  }
              }
              ImGui::TextWrapped("Requires live grid, restored pylons, active rail freight, and a functioning logistics backbone. Grants orbital scans and route intelligence.");
              if (worldFieldState->orbitalUplinkActive && !orbitalReady) {
                  ImGui::TextDisabled("Orbital uplink is enabled but waiting on rail freight, pylon coverage, grid stability, or logistics support.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Rail Fortress");
          if (worldFieldState != nullptr) {
              const bool fortressReady = RailFortressReady(profile, *worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->railFortressActive
                      ? (fortressReady ? "deployed" : "blocked")
                      : "standby");
              ImGui::BulletText("Patrol cycles: %d", worldFieldState->railFortressDeployments);
              if (worldFieldState->railFortressActive) {
                  ImGui::BulletText("Next return: %.0fs", gameState.railFortressTimer);
              }
              if (ImGui::SmallButton(worldFieldState->railFortressActive ? "Recall Rail Fortress" : "Deploy Rail Fortress")) {
                  if (!worldFieldState->railFortressActive && !fortressReady) {
                      gameState.lastEvent = "Rail fortress needs active rail freight, an orbital uplink, a healthy grid, and two restored pylons.";
                  } else {
                      worldFieldState->railFortressActive = !worldFieldState->railFortressActive;
                      gameState.railFortressTimer = 520.0f;
                      gameState.lastEvent = worldFieldState->railFortressActive
                          ? "Rail fortress deployed along the restored spur."
                          : "Rail fortress returned to depot standby.";
                  }
              }
              ImGui::TextWrapped("Requires active rail freight, orbital uplink and a healthy grid. Defense doctrine improves armored patrol tempo.");
              if (worldFieldState->railFortressActive && !fortressReady) {
                  ImGui::TextDisabled("Rail fortress is deployed in the ledger but waiting on freight, uplink, grid, or pylon support.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Recovery Fabricator");
          if (worldFieldState != nullptr) {
              const bool fabricatorReady = RecoveryFabricatorReady(profile, *worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->recoveryFabricatorActive
                      ? (fabricatorReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Completed cycles: %d", worldFieldState->fabricatorCyclesCompleted);
              if (worldFieldState->recoveryFabricatorActive) {
                  ImGui::BulletText("Next cycle: %.0fs", gameState.fabricatorTimer);
              }
              if (ImGui::SmallButton(worldFieldState->recoveryFabricatorActive ? "Disable Fabricator" : "Enable Fabricator")) {
                  if (!worldFieldState->recoveryFabricatorActive && !fabricatorReady) {
                      gameState.lastEvent = "Recovery fabricator needs a live grid, logistics input, and either trade or orbital support.";
                  } else {
                      worldFieldState->recoveryFabricatorActive = !worldFieldState->recoveryFabricatorActive;
                      gameState.fabricatorTimer = 280.0f;
                      gameState.lastEvent = worldFieldState->recoveryFabricatorActive
                          ? "Recovery fabricator activated for Shelter 17."
                          : "Recovery fabricator returned to standby.";
                  }
              }
              ImGui::TextWrapped("Requires live grid, logistics input, and either trade or orbital support. Converts salvage into field supplies and recovery stock.");
              if (worldFieldState->recoveryFabricatorActive && !fabricatorReady) {
                  ImGui::TextDisabled("Recovery fabricator is enabled but waiting on grid stability, inbound logistics, or convoy/uplink support.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Industrial Gate");
          if (worldFieldState != nullptr) {
              ImGui::BulletText("State: %s", worldFieldState->industrialGateUnlocked ? "unlocked" : "sealed");
              ImGui::TextWrapped("Requires the active recovery backbone. Unlocking the gate opens the next industrial push beyond the starter recovery corridor.");
          }
          ImGui::Separator();
          ImGui::Text("Industrial Survey Beacon");
          if (worldFieldState != nullptr) {
              const bool surveyReady = IndustrialSurveyReady(*worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->industrialSurveyActive
                      ? (surveyReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Survey runs: %d", worldFieldState->surveyRunsCompleted);
              if (worldFieldState->industrialSurveyActive) {
                  ImGui::BulletText("Next sweep: %.0fs", gameState.surveyTimer);
              }
              ImGui::TextWrapped("Requires the unlocked industrial gate, orbital uplink, and trade support. Survey sweeps feed inner spur intel back to Shelter 17.");
              if (worldFieldState->industrialSurveyActive && !surveyReady) {
                  ImGui::TextDisabled("Survey beacon is enabled but waiting on gate access, orbital uplink, or trade support.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Inner Spur Outpost");
          if (worldFieldState != nullptr) {
              const bool outpostReady = IndustrialOutpostReady(*worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->industrialOutpostActive
                      ? (outpostReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Supply runs: %d", worldFieldState->outpostSupplyRuns);
              if (worldFieldState->industrialOutpostActive) {
                  ImGui::BulletText("Next return: %.0fs", gameState.outpostTimer);
              }
              ImGui::TextWrapped("Requires gate access, survey coverage, and trade support. Outpost supply runs reinforce the inner spur foothold.");
              if (worldFieldState->industrialOutpostActive && !outpostReady) {
                  ImGui::TextDisabled("Inner spur outpost is enabled but waiting on gate access, survey coverage, or trade support.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Assembly Cell");
          if (worldFieldState != nullptr) {
              const bool assemblyReady = AssemblyCellReady(*worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->assemblyCellActive
                      ? (assemblyReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Assembly cycles: %d", worldFieldState->assemblyCyclesCompleted);
              if (worldFieldState->assemblyCellActive) {
                  ImGui::BulletText("Next cycle: %.0fs", gameState.assemblyTimer);
              }
              ImGui::TextWrapped("Requires a live outpost, survey coverage, and the recovery fabricator. This is the first local production cell beyond the industrial gate.");
              if (worldFieldState->assemblyCellActive && !assemblyReady) {
                  ImGui::TextDisabled("Assembly cell is enabled but waiting on outpost support, survey coverage, or fabricator output.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Foundry Line");
          if (worldFieldState != nullptr) {
              const bool foundryReady = FoundryLineReady(*worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->foundryLineActive
                      ? (foundryReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Foundry cycles: %d", worldFieldState->foundryCyclesCompleted);
              if (worldFieldState->foundryLineActive) {
                  ImGui::BulletText("Next cycle: %.0fs", gameState.foundryTimer);
              }
              ImGui::TextWrapped("Requires assembly, outpost support, and rail security. This is the first heavy fabrication line beyond the gate.");
              if (worldFieldState->foundryLineActive && !foundryReady) {
                  ImGui::TextDisabled("Foundry line is enabled but waiting on assembly throughput, outpost support, or Rail Fortress cover.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Reactor Yard");
          if (worldFieldState != nullptr) {
              const bool reactorReady = ReactorYardReady(*worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->reactorYardActive
                      ? (reactorReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Reactor cycles: %d", worldFieldState->reactorCyclesCompleted);
              if (worldFieldState->reactorYardActive) {
                  ImGui::BulletText("Next cycle: %.0fs", gameState.reactorTimer);
              }
              ImGui::TextWrapped("Requires foundry, assembly, and orbital support. This is the first heavy energy yard beyond the industrial gate.");
              if (worldFieldState->reactorYardActive && !reactorReady) {
                  ImGui::TextDisabled("Reactor yard is enabled but waiting on foundry output, assembly support, or orbital coverage.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Capacitor Bank");
          if (worldFieldState != nullptr) {
              const bool capacitorReady = CapacitorBankReady(*worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->capacitorBankActive
                      ? (capacitorReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Discharge cycles: %d", worldFieldState->capacitorDischargeCycles);
              if (worldFieldState->capacitorBankActive) {
                  ImGui::BulletText("Next discharge: %.0fs", gameState.capacitorTimer);
              }
              ImGui::TextWrapped("Requires reactor and foundry support. Buffers heavy energy and stabilizes the recovery backbone.");
              if (worldFieldState->capacitorBankActive && !capacitorReady) {
                  ImGui::TextDisabled("Capacitor bank is enabled but waiting on reactor output, foundry throughput, or orbital support.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Relay Substation");
          if (worldFieldState != nullptr) {
              const bool relayReady = RelaySubstationReady(*worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->relaySubstationActive
                      ? (relayReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Sync cycles: %d", worldFieldState->relaySyncCycles);
              if (worldFieldState->relaySubstationActive) {
                  ImGui::BulletText("Next sync: %.0fs", gameState.relaySubstationTimer);
              }
              ImGui::TextWrapped("Requires capacitor, reactor, and a live outpost. Routes deeper industrial power back into Shelter 17.");
              if (worldFieldState->relaySubstationActive && !relayReady) {
                  ImGui::TextDisabled("Relay substation is enabled but waiting on capacitor output, reactor support, or the live inner spur outpost.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Service Bay");
          if (worldFieldState != nullptr) {
              const bool serviceReady = ServiceBayReady(*worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->serviceBayActive
                      ? (serviceReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Service cycles: %d", worldFieldState->serviceCyclesCompleted);
              if (worldFieldState->serviceBayActive) {
                  ImGui::BulletText("Next service: %.0fs", gameState.serviceBayTimer);
              }
              ImGui::TextWrapped("Requires relay, foundry, and a live outpost. Pushes BT-72 repair and heat recovery deeper into the inner spur.");
              if (worldFieldState->serviceBayActive && !serviceReady) {
                  ImGui::TextDisabled("Service bay is enabled but waiting on relay return flow, foundry support, or the live inner spur outpost.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Water Reclaimer");
          if (worldFieldState != nullptr) {
              const bool waterReady = WaterReclaimerReady(*worldFieldState);
              ImGui::BulletText("State: %s",
                  worldFieldState->waterReclaimerActive
                      ? (waterReady ? "active" : "blocked")
                      : "standby");
              ImGui::BulletText("Purification cycles: %d", worldFieldState->waterCyclesCompleted);
              if (worldFieldState->waterReclaimerActive) {
                  ImGui::BulletText("Next cycle: %.0fs", gameState.waterReclaimerTimer);
              }
              ImGui::TextWrapped("Requires service bay, relay support, and the recovery fabricator. Stabilizes camp recovery and produces clean water.");
              if (worldFieldState->waterReclaimerActive && !waterReady) {
                  ImGui::TextDisabled("Water reclaimer is enabled but waiting on service bay support, relay return flow, or fabricator output.");
              }
          }
          ImGui::Separator();
          ImGui::Text("Drone Sweep Station");
        if (worldFieldState != nullptr) {
            const bool droneReady = IsRegionalGridOnline(profile);
            ImGui::BulletText("State: %s",
                worldFieldState->droneStationsActive
                    ? (droneReady ? "active" : "blocked")
                    : "standby");
            ImGui::BulletText("Completed runs: %d", worldFieldState->droneRunsCompleted);
            if (worldFieldState->droneStationsActive) {
                ImGui::BulletText("Next sweep: %.0fs", gameState.droneTimer);
            }
            ImGui::TextWrapped("Requires a powered drone station in the world. Industry doctrine and restored pylons improve drone efficiency.");
            if (worldFieldState->droneStationsActive && !droneReady) {
                ImGui::TextDisabled("Drone sweep station is enabled but waiting on a live regional grid.");
            }
        }
        ImGui::Separator();
        ImGui::Text("Recovery Route");
        for (const auto& entry : BuildStarterRoute(profile, staticEraser)) {
            if (entry.completed) {
                ImGui::TextDisabled("%s", entry.text.c_str());
            } else {
                ImGui::BulletText("%s", entry.text.c_str());
            }
        }
        ImGui::Separator();
        ImGui::TextWrapped("%s",
            stableRecoveryBackbone
                ? "Shelter 17 has a live recovery backbone. The next priority is expanding into deeper industrial territory."
                : "Shelter 17 is still rebuilding. Keep restoring grid, logistics, and production nodes to raise the recovery index.");
    } else if (activeTab == 5) {
        LanlineSessionState sessionState;
        const bool hasSessionState = LoadLanlineSessionState(sessionState);
        ImGui::Text("Lanline - optime");
        if (!hasSessionState) {
            ImGui::TextDisabled("No active Lanline session state found. Launch through BunkerLauncher to seed roster and snapshot data.");
        } else {
            const std::string sessionWorldReference = NormalizeWorldReference(sessionState.worldName);
            const bool worldMatchesRuntime = sessionWorldReference == profile.selectedWorld;
            ImGui::Text("Session ID: %s", sessionState.sessionId.c_str());
            ImGui::Text("Mode: %s", sessionState.mode.c_str());
            ImGui::Text("World: %s", sessionWorldReference.c_str());
            ImGui::Text("Host: %s", sessionState.hostEndpoint.c_str());
            ImGui::Text("Updated: %s", sessionState.updatedAt.c_str());
            ImGui::Text("Runtime World Match: %s", worldMatchesRuntime ? "yes" : "no");
            ImGui::Separator();
            ImGui::Text("Session Roster");
            for (const auto& playerEntry : sessionState.players) {
                ImGui::BulletText("%s | %s | %s",
                    playerEntry.displayName.c_str(),
                    playerEntry.role.c_str(),
                    playerEntry.online ? "Online" : "Pending");
            }
            ImGui::Separator();
            ImGui::Text("Session Log");
            if (sessionState.eventLog.empty()) {
                ImGui::TextDisabled("No Lanline events recorded yet.");
            } else {
                for (const auto& eventLine : sessionState.eventLog) {
                    ImGui::BulletText("%s", eventLine.c_str());
                }
            }
            ImGui::Separator();
            const auto knownSessions = DiscoverLanlineSessionSnapshots();
            ImGui::Text("Known Sessions");
            if (knownSessions.empty()) {
                ImGui::TextDisabled("No session snapshots discovered.");
            } else {
                for (const auto& knownSession : knownSessions) {
                    ImGui::BulletText("%s | %s | %s | %s",
                        knownSession.sessionId.c_str(),
                        knownSession.mode.c_str(),
                        NormalizeWorldReference(knownSession.worldName).c_str(),
                        knownSession.hostEndpoint.c_str());
                }
            }
            ImGui::Separator();
            ImGui::TextWrapped("Lanline - optime keeps a visible session roster and snapshot trail even without Steam/Xbox auth.");
        }
    }

    ImGui::Separator();
    ImGui::TextWrapped("COMMS: %s", gameState.lastEvent.c_str());
    ImGui::End();
    ImGui::PopStyleColor(2);
}

}  // namespace bunker
