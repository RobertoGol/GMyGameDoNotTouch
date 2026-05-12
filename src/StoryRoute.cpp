#include "../include/StoryRoute.hpp"

#include <algorithm>

namespace bunker {

namespace {

int CountInventory(const SessionProfile& profile, const std::string& itemId) {
    int total = 0;
    for (const auto& entry : profile.character.inventory) {
        if (entry.itemId == itemId) {
            total += entry.count;
        }
    }
    return total;
}

bool HasAccessCardRecovered(const SessionProfile& profile) {
    return profile.firstPlayableRoute.accessCardRecovered || profile.story.pipPadRecovered;
}

bool ArchiveCorridorCleared(const SessionProfile& profile) {
    return profile.firstPlayableRoute.earlyVerminEncounterResolved ||
        profile.story.archiveRecovered ||
        profile.story.tankLinked;
}

bool ArchiveCorridorCleared(const SessionProfile& profile, const StaticEraser& staticEraser) {
    return ArchiveCorridorCleared(profile) || staticEraser.IsErased("[%enemy_laska_0001]");
}

bool Bt72HullInspected(const SessionProfile& profile) {
    return profile.firstPlayableRoute.bt72HullInspected || profile.story.tankLinked || profile.story.bucketRecovered;
}

bool Bt72CoreRecovered(const SessionProfile& profile) {
    return profile.firstPlayableRoute.bt72CoreRecovered || profile.story.tankLinked || profile.story.bucketRecovered;
}

bool Bt72ServiceNotesRecovered(const SessionProfile& profile) {
    return profile.firstPlayableRoute.bt72ServiceNotesRecovered ||
        profile.story.tankLinked ||
        profile.story.bucketRecovered ||
        HasCollectedTapeId(profile, "bt72_service_reel_001");
}

bool Bt72HullLockedInRestorationCradle(const SessionProfile& profile) {
    return profile.bt72HullLockedInRestorationCradle || profile.story.tankLinked || profile.story.bucketRecovered;
}

bool Bt72Restored(const SessionProfile& profile) {
    return profile.firstPlayableRoute.bt72Restored || profile.story.tankLinked || profile.story.bucketRecovered;
}

bool HasBt72RestorationKnowledge(const SessionProfile& profile) {
    return Bt72HullInspected(profile) &&
        Bt72CoreRecovered(profile) &&
        Bt72ServiceNotesRecovered(profile);
}

bool ClearanceBlueprintRecovered(const SessionProfile& profile) {
    return profile.firstPlayableRoute.clearanceBlueprintRecovered ||
        profile.story.bucketRecovered ||
        profile.story.exitedBunker;
}

bool ClearanceMaterialsRecovered(const SessionProfile& profile) {
    return profile.firstPlayableRoute.clearanceMaterialsRecovered ||
        profile.story.bucketRecovered ||
        profile.story.exitedBunker;
}

bool ClearanceModuleInstalled(const SessionProfile& profile) {
    return profile.firstPlayableRoute.clearanceModuleInstalled ||
        profile.story.bucketRecovered ||
        profile.story.exitedBunker;
}

bool SurfaceArrivalReached(const SessionProfile& profile) {
    return profile.firstPlayableRoute.surfaceArrivalReached ||
        profile.story.outerRoadCleared ||
        profile.story.relayRecovered ||
        profile.story.returnedToBase;
}

bool FirstTankCombatResolved(const SessionProfile& profile) {
    return profile.firstPlayableRoute.firstTankCombatResolved ||
        profile.story.relayRecovered ||
        profile.story.returnedToBase;
}

bool FirstTankCombatResolved(const SessionProfile& profile, const StaticEraser& staticEraser) {
    return FirstTankCombatResolved(profile) || staticEraser.IsErased("[%enemy_ghoul_0001]");
}

bool FirstServicePerformed(const SessionProfile& profile) {
    return profile.firstPlayableRoute.firstServicePerformed ||
        profile.story.relayRecovered ||
        profile.story.returnedToBase ||
        profile.character.awakening.fieldServiceUses > 0;
}

bool FirstRecoveryNodeActivated(const SessionProfile& profile) {
    return profile.firstPlayableRoute.firstRecoveryNodeActivated || profile.story.relayRecovered;
}

bool DebriefSummaryViewed(const SessionProfile& profile) {
    return profile.firstPlayableRoute.debriefSummaryViewed ||
        profile.story.returnedToBase ||
        HasCollectedTapeId(profile, "debrief_shelter17");
}

bool HasBt72RestorationPrerequisites(const SessionProfile& profile) {
    return HasBt72RestorationKnowledge(profile) &&
        Bt72HullLockedInRestorationCradle(profile);
}

bool HasBt72RestorationMaterials(const SessionProfile& profile) {
    return CountInventory(profile, "power_cell") >= 1 &&
        CountInventory(profile, "repair_patch") >= 1 &&
        CountInventory(profile, "old_plate") >= 1;
}

const char* RouteEventLabel(std::string_view routeEventType) {
    if (routeEventType == "service_call") {
        return "Service call";
    }
    if (routeEventType == "field_refuel") {
        return "Field refuel";
    }
    if (routeEventType == "relay_instability") {
        return "Relay instability";
    }
    if (routeEventType == "blocked_route") {
        return "Blocked route";
    }
    if (routeEventType == "damaged_convoy") {
        return "Damaged convoy";
    }
    if (routeEventType == "merchant_window") {
        return "Merchant window";
    }
    return "Route event";
}

int RouteEventGoal(const WorldFieldState& worldState) {
    if (worldState.activeRouteEventType == "service_call" ||
        worldState.activeRouteEventType == "field_refuel" ||
        worldState.activeRouteEventType == "relay_instability" ||
        worldState.activeRouteEventType == "blocked_route" ||
        worldState.activeRouteEventType == "damaged_convoy") {
        return 2;
    }
    return 1;
}

const WorldFieldState* SelectedWorldState(const SessionProfile& profile) {
    return FindWorldFieldState(profile, profile.selectedWorld);
}

SessionProfile BuildWorldScopedProfile(const SessionProfile& profile, std::string_view worldReference) {
    const std::string normalizedWorld = NormalizeWorldReference(std::string(worldReference));
    if (normalizedWorld.empty() || normalizedWorld == profile.selectedWorld) {
        return profile;
    }

    SessionProfile scopedProfile = profile;
    scopedProfile.selectedWorld = normalizedWorld;
    return scopedProfile;
}

std::string CurrentIndustrialObjective(const SessionProfile& profile, const WorldFieldState& worldState) {
    if (!IsRailFreightOperational(profile, worldState)) {
        return "Restore the industrial rail depot and bring heavy freight back to Shelter 17.";
    }
    if (!IsOrbitalUplinkOperational(profile, worldState)) {
        return "Align the orbital uplink to extend long-range recovery scans.";
    }
    if (!IsRailFortressOperational(profile, worldState)) {
        return "Deploy the Rail Fortress to secure the restored industrial spur.";
    }
    if (!IsRecoveryFabricatorOperational(profile, worldState)) {
        return "Prime the Recovery Fabricator to turn salvage into operational supplies.";
    }
    if (!worldState.industrialGateUnlocked) {
        return "Unlock the industrial gate and push Shelter 17 into the inner spur.";
    }
    if (!IsIndustrialSurveyOperational(worldState)) {
        return "Align the industrial survey beacon and start mapping the inner spur.";
    }
    if (!IsIndustrialOutpostOperational(worldState)) {
        return "Establish the inner spur outpost and secure a forward foothold beyond the gate.";
    }
    if (!IsAssemblyCellOperational(worldState)) {
        return "Bring the inner spur assembly cell online for local industrial recovery.";
    }
    if (!IsFoundryLineOperational(worldState)) {
        return "Restart the inner spur foundry line to resume heavy plate fabrication.";
    }
    if (!IsReactorYardOperational(worldState)) {
        return "Bring the inner spur reactor yard online to stabilize deeper industrial energy flow.";
    }
    if (!IsCapacitorBankOperational(worldState)) {
        return "Charge the inner spur capacitor bank to buffer and stabilize the heavy grid.";
    }
    if (!IsRelaySubstationOperational(worldState)) {
        return "Sync the relay substation and route inner spur power back into Shelter 17.";
    }
    if (!IsServiceBayOperational(worldState)) {
        return "Bring the inner spur service bay online to push BT-72 repairs deeper into the factory belt.";
    }
    if (!IsWaterReclaimerOperational(worldState)) {
        return "Bring the inner spur water reclaimer online to stabilize long-range recovery and camp support.";
    }
    return "Water reclaimer online. Shelter 17 now has a stable recovery backbone; expand deeper into the inner spur and wider factory belt.";
}

void AppendBackboneNodeLabel(std::vector<std::string>& labels, bool active, std::string_view label) {
    if (active) {
        labels.emplace_back(label);
    }
}

std::string CompactBackboneNodeList(const std::vector<std::string>& labels) {
    if (labels.empty()) {
        return "none";
    }
    if (labels.size() == 1) {
        return labels.front();
    }
    if (labels.size() == 2) {
        return labels[0] + ", " + labels[1];
    }
    if (labels.size() == 3) {
        return labels[0] + ", " + labels[1] + ", " + labels[2];
    }
    return labels[0] + ", " + labels[1] + ", " + labels[2] + " +" + std::to_string(labels.size() - 3) + " more";
}

std::string CurrentBackboneNextNodeLabel(const SessionProfile& profile, const WorldFieldState& worldState) {
    if (!IsRailFreightOperational(profile, worldState)) {
        return "rail freight";
    }
    if (!IsOrbitalUplinkOperational(profile, worldState)) {
        return "orbital uplink";
    }
    if (!IsRailFortressOperational(profile, worldState)) {
        return "Rail Fortress";
    }
    if (!IsRecoveryFabricatorOperational(profile, worldState)) {
        return "Recovery Fabricator";
    }
    if (!worldState.industrialGateUnlocked) {
        return "industrial gate";
    }
    if (!IsIndustrialSurveyOperational(worldState)) {
        return "industrial survey";
    }
    if (!IsIndustrialOutpostOperational(worldState)) {
        return "inner spur outpost";
    }
    if (!IsAssemblyCellOperational(worldState)) {
        return "assembly cell";
    }
    if (!IsFoundryLineOperational(worldState)) {
        return "foundry line";
    }
    if (!IsReactorYardOperational(worldState)) {
        return "reactor yard";
    }
    if (!IsCapacitorBankOperational(worldState)) {
        return "capacitor bank";
    }
    if (!IsRelaySubstationOperational(worldState)) {
        return "relay substation";
    }
    if (!IsServiceBayOperational(worldState)) {
        return "service bay";
    }
    if (!IsWaterReclaimerOperational(worldState)) {
        return "water reclaimer";
    }
    return "deeper factory belt";
}

std::string CurrentBackbonePayoff(const SessionProfile& profile, const WorldFieldState& worldState) {
    if (!IsRailFreightOperational(profile, worldState)) {
        return "Shelter 17 is still living off the first-route salvage and has no heavy logistics flow yet.";
    }
    if (!IsOrbitalUplinkOperational(profile, worldState)) {
        return "Heavy salvage is moving, but long-range recovery scans still end at the starter corridor.";
    }
    if (!IsRailFortressOperational(profile, worldState)) {
        return "Long-range scans are live, but the restored spur still lacks hardened control.";
    }
    if (!IsRecoveryFabricatorOperational(profile, worldState)) {
        return "The rail spur is secured, but salvage is not yet turning into field stock.";
    }
    if (!worldState.industrialGateUnlocked) {
        return "Shelter 17 can refine salvage into supplies, but the inner spur is still sealed.";
    }
    if (!IsIndustrialSurveyOperational(worldState)) {
        return "The inner spur is open, but it is still unmapped and risky to push.";
    }
    if (!IsIndustrialOutpostOperational(worldState)) {
        return "Survey coverage is live, but there is no forward logistics foothold beyond the gate.";
    }
    if (!IsAssemblyCellOperational(worldState)) {
        return "Forward logistics are live, but local industrial output has not started.";
    }
    if (!IsFoundryLineOperational(worldState)) {
        return "Local assembly is online, but heavy plate output is still dark.";
    }
    if (!IsReactorYardOperational(worldState)) {
        return "Heavy fabrication is back, but deep power remains unstable.";
    }
    if (!IsCapacitorBankOperational(worldState)) {
        return "Deep power is online, but it still needs buffered stability.";
    }
    if (!IsRelaySubstationOperational(worldState)) {
        return "The heavy grid is charged, but Shelter 17 is not yet receiving the return flow.";
    }
    if (!IsServiceBayOperational(worldState)) {
        return "Inner spur power is flowing home, but deep BT-72 support is still missing.";
    }
    if (!IsWaterReclaimerOperational(worldState)) {
        return "Deep BT-72 service is online, but frontier recovery still lacks long-range water support.";
    }
    return "Relay, service, and water loops are all online; Shelter 17 can now sustain a stable recovery backbone.";
}

RecoveryBackboneStatus CurrentRecoveryBackboneStatusForProfile(const SessionProfile& profile) {
    if (!FirstRecoveryNodeActivated(profile)) {
        return {
            "Route Locked",
            "First recovery sync is not online yet.",
            "The first route still needs its relay payoff before industrial recovery can start."
        };
    }
    if (!DebriefSummaryViewed(profile)) {
        return {
            "Debrief Pending",
            "Recovery payoff is captured, but Shelter 17 has not archived the debrief yet.",
            "Upload the debrief to turn the route payoff into honest rail, service, and industrial work."
        };
    }

    const auto* worldState = SelectedWorldState(profile);
    if (worldState == nullptr) {
        return {
            "Awaiting World State",
            "Selected-world industrial state is not loaded yet.",
            "Load the selected world to continue the post-debrief recovery backbone."
        };
    }

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

    std::vector<std::string> activeNodes;
    AppendBackboneNodeLabel(activeNodes, railOperational, "rail freight");
    AppendBackboneNodeLabel(activeNodes, orbitalOperational, "orbital uplink");
    AppendBackboneNodeLabel(activeNodes, fortressOperational, "Rail Fortress");
    AppendBackboneNodeLabel(activeNodes, fabricatorOperational, "Recovery Fabricator");
    AppendBackboneNodeLabel(activeNodes, worldState->industrialGateUnlocked, "industrial gate");
    AppendBackboneNodeLabel(activeNodes, surveyOperational, "industrial survey");
    AppendBackboneNodeLabel(activeNodes, outpostOperational, "inner spur outpost");
    AppendBackboneNodeLabel(activeNodes, assemblyOperational, "assembly cell");
    AppendBackboneNodeLabel(activeNodes, foundryOperational, "foundry line");
    AppendBackboneNodeLabel(activeNodes, reactorOperational, "reactor yard");
    AppendBackboneNodeLabel(activeNodes, capacitorOperational, "capacitor bank");
    AppendBackboneNodeLabel(activeNodes, relayOperational, "relay substation");
    AppendBackboneNodeLabel(activeNodes, serviceOperational, "service bay");
    AppendBackboneNodeLabel(activeNodes, waterOperational, "water reclaimer");

    const int starterOnline =
        (railOperational ? 1 : 0) +
        (orbitalOperational ? 1 : 0) +
        (fortressOperational ? 1 : 0) +
        (fabricatorOperational ? 1 : 0);
    const int innerOnline =
        (worldState->industrialGateUnlocked ? 1 : 0) +
        (surveyOperational ? 1 : 0) +
        (outpostOperational ? 1 : 0) +
        (assemblyOperational ? 1 : 0) +
        (foundryOperational ? 1 : 0) +
        (reactorOperational ? 1 : 0) +
        (capacitorOperational ? 1 : 0) +
        (relayOperational ? 1 : 0) +
        (serviceOperational ? 1 : 0) +
        (waterOperational ? 1 : 0);

    RecoveryBackboneStatus status{};
    if (IsStableRecoveryBackbone(profile, *worldState)) {
        status.stage = "Backbone Stable";
        status.status = "Recovery backbone " + std::to_string(starterOnline + innerOnline) +
            "/14 online // live: " + CompactBackboneNodeList(activeNodes) +
            " // next: deeper factory belt.";
    } else if (worldState->industrialGateUnlocked || innerOnline > 0) {
        status.stage = "Inner Spur Expansion";
        status.status = "Inner spur " + std::to_string(innerOnline) +
            "/10 online // live: " + CompactBackboneNodeList(activeNodes) +
            " // next: " + CurrentBackboneNextNodeLabel(profile, *worldState) + ".";
    } else {
        status.stage = "Starter Backbone";
        status.status = "Starter backbone " + std::to_string(starterOnline) +
            "/4 online // live: " + CompactBackboneNodeList(activeNodes) +
            " // next: " + CurrentBackboneNextNodeLabel(profile, *worldState) + ".";
    }
    status.payoff = CurrentBackbonePayoff(profile, *worldState);
    return status;
}

}  // namespace

bool HasLanlineServicesObjective(const SessionProfile& profile) {
    const auto* state = SelectedWorldState(profile);
    return state != nullptr && state->towerSyncRecovered;
}

bool HasFeyRingIntercityObjective(const SessionProfile& profile) {
    const auto* state = SelectedWorldState(profile);
    return state != nullptr && state->feyRingIntercityUnlocked;
}

bool HasFeyRingInterserverObjective(const SessionProfile& profile) {
    const auto* state = SelectedWorldState(profile);
    return state != nullptr && state->feyRingInterserverUnlocked;
}

std::string CurrentStoryCheckpointLabel(const SessionProfile& profile) {
    const auto& route = profile.firstPlayableRoute;
    if (!profile.story.awakenedFromCryo) {
        return "Intro // Cryo Wake";
    }
    if (!profile.story.pipPadRecovered) {
        if (!HasAccessCardRecovered(profile)) {
            return "Bunker Passage";
        }
        return route.prePipPadClueCount >= 2 ? "Pip-Pad Recovery" : "Bunker Passage";
    }
    if (!profile.story.archiveRecovered) {
        return ArchiveCorridorCleared(profile) ? "Archive Sync" : "Archive Corridor";
    }
    if (!Bt72Restored(profile)) {
        return "BT-72 Restoration";
    }
    if (!profile.story.tankLinked) {
        return "BT-72 Sync Link";
    }
    if (!ClearanceBlueprintRecovered(profile) || !ClearanceModuleInstalled(profile)) {
        return "Clearance Module";
    }
    if (!profile.story.exitedBunker) {
        return "Outer Bulkhead";
    }
    if (!SurfaceArrivalReached(profile)) {
        return "Surface Arrival";
    }
    if (!profile.story.outerRoadCleared) {
        return "Heavy Clearance";
    }
    if (!FirstTankCombatResolved(profile)) {
        return "First Tank Combat";
    }
    if (!FirstServicePerformed(profile)) {
        return "First Service Halt";
    }
    if (!FirstRecoveryNodeActivated(profile)) {
        return "Recovery Node";
    }
    if (!DebriefSummaryViewed(profile)) {
        return "Debrief";
    }
    return "Industrial Expansion";
}

namespace {

std::string CurrentStoryObjectivePreviewForProfile(const SessionProfile& profile) {
    const auto& route = profile.firstPlayableRoute;
    if (!profile.story.awakenedFromCryo) {
        return "Wake from the cryo capsule and stabilize your first bunker route.";
    }
    if (!profile.story.pipPadRecovered) {
        if (!HasAccessCardRecovered(profile)) {
            return "Recover a bunker access card and trace the paper trail to the missing Pip-Pad.";
        }
        if (route.prePipPadClueCount < 2) {
            return "Sweep the bunker passage, gather the paper trail, and recover the missing Pip-Pad.";
        }
        return "Recover the missing Pip-Pad from the locker bay.";
    }
    if (!profile.story.archiveRecovered) {
        return route.earlyVerminEncounterResolved
            ? "Sync the archive and pull the missing personnel trail into the Pip-Pad."
            : "Clear the archive corridor and put down the first vermin nest.";
    }
    if (!Bt72Restored(profile)) {
        if (!HasBt72RestorationKnowledge(profile)) {
            return "Survey the BT-72 hull, recover the starter core, and decode the service notes.";
        }
        if (!Bt72HullLockedInRestorationCradle(profile)) {
            return "Restore hangar power, clear the crane path, and move the BT-72 hull into the service lift cradle.";
        }
        if (!HasBt72RestorationMaterials(profile)) {
            return "Gather the salvage kit for BT-72 restoration: power cell, repair patch, and hull plate.";
        }
        return "Restore BT-72 to partial operating condition from bunker salvage.";
    }
    if (!profile.story.tankLinked) {
        return "Climb into the BT-72 cockpit and complete the first sync link.";
    }
    if (!ClearanceBlueprintRecovered(profile)) {
        return "Decode the clearance module blueprint from the maintenance echo.";
    }
    if (!ClearanceMaterialsRecovered(profile)) {
        return "Recover the clearance-module parts from the bucket rack.";
    }
    if (!ClearanceModuleInstalled(profile)) {
        return "Install the BT-72 clearance module and prep for heavy debris work.";
    }
    if (!profile.story.exitedBunker) {
        return "Cycle the outer bulkhead and push BT-72 into the blocked corridor.";
    }
    if (!SurfaceArrivalReached(profile)) {
        return "Push BT-72 through the lift route and establish the first exterior foothold.";
    }
    if (!profile.story.outerRoadCleared) {
        return "Use the clearance module to break the outer debris barrier.";
    }
    if (!FirstTankCombatResolved(profile)) {
        return "Hold the route through BT-72's first real combat contact.";
    }
    if (!FirstServicePerformed(profile)) {
        return "Take a first service/rest stop before pushing the recovery node.";
    }
    if (!FirstRecoveryNodeActivated(profile)) {
        return "Sync the first recovery node and prove the route changed the world.";
    }
    if (!DebriefSummaryViewed(profile)) {
        return "Return for debrief, summarize the route, and pull the next recovery hook.";
    }
    const auto* worldState = SelectedWorldState(profile);
    if (worldState == nullptr) {
        return "Recovery buildout active. Runtime world state not loaded yet.";
    }
    return CurrentIndustrialObjective(profile, *worldState);
}

}  // namespace

std::string CurrentStoryObjectivePreview(const SessionProfile& profile) {
    return CurrentStoryObjectivePreviewForProfile(profile);
}

std::string CurrentStoryObjectivePreview(const SessionProfile& profile, std::string_view worldReference) {
    return CurrentStoryObjectivePreviewForProfile(BuildWorldScopedProfile(profile, worldReference));
}

std::string CurrentStoryObjective(const SessionProfile& profile, const StaticEraser& staticEraser) {
    const auto& route = profile.firstPlayableRoute;
    if (!profile.story.awakenedFromCryo) {
        return "Wake from the cryo capsule and stabilize the bunker route.";
    }
    if (!profile.story.pipPadRecovered) {
        if (!HasAccessCardRecovered(profile)) {
            return "Recover a bunker access card and unlock the route toward the missing Pip-Pad.";
        }
        if (route.prePipPadClueCount < 2) {
            return "Sweep the bunker passage, gather the paper trail, and recover the missing Pip-Pad.";
        }
        return "Recover the missing Pip-Pad from the locker bay.";
    }
    if (!ArchiveCorridorCleared(profile, staticEraser)) {
        return "Clear the archive corridor and put down the first vermin nest.";
    }
    if (!profile.story.archiveRecovered) {
        return "Read the archive terminal and reconstruct what happened in Shelter 17.";
    }
    if (!Bt72Restored(profile)) {
        if (!HasBt72RestorationKnowledge(profile)) {
            return "Survey the BT-72 hull, recover the starter core, and decode the service notes.";
        }
        if (!Bt72HullLockedInRestorationCradle(profile)) {
            return "Power the hangar crane, clear the crane path, and lock BT-72 into the service lift cradle.";
        }
        if (!HasBt72RestorationMaterials(profile)) {
            return "Gather a power cell, repair patch, and hull plate before restoring BT-72.";
        }
        return "Restore BT-72 to partial operating condition from bunker salvage.";
    }
    if (!profile.story.tankLinked) {
        return "Reach the garage anchor and establish the BT-72 cockpit link.";
    }
    if (!ClearanceBlueprintRecovered(profile)) {
        return "Recover the clearance module blueprint from the maintenance echo.";
    }
    if (!ClearanceMaterialsRecovered(profile)) {
        return "Recover the clearance-module parts from the bucket rack.";
    }
    if (!ClearanceModuleInstalled(profile)) {
        return "Install the BT-72 clearance module before opening the outer route.";
    }
    if (!profile.story.exitedBunker) {
        return "Open the outer bulkhead and move BT-72 into the blocked recovery corridor.";
    }
    if (!SurfaceArrivalReached(profile)) {
        return "Advance through the lift route and secure the first surface arrival point.";
    }
    if (!profile.story.outerRoadCleared) {
        return "Raise the clearance module and break the outer debris barrier.";
    }
    if (!FirstTankCombatResolved(profile, staticEraser)) {
        return "Destroy the hostile contact guarding the first recovery node.";
    }
    if (!FirstServicePerformed(profile)) {
        return "Run a first service cycle before pushing the relay node.";
    }
    if (!FirstRecoveryNodeActivated(profile)) {
        return "Sync the relay terminal and recover the reconstruction schematics.";
    }
    if (!DebriefSummaryViewed(profile)) {
        return "Return to the debrief console inside Shelter 17.";
    }
    const auto* worldState = SelectedWorldState(profile);
    if (worldState == nullptr) {
        return "Recovery buildout active. Runtime world state not loaded yet.";
    }
    return CurrentIndustrialObjective(profile, *worldState);
}

namespace {

std::string CurrentRecoveryHandoffSummaryForProfile(const SessionProfile& profile) {
    if (!FirstRecoveryNodeActivated(profile)) {
        return "Recovery handoff locked until the first relay node comes online.";
    }
    if (!DebriefSummaryViewed(profile)) {
        return "Return to Shelter 17, upload the debrief, and turn the route payoff into recovery planning.";
    }

    const auto* worldState = SelectedWorldState(profile);
    if (worldState == nullptr) {
        return "Recovery handoff waiting for selected-world state.";
    }

    if (!IsRailFreightOperational(profile, *worldState)) {
        return "Handoff: restore rail freight and move heavy salvage off the starter route.";
    }
    if (!IsOrbitalUplinkOperational(profile, *worldState)) {
        return "Handoff: align the orbital uplink and extend recovery scans past the first corridor.";
    }
    if (!IsRailFortressOperational(profile, *worldState)) {
        return "Handoff: deploy the Rail Fortress and harden the restored spur.";
    }
    if (!IsRecoveryFabricatorOperational(profile, *worldState)) {
        return "Handoff: prime the Recovery Fabricator and turn salvage into field stock.";
    }
    if (!worldState->industrialGateUnlocked) {
        return "Handoff: unlock the industrial gate and push beyond the starter recovery lane.";
    }
    if (!IsIndustrialSurveyOperational(*worldState)) {
        return "Handoff: start industrial survey coverage for the inner spur.";
    }
    if (!IsIndustrialOutpostOperational(*worldState)) {
        return "Handoff: establish the inner spur outpost and secure forward logistics.";
    }
    if (!IsAssemblyCellOperational(*worldState)) {
        return "Handoff: bring the assembly cell online for local industrial recovery.";
    }
    if (!IsFoundryLineOperational(*worldState)) {
        return "Handoff: restart the foundry line for heavy plate output.";
    }
    if (!IsReactorYardOperational(*worldState)) {
        return "Handoff: stabilize the reactor yard for deeper industrial power.";
    }
    if (!IsCapacitorBankOperational(*worldState)) {
        return "Handoff: charge the capacitor bank and buffer the heavy grid.";
    }
    if (!IsRelaySubstationOperational(*worldState)) {
        return "Handoff: sync the relay substation back into Shelter 17.";
    }
    if (!IsServiceBayOperational(*worldState)) {
        return "Handoff: bring the service bay online for deep BT-72 support.";
    }
    if (!IsWaterReclaimerOperational(*worldState)) {
        return "Handoff: bring the water reclaimer online and stabilize frontier recovery.";
    }
    return "Handoff complete: Shelter 17 has a stable recovery backbone and can push deeper into the inner spur.";
}

std::string ActiveRouteEventSummaryForProfile(const SessionProfile& profile) {
    const auto* worldState = SelectedWorldState(profile);
    if (worldState == nullptr) {
        return "Route event layer waiting for selected-world state.";
    }

    const bool onboardingComplete =
        profile.story.awakenedFromCryo &&
        profile.story.pipPadRecovered &&
        profile.story.archiveRecovered &&
        profile.firstPlayableRoute.bt72Restored &&
        profile.story.tankLinked;

    if (!onboardingComplete) {
        return "Route event layer locked before onboarding complete.";
    }

    if (!(DebriefSummaryViewed(profile) || profile.story.returnedToBase || HasCollectedTapeId(profile, "debrief_shelter17"))) {
        return "Route event layer unlocked by onboarding but held until the first-route debrief handoff.";
    }

    if (HasActiveRouteEvent(*worldState)) {
        const int goal = RouteEventGoal(*worldState);
        const int secondsRemaining = worldState->routeEventTimeRemaining > 0.0f
            ? static_cast<int>(worldState->routeEventTimeRemaining + 0.5f)
            : 0;
        const char* phase = worldState->routeEventOfferTimeRemaining > 0.0f
            ? "offered"
            : (worldState->routeEventStage > 0 ? "escalating" : "active");
        return std::string(RouteEventLabel(worldState->activeRouteEventType)) + " " + phase +
            " // progress " + std::to_string(worldState->routeEventProgress) + "/" + std::to_string(goal) +
            " // " + std::to_string(secondsRemaining) + "s left";
    }

    if (!worldState->lastRouteEventOutcome.empty() && !worldState->lastRouteEventType.empty() && worldState->routeEventCooldown > 0.0f) {
        const int secondsRemaining = static_cast<int>(worldState->routeEventCooldown + 0.5f);
        return std::string(RouteEventLabel(worldState->lastRouteEventType)) + " " + worldState->lastRouteEventOutcome +
            " // cooldown " + std::to_string(secondsRemaining) + "s";
    }
    if (worldState->routeEventCooldown > 0.0f) {
        const int secondsRemaining = static_cast<int>(worldState->routeEventCooldown + 0.5f);
        return "Route event cooldown // next field incident window in " + std::to_string(secondsRemaining) + "s";
    }

    return "Route event layer unlocked // waiting for the next rare field prompt.";
}

const char* FirstPlayableRouteSurfaceStatus(const SessionProfile& profile) {
    if (!profile.story.exitedBunker) {
        return "bunker-bound";
    }
    if (!SurfaceArrivalReached(profile)) {
        return "ascent corridor live";
    }
    if (!profile.story.outerRoadCleared) {
        return "surface foothold secured";
    }
    if (!FirstTankCombatResolved(profile)) {
        return "combat lane open";
    }
    if (!FirstServicePerformed(profile)) {
        return "contact resolved, service halt pending";
    }
    if (!FirstRecoveryNodeActivated(profile)) {
        return "service complete, recovery node pending";
    }
    if (!DebriefSummaryViewed(profile)) {
        return "recovery node live, debrief pending";
    }
    return "route closed, industrial handoff live";
}

FirstPlayableRouteReadout BuildFirstPlayableRouteReadoutForProfile(const SessionProfile& profile) {
    const auto routeBeat = CurrentFirstPlayableRouteBeat(profile);
    const auto verticalSliceRoute = BuildFirstPlayableRouteSlice(profile);
    const auto nextSliceStep = std::find_if(verticalSliceRoute.begin(), verticalSliceRoute.end(),
        [](const StoryRouteEntry& entry) { return !entry.completed; });

    FirstPlayableRouteReadout readout{};
    readout.checkpoint = CurrentStoryCheckpointLabel(profile);
    readout.beat = routeBeat.label;
    readout.completedSteps = static_cast<int>(std::count_if(verticalSliceRoute.begin(), verticalSliceRoute.end(),
        [](const StoryRouteEntry& entry) { return entry.completed; }));
    readout.totalSteps = static_cast<int>(verticalSliceRoute.size());
    readout.nextPayoff = nextSliceStep != verticalSliceRoute.end()
        ? nextSliceStep->text
        : CurrentRecoveryHandoffSummary(profile);
    readout.surfaceStatus = FirstPlayableRouteSurfaceStatus(profile);
    readout.brief = routeBeat.cue;
    if (!readout.nextPayoff.empty() && readout.brief.find(readout.nextPayoff) == std::string::npos) {
        readout.brief += " Next: " + readout.nextPayoff;
    }
    if (!routeBeat.payoff.empty() && readout.brief.find(routeBeat.payoff) == std::string::npos) {
        readout.brief += " Payoff: " + routeBeat.payoff;
    }
    return readout;
}

FirstPlayableRouteBeat CurrentFirstPlayableRouteBeatForProfile(const SessionProfile& profile) {
    if (!profile.story.pipPadRecovered) {
        return {
            "Bunker Trace",
            "Recover the access card, stabilize the paper trail, and pull the missing Pip-Pad into the route.",
            "The archive trail should point cleanly toward the hangar and BT-72."
        };
    }
    if (!profile.story.archiveRecovered) {
        return {
            "Archive Corridor",
            "Clear the vermin gate and sync the missing personnel trail into the Pip-Pad.",
            "The hangar berth should read as the next honest destination."
        };
    }
    if (!Bt72Restored(profile)) {
        return {
            "Hangar Recovery",
            "Read the BT-72 hull, core, and service notes, move the hull through the crane and service lift cradle, then restore the chassis from bunker salvage.",
            "The slice shifts from bunker scavenging into mechanized recovery."
        };
    }
    if (!profile.story.tankLinked) {
        return {
            "BT-72 Sync",
            "Climb into the cockpit and stabilize the first BT-72 link.",
            "Pilot and gunner identity should become readable before the outer push."
        };
    }
    if (!ClearanceModuleInstalled(profile)) {
        return {
            "Hangar Prep",
            "Recover and install the clearance module before forcing the outer route.",
            "The bulkhead breakout and lift ascent become viable."
        };
    }
    if (!profile.story.exitedBunker) {
        return {
            "Bulkhead Breakout",
            "Cycle the outer bulkhead and line BT-72 up for the ascent corridor.",
            "The bunker should finally give way to the surface approach."
        };
    }
    if (!SurfaceArrivalReached(profile)) {
        return {
            "Surface Ascent",
            "Push BT-72 through the lift and secure the first exterior foothold.",
            "Skyline, exposure, and the debris wall should read as the route's first hard reveal."
        };
    }
    if (!profile.story.outerRoadCleared) {
        return {
            "Exterior Exposure",
            "Hold the exterior line, read the debris wall, and clear the blocked lane with the bucket rig.",
            "A clean clearance pass should open the first real combat lane."
        };
    }
    if (!FirstTankCombatResolved(profile)) {
        return {
            "First Contact",
            "Use BT-72 to survive the first surface contact without losing the lane.",
            "A short service halt should feel earned instead of menu-like."
        };
    }
    if (!FirstServicePerformed(profile)) {
        return {
            "Service Halt",
            "Run one field or workshop service cycle to cool BT-72 and reset the push.",
            "The recovery node becomes the next honest objective."
        };
    }
    if (!FirstRecoveryNodeActivated(profile)) {
        return {
            "Recovery Sync",
            "Sync the first recovery node and prove the route changed Shelter 17.",
            "Debrief turns the slice into a real recovery handoff."
        };
    }
    if (!DebriefSummaryViewed(profile)) {
        return {
            "Debrief Window",
            "Return to Shelter 17, upload the route summary, and close the first sortie cleanly.",
            "Industrial planning becomes the mid-game continuation."
        };
    }
    return {
        "Industrial Handoff",
        CurrentRecoveryHandoffSummaryForProfile(profile),
        "Route events, recovery state, and BT-72 service now carry progression beyond the starter lane."
    };
}

}  // namespace

std::string CurrentRecoveryHandoffSummary(const SessionProfile& profile) {
    return CurrentRecoveryHandoffSummaryForProfile(profile);
}

std::string CurrentRecoveryHandoffSummary(const SessionProfile& profile, std::string_view worldReference) {
    return CurrentRecoveryHandoffSummaryForProfile(BuildWorldScopedProfile(profile, worldReference));
}

RecoveryBackboneStatus CurrentRecoveryBackboneStatus(const SessionProfile& profile) {
    return CurrentRecoveryBackboneStatusForProfile(profile);
}

RecoveryBackboneStatus CurrentRecoveryBackboneStatus(const SessionProfile& profile, std::string_view worldReference) {
    return CurrentRecoveryBackboneStatusForProfile(BuildWorldScopedProfile(profile, worldReference));
}

std::string ActiveRouteEventSummary(const SessionProfile& profile) {
    return ActiveRouteEventSummaryForProfile(profile);
}

std::string ActiveRouteEventSummary(const SessionProfile& profile, std::string_view worldReference) {
    return ActiveRouteEventSummaryForProfile(BuildWorldScopedProfile(profile, worldReference));
}

FirstPlayableRouteBeat CurrentFirstPlayableRouteBeat(const SessionProfile& profile) {
    return CurrentFirstPlayableRouteBeatForProfile(profile);
}

FirstPlayableRouteBeat CurrentFirstPlayableRouteBeat(const SessionProfile& profile, std::string_view worldReference) {
    return CurrentFirstPlayableRouteBeatForProfile(BuildWorldScopedProfile(profile, worldReference));
}

FirstPlayableRouteReadout BuildFirstPlayableRouteReadout(const SessionProfile& profile) {
    return BuildFirstPlayableRouteReadoutForProfile(profile);
}

FirstPlayableRouteReadout BuildFirstPlayableRouteReadout(const SessionProfile& profile, std::string_view worldReference) {
    return BuildFirstPlayableRouteReadoutForProfile(BuildWorldScopedProfile(profile, worldReference));
}

std::vector<StoryRouteEntry> BuildBt72RestorationRoute(const SessionProfile& profile) {
    return {
        {"Inspect the BT-72 hull berth.", Bt72HullInspected(profile)},
        {"Recover the starter core from the bunker rack.", Bt72CoreRecovered(profile)},
        {"Decode BT-72 service notes and holo-records.", Bt72ServiceNotesRecovered(profile)},
        {"Lock the BT-72 hull in the service lift cradle.", Bt72HullLockedInRestorationCradle(profile)},
        {"Gather a power cell, repair patch, and hull plate.", Bt72Restored(profile) || HasBt72RestorationMaterials(profile)},
        {"Restore BT-72 to partial operating condition.", Bt72Restored(profile)},
        {"Complete the first cockpit sync link.", profile.story.tankLinked},
        {"Recover the clearance-module blueprint.", ClearanceBlueprintRecovered(profile)},
        {"Recover bucket-rack parts for the clearance module.", ClearanceMaterialsRecovered(profile)},
        {"Install the BT-72 clearance module.", ClearanceModuleInstalled(profile)},
    };
}

std::vector<StoryRouteEntry> BuildFirstPlayableRouteSlice(const SessionProfile& profile) {
    return {
        {"Wake from cryostasis.", profile.story.awakenedFromCryo},
        {"Recover the bunker access card and paper trail.", HasAccessCardRecovered(profile) &&
                (profile.firstPlayableRoute.prePipPadClueCount >= 2 || profile.story.pipPadRecovered)},
        {"Recover the missing Pip-Pad.", profile.story.pipPadRecovered},
        {"Sync the archive corridor and clear the first vermin gate.", profile.story.archiveRecovered &&
                ArchiveCorridorCleared(profile)},
        {"Restore BT-72 from bunker salvage.", Bt72Restored(profile)},
        {"Establish the first BT-72 sync link.", profile.story.tankLinked},
        {"Install the BT-72 clearance module.", ClearanceModuleInstalled(profile)},
        {"Open the outer bulkhead.", profile.story.exitedBunker},
        {"Reach the first surface arrival foothold.", SurfaceArrivalReached(profile)},
        {"Clear the outer debris barrier.", profile.story.outerRoadCleared},
        {"Resolve the first BT-72 combat contact.", FirstTankCombatResolved(profile)},
        {"Take the first service/rest halt.", FirstServicePerformed(profile)},
        {"Sync the first recovery node.", FirstRecoveryNodeActivated(profile)},
        {"Upload the debrief summary.", DebriefSummaryViewed(profile)},
    };
}

std::vector<StoryRouteEntry> BuildStarterRoute(const SessionProfile& profile, const StaticEraser& staticEraser) {
    const auto* worldState = SelectedWorldState(profile);
    const bool railOperational = worldState != nullptr && IsRailFreightOperational(profile, *worldState);
    const bool orbitalOperational = worldState != nullptr && IsOrbitalUplinkOperational(profile, *worldState);
    const bool fortressOperational = worldState != nullptr && IsRailFortressOperational(profile, *worldState);
    const bool fabricatorOperational = worldState != nullptr && IsRecoveryFabricatorOperational(profile, *worldState);
    const bool surveyOperational = worldState != nullptr && IsIndustrialSurveyOperational(*worldState);
    const bool outpostOperational = worldState != nullptr && IsIndustrialOutpostOperational(*worldState);
    const bool assemblyOperational = worldState != nullptr && IsAssemblyCellOperational(*worldState);
    const bool foundryOperational = worldState != nullptr && IsFoundryLineOperational(*worldState);
    const bool reactorOperational = worldState != nullptr && IsReactorYardOperational(*worldState);
    const bool capacitorOperational = worldState != nullptr && IsCapacitorBankOperational(*worldState);
    const bool relayOperational = worldState != nullptr && IsRelaySubstationOperational(*worldState);
    const bool serviceOperational = worldState != nullptr && IsServiceBayOperational(*worldState);
    const bool waterOperational = worldState != nullptr && IsWaterReclaimerOperational(*worldState);

    return {
        {"Wake from cryostasis.", profile.story.awakenedFromCryo},
        {"Recover the first emergency melee tool.", profile.firstPlayableRoute.emergencyMeleeRecovered},
        {"Recover a bunker access card before the Pip-Pad locker.", profile.firstPlayableRoute.accessCardRecovered},
        {"Gather bunker paper clues before the Pip-Pad pickup.", profile.firstPlayableRoute.prePipPadClueCount >= 2},
        {"Recover the missing Pip-Pad.", profile.story.pipPadRecovered},
        {"Clear the first vermin encounter in the archive corridor.", ArchiveCorridorCleared(profile, staticEraser)},
        {"Read the archive terminal.", profile.story.archiveRecovered},
        {"Inspect the BT-72 hull berth.", profile.firstPlayableRoute.bt72HullInspected},
        {"Recover the BT-72 starter core.", profile.firstPlayableRoute.bt72CoreRecovered},
        {"Decode BT-72 service notes.", profile.firstPlayableRoute.bt72ServiceNotesRecovered},
        {"Restore BT-72 from bunker salvage.", profile.firstPlayableRoute.bt72Restored},
        {"Establish the BT-72 tank link.", profile.story.tankLinked},
        {"Recover the clearance module blueprint.", profile.firstPlayableRoute.clearanceBlueprintRecovered},
        {"Recover the clearance module materials.", profile.firstPlayableRoute.clearanceMaterialsRecovered},
        {"Install the clearance module.", profile.firstPlayableRoute.clearanceModuleInstalled},
        {"Open the outer bulkhead.", profile.story.exitedBunker},
        {"Reach the first surface arrival foothold.", profile.firstPlayableRoute.surfaceArrivalReached},
        {"Clear the outer debris barrier.", profile.story.outerRoadCleared || staticEraser.IsErased("#%res_scrap_0001")},
        {"Resolve the first tank combat contact.", FirstTankCombatResolved(profile, staticEraser)},
        {"Take a first service/rest cycle.", profile.firstPlayableRoute.firstServicePerformed},
        {"Sync the first recovery node.", profile.firstPlayableRoute.firstRecoveryNodeActivated},
        {"Return to the debrief console.", profile.firstPlayableRoute.debriefSummaryViewed},
        {"Restore the rail freight depot.", railOperational},
        {"Align the orbital uplink.", orbitalOperational},
        {"Deploy the Rail Fortress.", fortressOperational},
        {"Prime the Recovery Fabricator.", fabricatorOperational},
        {"Unlock the industrial gate.", worldState != nullptr && worldState->industrialGateUnlocked},
        {"Align the industrial survey beacon.", surveyOperational},
        {"Establish the inner spur outpost.", outpostOperational},
        {"Bring the inner spur assembly cell online.", assemblyOperational},
        {"Restart the inner spur foundry line.", foundryOperational},
        {"Bring the inner spur reactor yard online.", reactorOperational},
        {"Charge the inner spur capacitor bank.", capacitorOperational},
        {"Sync the relay substation back into Shelter 17.", relayOperational},
        {"Bring the inner spur service bay online.", serviceOperational},
        {"Bring the inner spur water reclaimer online.", waterOperational},
    };
}

}  // namespace bunker
