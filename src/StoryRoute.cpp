#include "../include/StoryRoute.hpp"

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

bool ArchiveCorridorCleared(const SessionProfile& profile, const StaticEraser& staticEraser) {
    return profile.firstPlayableRoute.earlyVerminEncounterResolved || staticEraser.IsErased("[%enemy_laska_0001]");
}

bool FirstTankCombatResolved(const SessionProfile& profile, const StaticEraser& staticEraser) {
    return profile.firstPlayableRoute.firstTankCombatResolved || staticEraser.IsErased("[%enemy_ghoul_0001]");
}

bool HasBt72RestorationPrerequisites(const SessionProfile& profile) {
    const auto& route = profile.firstPlayableRoute;
    return route.bt72HullInspected && route.bt72CoreRecovered && route.bt72ServiceNotesRecovered;
}

bool HasBt72RestorationMaterials(const SessionProfile& profile) {
    return CountInventory(profile, "power_cell") >= 1 &&
        CountInventory(profile, "repair_patch") >= 1 &&
        CountInventory(profile, "old_plate") >= 1;
}

const WorldFieldState* SelectedWorldState(const SessionProfile& profile) {
    return FindWorldFieldState(profile, profile.selectedWorld);
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
        return route.prePipPadClueCount >= 2 ? "Pip-Pad Recovery" : "Bunker Passage";
    }
    if (!profile.story.archiveRecovered) {
        return route.earlyVerminEncounterResolved ? "Archive Sync" : "Archive Corridor";
    }
    if (!route.bt72Restored) {
        return "BT-72 Restoration";
    }
    if (!profile.story.tankLinked) {
        return "BT-72 Sync Link";
    }
    if (!route.clearanceBlueprintRecovered || !route.clearanceModuleInstalled) {
        return "Clearance Module";
    }
    if (!profile.story.exitedBunker) {
        return "Outer Bulkhead";
    }
    if (!profile.story.outerRoadCleared) {
        return "Heavy Clearance";
    }
    if (!route.firstTankCombatResolved) {
        return "First Tank Combat";
    }
    if (!route.firstServicePerformed) {
        return "First Service Halt";
    }
    if (!route.firstRecoveryNodeActivated) {
        return "Recovery Node";
    }
    if (!route.debriefSummaryViewed) {
        return "Debrief";
    }
    return "Industrial Expansion";
}

