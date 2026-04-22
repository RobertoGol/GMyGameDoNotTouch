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

    if (profile.story.exitedBunker && !gameState.zoneEventExterior) {
        gameState.zoneEventExterior = true;
        gameState.lastEvent = "SURFACE APPROACH: 'Lift route reached. Radiation low, debris high, skyline active ahead. Clear the route, survive first contact in BT-72, then push a service stop before the relay node.'";
        return;
    }

    if (profile.story.relayRecovered && !profile.story.returnedToBase && !gameState.zoneEventReturn &&
        IsNear(player, -2.5f, 3.2f, 3.0f)) {
        gameState.zoneEventReturn = true;
        gameState.lastEvent = "DEBRIEF: 'Recovery node recognized. Upload the route summary and authorize Shelter 17 planning: rail, orbital, fabrication, then the inner spur.'";
        return;
    }

    if (profile.story.returnedToBase && !gameState.zoneEventReturn &&
        IsNear(player, -2.5f, 3.2f, 3.0f)) {
        gameState.zoneEventReturn = true;
        gameState.lastEvent = "DEBRIEF: 'Recovery planning remains active. Keep rebuilding rail freight, orbital support, fabrication, and the inner spur backbone.'";
    }
}

}  // namespace bunker
