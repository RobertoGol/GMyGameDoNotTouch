#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <string_view>
#include <GLFW/glfw3.h>

#include "GameExecution.hpp"
#include "Renderer.hpp"
#include "Progression.hpp"
#include "SessionProfiles.hpp"
#include "SkillSystem.hpp"
#include "StoryRoute.hpp"
#include "StaticEraser.hpp"
#include "World.hpp"

namespace bunker {

struct HostileAwarenessState {
    std::string registryId;
    float awareness = 0.0f;
    float lostTimer = 0.0f;
    float attackWindup = 0.0f;
};

struct MechanicalHostileDamageState {
    std::string registryId;
    float sensorDamage = 0.0f;
    float weaponDamage = 0.0f;
    float mobilityDamage = 0.0f;
};

enum class RuntimeGamePhase {
    MAIN_MENU,
    WORLD_LOADING,
    ACTIVE_GAME,
    UI_INTERACTION
};

struct OverlayState {
    bool visible = false;
    std::string title;
    std::string body;
};

struct GameState {
    RuntimeGamePhase phase = RuntimeGamePhase::MAIN_MENU;
    bool cycleViewPressed = false;
    bool usePressed = false;
    bool contextualPressed = false;
    bool bucketPressed = false;
    bool seatSwapPressed = false;
    bool uiPressed = false;
    bool savePressed = false;
    bool healPressed = false;
    bool reloadPressed = false;
    float radioTimer = 12.0f;
    float attackCooldown = 0.0f;
    float specialAttackCooldown = 0.0f;
    float damageCooldown = 0.0f;
    float damageFlashTimer = 0.0f;
    float weatherTimer = 70.0f;
    float weatherEventTimer = 0.0f;
    float weatherIntensity = 0.0f;
    std::size_t radioPhase = 0;
    bool zoneEventCryoLocker = false;
    bool zoneEventArchive = false;
    bool zoneEventGarage = false;
    bool zoneEventExterior = false;
    bool zoneEventFirstCombat = false;
    bool zoneEventReturn = false;
    bool stressThresholdTriggered = false;
    bool secondWindTriggered = false;
    bool soulLineTriggered = false;
    float rationEffectTimer = 0.0f;
    int rationStrengthBonus = 0;
    int rationIntelligencePenalty = 0;
    float scavengerTimer = 180.0f;
    float caravanTimer = 260.0f;
    float droneTimer = 210.0f;
    float tradeTimer = 240.0f;
    float railTimer = 320.0f;
    float orbitalTimer = 420.0f;
    float railFortressTimer = 520.0f;
    float fabricatorTimer = 280.0f;
    float surveyTimer = 360.0f;
    float outpostTimer = 300.0f;
    float assemblyTimer = 340.0f;
    float foundryTimer = 420.0f;
    float reactorTimer = 460.0f;
    float capacitorTimer = 390.0f;
    float relaySubstationTimer = 430.0f;
    float serviceBayTimer = 300.0f;
    float waterReclaimerTimer = 260.0f;
    float etherErosionEventTimer = 0.0f;
    float fieldWorkbenchCooldown = 0.0f;
    float workshopServiceCooldown = 0.0f;
    float heavyCarryTimer = 0.0f;
    float supportRefreshTimer = 0.0f;
    float feyRingRefreshTimer = 0.0f;
    float tankThermalLoad = 12.0f;
    bool lanlineServicesVisible = false;
    bool feyRingScheduleVisible = false;
    bool supportTerminalNearby = false;
    bool tankServiceNearby = false;
    bool medicalSupportNearby = false;
    WeatherAnomaly weather = WeatherAnomaly::Clear;
    OverlayState overlay{};
    std::vector<HostileAwarenessState> hostileAwareness{};
    std::vector<MechanicalHostileDamageState> mechanicalHostileDamage{};
    std::string lastEvent = "Cryostasis breached. Reorient, secure an access card, recover the Pip-Pad, and trace the BT-72 berth.";
    std::string lastSupportAction = "No support activity.";
    std::string lastPortalAction = "No portal updates.";
    std::vector<std::string> radioMessages = {
        "SYSTEM: 'Cryo wing unstable. Sweep the shared cryo tier, recover an access card, and follow the paper trail to the Pip-Pad.'",
        "ARCHIVE: 'One reactor core is missing. One body is missing. Garage service traces remain incomplete.'",
        "BT-72: 'Hull detected. Restore the chassis before any cockpit link attempt.'",
        "HQ: 'The outer bulkhead is blocked. Install a clearance module before forcing that route.'",
        "ARCHIVE: 'Relay and city-fringe schematics remain fragmented. Continue recovery.'"
    };
};

const char* RuntimeGamePhaseLabel(RuntimeGamePhase phase);
void AddInventoryItem(SessionProfile& profile, const std::string& itemId, int count, float weight);
bool HasInventoryItem(const SessionProfile& profile, const std::string& itemId);
bool ConsumeInventoryItem(SessionProfile& profile, const std::string& itemId, int count);
float CurrentInventoryWeight(const SessionProfile& profile);
int EffectiveStatValue(const SessionProfile& profile, const GameState& gameState, char statCode);
bool TryConsumeFieldRation(SessionProfile& profile, GameState& gameState);
bool HasPipPad(const SessionProfile& profile);
bool RecoverPipPad(SessionProfile& profile);
bool PlayerHasPipPadAccess(const SessionProfile& profile);
bool TryTogglePipPadUi(PlayerState& player, const SessionProfile& profile, GameState& gameState);
void AdvanceViewMode(PlayerState& player);
void TryToggleBt72CrewSeat(PlayerState& player, SessionProfile& profile, GameState& gameState);
void ApplyStaticEraser(World& world, const StaticEraser& staticEraser);
bool ShouldUseStarterStoryFlow(const World& world);
void SyncStoryFlagsFromWorld(SessionProfile& profile, const StaticEraser& staticEraser);
void UpdateWorldMetadata(World& world, const SessionProfile& profile, const StaticEraser& staticEraser);
void UpdateWindowTitle(GLFWwindow* window, const PlayerState& player, const World& world, const SessionProfile& sessionProfile);
void UpdateRadio(GameState& gameState, const World& world, const SessionProfile& profile, const StaticEraser& staticEraser, float dt);
const MapObject* FindNearestHostile(const World& world, float x, float y, float radius);
const char* TankIntegrityBand(float integrity);
std::string DescribeHostileReadability(const MapObject& object, const GameState& gameState);
void UpdateHostiles(World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState,
    float dt);
void SyncPartnerTankAnchor(World& world,
    const PlayerState& player,
    SessionProfile& profile);
void UpdateAmbientTankCharging(const World& world,
    SessionProfile& profile,
    GameState& gameState,
    float dt);
void UpdateWeatherAnomaly(const World& world,
    PlayerState& player,
    SessionProfile& profile,
    GameState& gameState,
    float dt);
void UpdateEtherErosion(const World& world,
    const PlayerState& player,
    SessionProfile& profile,
    GameState& gameState,
    float dt);
void UpdateInfrastructureDecay(const World& world,
    const PlayerState& player,
    SessionProfile& profile,
    GameState& gameState,
    float dt);
void UpdateRouteContamination(World& world,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState,
    float dt);
void UpdateScavengerTeams(SessionProfile& profile, GameState& gameState, float dt);
void UpdateCaravanRoute(SessionProfile& profile, GameState& gameState, float dt);
void UpdateDroneStations(SessionProfile& profile, GameState& gameState, float dt);
void UpdateTradeNetwork(SessionProfile& profile, GameState& gameState, float dt);
void UpdateRailFreight(SessionProfile& profile, GameState& gameState, float dt);
void UpdateOrbitalUplink(SessionProfile& profile, GameState& gameState, float dt);
void UpdateRailFortress(SessionProfile& profile, GameState& gameState, float dt);
void UpdateRecoveryFabricator(SessionProfile& profile, GameState& gameState, float dt);
void UpdateRecoveryMilestones(SessionProfile& profile, GameState& gameState);
void UpdateIndustrialSurvey(SessionProfile& profile, GameState& gameState, float dt);
void UpdateIndustrialOutpost(SessionProfile& profile, GameState& gameState, float dt);
void UpdateAssemblyCell(SessionProfile& profile, GameState& gameState, float dt);
void UpdateFoundryLine(SessionProfile& profile, GameState& gameState, float dt);
void UpdateReactorYard(SessionProfile& profile, GameState& gameState, float dt);
void UpdateCapacitorBank(SessionProfile& profile, GameState& gameState, float dt);
void UpdateRelaySubstation(SessionProfile& profile, GameState& gameState, float dt);
void UpdateServiceBay(SessionProfile& profile, GameState& gameState, float dt);
void UpdateWaterReclaimer(SessionProfile& profile, GameState& gameState, float dt);
void UpdateRouteRandomEvents(SessionProfile& profile, GameState& gameState, float dt);
bool TryResolveMerchantRouteEvent(SessionProfile& profile, GameState& gameState);
void HandleAttack(World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState);
void HandleSpecialAttack(World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState);
void HandleInteraction(const MapObject* nearest,
    World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState,
    const WorldExecutionContext* executionContext = nullptr);
bool WantsUseKey(const MapObject* nearest);
bool WantsContextKey(const MapObject* nearest);
void DrawPipPad(const World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState);
bool SweepMovePlayerAgainstWorld(const World& world, PlayerState& player, float deltaX, float deltaY);

}  // namespace bunker