std::string CurrentStoryObjectivePreview(const SessionProfile& profile) {
    const auto& route = profile.firstPlayableRoute;
    if (!profile.story.awakenedFromCryo) {
        return "Wake from the cryo capsule and stabilize your first bunker route.";
    }
    if (!profile.story.pipPadRecovered) {
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
    if (!route.bt72Restored) {
        if (!HasBt72RestorationPrerequisites(profile)) {
            return "Survey the BT-72 hull, recover the starter core, and decode the service notes.";
        }
        if (!HasBt72RestorationMaterials(profile)) {
            return "Gather the salvage kit for BT-72 restoration: power cell, repair patch, and hull plate.";
        }
        return "Restore BT-72 to partial operating condition from bunker salvage.";
    }
    if (!profile.story.tankLinked) {
        return "Climb into the BT-72 cockpit and complete the first sync link.";
    }
    if (!route.clearanceBlueprintRecovered) {
        return "Decode the clearance module blueprint from the maintenance echo.";
    }
    if (!route.clearanceMaterialsRecovered) {
        return "Recover the clearance-module parts from the bucket rack.";
    }
    if (!route.clearanceModuleInstalled) {
        return "Install the BT-72 clearance module and prep for heavy debris work.";
    }
    if (!profile.story.exitedBunker) {
        return "Cycle the outer bulkhead and push BT-72 into the blocked corridor.";
    }
    if (!profile.story.outerRoadCleared) {
        return "Use the clearance module to break the outer debris barrier.";
    }
    if (!route.firstTankCombatResolved) {
        return "Hold the route through BT-72's first real combat contact.";
    }
    if (!route.firstServicePerformed) {
        return "Take a first service/rest stop before pushing the recovery node.";
    }
    if (!route.firstRecoveryNodeActivated) {
        return "Sync the first recovery node and prove the route changed the world.";
    }
    if (!route.debriefSummaryViewed) {
        return "Return for debrief, summarize the route, and pull the next recovery hook.";
    }
    const auto* worldState = SelectedWorldState(profile);
    if (worldState == nullptr) {
        return "Recovery buildout active. Runtime world state not loaded yet.";
    }
    return CurrentIndustrialObjective(profile, *worldState);
}

std::string CurrentStoryObjective(const SessionProfile& profile, const StaticEraser& staticEraser) {
    const auto& route = profile.firstPlayableRoute;
    if (!profile.story.awakenedFromCryo) {
        return "Wake from the cryo capsule and stabilize the bunker route.";
    }
    if (!profile.story.pipPadRecovered) {
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
    if (!route.bt72Restored) {
        if (!HasBt72RestorationPrerequisites(profile)) {
            return "Survey the BT-72 hull, recover the starter core, and decode the service notes.";
        }
        if (!HasBt72RestorationMaterials(profile)) {
            return "Gather a power cell, repair patch, and hull plate before restoring BT-72.";
        }
        return "Restore BT-72 to partial operating condition from bunker salvage.";
    }
    if (!profile.story.tankLinked) {
        return "Reach the garage anchor and establish the BT-72 cockpit link.";
    }
    if (!route.clearanceBlueprintRecovered) {
        return "Recover the clearance module blueprint from the maintenance echo.";
    }
    if (!route.clearanceMaterialsRecovered) {
        return "Recover the clearance-module parts from the bucket rack.";
    }
    if (!route.clearanceModuleInstalled) {
        return "Install the BT-72 clearance module before opening the outer route.";
    }
    if (!profile.story.exitedBunker) {
        return "Open the outer bulkhead and move BT-72 into the blocked recovery corridor.";
    }
    if (!profile.story.outerRoadCleared) {
        return "Raise the clearance module and break the outer debris barrier.";
    }
    if (!FirstTankCombatResolved(profile, staticEraser)) {
        return "Destroy the hostile contact guarding the first recovery node.";
    }
    if (!route.firstServicePerformed) {
        return "Run a first service cycle before pushing the relay node.";
    }
    if (!route.firstRecoveryNodeActivated) {
        return "Sync the relay terminal and recover the reconstruction schematics.";
    }
    if (!route.debriefSummaryViewed) {
        return "Return to the debrief console inside Shelter 17.";
    }
    const auto* worldState = SelectedWorldState(profile);
    if (worldState == nullptr) {
        return "Recovery buildout active. Runtime world state not loaded yet.";
    }
    return CurrentIndustrialObjective(profile, *worldState);
}

std::vector<StoryRouteEntry> BuildBt72RestorationRoute(const SessionProfile& profile) {
    const auto& route = profile.firstPlayableRoute;
    return {
        {"Inspect the BT-72 hull berth.", route.bt72HullInspected},
        {"Recover the starter core from the bunker rack.", route.bt72CoreRecovered},
        {"Decode BT-72 service notes and holo-records.", route.bt72ServiceNotesRecovered},
        {"Gather a power cell, repair patch, and hull plate.", route.bt72Restored || HasBt72RestorationMaterials(profile)},
        {"Restore BT-72 to partial operating condition.", route.bt72Restored},
        {"Complete the first cockpit sync link.", profile.story.tankLinked},
        {"Recover the clearance-module blueprint.", route.clearanceBlueprintRecovered},
        {"Recover bucket-rack parts for the clearance module.", route.clearanceMaterialsRecovered},
        {"Install the BT-72 clearance module.", route.clearanceModuleInstalled},
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
