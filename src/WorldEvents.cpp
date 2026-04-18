#include "../include/WorldEvents.hpp"

#include <cmath>

namespace bunker {

namespace {

bool IsNear(const PlayerState& player, float x, float y, float radius) {
    const float dx = player.x - x;
    const float dy = player.y - y;
    return (dx * dx) + (dy * dy) <= radius * radius;
}

}  // namespace

void ProcessScriptedWorldEvents(const World& world, const PlayerState& player, const SessionProfile& profile, GameState& gameState) {
    if (!world.IsStarterScenarioWorld()) {
        return;
    }

    if (profile.story.awakenedFromCryo && !profile.story.pipPadRecovered && !gameState.zoneEventCryoLocker &&
        IsNear(player, -9.0f, -6.0f, 2.5f)) {
        gameState.zoneEventCryoLocker = true;
        gameState.lastEvent = "LOCKER BAY: 'Emergency recovery locker detected. Pip-Pad signature nearby.'";
        return;
    }

    if (profile.story.pipPadRecovered && !profile.story.archiveRecovered && !gameState.zoneEventArchive &&
        IsNear(player, -7.0f, -1.0f, 3.0f)) {
        gameState.zoneEventArchive = true;
        gameState.lastEvent = "ARCHIVE: 'Missing personnel logs available. Expect hostile contamination in the corridor.'";
        return;
    }

    if (profile.story.archiveRecovered && !profile.story.tankLinked && !gameState.zoneEventGarage &&
        IsNear(player, 4.0f, -1.5f, 4.0f)) {
        gameState.zoneEventGarage = true;
        gameState.lastEvent = "GARAGE: 'BT-72 partner hull responding. Pairing can begin from the anchor frame. Recover the bucket rig and this route can be reclaimed.'";
        return;
    }

    if (profile.story.exitedBunker && !gameState.zoneEventExterior) {
        gameState.zoneEventExterior = true;
        gameState.lastEvent = "OUTER ROUTE: 'Radiation low. Debris high. Secure the corridor, recover the relay packet, and establish a field checkpoint beyond the bunker.'";
        return;
    }

    if (profile.story.relayRecovered && !profile.story.returnedToBase && !gameState.zoneEventReturn &&
        IsNear(player, -2.5f, 3.2f, 3.0f)) {
        gameState.zoneEventReturn = true;
        gameState.lastEvent = "DEBRIEF: 'Relay packet recognized. Upload the reconstruction data and authorize Shelter 17 recovery planning: rail, orbital, fabrication, then the inner spur.'";
        return;
    }

    if (profile.story.returnedToBase && !gameState.zoneEventReturn &&
        IsNear(player, -2.5f, 3.2f, 3.0f)) {
        gameState.zoneEventReturn = true;
        gameState.lastEvent = "DEBRIEF: 'Recovery planning remains active. Keep rebuilding rail freight, orbital support, fabrication, and the inner spur backbone.'";
    }
}

}  // namespace bunker
