#include "../include/StoryRoute.hpp"

namespace bunker {

bool HasLanlineServicesObjective(const SessionProfile& profile) {
    const auto* state = FindWorldFieldState(profile, profile.selectedWorld);
    return state != nullptr && state->towerSyncRecovered;
}

bool HasFeyRingIntercityObjective(const SessionProfile& profile) {
    const auto* state = FindWorldFieldState(profile, profile.selectedWorld);
    return state != nullptr && state->feyRingIntercityUnlocked;
}

bool HasFeyRingInterserverObjective(const SessionProfile& profile) {
    const auto* state = FindWorldFieldState(profile, profile.selectedWorld);
    return state != nullptr && state->feyRingInterserverUnlocked;
}

std::string CurrentStoryObjective(const SessionProfile& profile, const StaticEraser& staticEraser) {
    const auto* worldState = FindWorldFieldState(profile, profile.selectedWorld);
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
    if (!profile.story.awakenedFromCryo) {
        return "Wake from the cryo capsule and stabilize the recovery route.";
    }
    if (!profile.story.pipPadRecovered) {
        return "Recover the missing Pip-Pad from the locker bay.";
    }
    if (!profile.story.archiveRecovered) {
        return "Read the archive terminal and reconstruct what happened in Shelter 17.";
    }
    if (!staticEraser.IsErased("[%enemy_laska_0001]")) {
        return "Neutralize the feral laska blocking the archive corridor.";
    }
    if (!profile.story.tankLinked) {
        return "Reach the garage and establish the BT-72 tank link.";
    }
    if (!profile.story.bucketRecovered) {
        return "Recover the bucket plow rack before opening the outer route.";
    }
    if (!profile.story.exitedBunker) {
        return "Open the outer bulkhead and move into the recovery corridor.";
    }
    if (!profile.story.outerRoadCleared) {
        return "Raise the bucket and clear the outer debris barrier.";
    }
    if (!staticEraser.IsErased("[%enemy_ghoul_0001]")) {
        return "Neutralize the ghoul guarding the outer relay.";
    }
    if (!profile.story.relayRecovered) {
        return "Sync the relay terminal and recover the reconstruction schematics.";
    }
    if (!profile.story.returnedToBase) {
        return "Return to the debrief console inside Shelter 17.";
    }
    if (worldState != nullptr) {
        if (!railOperational) {
            return "Restore the industrial rail depot and bring heavy freight back to Shelter 17.";
        }
        if (!orbitalOperational) {
            return "Align the orbital uplink to extend long-range recovery scans.";
        }
        if (!fortressOperational) {
            return "Deploy the Rail Fortress to secure the restored industrial spur.";
        }
        if (!fabricatorOperational) {
            return "Prime the Recovery Fabricator to turn salvage into operational supplies.";
        }
        if (!worldState->industrialGateUnlocked) {
            return "Unlock the industrial gate and push Shelter 17 into the inner spur.";
        }
        if (!surveyOperational) {
            return "Align the industrial survey beacon and start mapping the inner spur.";
        }
        if (!outpostOperational) {
            return "Establish the inner spur outpost and secure a forward foothold beyond the gate.";
        }
        if (!assemblyOperational) {
            return "Bring the inner spur assembly cell online for local industrial recovery.";
        }
        if (!foundryOperational) {
            return "Restart the inner spur foundry line to resume heavy plate fabrication.";
        }
        if (!reactorOperational) {
            return "Bring the inner spur reactor yard online to stabilize deeper industrial energy flow.";
        }
        if (!capacitorOperational) {
            return "Charge the inner spur capacitor bank to buffer and stabilize the heavy grid.";
        }
        if (!relayOperational) {
            return "Sync the relay substation and route inner spur power back into Shelter 17.";
        }
        if (!serviceOperational) {
            return "Bring the inner spur service bay online to push BT-72 repairs deeper into the factory belt.";
        }
        if (!waterOperational) {
            return "Bring the inner spur water reclaimer online to stabilize long-range recovery and camp support.";
        }
    }
    return "Water reclaimer online. Shelter 17 now has a stable recovery backbone; expand deeper into the inner spur and wider factory belt.";
}

std::vector<StoryRouteEntry> BuildStarterRoute(const SessionProfile& profile, const StaticEraser& staticEraser) {
    const auto* worldState = FindWorldFieldState(profile, profile.selectedWorld);
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
        {"Recover the missing Pip-Pad.", profile.story.pipPadRecovered},
        {"Read the archive terminal.", profile.story.archiveRecovered},
        {"Neutralize the feral laska in the archive corridor.", staticEraser.IsErased("[%enemy_laska_0001]")},
        {"Establish the BT-72 tank link.", profile.story.tankLinked},
        {"Recover the bucket plow rack.", profile.story.bucketRecovered || staticEraser.IsErased("#%it_bucket_0001")},
        {"Open the outer bulkhead.", profile.story.exitedBunker},
        {"Clear the outer debris barrier.", profile.story.outerRoadCleared || staticEraser.IsErased("#%res_scrap_0001")},
        {"Neutralize the ghoul guarding the outer relay.", staticEraser.IsErased("[%enemy_ghoul_0001]")},
        {"Sync the outer relay terminal.", profile.story.relayRecovered},
        {"Return to the debrief console inside the bunker.", profile.story.returnedToBase},
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
