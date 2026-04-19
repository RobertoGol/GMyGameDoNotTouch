#pragma once

#include "../include/GameRuntime.hpp"

namespace bunker {

void DrawPipPadTabBar(int& activeTab);
void DrawPipPadStatTab(const SessionProfile& profile, const GameState& gameState);
void DrawPipPadInventoryTab(SessionProfile& profile, GameState& gameState);
void DrawPipPadDataTab(PlayerState& player, SessionProfile& profile, GameState& gameState);
void DrawPipPadMapTab(const World& world);
void DrawPipPadNetTab(const SessionProfile& profile, GameState& gameState);
void DrawPipPadServicesTab(const World& world, const PlayerState& player, SessionProfile& profile, GameState& gameState);

}  // namespace bunker
