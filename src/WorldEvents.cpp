#include "../include/WorldEvents.hpp"

#include <cmath>

namespace bunker {

namespace {

bool IsNear(const PlayerState& player, float x, float y, float radius) {
    const float dx = player.x - x;
    const float dy = player.y - y;
    return (dx * dx) + (dy * dy) <= radius * radius;
}

void AppendObjectiveHint(std::string& message, const std::string& objective) {
    if (!objective.empty() && message.find(objective) == std::string::npos) {
        message += " Next: " + objective;
    }
}

void AppendObjectivePreviewHint(std::string& message, const SessionProfile& profile) {
    AppendObjectiveHint(message, CurrentStoryObjectivePreview(profile));
}

void AppendRouteBeatReadabilityHint(std::string& message, const SessionProfile& profile) {
    const auto beat = CurrentFirstPlayableRouteBeat(profile);
    if (!beat.label.empty() && message.find("Route beat: " + beat.label) == std::string::npos) {
        message += " Route beat: " + beat.label + ".";
    }
    if (!beat.payoff.empty() && message.find(beat.payoff) == std::string::npos) {
        message += " Readable payoff: " + beat.payoff;
    }
}

bool IsAtSurfaceArrivalAnchor(const World& world, const PlayerState& player) {
    if (const auto* campMarker = world.FindObjectByRegistryId("[%camp_0001]"); campMarker != nullptr) {
        return IsNear(player, campMarker->x, campMarker->y, 5.0f);
    }
    return player.x >= 18.0f;
}

bool IsAtFirstCombatAnchor(const World& world, const PlayerState& player) {
    if (const auto* firstContact = world.FindObjectByRegistryId("[%enemy_ghoul_0001]"); firstContact != nullptr) {
        return IsNear(player, firstContact->x, firstContact->y, 6.5f);
    }
    return player.x >= 17.0f;
}

}  // namespace

void ProcessScriptedWorldEvents(const World& world, const PlayerState& player, SessionProfile& profile, GameState& gameState) {
    if (!world.IsStarterScenarioWorld()) {
        return;
    }

    if (profile.story.awakenedFromCryo && !profile.story.pipPadRecovered && !gameState.zoneEventCryoLocker &&
        IsNear(player, -9.0f, -6.0f, 2.5f)) {
        gameState.zoneEventCryoLocker = true;
        gameState.lastEvent = profile.firstPlayableRoute.accessCardRecovered
            ? "LOCKER BAY: 'Emergency recovery locker detected. Card interlock accepts the bunker access card. Pip-Pad signature nearby.'"
            : "LOCKER BAY: 'Emergency recovery locker detected. Mechanical interlock wants a bunker access card from the core service racks.'";
        return;
    }

    if (profile.story.pipPadRecovered && !profile.story.archiveRecovered && !gameState.zoneEventArchive &&
        IsNear(player, -7.0f, -1.0f, 3.0f)) {
        gameState.zoneEventArchive = true;
        gameState.lastEvent = "ARCHIVE: 'Missing personnel logs available. Expect vermin contamination in the corridor and garage references in the recovered notes.'";
        return;
    }

    if (profile.story.archiveRecovered && !profile.firstPlayableRoute.bt72Restored && !gameState.zoneEventGarage &&
        IsNear(player, 4.0f, -1.5f, 4.0f)) {
        gameState.zoneEventGarage = true;
        gameState.lastEvent = "GARAGE: 'BT-72 hull located. Pairing is still locked: inspect the hull, recover the core, and pull the maintenance echo first.'";
        return;
    }

    if (profile.story.exitedBunker &&
        !profile.firstPlayableRoute.surfaceArrivalReached &&
        IsAtSurfaceArrivalAnchor(world, player)) {
        profile.firstPlayableRoute.surfaceArrivalReached = true;
        gameState.zoneEventExterior = true;
        gameState.lastEvent = "SURFACE ARRIVAL: 'BT-72 reached the first exterior foothold. Skyline active, debris barrier ahead, and the first hostile contact is forming beyond the lift route.'";
        AppendObjectivePreviewHint(gameState.lastEvent, profile);
        AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
        return;
    }

    if (profile.story.outerRoadCleared &&
        !profile.firstPlayableRoute.firstTankCombatResolved &&
        !gameState.zoneEventFirstCombat &&
        IsAtFirstCombatAnchor(world, player)) {
        gameState.zoneEventFirstCombat = true;
        gameState.lastEvent = player.insideTank
            ? "CONTACT: 'Outer Ghoul charging the cleared lane. Hold BT-72 through the rush, then take one service halt before syncing the relay.'"
            : "CONTACT: 'Outer Ghoul detected beyond the cleared debris lane. BT-72 is the intended answer for the first surface contact.'";
        AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
        return;
    }

    if (profile.story.relayRecovered && !profile.story.returnedToBase && !gameState.zoneEventReturn &&
        IsNear(player, -2.5f, 3.2f, 3.0f)) {
        gameState.zoneEventReturn = true;
        gameState.lastEvent = "DEBRIEF: 'Recovery node recognized. Upload the route summary and authorize Shelter 17 planning: rail, orbital, fabrication, then the inner spur.'";
        AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
        return;
    }

    if (profile.story.returnedToBase && !gameState.zoneEventReturn &&
        IsNear(player, -2.5f, 3.2f, 3.0f)) {
        gameState.zoneEventReturn = true;
        gameState.lastEvent = "DEBRIEF: 'Recovery planning remains active. Keep rebuilding rail freight, orbital support, fabrication, and the inner spur backbone.'";
    }
}

}  // namespace bunker
