#pragma once

#include "GameRuntime.hpp"

namespace bunker {

void ProcessScriptedWorldEvents(const World& world, const PlayerState& player, SessionProfile& profile, GameState& gameState);

}  // namespace bunker
