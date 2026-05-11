#include "../include/GameRuntime.hpp"
#include "../include/GameplayDescriptorRegistry.hpp"
#include "../include/BuildAnnouncement.hpp"
#include "../include/HangarSystem.hpp"
#include "../include/LanlineLobbyLogic.hpp"
#include "../include/LanlineServices.hpp"
#include "../include/LanlineSession.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string_view>

#include "GameRuntimePipPad.hpp"

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

std::string AsciiLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

void AppendObjectiveHint(std::string& message, const std::string& objective) {
    if (!objective.empty() && message.find(objective) == std::string::npos) {
        message += " Next: " + objective;
    }
}

void AppendObjectivePreviewHint(std::string& message, const SessionProfile& profile) {
    AppendObjectiveHint(message, CurrentStoryObjectivePreview(profile));
}

void AppendObjectiveRuntimeHint(std::string& message, const SessionProfile& profile, const StaticEraser& staticEraser) {
    AppendObjectiveHint(message, CurrentStoryObjective(profile, staticEraser));
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

void AppendRecoveryBackboneReadabilityHint(std::string& message, const SessionProfile& profile) {
    const auto backbone = CurrentRecoveryBackboneStatus(profile);
    if (!backbone.stage.empty() && message.find("Backbone stage: " + backbone.stage) == std::string::npos) {
        message += " Backbone stage: " + backbone.stage + ".";
    }
    if (!backbone.payoff.empty() && message.find(backbone.payoff) == std::string::npos) {
        message += " Backbone payoff: " + backbone.payoff;
    }
}

enum class HostileRole {
    VerminRush,
    GhoulRush,
    HumanTactical,
    RobotControl,
    Unknown
};

enum class ReactiveBreakableKind {
    None,
    Glass,
    LightVegetation
};

HostileRole ClassifyHostileRole(const MapObject& object) {
    const std::string normalizedTag = std::string(NormalizeGameplayDescriptorTag(object.scriptTag));
    const std::string registryIdLower = AsciiLower(object.registryId);
    const std::string displayNameLower = AsciiLower(object.displayName);
    if (normalizedTag == "vermin_rush" || registryIdLower.find("laska") != std::string::npos) {
        return HostileRole::VerminRush;
    }
    if (normalizedTag == "ghoul_rush" ||
        registryIdLower.find("ghoul") != std::string::npos ||
        displayNameLower.find("ghoul") != std::string::npos) {
        return HostileRole::GhoulRush;
    }
    if (normalizedTag == "human_tactical" ||
        registryIdLower.find("raider") != std::string::npos ||
        registryIdLower.find("bandit") != std::string::npos ||
        displayNameLower.find("rifleman") != std::string::npos ||
        displayNameLower.find("raider") != std::string::npos) {
        return HostileRole::HumanTactical;
    }
    if (normalizedTag == "robot_control" ||
        registryIdLower.find("robot") != std::string::npos ||
        registryIdLower.find("drone") != std::string::npos ||
        registryIdLower.find("sentinel") != std::string::npos ||
        displayNameLower.find("robot") != std::string::npos ||
        displayNameLower.find("drone") != std::string::npos ||
        displayNameLower.find("sentinel") != std::string::npos) {
        return HostileRole::RobotControl;
    }
    return HostileRole::Unknown;
}

ReactiveBreakableKind ClassifyReactiveBreakable(const MapObject& object) {
    if (object.interaction == InteractionType::Hostile) {
        return ReactiveBreakableKind::None;
    }

    const std::string registryIdLower = AsciiLower(object.registryId);
    const std::string displayNameLower = AsciiLower(object.displayName);
    const std::string scriptTagLower = AsciiLower(object.scriptTag);
    const bool looksLikeGlass = registryIdLower.find("glass") != std::string::npos ||
        displayNameLower.find("glass") != std::string::npos ||
        displayNameLower.find("window") != std::string::npos ||
        scriptTagLower.find("glass") != std::string::npos;
    if (looksLikeGlass) {
        return ReactiveBreakableKind::Glass;
    }

    const bool looksLikeVegetation = registryIdLower.find("brush") != std::string::npos ||
        registryIdLower.find("shrub") != std::string::npos ||
        displayNameLower.find("brush") != std::string::npos ||
        displayNameLower.find("shrub") != std::string::npos ||
        displayNameLower.find("foliage") != std::string::npos ||
        displayNameLower.find("vine") != std::string::npos ||
        scriptTagLower.find("foliage") != std::string::npos ||
        scriptTagLower.find("vegetation") != std::string::npos;
    if (looksLikeVegetation) {
        return ReactiveBreakableKind::LightVegetation;
    }

    return ReactiveBreakableKind::None;
}

float HostileAlertRadius(HostileRole role, bool insideTank) {
    switch (role) {
        case HostileRole::VerminRush: return insideTank ? 6.0f : 4.8f;
        case HostileRole::GhoulRush: return insideTank ? 8.2f : 6.6f;
        case HostileRole::HumanTactical: return insideTank ? 9.0f : 7.4f;
        case HostileRole::RobotControl: return insideTank ? 10.0f : 8.6f;
        case HostileRole::Unknown:
        default: return insideTank ? 7.5f : 6.0f;
    }
}

float HostileAdvanceSpeed(HostileRole role) {
    switch (role) {
        case HostileRole::VerminRush: return 2.2f;
        case HostileRole::GhoulRush: return 1.6f;
        case HostileRole::HumanTactical: return 1.15f;
        case HostileRole::RobotControl: return 0.95f;
        case HostileRole::Unknown:
        default: return 1.5f;
    }
}

float HostileAttackRange(HostileRole role, bool insideTank) {
    switch (role) {
        case HostileRole::VerminRush: return insideTank ? 1.1f : 0.95f;
        case HostileRole::GhoulRush: return insideTank ? 1.55f : 1.35f;
        case HostileRole::HumanTactical: return insideTank ? 3.2f : 2.8f;
        case HostileRole::RobotControl: return insideTank ? 4.2f : 3.6f;
        case HostileRole::Unknown:
        default: return insideTank ? 1.4f : 1.2f;
    }
}

float HostileAttackWindupDuration(HostileRole role, bool insideTank) {
    switch (role) {
        case HostileRole::VerminRush: return insideTank ? 0.28f : 0.18f;
        case HostileRole::GhoulRush: return insideTank ? 0.36f : 0.22f;
        case HostileRole::HumanTactical: return insideTank ? 0.72f : 0.54f;
        case HostileRole::RobotControl: return insideTank ? 0.86f : 0.64f;
        case HostileRole::Unknown:
        default: return insideTank ? 0.42f : 0.30f;
    }
}

float HostilePreferredMinRange(HostileRole role) {
    switch (role) {
        case HostileRole::HumanTactical: return 2.6f;
        case HostileRole::RobotControl: return 3.4f;
        default: return 0.0f;
    }
}

float HostilePreferredMaxRange(HostileRole role) {
    switch (role) {
        case HostileRole::HumanTactical: return 3.8f;
        case HostileRole::RobotControl: return 5.4f;
        default: return 0.0f;
    }
}

float HostileAttackCooldown(HostileRole role) {
    switch (role) {
        case HostileRole::VerminRush: return 0.9f;
        case HostileRole::GhoulRush: return 1.2f;
        case HostileRole::HumanTactical: return 1.4f;
        case HostileRole::RobotControl: return 1.6f;
        case HostileRole::Unknown:
        default: return 1.1f;
    }
}

float HostileTankDamage(HostileRole role) {
    switch (role) {
        case HostileRole::VerminRush: return 5.0f;
        case HostileRole::GhoulRush: return 8.0f;
        case HostileRole::HumanTactical: return 6.5f;
        case HostileRole::RobotControl: return 7.0f;
        case HostileRole::Unknown:
        default: return 5.5f;
    }
}

float HostileFootDamage(HostileRole role, const SessionProfile& profile, const GameState& gameState) {
    const float endurance = static_cast<float>(EffectiveStatValue(profile, gameState, 'E'));
    switch (role) {
        case HostileRole::VerminRush: return std::max(3.5f, 9.5f - endurance * 0.55f);
        case HostileRole::GhoulRush: return std::max(4.5f, 11.5f - endurance * 0.55f);
        case HostileRole::HumanTactical: return std::max(5.0f, 10.0f - endurance * 0.45f);
        case HostileRole::RobotControl: return std::max(5.5f, 10.5f - endurance * 0.35f);
        case HostileRole::Unknown:
        default: return std::max(4.0f, 10.5f - endurance * 0.5f);
    }
}

float HostileMaxHealthHint(HostileRole role) {
    switch (role) {
        case HostileRole::VerminRush: return 35.0f;
        case HostileRole::GhoulRush: return 55.0f;
        case HostileRole::HumanTactical: return 48.0f;
        case HostileRole::RobotControl: return 72.0f;
        case HostileRole::Unknown:
        default: return 50.0f;
    }
}

float HostileLateralBias(const MapObject& object) {
    if (!object.registryId.empty() && (static_cast<unsigned char>(object.registryId.back()) % 2 == 0)) {
        return 1.0f;
    }
    return -1.0f;
}

std::string DescribeHostileImpact(const MapObject& object, HostileRole role, bool insideTank, bool guarded) {
    if (insideTank) {
        switch (role) {
            case HostileRole::VerminRush:
                return guarded
                    ? object.displayName + " clawed across the armor, but BT-72 shed most of the impact."
                    : object.displayName + " clawed across BT-72's hull.";
            case HostileRole::GhoulRush:
                return guarded
                    ? object.displayName + " slammed the hull, but BT-72 bled the impact through armor."
                    : object.displayName + " slammed into BT-72's hull.";
            case HostileRole::HumanTactical:
                return guarded
                    ? object.displayName + " peppered the hull, but the armor package held the burst."
                    : object.displayName + " peppered BT-72 with suppressive fire.";
            case HostileRole::RobotControl:
                return guarded
                    ? object.displayName + " scored the hull with control-zone fire, but the armor package held."
                    : object.displayName + " raked BT-72 with control-zone fire.";
            case HostileRole::Unknown:
            default:
                return guarded
                    ? object.displayName + " clipped the tank, but the armor package absorbed most of it."
                    : object.displayName + " scraped the tank hull.";
        }
    }

    switch (role) {
        case HostileRole::VerminRush: return object.displayName + " lunged at the operator.";
        case HostileRole::GhoulRush: return object.displayName + " crashed into the operator.";
        case HostileRole::HumanTactical: return object.displayName + " tagged the operator with a close burst.";
        case HostileRole::RobotControl: return object.displayName + " burned across the operator's cover line.";
        case HostileRole::Unknown:
        default: return object.displayName + " hit the operator.";
    }
}

HostileAwarenessState& EnsureHostileAwarenessState(GameState& gameState, const std::string& registryId) {
    auto existing = std::find_if(
        gameState.hostileAwareness.begin(),
        gameState.hostileAwareness.end(),
        [&](const HostileAwarenessState& state) { return state.registryId == registryId; });
    if (existing != gameState.hostileAwareness.end()) {
        return *existing;
    }
    gameState.hostileAwareness.push_back({registryId, 0.0f, 0.0f});
    return gameState.hostileAwareness.back();
}

const MechanicalHostileDamageState* FindMechanicalHostileDamageState(const GameState& gameState, std::string_view registryId) {
    const auto existing = std::find_if(
        gameState.mechanicalHostileDamage.begin(),
        gameState.mechanicalHostileDamage.end(),
        [&](const MechanicalHostileDamageState& state) { return state.registryId == registryId; });
    return existing != gameState.mechanicalHostileDamage.end() ? &(*existing) : nullptr;
}

MechanicalHostileDamageState& EnsureMechanicalHostileDamageState(GameState& gameState, const std::string& registryId) {
    auto existing = std::find_if(
        gameState.mechanicalHostileDamage.begin(),
        gameState.mechanicalHostileDamage.end(),
        [&](const MechanicalHostileDamageState& state) { return state.registryId == registryId; });
    if (existing != gameState.mechanicalHostileDamage.end()) {
        return *existing;
    }
    gameState.mechanicalHostileDamage.push_back({registryId, 0.0f, 0.0f, 0.0f});
    return gameState.mechanicalHostileDamage.back();
}

bool SupportsMechanicalDamage(HostileRole role) {
    return role == HostileRole::RobotControl;
}

float MechanicalThresholdScale(float damage, float wornScale, float brokenScale) {
    if (damage >= 75.0f) {
        return brokenScale;
    }
    if (damage >= 40.0f) {
        return wornScale;
    }
    return 1.0f;
}

float HostileVisualScale(HostileRole role, const MechanicalHostileDamageState* damageState) {
    if (!SupportsMechanicalDamage(role) || damageState == nullptr) {
        return 1.0f;
    }
    return MechanicalThresholdScale(damageState->sensorDamage, 0.8f, 0.58f);
}

float HostileWeaponRangeScale(HostileRole role, const MechanicalHostileDamageState* damageState) {
    if (!SupportsMechanicalDamage(role) || damageState == nullptr) {
        return 1.0f;
    }
    return MechanicalThresholdScale(damageState->weaponDamage, 0.78f, 0.54f);
}

float HostileWeaponDamageScale(HostileRole role, const MechanicalHostileDamageState* damageState) {
    if (!SupportsMechanicalDamage(role) || damageState == nullptr) {
        return 1.0f;
    }
    return MechanicalThresholdScale(damageState->weaponDamage, 0.76f, 0.52f);
}

float HostileMovementScale(HostileRole role, const MechanicalHostileDamageState* damageState) {
    if (!SupportsMechanicalDamage(role) || damageState == nullptr) {
        return 1.0f;
    }
    return MechanicalThresholdScale(damageState->mobilityDamage, 0.72f, 0.46f);
}

std::string MechanicalDamageDescriptor(const MechanicalHostileDamageState* damageState) {
    if (damageState == nullptr) {
        return {};
    }

    std::string descriptor;
    if (damageState->sensorDamage >= 75.0f) {
        descriptor += " | sensors crippled";
    } else if (damageState->sensorDamage >= 40.0f) {
        descriptor += " | sensors clipped";
    }

    if (damageState->weaponDamage >= 75.0f) {
        descriptor += " | weapon crippled";
    } else if (damageState->weaponDamage >= 40.0f) {
        descriptor += " | weapon unstable";
    }

    if (damageState->mobilityDamage >= 75.0f) {
        descriptor += " | drive crippled";
    } else if (damageState->mobilityDamage >= 40.0f) {
        descriptor += " | drive staggered";
    }

    return descriptor;
}

void AppendSubsystemThresholdFeedback(std::string& feedback, float before, float after, const char* wornText, const char* brokenText) {
    if (before < 75.0f && after >= 75.0f) {
        feedback += brokenText;
        return;
    }
    if (before < 40.0f && after >= 40.0f) {
        feedback += wornText;
    }
}

std::string ApplyMechanicalHostileDamage(MapObject& object,
    HostileRole role,
    const PlayerState& player,
    const SessionProfile& profile,
    GameState& gameState,
    bool specialAttack,
    float momentum) {
    if (!SupportsMechanicalDamage(role)) {
        return {};
    }

    MechanicalHostileDamageState& damageState = EnsureMechanicalHostileDamageState(gameState, object.registryId);
    const float sensorBefore = damageState.sensorDamage;
    const float weaponBefore = damageState.weaponDamage;
    const float mobilityBefore = damageState.mobilityDamage;

    if (player.insideTank) {
        if (player.bt72GunnerSeat) {
            damageState.sensorDamage += specialAttack ? 58.0f : 24.0f;
            damageState.weaponDamage += specialAttack ? 52.0f : 18.0f;
            damageState.mobilityDamage += specialAttack ? 18.0f : 6.0f;
        } else if (specialAttack) {
            damageState.weaponDamage += 48.0f;
            damageState.mobilityDamage += 32.0f;
            damageState.sensorDamage += 18.0f;
        } else {
            bool ramShieldMounted = false;
            for (const auto& module : profile.partnerTank.loadout.modules) {
                if (module.type == TankModuleSlotType::Bucket && module.moduleId == "ram_shield_mk1") {
                    ramShieldMounted = true;
                    break;
                }
            }
            damageState.mobilityDamage += 18.0f + std::min(38.0f, momentum * (ramShieldMounted ? 22.0f : 16.0f));
            damageState.weaponDamage += ramShieldMounted ? 22.0f : 10.0f;
            damageState.sensorDamage += momentum >= 1.2f ? 10.0f : 4.0f;
        }
    } else if (specialAttack) {
        damageState.sensorDamage += 16.0f;
        damageState.weaponDamage += 12.0f;
    }

    damageState.sensorDamage = std::clamp(damageState.sensorDamage, 0.0f, 100.0f);
    damageState.weaponDamage = std::clamp(damageState.weaponDamage, 0.0f, 100.0f);
    damageState.mobilityDamage = std::clamp(damageState.mobilityDamage, 0.0f, 100.0f);

    std::string feedback;
    AppendSubsystemThresholdFeedback(
        feedback,
        sensorBefore,
        damageState.sensorDamage,
        " Sensor mast clipped; the control zone is narrowing.",
        " Sensor mast shattered; target lock is collapsing.");
    AppendSubsystemThresholdFeedback(
        feedback,
        weaponBefore,
        damageState.weaponDamage,
        " Weapon arm destabilized.",
        " Weapon arm crippled; the fire pattern is breaking up.");
    AppendSubsystemThresholdFeedback(
        feedback,
        mobilityBefore,
        damageState.mobilityDamage,
        " Drive ring staggered.",
        " Drive section crippled; the chassis is dragging.");
    return feedback;
}

float PlayerMotionNoise(const PlayerState& player) {
    const float movementSpeed = std::sqrt((player.velocityX * player.velocityX) + (player.velocityY * player.velocityY));
    return player.insideTank ? movementSpeed * 1.3f : movementSpeed;
}

float PlayerCombatNoise(const PlayerState& player) {
    float noise = PlayerMotionNoise(player);
    noise += player.recoilOffset * (player.insideTank ? 4.8f : 2.8f);
    if (player.muzzleFlashTimer > 0.0f) {
        noise += player.insideTank ? (2.4f + player.muzzleFlashStrength * 2.2f) : (1.4f + player.muzzleFlashStrength * 1.1f);
    }
    if (player.shockWaveTimer > 0.0f) {
        noise += 1.8f + player.shockWaveStrength * 2.6f;
    }
    return noise;
}

float HostileVisualRadius(HostileRole role, const PlayerState& player, const GameState& gameState) {
    float radius = HostileAlertRadius(role, player.insideTank) * (player.insideTank ? 0.9f : 0.82f);
    if (gameState.weather == WeatherAnomaly::EtherFog) {
        radius *= 0.72f;
    } else if (gameState.weather == WeatherAnomaly::AcidRain) {
        radius *= 0.85f;
    }
    if (role == HostileRole::RobotControl) {
        radius += 0.5f;
    }
    return std::max(2.1f, radius);
}

float HostileHearingRadius(HostileRole role, const PlayerState& player, const GameState& gameState, float playerNoise) {
    float baseRadius = 2.1f;
    switch (role) {
        case HostileRole::VerminRush: baseRadius = 1.9f; break;
        case HostileRole::GhoulRush: baseRadius = 2.3f; break;
        case HostileRole::HumanTactical: baseRadius = 2.6f; break;
        case HostileRole::RobotControl: baseRadius = 3.0f; break;
        case HostileRole::Unknown:
        default: baseRadius = 2.2f; break;
    }
    float hearingRadius = baseRadius + std::min(playerNoise, player.insideTank ? 3.6f : 2.3f);
    if (gameState.weather == WeatherAnomaly::AcidRain) {
        hearingRadius += 0.35f;
    }
    return std::min(6.6f, hearingRadius);
}

void UpdateHostileAwareness(HostileAwarenessState& state, HostileRole role, bool seesTarget, bool hearsTarget, float dt) {
    if (seesTarget) {
        const float sightGain = role == HostileRole::RobotControl ? 80.0f : (role == HostileRole::HumanTactical ? 68.0f : 58.0f);
        state.awareness = std::min(100.0f, state.awareness + dt * sightGain);
        state.lostTimer = 0.0f;
        return;
    }

    if (hearsTarget) {
        const float hearingGain = role == HostileRole::GhoulRush ? 26.0f : 22.0f;
        state.awareness = std::min(72.0f, state.awareness + dt * hearingGain);
        state.lostTimer = 0.0f;
        return;
    }

    state.lostTimer += dt;
    const float lossRate = state.awareness >= 50.0f ? 16.0f : 24.0f;
    state.awareness = std::max(0.0f, state.awareness - dt * lossRate);
}

bool ObjectContainsPoint(const MapObject& object, float x, float y, float padding = 0.0f) {
    return x >= object.x - object.width * 0.5f - padding &&
        x <= object.x + object.width * 0.5f + padding &&
        y >= object.y - object.depth * 0.5f - padding &&
        y <= object.y + object.depth * 0.5f + padding;
}

bool ObjectIntersectsPlayerBounds(const MapObject& object, const PlayerState& player, float x, float y, float padding = 0.0f) {
    const float halfWidth = player.collisionWidth * 0.5f + padding;
    const float halfDepth = player.collisionDepth * 0.5f + padding;
    const float objectHalfWidth = object.width * 0.5f + padding;
    const float objectHalfDepth = object.depth * 0.5f + padding;
    return std::abs(object.x - x) <= (objectHalfWidth + halfWidth) &&
        std::abs(object.y - y) <= (objectHalfDepth + halfDepth);
}

bool BlocksPlayerMotion(const MapObject& object) {
    if (!object.blocksMovement) {
        return false;
    }
    switch (object.category) {
        case ObjectCategory::Structure:
        case ObjectCategory::Vehicle:
        case ObjectCategory::Landmark:
        case ObjectCategory::Container:
        case ObjectCategory::Hangar:
        case ObjectCategory::Hostile:
            return true;
        case ObjectCategory::ResourceNode:
        case ObjectCategory::Terminal:
            return object.blocksMovement;
    }
    return object.blocksMovement;
}

bool SegmentSamplesHitObject(const MapObject& object, float x1, float y1, float x2, float y2) {
    for (int sample = 1; sample < 12; ++sample) {
        const float t = static_cast<float>(sample) / 12.0f;
        const float x = x1 + (x2 - x1) * t;
        const float y = y1 + (y2 - y1) * t;
        if (ObjectContainsPoint(object, x, y, 0.08f)) {
            return true;
        }
    }
    return false;
}

bool HasBlockingGeometryOnLine(const World& world,
    const MapObject& source,
    float targetX,
    float targetY) {
    for (const auto& blocker : world.objects) {
        if (blocker.registryId == source.registryId ||
            !blocker.blocksMovement ||
            blocker.interaction == InteractionType::Hostile ||
            ObjectContainsPoint(blocker, targetX, targetY, 0.15f)) {
            continue;
        }
        if (SegmentSamplesHitObject(blocker, source.x, source.y, targetX, targetY)) {
            return true;
        }
    }
    return false;
}

float DistancePointToSegment(float px, float py, float x1, float y1, float x2, float y2) {
    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float lengthSq = dx * dx + dy * dy;
    if (lengthSq <= 0.0001f) {
        const float localDx = px - x1;
        const float localDy = py - y1;
        return std::sqrt(localDx * localDx + localDy * localDy);
    }

    const float projection = std::clamp(((px - x1) * dx + (py - y1) * dy) / lengthSq, 0.0f, 1.0f);
    const float nearestX = x1 + dx * projection;
    const float nearestY = y1 + dy * projection;
    const float offsetX = px - nearestX;
    const float offsetY = py - nearestY;
    return std::sqrt(offsetX * offsetX + offsetY * offsetY);
}

float ReactiveBreakableRadius(const MapObject& object) {
    return std::max(0.4f, std::max(object.width, object.depth) * 0.58f);
}

const MapObject* FindNearestReactiveBreakable(const World& world,
    const PlayerState& player,
    float radius,
    bool requireForwardArc) {
    const MapObject* nearest = nullptr;
    float bestScore = radius * radius + 1.0f;
    const float forwardX = std::cos(player.facingRadians);
    const float forwardY = std::sin(player.facingRadians);

    for (const auto& object : world.objects) {
        if (ClassifyReactiveBreakable(object) == ReactiveBreakableKind::None) {
            continue;
        }

        const float dx = object.x - player.x;
        const float dy = object.y - player.y;
        const float distanceSq = dx * dx + dy * dy;
        if (distanceSq > radius * radius) {
            continue;
        }

        const float distance = std::sqrt(std::max(0.0001f, distanceSq));
        const float alignment = (dx * forwardX + dy * forwardY) / distance;
        if (requireForwardArc && alignment < (player.insideTank ? 0.0f : -0.12f)) {
            continue;
        }

        const float score = distanceSq - std::max(0.0f, alignment) * 0.4f;
        if (score <= bestScore) {
            bestScore = score;
            nearest = &object;
        }
    }

    return nearest;
}

MapObject* FindReactiveBreakableOnShotLine(World& world,
    const PlayerState& player,
    float targetX,
    float targetY) {
    const float totalDx = targetX - player.x;
    const float totalDy = targetY - player.y;
    const float totalLengthSq = totalDx * totalDx + totalDy * totalDy;
    if (totalLengthSq <= 0.0001f) {
        return nullptr;
    }

    MapObject* nearest = nullptr;
    float bestProjection = 1.0f;
    for (auto& object : world.objects) {
        if (ClassifyReactiveBreakable(object) == ReactiveBreakableKind::None) {
            continue;
        }

        const float projection = ((object.x - player.x) * totalDx + (object.y - player.y) * totalDy) / totalLengthSq;
        if (projection <= 0.06f || projection >= 0.94f) {
            continue;
        }

        const float laneDistance = DistancePointToSegment(object.x, object.y, player.x, player.y, targetX, targetY);
        if (laneDistance > ReactiveBreakableRadius(object) + 0.16f) {
            continue;
        }

        if (projection <= bestProjection) {
            bestProjection = projection;
            nearest = &object;
        }
    }
    return nearest;
}

float ReactiveBreakableDamage(ReactiveBreakableKind kind,
    const PlayerState& player,
    bool specialAttack,
    float momentum) {
    if (kind == ReactiveBreakableKind::Glass) {
        if (player.insideTank) {
            if (specialAttack) {
                return player.bt72GunnerSeat ? 68.0f : 82.0f;
            }
            return player.bt72GunnerSeat ? 26.0f : (28.0f + momentum * 14.0f);
        }
        return specialAttack ? 42.0f : 24.0f;
    }

    if (kind == ReactiveBreakableKind::LightVegetation) {
        if (player.insideTank) {
            if (specialAttack) {
                return player.bt72GunnerSeat ? 74.0f : 96.0f;
            }
            return player.bt72GunnerSeat ? 34.0f : (30.0f + momentum * 18.0f);
        }
        return specialAttack ? 50.0f : 28.0f;
    }

    return 0.0f;
}

std::string ReactiveBreakableImpactText(std::string_view displayName,
    ReactiveBreakableKind kind,
    const PlayerState& player,
    bool destroyed,
    bool specialAttack,
    bool lineIntercept,
    float momentum) {
    const std::string name(displayName);
    if (kind == ReactiveBreakableKind::Glass) {
        if (destroyed) {
            if (player.insideTank) {
                if (specialAttack) {
                    return lineIntercept
                        ? "Cannon strike shattered " + name + " and opened the firing lane."
                        : "Cannon strike shattered " + name + " into the lift dust.";
                }
                return player.bt72GunnerSeat
                    ? "BT-72 support burst shattered " + name + " and cleared the lane."
                    : "BT-72 rammed through " + name + " and burst the glass across the corridor.";
            }
            return specialAttack
                ? "Precision shot shattered " + name + " and cleared the sight line."
                : "Emergency baton smashed " + name + ".";
        }
        return name + " cracked but still hangs in the lane.";
    }

    if (kind == ReactiveBreakableKind::LightVegetation) {
        if (destroyed) {
            if (player.insideTank) {
                if (specialAttack) {
                    return "Heavy fire ripped through " + name + " and stripped the route edge clear.";
                }
                return momentum >= 1.0f
                    ? "BT-72 plowed through " + name + " and threw brush clear of the route."
                    : "BT-72 crushed " + name + " and cleared the route edge.";
            }
            return specialAttack
                ? "Shot tore through " + name + " and dropped the route clutter."
                : "Emergency baton chopped through " + name + ".";
        }
        return name + " thrashed but still clings to the route edge.";
    }

    return name + " reacted to the hit.";
}

bool ApplyReactiveBreakableHit(World& world,
    const MapObject& object,
    const PlayerState& player,
    StaticEraser& staticEraser,
    GameState& gameState,
    std::string_view worldName,
    bool specialAttack,
    bool lineIntercept,
    float momentum) {
    const ReactiveBreakableKind kind = ClassifyReactiveBreakable(object);
    if (kind == ReactiveBreakableKind::None) {
        return false;
    }

    const std::string registryId = object.registryId;
    const std::string displayName = object.displayName;
    MapObject* mutableObject = world.FindObjectByRegistryId(registryId);
    if (mutableObject == nullptr) {
        return false;
    }

    mutableObject->health -= ReactiveBreakableDamage(kind, player, specialAttack, momentum);
    const bool destroyed = mutableObject->health <= 0.0f;
    gameState.lastEvent = ReactiveBreakableImpactText(
        displayName,
        kind,
        player,
        destroyed,
        specialAttack,
        lineIntercept,
        momentum);

    if (destroyed) {
        staticEraser.Erase(registryId);
        staticEraser.Save(worldName);
        world.RemoveObject(registryId);
    }
    return true;
}

bool HasFriendlyOnShotLine(const World& world,
    const MapObject& source,
    float targetX,
    float targetY) {
    const float totalDx = targetX - source.x;
    const float totalDy = targetY - source.y;
    const float totalLengthSq = totalDx * totalDx + totalDy * totalDy;
    if (totalLengthSq <= 0.0001f) {
        return false;
    }

    for (const auto& ally : world.objects) {
        if (ally.registryId == source.registryId || ally.interaction != InteractionType::Hostile) {
            continue;
        }

        const float projection = ((ally.x - source.x) * totalDx + (ally.y - source.y) * totalDy) / totalLengthSq;
        if (projection <= 0.1f || projection >= 0.92f) {
            continue;
        }

        const float laneDistance = DistancePointToSegment(ally.x, ally.y, source.x, source.y, targetX, targetY);
        const float safeRadius = std::max(0.55f, std::max(ally.width, ally.depth) * 0.6f);
        if (laneDistance <= safeRadius) {
            return true;
        }
    }
    return false;
}

bool WouldOverlapBlockingObject(const World& world,
    const MapObject& movingObject,
    float targetX,
    float targetY);

bool FindHumanCoverTarget(const World& world,
    const MapObject& object,
    float playerX,
    float playerY,
    float& outTargetX,
    float& outTargetY) {
    float bestScore = 1.0e9f;
    bool found = false;

    for (const auto& blocker : world.objects) {
        if (blocker.registryId == object.registryId ||
            !blocker.blocksMovement ||
            blocker.interaction == InteractionType::Hostile) {
            continue;
        }

        const float blockerDx = blocker.x - object.x;
        const float blockerDy = blocker.y - object.y;
        const float blockerDistanceSq = blockerDx * blockerDx + blockerDy * blockerDy;
        if (blockerDistanceSq > 4.8f * 4.8f) {
            continue;
        }

        const float awayFromPlayerX = blocker.x - playerX;
        const float awayFromPlayerY = blocker.y - playerY;
        const float awayFromPlayerLength = std::sqrt(awayFromPlayerX * awayFromPlayerX + awayFromPlayerY * awayFromPlayerY);
        if (awayFromPlayerLength <= 0.0001f) {
            continue;
        }

        const float awayNormalizedX = awayFromPlayerX / awayFromPlayerLength;
        const float awayNormalizedY = awayFromPlayerY / awayFromPlayerLength;
        const float flankX = -awayNormalizedY;
        const float flankY = awayNormalizedX;
        const float coverPadding =
            std::max(blocker.width, blocker.depth) * 0.5f +
            std::max(object.width, object.depth) * 0.58f +
            0.16f;
        const float flankPadding =
            std::max(blocker.width, blocker.depth) * 0.34f +
            std::max(object.width, object.depth) * 0.24f +
            0.08f;
        for (const float flankOffset : std::array<float, 3>{0.0f, flankPadding, -flankPadding}) {
            const float candidateX = blocker.x + awayNormalizedX * coverPadding + flankX * flankOffset;
            const float candidateY = blocker.y + awayNormalizedY * coverPadding + flankY * flankOffset;
            if (WouldOverlapBlockingObject(world, object, candidateX, candidateY)) {
                continue;
            }

            MapObject coverProbe = object;
            coverProbe.x = candidateX;
            coverProbe.y = candidateY;
            if (!HasBlockingGeometryOnLine(world, coverProbe, playerX, playerY)) {
                continue;
            }

            const float moveDx = candidateX - object.x;
            const float moveDy = candidateY - object.y;
            const float playerDistanceSq = (candidateX - playerX) * (candidateX - playerX) +
                (candidateY - playerY) * (candidateY - playerY);
            const float score = moveDx * moveDx + moveDy * moveDy - playerDistanceSq * 0.08f;
            if (score < bestScore) {
                bestScore = score;
                outTargetX = candidateX;
                outTargetY = candidateY;
                found = true;
            }
        }
    }

    return found;
}

bool WouldOverlapBlockingObject(const World& world,
    const MapObject& movingObject,
    float targetX,
    float targetY) {
    for (const auto& blocker : world.objects) {
        if (blocker.registryId == movingObject.registryId ||
            !blocker.blocksMovement ||
            blocker.interaction == InteractionType::Hostile) {
            continue;
        }

        const float combinedHalfWidth = movingObject.width * 0.5f + blocker.width * 0.5f + 0.08f;
        const float combinedHalfDepth = movingObject.depth * 0.5f + blocker.depth * 0.5f + 0.08f;
        if (std::abs(targetX - blocker.x) < combinedHalfWidth &&
            std::abs(targetY - blocker.y) < combinedHalfDepth) {
            return true;
        }
    }
    return false;
}

void TryMoveHostile(World& world, MapObject& object, float deltaX, float deltaY) {
    const float targetX = object.x + deltaX;
    const float targetY = object.y + deltaY;
    if (!WouldOverlapBlockingObject(world, object, targetX, targetY)) {
        object.x = targetX;
        object.y = targetY;
        return;
    }

    if (std::abs(deltaX) > 0.001f &&
        !WouldOverlapBlockingObject(world, object, object.x + deltaX, object.y)) {
        object.x += deltaX;
        return;
    }

    if (std::abs(deltaY) > 0.001f &&
        !WouldOverlapBlockingObject(world, object, object.x, object.y + deltaY)) {
        object.y += deltaY;
        return;
    }

    const float lateralLength = std::sqrt(deltaX * deltaX + deltaY * deltaY);
    if (lateralLength <= 0.0001f) {
        return;
    }

    const float lateralX = -deltaY / lateralLength * lateralLength * 0.85f;
    const float lateralY = deltaX / lateralLength * lateralLength * 0.85f;
    if (!WouldOverlapBlockingObject(world, object, object.x + lateralX, object.y + lateralY)) {
        object.x += lateralX;
        object.y += lateralY;
        return;
    }
    if (!WouldOverlapBlockingObject(world, object, object.x - lateralX, object.y - lateralY)) {
        object.x -= lateralX;
        object.y -= lateralY;
    }
}

bool TryMoveHostileAdaptive(World& world, MapObject& object, float deltaX, float deltaY) {
    const float targetX = object.x + deltaX;
    const float targetY = object.y + deltaY;
    if (!WouldOverlapBlockingObject(world, object, targetX, targetY)) {
        object.x = targetX;
        object.y = targetY;
        return true;
    }

    for (const float scale : std::array<float, 4>{0.72f, 0.54f, 0.38f, 0.24f}) {
        const float scaledTargetX = object.x + deltaX * scale;
        const float scaledTargetY = object.y + deltaY * scale;
        if (!WouldOverlapBlockingObject(world, object, scaledTargetX, scaledTargetY)) {
            object.x = scaledTargetX;
            object.y = scaledTargetY;
            return true;
        }
    }

    return false;
}

bool IsRangedDisciplineRole(HostileRole role) {
    return role == HostileRole::HumanTactical || role == HostileRole::RobotControl;
}

void TriggerCombatFeedback(PlayerState& player,
    float muzzleFlashTimer,
    float muzzleFlashStrength,
    float shockWaveDuration,
    float shockWaveStrength) {
    player.muzzleFlashTimer = std::max(player.muzzleFlashTimer, muzzleFlashTimer);
    player.muzzleFlashStrength = std::max(player.muzzleFlashStrength, muzzleFlashStrength);
    if (shockWaveDuration > 0.0f) {
        player.shockWaveTimer = std::max(player.shockWaveTimer, shockWaveDuration);
        player.shockWaveDuration = std::max(player.shockWaveDuration, shockWaveDuration);
        player.shockWaveStrength = std::max(player.shockWaveStrength, shockWaveStrength);
    }
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
    if (!profile.firstPlayableRoute.clearanceModuleInstalled && !profile.story.bucketRecovered) {
        return "Utility Hardpoint Unfitted";
    }
    if (TankUsesRamShield(profile)) {
        return "Ram Shield Mk.I";
    }
    if (TankUsesTowCoupler(profile)) {
        return "Tow Coupler Mk.I";
    }
    return "Bucket Rig Mk.I";
}

std::string CurrentBt72SeatAssignment(const PlayerState& player) {
    if (!player.insideTank) {
        return "on_foot";
    }
    return player.bt72GunnerSeat ? "gunner" : "pilot";
}

std::string Bt72SeatPolicyBlockReason(const SessionProfile& profile) {
    const std::string normalizedPolicy = NormalizeBt72SecondSeatPolicy(profile.partnerTank.secondSeatPolicy);
    if (normalizedPolicy == "trusted_only") {
        if (profile.partnerTank.trustedGunnerHandle.empty()) {
            return "BT-72 gunner seat is restricted to a trusted gunner, but no trusted gunner is assigned yet.";
        }
        return "BT-72 gunner seat is restricted to trusted gunner " + profile.partnerTank.trustedGunnerHandle + ".";
    }
    return std::string("BT-72 gunner seat is restricted by current second-seat policy: ") +
        Bt72SecondSeatPolicyLabel(profile.partnerTank.secondSeatPolicy) + ".";
}

void ToggleTankUtilityModule(SessionProfile& profile, PlayerState& player, GameState& gameState) {
    auto* module = FindTankModule(profile, TankModuleSlotType::Bucket);
    if (module == nullptr) {
        gameState.lastEvent = "No utility hardpoint found on BT-72.";
        return;
    }
    if (player.bt72GunnerSeat) {
        gameState.lastEvent = "Return to pilot controls before retuning BT-72 utility hardpoints.";
        return;
    }
    if (!profile.firstPlayableRoute.clearanceModuleInstalled && !profile.story.bucketRecovered) {
        gameState.lastEvent = "BT-72 utility hardpoint is still unfitted. Install the clearance module first.";
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

float Bt72ReturnFireMitigation(const SessionProfile& profile) {
    float damageScale = 1.0f;
    if (TankHasBulwarkSync(profile)) {
        damageScale *= 0.82f;
    }
    if (TankUsesRamShield(profile)) {
        damageScale *= 0.85f;
    }
    return 1.0f - damageScale;
}

std::string Bt72DefenseStatusLabel(const SessionProfile& profile) {
    const bool bulwark = TankHasBulwarkSync(profile);
    const bool ramShield = TankUsesRamShield(profile);
    if (bulwark && ramShield) {
        return "Bulwark Sync + Ram Shield";
    }
    if (bulwark) {
        return "Bulwark Sync";
    }
    if (ramShield) {
        return "Ram Shield Mk.I";
    }
    return "Baseline armor";
}

std::string DescribeBt72DefenseImpact(const SessionProfile& profile,
    float hullBefore,
    float hullAfter,
    float sensorsBefore,
    float sensorsAfter,
    float cockpitBefore,
    float cockpitAfter) {
    const std::string defenseState = Bt72DefenseStatusLabel(profile);
    char buffer[224];
    std::snprintf(
        buffer,
        sizeof(buffer),
        " Guard: %s // %.0f%% return-fire cut. Hull -%.1f, Sensors -%.1f, Cockpit -%.1f. Hull state: %s.",
        defenseState.c_str(),
        Bt72ReturnFireMitigation(profile) * 100.0f,
        std::max(0.0f, hullBefore - hullAfter),
        std::max(0.0f, sensorsBefore - sensorsAfter),
        std::max(0.0f, cockpitBefore - cockpitAfter),
        TankIntegrityBand(hullAfter));
    return buffer;
}

bool HasBt72CrewSupport(const SessionProfile& profile) {
    return Bt72SecondSeatUnlocked(profile) &&
        (profile.partnerTank.gunnerDrillSeen ||
            !profile.partnerTank.trustedGunnerHandle.empty() ||
            !profile.partnerTank.assignedGunnerHandle.empty());
}

float Bt72CrewCoordinationBoost(const SessionProfile& profile, const GameState& gameState, bool gunnerSeat) {
    float boost = 1.0f;
    if (Bt72SecondSeatUnlocked(profile)) {
        boost += 0.03f;
    }
    if (profile.partnerTank.gunnerDrillSeen) {
        boost += gunnerSeat ? 0.06f : 0.03f;
    }
    if (!profile.partnerTank.trustedGunnerHandle.empty()) {
        boost += 0.05f;
    }
    if (!profile.partnerTank.assignedGunnerHandle.empty()) {
        boost += gunnerSeat ? 0.07f : 0.04f;
    }
    boost += static_cast<float>(std::max(0, EffectiveStatValue(profile, gameState, 'C') - 5)) * 0.015f;
    boost += static_cast<float>(std::max(0, EffectiveStatValue(profile, gameState, 'I') - 5)) * 0.01f;
    if (HasEquippedPassiveSkill(profile, "skill_pilot_sync")) {
        boost += gunnerSeat ? 0.08f : 0.05f;
    }
    return boost;
}

float Bt72AttackEnergyDiscount(const SessionProfile& profile, const GameState& gameState, bool gunnerSeat) {
    float discount = static_cast<float>(std::max(0, EffectiveStatValue(profile, gameState, 'I') - 5)) * 0.1f;
    if (HasEquippedPassiveSkill(profile, "skill_pilot_sync")) {
        discount += gunnerSeat ? 0.7f : 0.8f;
    }
    if (HasBt72CrewSupport(profile)) {
        discount += gunnerSeat ? 0.5f : 0.3f;
    }
    return discount;
}

float Bt72AttackAmmoDiscount(const SessionProfile& profile, bool gunnerSeat) {
    float discount = 0.0f;
    if (HasBt72CrewSupport(profile)) {
        discount += gunnerSeat ? 0.6f : 0.4f;
    }
    if (HasEquippedPassiveSkill(profile, "skill_pilot_sync")) {
        discount += gunnerSeat ? 0.2f : 0.0f;
    }
    return discount;
}

float FootReflexCombatBoost(const SessionProfile& profile, const GameState& gameState) {
    float boost = 1.0f;
    boost += static_cast<float>(std::max(0, EffectiveStatValue(profile, gameState, 'A') - 5)) * 0.03f;
    if (HasEquippedPassiveSkill(profile, "skill_field_reflex")) {
        boost += 0.12f;
    }
    return boost;
}

struct CombatDamageWindow {
    float multiplier = 1.0f;
    float flatDamage = 0.0f;
    bool weakPoint = false;
    bool execute = false;
    bool stagger = false;
};

CombatDamageWindow EvaluateCombatDamageWindow(const MapObject& object,
    HostileRole role,
    const MechanicalHostileDamageState* mechanicalDamage,
    const PlayerState& player,
    const SessionProfile& profile,
    const GameState& gameState,
    bool specialAttack,
    float momentum) {
    CombatDamageWindow window;
    const bool gunnerSeat = player.insideTank && player.bt72GunnerSeat;
    const float maxHealthHint = std::max(1.0f, HostileMaxHealthHint(role));
    const bool executeWindow = object.health <= maxHealthHint * (specialAttack ? 0.62f : 0.48f);

    if (player.insideTank) {
        if (gameState.tankThermalLoad >= 25.0f && gameState.tankThermalLoad < 55.0f) {
            window.multiplier += gunnerSeat
                ? (specialAttack ? 0.06f : 0.04f)
                : (specialAttack ? 0.05f : 0.03f);
        }

        if (mechanicalDamage != nullptr) {
            if (gunnerSeat) {
                if (mechanicalDamage->sensorDamage >= 40.0f) {
                    window.flatDamage += specialAttack ? 10.0f : 6.0f;
                    window.weakPoint = true;
                }
                if (mechanicalDamage->weaponDamage >= 40.0f) {
                    window.flatDamage += specialAttack ? 6.0f : 4.0f;
                    window.weakPoint = true;
                }
            } else if (specialAttack) {
                if (mechanicalDamage->mobilityDamage >= 40.0f) {
                    window.flatDamage += 9.0f;
                    window.stagger = true;
                }
                if (mechanicalDamage->weaponDamage >= 40.0f) {
                    window.flatDamage += 5.0f;
                    window.weakPoint = true;
                }
            } else {
                if (mechanicalDamage->mobilityDamage >= 40.0f) {
                    window.flatDamage += 6.0f + std::min(8.0f, momentum * 3.0f);
                    window.stagger = true;
                }
                if (mechanicalDamage->sensorDamage >= 75.0f) {
                    window.flatDamage += 3.0f;
                    window.weakPoint = true;
                }
            }
        }

        if (executeWindow) {
            window.flatDamage += gunnerSeat
                ? (specialAttack ? 12.0f : 7.0f)
                : (specialAttack ? 10.0f : 6.0f);
            window.execute = true;
        }
        if (!specialAttack && !gunnerSeat && TankUsesRamShield(profile) && momentum >= 1.6f) {
            window.multiplier += 0.08f;
        }
        return window;
    }

    const float strength = static_cast<float>(std::max(0, EffectiveStatValue(profile, gameState, 'S') - 5));
    const float agility = static_cast<float>(std::max(0, EffectiveStatValue(profile, gameState, 'A') - 5));
    const float perception = static_cast<float>(std::max(0, EffectiveStatValue(profile, gameState, 'P') - 5));
    if (executeWindow) {
        window.flatDamage += specialAttack
            ? (5.0f + perception * 0.8f + agility * 0.5f)
            : (4.0f + strength * 0.7f + agility * 0.4f);
        window.execute = true;
    }
    if (HasEquippedPassiveSkill(profile, "skill_field_reflex")) {
        if (role == HostileRole::GhoulRush || role == HostileRole::VerminRush) {
            window.multiplier += specialAttack ? 0.04f : 0.06f;
        }
        if (executeWindow) {
            window.flatDamage += specialAttack ? 2.0f : 1.5f;
        }
    }
    return window;
}

std::string DescribeCombatDamageWindow(const CombatDamageWindow& window,
    bool insideTank,
    bool gunnerSeat,
    bool specialAttack) {
    std::string feedback;
    if (window.weakPoint) {
        feedback += insideTank && gunnerSeat
            ? " Weak-point lane flashed open across the target frame."
            : " Weak-point plating gave way under the hit.";
    }
    if (window.stagger) {
        feedback += specialAttack
            ? " The blast rode the staggered chassis."
            : " The hit folded into the staggered drive line.";
    }
    if (window.execute) {
        feedback += insideTank
            ? " Finish window held and the strike bit deeper."
            : " Finish window opened and the strike bit deeper.";
    }
    return feedback;
}

float Bt72ServiceSkillBoost(const SessionProfile& profile, const GameState& gameState) {
    float boost = 1.0f;
    boost += static_cast<float>(std::max(0, EffectiveStatValue(profile, gameState, 'I') - 5)) * 0.03f;
    if (HasEquippedPassiveSkill(profile, "skill_pilot_sync")) {
        boost += 0.08f;
    }
    if (HasEquippedPassiveSkill(profile, "skill_muscle_memory")) {
        boost += 0.04f;
    }
    if (HasBt72CrewSupport(profile)) {
        boost += 0.05f;
    }
    return boost;
}

void ApplyServiceChoiceBonuses(SessionProfile& profile,
    GameState& gameState,
    bool fieldService,
    std::string& eventText) {
    const float energyBonus = fieldService ? 4.0f : 6.0f;
    const float ammoBonus = fieldService ? 2.0f : 4.0f;
    const float sensorBonus = fieldService ? 2.0f : 3.0f;
    if (HasEquippedPassiveSkill(profile, "skill_pilot_sync")) {
        profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + energyBonus);
        profile.partnerTank.damage.sensors = std::min(100.0f, profile.partnerTank.damage.sensors + sensorBonus);
        profile.partnerTank.trustLink = std::min(1.0f, profile.partnerTank.trustLink + (fieldService ? 0.015f : 0.025f));
        eventText += " Pilot Sync trimmed service losses and tightened the link.";
    }
    if (HasBt72CrewSupport(profile)) {
        profile.partnerTank.ammoReserve = std::min(100.0f, profile.partnerTank.ammoReserve + ammoBonus);
        profile.partnerTank.damage.turret = std::min(100.0f, profile.partnerTank.damage.turret + (fieldService ? 2.0f : 3.0f));
        eventText += " Crew coordination sped the reload and sight reset.";
    }

    switch (profile.doctrine) {
        case ShelterDoctrine::Industry:
            profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + (fieldService ? 3.0f : 5.0f));
            profile.partnerTank.ammoReserve = std::min(100.0f, profile.partnerTank.ammoReserve + (fieldService ? 3.0f : 5.0f));
            profile.partnerTank.damage.bucket = std::min(100.0f, profile.partnerTank.damage.bucket + (fieldService ? 2.0f : 4.0f));
            eventText += " Industry doctrine prioritized reserves and utility throughput.";
            break;
        case ShelterDoctrine::Defense:
            profile.partnerTank.damage.hull = std::min(100.0f, profile.partnerTank.damage.hull + (fieldService ? 4.0f : 6.0f));
            profile.partnerTank.damage.turret = std::min(100.0f, profile.partnerTank.damage.turret + (fieldService ? 4.0f : 6.0f));
            gameState.tankThermalLoad = std::max(0.0f, gameState.tankThermalLoad - (fieldService ? 8.0f : 12.0f));
            eventText += " Defense doctrine hardened the combat package.";
            break;
        case ShelterDoctrine::Medical:
            profile.partnerTank.damage.cockpit = std::min(100.0f, profile.partnerTank.damage.cockpit + (fieldService ? 3.0f : 5.0f));
            profile.character.hp = std::min(profile.character.maxHp, profile.character.hp + (fieldService ? 10.0f : 16.0f));
            profile.character.mp = std::min(profile.character.maxMp, profile.character.mp + (fieldService ? 12.0f : 18.0f));
            eventText += " Medical doctrine stabilized the operator during service.";
            break;
        case ShelterDoctrine::Balanced:
        default:
            break;
    }
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

const char* TryGetTerminalSyncText(std::string_view tag) {
    static constexpr std::array<std::pair<std::string_view, const char*>, 24> kSyncTexts{{
        {"tower_sync", "Tower sync complete. Regional grid reach expanded."},
        {"power_pylon", "Pylon registry mirrored. Grid restoration route updated."},
        {"drone_station", "Drone station ledger mirrored. Sweep routes registered to Pip-Pad."},
        {"rail_depot", "Rail freight depot records mirrored. Heavy spur logistics registered."},
        {"orbital_uplink", "Orbital uplink records mirrored. Long-range scan queue registered."},
        {"rail_fortress_hub", "Rail Fortress patrol package mirrored. Spur security doctrine updated."},
        {"recovery_fabricator", "Recovery fabricator recipes mirrored. Shelter supply pipeline updated."},
        {"industrial_gate", "Industrial gate overrides mirrored. Inner spur access route logged."},
        {"industrial_survey", "Industrial survey notes mirrored. Inner spur reconnaissance queue updated."},
        {"industrial_outpost", "Inner spur outpost records mirrored. Forward recovery foothold logged."},
        {"assembly_cell", "Assembly cell notes mirrored. Local recovery production registered."},
        {"foundry_line", "Foundry line records mirrored. Heavy fabrication route registered."},
        {"reactor_yard", "Reactor yard records mirrored. Heavy energy recovery route registered."},
        {"capacitor_bank", "Capacitor bank records mirrored. Buffered grid discharge route registered."},
        {"relay_substation", "Relay substation notes mirrored. Backbone return flow updated."},
        {"service_bay", "Service bay notes mirrored. BT-72 support route updated."},
        {"water_reclaimer", "Water reclaimer notes mirrored. Frontier recovery reserves updated."},
        {"lanline_service_hub", "Lanline service hub mirrored. Shelter 17 service catalog now resolves through authored relay infrastructure."},
        {"fey_ring", "Fey Ring route mirrored. Transit windows registered to the relay map."},
        {"medical_support", "Medical support node mirrored. Field treatment requests now route through authored relay anchors."},
        {"tank_service", "Tank service anchor mirrored. BT-72 maintenance route updated."},
        {"specialist_cryo", "Cryo specialist registry mirrored. Shelter staffing ledger updated."},
        {"echo_trace", "Residual echo trace mirrored to Pip-Pad."},
        {"workshop_service", "Workshop terminal mirrored. BT-72 service route and repair notes updated."},
    }};

    const std::string_view normalizedTag = NormalizeGameplayDescriptorTag(tag);
    for (const auto& [entryTag, text] : kSyncTexts) {
        if (entryTag == normalizedTag) {
            return text;
        }
    }
    return nullptr;
}

std::string DescribeTerminalSync(const MapObject& object) {
    if (const char* text = TryGetTerminalSyncText(object.scriptTag)) {
        return text;
    }
    return object.scriptTag.empty()
        ? "Terminal sync complete. Additional archive fragments copied to Pip-Pad."
        : "Terminal sync complete: " + object.scriptTag;
}

const LanlineDiagnostics& CachedLanlineDiagnostics(const LanlineSessionState& session, std::string_view runtimeWorldName) {
    static std::string cachedSessionKey;
    static std::string cachedWorldName;
    static double lastProbeTime = -10.0;
    static LanlineDiagnostics cachedDiagnostics;

    const std::string probeKey = session.sessionId + "|" + session.updatedAt + "|" + session.hostEndpoint;
    const std::string normalizedWorldName = NormalizeWorldReference(runtimeWorldName);
    const double now = ImGui::GetTime();
    if (probeKey != cachedSessionKey || normalizedWorldName != cachedWorldName || (now - lastProbeTime) >= 1.5) {
        cachedDiagnostics = ProbeLanlineHost(session, normalizedWorldName);
        cachedSessionKey = probeKey;
        cachedWorldName = normalizedWorldName;
        lastProbeTime = now;
    }
    return cachedDiagnostics;
}

const std::vector<std::string>& UpdateLanlineRuntimeNotifications(const LanlineSessionState* session,
    std::string_view runtimeWorldName,
    GameState& gameState) {
    static bool hadPreviousSession = false;
    static LanlineSessionState previousSession;
    static std::vector<std::string> notifications;

    const auto pushNotification = [&](std::string message) {
        if (message.empty()) {
            return;
        }
        notifications.push_back(std::move(message));
        if (notifications.size() > 8) {
            notifications.erase(notifications.begin());
        }
        gameState.lastEvent = notifications.back();
    };

    if (session == nullptr) {
        if (hadPreviousSession) {
            pushNotification("Lanline session link lost. Waiting for refreshed launcher/session state.");
            hadPreviousSession = false;
            previousSession = {};
        }
        return notifications;
    }

    const std::string normalizedRuntimeWorld = NormalizeWorldReference(runtimeWorldName);
    const std::string normalizedSessionWorld = NormalizeWorldReference(session->worldName);
    const bool worldMatchesRuntime = normalizedRuntimeWorld == normalizedSessionWorld;

    if (!hadPreviousSession) {
        pushNotification("Lanline session linked: " + session->sessionId + " on " + normalizedSessionWorld + ".");
        if (!worldMatchesRuntime) {
            pushNotification("Lanline world mismatch: runtime is on " + normalizedRuntimeWorld +
                ", session points to " + normalizedSessionWorld + ".");
        }
        previousSession = *session;
        hadPreviousSession = true;
        return notifications;
    }

    const std::string previousNormalizedWorld = NormalizeWorldReference(previousSession.worldName);
    if (session->sessionId != previousSession.sessionId) {
        pushNotification("Lanline session switched to " + session->sessionId + " (" + session->mode + ").");
    }
    if (session->mode != previousSession.mode) {
        pushNotification("Lanline mode changed to " + session->mode + ".");
    }
    if (session->lifecycleStage != previousSession.lifecycleStage) {
        pushNotification("Lanline lifecycle advanced to " + session->lifecycleStage + ".");
    }
    if (session->pendingPeer != previousSession.pendingPeer) {
        pushNotification(session->pendingPeer.empty()
            ? "Lanline pending peer queue cleared."
            : "Lanline pending peer updated: " + session->pendingPeer + ".");
    }
    if (session->connectedPeer != previousSession.connectedPeer) {
        pushNotification(session->connectedPeer.empty()
            ? "Lanline connected peer link cleared."
            : "Lanline peer link confirmed with " + session->connectedPeer + ".");
    }
    if (normalizedSessionWorld != previousNormalizedWorld) {
        pushNotification("Lanline session world updated: " + previousNormalizedWorld + " -> " + normalizedSessionWorld + ".");
    }

    const bool previousWorldMatch = previousNormalizedWorld == normalizedRuntimeWorld;
    if (worldMatchesRuntime != previousWorldMatch) {
        pushNotification(worldMatchesRuntime
            ? "Lanline session world now matches the active runtime world."
            : "Lanline session world drifted away from the active runtime world.");
    }

    for (const auto& playerEntry : session->players) {
        const auto previousIt = std::find_if(previousSession.players.begin(), previousSession.players.end(),
            [&](const LanlinePlayerEntry& previousEntry) {
                return previousEntry.displayName == playerEntry.displayName;
            });
        if (previousIt == previousSession.players.end()) {
            pushNotification(playerEntry.displayName + " joined Lanline roster as " + playerEntry.role + ".");
            continue;
        }
        if (playerEntry.role != previousIt->role) {
            pushNotification(playerEntry.displayName + " moved from " + previousIt->role + " to " + playerEntry.role + " in Lanline lobby.");
        }
        if (playerEntry.online != previousIt->online) {
            pushNotification(playerEntry.displayName +
                (playerEntry.online ? " came online in Lanline." : " went offline in Lanline."));
        }
        if (bunker::IsLanlineReadyEligibleSlot(playerEntry) && playerEntry.ready != previousIt->ready) {
            pushNotification(playerEntry.displayName +
                (playerEntry.ready ? " is now ready for Lanline deployment." : " is no longer ready for Lanline deployment."));
        }
    }

    for (const auto& previousEntry : previousSession.players) {
        const auto currentIt = std::find_if(session->players.begin(), session->players.end(),
            [&](const LanlinePlayerEntry& currentEntry) {
                return currentEntry.displayName == previousEntry.displayName;
            });
        if (currentIt == session->players.end()) {
            pushNotification(previousEntry.displayName + " left the Lanline roster.");
        }
    }

    if (session->relayMessages.size() > previousSession.relayMessages.size()) {
        for (std::size_t index = previousSession.relayMessages.size(); index < session->relayMessages.size(); ++index) {
            const auto& relayMessage = session->relayMessages[index];
            pushNotification("Relay chat [" + relayMessage.channelId + "] " +
                relayMessage.author + ": " + relayMessage.body);
        }
    }

    for (const auto& voicePresence : session->voicePresence) {
        const auto previousVoiceIt = std::find_if(
            previousSession.voicePresence.begin(),
            previousSession.voicePresence.end(),
            [&](const LanlineVoicePresence& previousVoice) {
                return previousVoice.handle == voicePresence.handle;
            });
        if (previousVoiceIt == previousSession.voicePresence.end()) {
            pushNotification("Voice presence linked: " + voicePresence.handle + ".");
            continue;
        }
        if (voicePresence.speaking != previousVoiceIt->speaking) {
            pushNotification(voicePresence.handle +
                (voicePresence.speaking ? " started voice transmission." : " stopped voice transmission."));
        }
        const bool peakChangedMeaningfully =
            std::abs(voicePresence.peakLevel - previousVoiceIt->peakLevel) >= 0.2f;
        if (voicePresence.speaking && peakChangedMeaningfully) {
            pushNotification(voicePresence.handle + " voice peak now at " +
                std::to_string(static_cast<int>(voicePresence.peakLevel * 100.0f)) + "%.");
        }
    }

    for (const auto& previousVoice : previousSession.voicePresence) {
        const auto currentVoiceIt = std::find_if(
            session->voicePresence.begin(),
            session->voicePresence.end(),
            [&](const LanlineVoicePresence& currentVoice) {
                return currentVoice.handle == previousVoice.handle;
            });
        if (currentVoiceIt == session->voicePresence.end()) {
            pushNotification("Voice presence expired: " + previousVoice.handle + ".");
        }
    }

    previousSession = *session;
    hadPreviousSession = true;
    return notifications;
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

const char* JoinabilityLabel(const LanlineSessionState& session) {
    if (session.mode != "LAN Host") {
        return "non-host";
    }
    if (!session.connectedPeer.empty()) {
        return "linked";
    }
    if (!session.pendingPeer.empty()) {
        return "pending";
    }
    if (session.lifecycleStage == "HostLobbyOpen" || session.lifecycleStage == "HostRuntimeActive") {
        return "joinable";
    }
    return "locked";
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

bool IsNearTaggedObject(const World& world,
    float playerX,
    float playerY,
    const std::string& scriptTag,
    float maxDistance) {
    const auto* object = world.FindObjectByScriptTag(scriptTag);
    if (object == nullptr) {
        return false;
    }

    const float dx = object->x - playerX;
    const float dy = object->y - playerY;
    return (dx * dx + dy * dy) <= (maxDistance * maxDistance);
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
    if (player.bt72GunnerSeat) {
        gameState.lastEvent = "Field service requires BT-72 pilot controls, not the gunner station.";
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
    const float intelligence = static_cast<float>(EffectiveStatValue(profile, gameState, 'I'));
    const float boost = (hasEngineer ? 1.12f : 1.0f) * Bt72ServiceSkillBoost(profile, gameState) *
        (1.0f + std::max(0.0f, intelligence - 5.0f) * 0.015f);
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
    ApplyServiceChoiceBonuses(profile, gameState, true, gameState.lastEvent);
    if (profile.firstPlayableRoute.firstTankCombatResolved && !profile.firstPlayableRoute.firstServicePerformed) {
        profile.firstPlayableRoute.firstServicePerformed = true;
        gameState.lastEvent += " First service halt logged for the route.";
        AppendObjectivePreviewHint(gameState.lastEvent, profile);
        AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
    }
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

bool HasEmergencyMeleeTool(const SessionProfile& profile) {
    return InventoryCount(profile, "#%it_emergency_baton") > 0;
}

int CountBt72RestorationMilestones(const SessionProfile& profile) {
    return (profile.firstPlayableRoute.bt72HullInspected ? 1 : 0) +
        (profile.firstPlayableRoute.bt72CoreRecovered ? 1 : 0) +
        (profile.firstPlayableRoute.bt72ServiceNotesRecovered ? 1 : 0);
}

bool HasBt72RestorationPrerequisites(const SessionProfile& profile) {
    return CountBt72RestorationMilestones(profile) == 3 &&
        CanCompleteBt72StagedRestoration(profile);
}

bool HasBt72RestorationMaterials(const SessionProfile& profile) {
    return InventoryCount(profile, "power_cell") >= 1 &&
        InventoryCount(profile, "repair_patch") >= 1 &&
        InventoryCount(profile, "old_plate") >= 1;
}

bool HasClearanceInstallMaterials(const SessionProfile& profile) {
    return InventoryCount(profile, "scrap_steel") >= 1 &&
        InventoryCount(profile, "hydraulic_seal") >= 1;
}

std::string JoinMissingRouteLabels(const std::vector<std::string>& labels) {
    if (labels.empty()) {
        return {};
    }
    std::string joined = labels.front();
    for (std::size_t index = 1; index < labels.size(); ++index) {
        joined += ", " + labels[index];
    }
    return joined;
}

std::string DescribeBt72RestorationNeeds(const SessionProfile& profile) {
    std::vector<std::string> missing;
    if (!profile.firstPlayableRoute.bt72HullInspected) {
        missing.push_back("hull survey");
    }
    if (!profile.firstPlayableRoute.bt72CoreRecovered) {
        missing.push_back("starter core");
    }
    if (!profile.firstPlayableRoute.bt72ServiceNotesRecovered) {
        missing.push_back("service notes");
    }
    if (!profile.bt72HullLockedInRestorationCradle) {
        missing.push_back("crane lift cradle");
    }
    if (!missing.empty()) {
        return "BT-72 restoration is missing: " + JoinMissingRouteLabels(missing) + ".";
    }

    if (InventoryCount(profile, "power_cell") < 1) {
        missing.push_back("power_cell");
    }
    if (InventoryCount(profile, "repair_patch") < 1) {
        missing.push_back("repair_patch");
    }
    if (InventoryCount(profile, "old_plate") < 1) {
        missing.push_back("old_plate");
    }
    if (!missing.empty()) {
        return "BT-72 restoration still needs: " + JoinMissingRouteLabels(missing) + ".";
    }

    return "BT-72 restoration kit is ready.";
}

std::string DescribeClearanceModuleNeeds(const SessionProfile& profile) {
    std::vector<std::string> missing;
    if (!profile.firstPlayableRoute.clearanceBlueprintRecovered) {
        missing.push_back("clearance blueprint");
    }
    if (!profile.firstPlayableRoute.clearanceMaterialsRecovered) {
        missing.push_back("bucket-rack parts");
    }
    if (profile.firstPlayableRoute.clearanceMaterialsRecovered && !HasClearanceInstallMaterials(profile)) {
        if (InventoryCount(profile, "scrap_steel") < 1) {
            missing.push_back("scrap_steel");
        }
        if (InventoryCount(profile, "hydraulic_seal") < 1) {
            missing.push_back("hydraulic_seal");
        }
    }
    return missing.empty()
        ? "Clearance module install kit is ready."
        : "Clearance module is missing: " + JoinMissingRouteLabels(missing) + ".";
}

void ApplyStarterBt72Restoration(SessionProfile& profile) {
    profile.firstPlayableRoute.bt72Restored = true;
    profile.partnerTank.deployed = false;
    profile.partnerTank.inRepair = false;
    profile.partnerTank.worldPositionKnown = false;
    profile.partnerTank.damage.hull = 78.0f;
    profile.partnerTank.damage.turret = 72.0f;
    profile.partnerTank.damage.bucket = 100.0f;
    profile.partnerTank.damage.sensors = 68.0f;
    profile.partnerTank.damage.cockpit = 76.0f;
    profile.partnerTank.damage.powerCore = 74.0f;
    profile.partnerTank.energyReserve = 54.0f;
    profile.partnerTank.ammoReserve = 28.0f;
}

std::string ResolveFirstPlayableRouteKill(const MapObject& object, bool insideTank, SessionProfile& profile) {
    if (object.registryId == "[%enemy_laska_0001]") {
        profile.firstPlayableRoute.earlyVerminEncounterResolved = true;
        return "Archive corridor vermin cleared. The archive sync window is now open.";
    }
    if (object.registryId == "[%enemy_ghoul_0001]" && !profile.firstPlayableRoute.firstTankCombatResolved) {
        profile.firstPlayableRoute.firstTankCombatResolved = true;
        return insideTank
            ? "First BT-72 combat contact resolved. Service halt window is now open before relay sync."
            : "First surface contact resolved. Run a BT-72 service halt before relay sync.";
    }
    return {};
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

    const std::string_view scriptTag = NormalizeGameplayDescriptorTag(nearest->scriptTag);

    if (scriptTag == "tower_sync") {
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
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        worldState.towerSyncRecovered = true;
        worldState.localRelayAvailable = true;
        worldState.regionalGridOnline = true;
        gameState.lastEvent = "Tower relay synchronized. Regional scan and power route tagged -> " + routeTarget + ". " + progressionEvent;
        if (purged > 0) {
            gameState.lastEvent += " Ether bloom purged by " + std::to_string(purged) + "%.";
        }
        return true;
    }

    if (scriptTag == "remote_link") {
        const std::string target = nearest->linkTarget.empty() ? "gate_control" : nearest->linkTarget;
        gameState.lastEvent = player.insideTank
            ? "Remote link established from cockpit. Control target -> " + target + "."
            : "Remote link available. Use from cockpit or field terminal. Control target -> " + target + ".";
        return true;
    }

    if (scriptTag == "power_pylon") {
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

    if (scriptTag == "drone_station") {
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

    if (scriptTag == "rail_depot") {
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

    if (scriptTag == "orbital_uplink") {
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

    if (scriptTag == "rail_fortress_hub") {
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

    if (scriptTag == "recovery_fabricator") {
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

    if (scriptTag == "industrial_gate") {
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

    if (scriptTag == "industrial_survey") {
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

    if (scriptTag == "industrial_outpost") {
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

    if (scriptTag == "assembly_cell") {
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

    if (scriptTag == "foundry_line") {
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

    if (scriptTag == "reactor_yard") {
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

    if (scriptTag == "capacitor_bank") {
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

    if (scriptTag == "relay_substation") {
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

    if (scriptTag == "service_bay") {
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

    if (scriptTag == "water_reclaimer") {
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

    if (scriptTag == "lanline_service_hub") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.towerSyncRecovered) {
            gameState.lastEvent = "Lanline Services stay locked until the first tower is synchronized.";
            return true;
        }
        worldState.localRelayAvailable = true;
        profile.lanlineServices.serviceHubKnown = true;
        gameState.lanlineServicesVisible = true;
        gameState.lastSupportAction = nearest->linkTarget.empty()
            ? "Lanline service hub handshake complete."
            : "Lanline service hub handshake complete -> " + nearest->linkTarget + ".";
        gameState.lastEvent = gameState.lastSupportAction;
        return true;
    }

    if (scriptTag == "tank_service") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.serviceBayActive) {
            gameState.lastEvent = "Tank service anchor is authored, but the service bay backbone is still offline.";
            return true;
        }
        std::string repairEvent;
        if (TryConsumeBestTankServiceKit(profile, &repairEvent)) {
            gameState.lastSupportAction = "BT-72 serviced through authored tank service anchor.";
            gameState.lastEvent = gameState.lastSupportAction + " " + repairEvent;
        } else {
            gameState.lastEvent = repairEvent.empty()
                ? "Tank service anchor ready. Bring a compatible service kit from Lanline support or field salvage."
                : "Tank service anchor ready. " + repairEvent;
        }
        return true;
    }

    if (scriptTag == "medical_support") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.localRelayAvailable) {
            gameState.lastEvent = "Medical support terminal has no relay authorization yet. Sync the tower first.";
            return true;
        }
        profile.character.hp = std::min(profile.character.maxHp, profile.character.hp + 35.0f);
        profile.character.mp = std::min(profile.character.maxMp, profile.character.mp + 15.0f);
        gameState.lastSupportAction = "Medical support request completed.";
        gameState.lastEvent = "Medical support anchor stabilized the operator and replenished field reserves.";
        return true;
    }

    if (scriptTag == "fey_ring") {
        auto& worldState = EnsureSelectedWorldFieldState(profile);
        if (!worldState.localRelayAvailable) {
            gameState.lastEvent = "Fey Ring routing remains sealed until Lanline Services come online through tower sync.";
            return true;
        }
        if (!worldState.feyRingIntercityUnlocked) {
            worldState.feyRingIntercityUnlocked = true;
            gameState.feyRingScheduleVisible = true;
            gameState.lastPortalAction = nearest->linkTarget.empty()
                ? "Intercity Fey Ring schedule mirrored."
                : "Intercity Fey Ring schedule mirrored -> " + nearest->linkTarget + ".";
            gameState.lastEvent = gameState.lastPortalAction;
            return true;
        }
        if (!worldState.feyRingInterserverUnlocked &&
            worldState.relaySubstationActive &&
            worldState.orbitalUplinkActive) {
            worldState.feyRingInterserverUnlocked = true;
            gameState.lastPortalAction = "Interserver Fey Ring route windows unlocked through relay + orbital chain.";
            gameState.lastEvent = gameState.lastPortalAction;
            return true;
        }
        gameState.feyRingScheduleVisible = true;
        gameState.lastEvent = "Fey Ring schedule refreshed. Watch for the next transit window.";
        return true;
    }

    if (scriptTag == "archive_sync") {
        AddCollectedTapeIfMissing(profile, nearest->registryId, nearest->displayName);
        gameState.lastEvent = "Archive sync complete. Personnel and recovery records mirrored to Pip-Pad.";
        return true;
    }

    if (scriptTag == "echo_trace") {
        if (!profile.story.pipPadRecovered) {
            gameState.lastEvent = "Pip-Pad AR layer required before echo traces can be reconstructed.";
            return true;
        }
        AddCollectedTapeIfMissing(profile, nearest->registryId + "_echo", nearest->displayName + " // Echo Trace");
        const std::string targetId = nearest->linkTarget;
        const auto revealLinkedTarget = [&]() -> bool {
            if (targetId.empty()) {
                return false;
            }
            if (auto* targetObject = FindObjectByRegistryId(world, targetId); targetObject != nullptr) {
                targetObject->discovered = true;
                gameState.lastEvent += " Trace now points to " + targetObject->displayName + ".";
                return true;
            }
            gameState.lastEvent += " Trace marker linked to " + targetId + ".";
            return true;
        };

        if (!profile.firstPlayableRoute.bt72ServiceNotesRecovered) {
            profile.firstPlayableRoute.bt72ServiceNotesRecovered = true;
            AddCollectedTapeIfMissing(profile, "bt72_service_reel_001", "BT-72 Service Reel");
            gameState.lastEvent = "Maintenance echo resolved. BT-72 service notes and holo-records mirrored to Pip-Pad.";
            (void)revealLinkedTarget();
            return true;
        }
        if (!profile.story.tankLinked) {
            gameState.lastEvent = "Echo residue still contains a deeper clearance schema, but a live BT-72 link is required to compile it.";
            return true;
        }
        if (!profile.firstPlayableRoute.clearanceBlueprintRecovered) {
            profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
            AddCollectedTapeIfMissing(profile, "bt72_clearance_blueprint", "BT-72 Clearance Module Blueprint");
            gameState.lastEvent = "Maintenance echo recompiled into a clearance module blueprint for BT-72.";
            (void)revealLinkedTarget();
            return true;
        }
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

    if (scriptTag == "specialist_cryo") {
        const std::string role = nearest->linkTarget.empty() ? "specialist" : nearest->linkTarget;
        AddRescuedSpecialist(profile, nearest->registryId, nearest->displayName, role);
        AddCollectedTapeIfMissing(profile, nearest->registryId + "_personnel", nearest->displayName + " // Personnel Recovery");
        gameState.lastEvent = nearest->displayName + " recovered from cryostasis. Role assigned: " + role + ".";
        return true;
    }

    if (scriptTag == "terminal_sync") {
        AddCollectedTapeIfMissing(profile, nearest->registryId, nearest->displayName);
        gameState.lastEvent = "Terminal sync complete. General system notes mirrored to Pip-Pad.";
        return true;
    }

    if (scriptTag == "workshop_service") {
        return false;
    }

    return false;
}

}  // namespace

const char* RuntimeGamePhaseLabel(RuntimeGamePhase phase) {
    switch (phase) {
        case RuntimeGamePhase::MAIN_MENU:
            return "Main Menu";
        case RuntimeGamePhase::WORLD_LOADING:
            return "World Loading";
        case RuntimeGamePhase::ACTIVE_GAME:
            return "Active Game";
        case RuntimeGamePhase::UI_INTERACTION:
            return "UI Interaction";
    }
    return "Unknown";
}

const char* TankIntegrityBand(float integrity) {
    if (integrity >= 85.0f) {
        return "Ready";
    }
    if (integrity >= 65.0f) {
        return "Scuffed";
    }
    if (integrity >= 40.0f) {
        return "Strained";
    }
    if (integrity >= 20.0f) {
        return "Critical";
    }
    return "Failing";
}

std::string DescribeHostileReadability(const MapObject& object, const GameState& gameState) {
    const HostileRole role = ClassifyHostileRole(object);
    switch (role) {
        case HostileRole::VerminRush:
            return "Vermin rush | fast swarm, no meaningful self-preservation";
        case HostileRole::GhoulRush:
            return "Ghoul rush | melee pressure, low self-preservation";
        case HostileRole::HumanTactical:
            return "Human tactical | standoff bursts, line-of-fire discipline, will retreat";
        case HostileRole::RobotControl:
            return "Control robot | lane pressure, standoff fire" +
                MechanicalDamageDescriptor(FindMechanicalHostileDamageState(gameState, object.registryId));
        case HostileRole::Unknown:
        default:
            return "Unknown contact | behavior unresolved";
    }
}

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

int RollLootCount(const LootEntry& entry) {
    const int minCount = std::max(1, entry.minCount);
    const int maxCount = std::max(minCount, entry.maxCount);
    if (maxCount == minCount) {
        return minCount;
    }
    return minCount + (std::rand() % (maxCount - minCount + 1));
}

void AddLootEntryToInventory(SessionProfile& profile, const LootEntry& entry, float weight) {
    if (entry.itemId.empty()) {
        return;
    }
    AddInventoryItem(profile, entry.itemId, RollLootCount(entry), weight);
}

void GrantContainerLoot(SessionProfile& profile, const MapObject& object, float inventoryWeight) {
    if (object.lootEntries.empty()) {
        for (const auto& lootId : object.manualLootIds) {
            if (!lootId.empty()) {
                AddInventoryItem(profile, lootId, 1, inventoryWeight);
            }
        }
        return;
    }

    if (object.lootMode == LootMode::RandomTable) {
        float totalWeight = 0.0f;
        const LootEntry* lastValidEntry = nullptr;
        for (const auto& entry : object.lootEntries) {
            if (!entry.itemId.empty() && entry.weight > 0.0f) {
                totalWeight += entry.weight;
                lastValidEntry = &entry;
            }
        }
        if (totalWeight <= 0.0f) {
            return;
        }
        const float roll = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * totalWeight;
        float cursor = 0.0f;
        for (const auto& entry : object.lootEntries) {
            if (entry.itemId.empty() || entry.weight <= 0.0f) {
                continue;
            }
            cursor += entry.weight;
            if (roll <= cursor) {
                AddLootEntryToInventory(profile, entry, inventoryWeight);
                return;
            }
        }
        if (lastValidEntry != nullptr) {
            AddLootEntryToInventory(profile, *lastValidEntry, inventoryWeight);
        }
        return;
    }

    for (const auto& entry : object.lootEntries) {
        AddLootEntryToInventory(profile, entry, inventoryWeight);
    }
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

bool CanAttachBt72HullToCrane(const SessionProfile& profile) {
    return profile.firstPlayableRoute.bt72HullInspected &&
        profile.hangarPowerRestored &&
        profile.bt72CraneControlOnline &&
        profile.bt72CranePathClear &&
        !profile.bt72HullMovedToServiceLift &&
        !profile.bt72HullLockedInRestorationCradle;
}

bool AttachBt72HullToCrane(SessionProfile& profile) {
    if (!CanAttachBt72HullToCrane(profile)) {
        return false;
    }
    profile.bt72HullAttachedToCrane = true;
    return true;
}

bool CanMoveBt72HullToServiceLift(const SessionProfile& profile) {
    return profile.bt72HullAttachedToCrane &&
        profile.hangarPowerRestored &&
        profile.bt72CraneControlOnline &&
        profile.bt72CranePathClear &&
        !profile.bt72HullMovedToServiceLift &&
        !profile.bt72HullLockedInRestorationCradle;
}

bool MoveBt72HullToServiceLift(SessionProfile& profile) {
    if (!CanMoveBt72HullToServiceLift(profile)) {
        return false;
    }
    profile.bt72HullAttachedToCrane = false;
    profile.bt72HullMovedToServiceLift = true;
    profile.bt72HullLockedInRestorationCradle = true;
    return true;
}

bool CanInstallBt72Core(const SessionProfile& profile) {
    return profile.firstPlayableRoute.bt72CoreRecovered &&
        profile.bt72HullMovedToServiceLift &&
        profile.bt72HullLockedInRestorationCradle;
}

bool CanCompleteBt72StagedRestoration(const SessionProfile& profile) {
    return profile.firstPlayableRoute.bt72HullInspected &&
        CanInstallBt72Core(profile) &&
        profile.firstPlayableRoute.bt72ServiceNotesRecovered;
}

PipDeviceCapabilities GetPipDeviceCapabilities(PipDeviceModel model) {
    switch (model) {
        case PipDeviceModel::PipBoy01:
            return {"Pip-Boy 0.1", true, false, false, true, false, false, false, false};
        case PipDeviceModel::PipBoy10:
            return {"Pip-Boy 1.0", true, false, false, true, false, false, false, false};
        case PipDeviceModel::PipBoy2000Classic:
            return {"Pip-Boy 2000 Classic", true, false, true, false, false, false, false, false};
        case PipDeviceModel::PipBoy2000MarkVI:
            return {"Pip-Boy 2000 Mark VI / FO76-style", true, false, false, true, true, false, false, true};
        case PipDeviceModel::PipBoy3000MarkIV:
            return {"Pip-Boy 3000 / Mark IV", true, false, true, false, true, false, false, false};
        case PipDeviceModel::PipBoy3000MarkV:
            return {"Pip-Boy 3000 Mark V", false, true, false, false, false, false, false, false};
        case PipDeviceModel::PipPad3500:
            return {"Pip-Pad 3500", true, false, true, false, true, true, true, false};
    }
    return {};
}

const char* DeviceDisplayName(PipDeviceModel model) {
    return GetPipDeviceCapabilities(model).displayName;
}

bool IsKnownPipDeviceId(std::string_view deviceId) {
    return deviceId == "#%it_pippad" ||
        deviceId == "#%it_pipboy_1_0" ||
        deviceId == "#%it_pipboy_3000";
}

std::string ActivePipDeviceIdOrDefault(const SessionProfile& profile) {
    if (IsKnownPipDeviceId(profile.activePipDeviceId)) {
        return profile.activePipDeviceId;
    }
    if (HasInventoryItem(profile, "#%it_pippad")) {
        return "#%it_pippad";
    }
    if (HasInventoryItem(profile, "#%it_pipboy_1_0")) {
        return "#%it_pipboy_1_0";
    }
    if (HasInventoryItem(profile, "#%it_pipboy_3000")) {
        return "#%it_pipboy_3000";
    }
    return "#%it_pippad";
}

bool HasActivePipDevice(const SessionProfile& profile) {
    return IsKnownPipDeviceId(profile.activePipDeviceId) ||
        profile.story.pipPadRecovered ||
        HasInventoryItem(profile, "#%it_pippad") ||
        HasInventoryItem(profile, "#%it_pipboy_1_0") ||
        HasInventoryItem(profile, "#%it_pipboy_3000");
}

bool SelectPipDevice(SessionProfile& profile, std::string_view deviceId) {
    if (!IsKnownPipDeviceId(deviceId)) {
        return false;
    }
    const std::string selectedDeviceId(deviceId);
    const bool changed = profile.activePipDeviceId != selectedDeviceId || !profile.story.pipPadRecovered;
    profile.activePipDeviceId = selectedDeviceId;
    if (!HasInventoryItem(profile, selectedDeviceId)) {
        AddInventoryItem(profile, selectedDeviceId, 1, selectedDeviceId == "#%it_pippad" ? 0.8f : 1.0f);
    }
    profile.story.pipPadRecovered = true;
    profile.pipDeviceReselectPending = false;
    return changed;
}

bool BeginPipDeviceReselect(SessionProfile& profile) {
    if (!HasActivePipDevice(profile)) {
        return false;
    }
    profile.pipDeviceReselectPending = true;
    return true;
}

std::string ActivePipDeviceDisplayName(const SessionProfile& profile) {
    const std::string deviceId = ActivePipDeviceIdOrDefault(profile);
    if (deviceId == "#%it_pipboy_1_0") {
        return "Pip-Boy 1.0";
    }
    if (deviceId == "#%it_pipboy_3000") {
        return "Pip-Boy 3000 / Mark IV";
    }
    return "Pip-Pad 3500";
}

bool IsSelectablePipDevice(PipDeviceModel model) {
    return GetPipDeviceCapabilities(model).selectable;
}

bool IsPropOnlyPipDevice(PipDeviceModel model) {
    return GetPipDeviceCapabilities(model).propOnly;
}

bool HasDigitalMap(PipDeviceModel model) {
    return GetPipDeviceCapabilities(model).hasDigitalMap;
}

bool UsesPhysicalNavigation(PipDeviceModel model) {
    return GetPipDeviceCapabilities(model).physicalNavigationOnly;
}

bool SupportsMediaIndex(PipDeviceModel model) {
    return GetPipDeviceCapabilities(model).supportsMediaIndex;
}

bool SupportsFullPipPadWorkspace(PipDeviceModel model) {
    return GetPipDeviceCapabilities(model).supportsFullPipPadWorkspace;
}

bool HasPipPad(const SessionProfile& profile) {
    return HasActivePipDevice(profile);
}

bool RecoverPipPad(SessionProfile& profile) {
    return SelectPipDevice(profile, ActivePipDeviceIdOrDefault(profile));
}

bool HasBlueLinkModule(const SessionProfile& profile) {
    return profile.blueLinkModuleRecovered || profile.blueLinkModuleInstalled;
}

bool IsBlueLinkInstalled(const SessionProfile& profile) {
    return HasPipPad(profile) &&
        profile.blueLinkModuleInstalled &&
        !profile.pipPadExpansionCoverPresent;
}

bool InstallBlueLinkModule(SessionProfile& profile) {
    if (!HasPipPad(profile) || !HasBlueLinkModule(profile)) {
        return false;
    }
    const bool alreadyInstalled = IsBlueLinkInstalled(profile);
    profile.blueLinkModuleRecovered = true;
    profile.blueLinkModuleInstalled = true;
    profile.pipPadExpansionCoverPresent = false;
    return !alreadyInstalled;
}

bool CanUsePipPadMediaIndex(const SessionProfile& profile) {
    return IsBlueLinkInstalled(profile);
}

bool PlayerHasPipPadAccess(const SessionProfile& profile) {
    return HasPipPad(profile);
}

bool TryTogglePipPadUi(PlayerState& player, const SessionProfile& profile, GameState& gameState) {
    if (!HasPipPad(profile)) {
        player.uiVisible = false;
        gameState.lastEvent = "No Pip-Pad linked yet. Recover the Pip-Pad first.";
        return false;
    }

    player.uiVisible = !player.uiVisible;
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

bool SweepMovePlayerAgainstWorld(const World& world, PlayerState& player, float deltaX, float deltaY) {
    auto axisMoveAllowed = [&](float targetX, float targetY) {
        const float searchRadius = std::max(player.collisionWidth, player.collisionDepth) + 1.8f;
        if (const auto* nearest = world.FindNearestInteractive(targetX, targetY, searchRadius);
            nearest != nullptr && BlocksPlayerMotion(*nearest) && ObjectIntersectsPlayerBounds(*nearest, player, targetX, targetY, 0.04f)) {
            return false;
        }

        for (const auto& object : world.objects) {
            if (!BlocksPlayerMotion(object) || object.interaction == InteractionType::Hostile) {
                continue;
            }
            if (ObjectIntersectsPlayerBounds(object, player, targetX, targetY, 0.04f)) {
                return false;
            }
        }
        return true;
    };

    bool moved = false;
    if (std::abs(deltaX) > 0.0001f) {
        const float targetX = player.x + deltaX;
        if (axisMoveAllowed(targetX, player.y)) {
            player.x = targetX;
            moved = true;
        }
    }
    if (std::abs(deltaY) > 0.0001f) {
        const float targetY = player.y + deltaY;
        if (axisMoveAllowed(player.x, targetY)) {
            player.y = targetY;
            moved = true;
        }
    }
    return moved;
}

void TryToggleBt72CrewSeat(PlayerState& player, SessionProfile& profile, GameState& gameState) {
    if (!player.insideTank) {
        gameState.lastEvent = "Enter BT-72 before shifting crew stations.";
        return;
    }
    if (!Bt72SecondSeatUnlocked(profile)) {
        gameState.lastEvent = "BT-72 second seat is still sealed until the first sync link is stabilized.";
        return;
    }
    if (!profile.story.tankLinked) {
        gameState.lastEvent = "BT-72 crew routing is unavailable until the cockpit link is established.";
        return;
    }

    if (!player.bt72GunnerSeat &&
        !Bt72GunnerHandleAllowed(profile, profile.character.displayName)) {
        gameState.lastEvent = Bt72SeatPolicyBlockReason(profile);
        return;
    }

    player.bt72GunnerSeat = !player.bt72GunnerSeat;
    if (player.bt72GunnerSeat) {
        profile.partnerTank.gunnerDrillSeen = true;
        profile.partnerTank.assignedGunnerHandle = profile.character.displayName;
        player.bucketRaised = false;
        gameState.lastEvent = std::string("Crew station shifted to the BT-72 gunner seat. Driver assist holds the hull at crawl speed. Seat policy: ") +
            Bt72SecondSeatPolicyLabel(profile.partnerTank.secondSeatPolicy) + ".";
    } else {
        if (profile.partnerTank.assignedGunnerHandle == profile.character.displayName) {
            profile.partnerTank.assignedGunnerHandle.clear();
        }
        gameState.lastEvent = "Crew station shifted back to BT-72 pilot controls.";
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
    profile.firstPlayableRoute.accessCardRecovered =
        profile.firstPlayableRoute.accessCardRecovered ||
        HasInventoryItem(profile, "bunker_access_card") ||
        profile.story.pipPadRecovered;
    profile.firstPlayableRoute.earlyVerminEncounterResolved =
        profile.firstPlayableRoute.earlyVerminEncounterResolved || staticEraser.IsErased("[%enemy_laska_0001]");
    profile.firstPlayableRoute.clearanceMaterialsRecovered =
        profile.firstPlayableRoute.clearanceMaterialsRecovered || staticEraser.IsErased("#%it_bucket_0001");
    profile.firstPlayableRoute.clearanceModuleInstalled =
        profile.firstPlayableRoute.clearanceModuleInstalled || profile.story.bucketRecovered;
    profile.firstPlayableRoute.surfaceArrivalReached =
        profile.firstPlayableRoute.surfaceArrivalReached || profile.story.outerRoadCleared;
    profile.firstPlayableRoute.firstTankCombatResolved =
        profile.firstPlayableRoute.firstTankCombatResolved || staticEraser.IsErased("[%enemy_ghoul_0001]");
    profile.firstPlayableRoute.firstRecoveryNodeActivated =
        profile.firstPlayableRoute.firstRecoveryNodeActivated || profile.story.relayRecovered;
    profile.firstPlayableRoute.debriefSummaryViewed =
        profile.firstPlayableRoute.debriefSummaryViewed || profile.story.returnedToBase;
    profile.story.bucketRecovered =
        profile.story.bucketRecovered || profile.firstPlayableRoute.clearanceModuleInstalled || staticEraser.IsErased("#%it_bucket_0001");
    profile.story.outerRoadCleared = profile.story.outerRoadCleared || staticEraser.IsErased("#%res_scrap_0001");
    profile.story.pipPadRecovered = profile.story.pipPadRecovered || HasPipPad(profile);
    if (profile.story.pipPadRecovered && profile.activePipDeviceId.empty()) {
        profile.activePipDeviceId = "#%it_pippad";
    }
    profile.story.tankLinked = profile.story.tankLinked || profile.partnerTank.deployed;
    profile.partnerTank.secondSeatUnlocked = profile.partnerTank.secondSeatUnlocked || profile.story.tankLinked;
    profile.partnerTank.secondSeatPolicy = NormalizeBt72SecondSeatPolicy(profile.partnerTank.secondSeatPolicy);
    profile.story.relayRecovered =
        profile.story.relayRecovered || profile.firstPlayableRoute.firstRecoveryNodeActivated || HasInventoryItem(profile, "relay_reconstruction_data");
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
    char title[320];
    const std::string seatAssignment = CurrentBt72SeatAssignment(player);
    std::snprintf(
        title,
        sizeof(title),
        "BunkerGame | %s | %s | %s | %s | %s | %s | %s",
        sessionProfile.character.displayName.c_str(),
        player.insideTank ? sessionProfile.partnerTank.callSign.c_str() : "On Foot",
        Bt72SeatAssignmentLabel(seatAssignment),
        ToString(player.viewMode),
        world.metadata.name.c_str(),
        RecoveryStatusLabel(sessionProfile, worldState),
        kCurrentVersionLabel.data());
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
        AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
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
                {},
                "ghoul_rush"
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

namespace {

bool RouteEventOnboardingComplete(const SessionProfile& profile) {
    return profile.story.awakenedFromCryo &&
        profile.story.pipPadRecovered &&
        profile.story.archiveRecovered &&
        profile.firstPlayableRoute.bt72Restored &&
        profile.story.tankLinked;
}

bool RouteEventLayerUnlocked(const SessionProfile& profile) {
    return profile.firstPlayableRoute.debriefSummaryViewed ||
        profile.story.returnedToBase ||
        HasCollectedTapeId(profile, "debrief_shelter17");
}

float RouteEventOfferDuration(std::string_view routeEventType) {
    return routeEventType == "merchant_window" ? 22.0f : 14.0f;
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

float RouteEventDuration(std::string_view routeEventType) {
    if (routeEventType == "service_call") {
        return 150.0f;
    }
    if (routeEventType == "field_refuel") {
        return 135.0f;
    }
    if (routeEventType == "relay_instability") {
        return 145.0f;
    }
    if (routeEventType == "blocked_route") {
        return 165.0f;
    }
    if (routeEventType == "damaged_convoy") {
        return 155.0f;
    }
    if (routeEventType == "merchant_window") {
        return 110.0f;
    }
    return 120.0f;
}

float RouteEventCooldown(std::string_view routeEventType, bool success) {
    const float baseCooldown = success ? 120.0f : 180.0f;
    if (routeEventType == "blocked_route") {
        return baseCooldown + 20.0f;
    }
    if (routeEventType == "relay_instability") {
        return baseCooldown + 10.0f;
    }
    if (routeEventType == "merchant_window") {
        return baseCooldown + (success ? 45.0f : 70.0f);
    }
    return baseCooldown;
}

int RouteEventGoal(std::string_view routeEventType) {
    if (routeEventType == "merchant_window") {
        return 1;
    }
    return 2;
}

std::string RouteEventStartText(std::string_view routeEventType) {
    if (routeEventType == "service_call") {
        return "ROUTE EVENT OFFERED: Service call tagged near the recovery fringe. Stabilize BT-72 and complete one honest repair/service pass before the window closes.";
    }
    if (routeEventType == "field_refuel") {
        return "ROUTE EVENT OFFERED: Field refuel window opened. Rebuild BT-72 reserves and secure one upstream fuel/support line.";
    }
    if (routeEventType == "relay_instability") {
        return "ROUTE EVENT OFFERED: Relay instability is bleeding across the starter route. Re-stabilize the local signal backbone before the packet degrades.";
    }
    if (routeEventType == "blocked_route") {
        return "ROUTE EVENT OFFERED: Fresh debris pressure is choking the cleared route. Reassert route control before the lane hard-locks again.";
    }
    if (routeEventType == "damaged_convoy") {
        return "ROUTE EVENT OFFERED: A damaged convoy drifted into the recovery fringe. Stabilize logistics and pull the cargo back into Shelter 17.";
    }
    if (routeEventType == "merchant_window") {
        return "ROUTE EVENT OFFERED: A field merchant trace surfaced on the recovery fringe. One discreet exchange window is open before the broker folds the signal.";
    }
    return "ROUTE EVENT OFFERED: A field incident was tagged on the active recovery route.";
}

std::string RouteEventWeatherText(std::string_view routeEventType, WeatherAnomaly weather) {
    if (weather == WeatherAnomaly::AcidRain) {
        if (routeEventType == "service_call") {
            return " Acid rain is chewing through exposed BT-72 systems.";
        }
        if (routeEventType == "field_refuel") {
            return " Acid rain is burning reserve stock off the route.";
        }
        if (routeEventType == "blocked_route") {
            return " Acid rain is turning the cleared lane into fresh slurry and debris drag.";
        }
    } else if (weather == WeatherAnomaly::EtherFog) {
        if (routeEventType == "relay_instability") {
            return " Ether fog is smearing the relay packet across the route.";
        }
        if (routeEventType == "blocked_route") {
            return " Ether fog is hiding the lane edges and accelerating route drift.";
        }
        if (routeEventType == "damaged_convoy") {
            return " Ether fog is swallowing convoy traces before crews can lock them.";
        }
    }
    return {};
}

std::string RouteEventActivationText(std::string_view routeEventType) {
    return std::string("ROUTE EVENT ACTIVE: ") + RouteEventLabel(routeEventType) + " is now live on the active recovery route.";
}

std::string RouteEventEscalationText(std::string_view routeEventType) {
    if (routeEventType == "service_call") {
        return "ROUTE EVENT ESCALATING: Service call reserves are drying up. BT-72 support stock is about to be lost.";
    }
    if (routeEventType == "field_refuel") {
        return "ROUTE EVENT ESCALATING: The refuel window is collapsing and the support cache is about to burn off.";
    }
    if (routeEventType == "relay_instability") {
        return "ROUTE EVENT ESCALATING: Relay noise is spiking and the local route packet is starting to tear.";
    }
    if (routeEventType == "blocked_route") {
        return "ROUTE EVENT ESCALATING: Debris and ether buildup are reclaiming the cleared lane.";
    }
    if (routeEventType == "damaged_convoy") {
        return "ROUTE EVENT ESCALATING: The damaged convoy is breaking apart and recovery stock is spilling across the route.";
    }
    if (routeEventType == "merchant_window") {
        return "ROUTE EVENT ESCALATING: The merchant trace is thinning out and the broker is about to cut the channel.";
    }
    return "ROUTE EVENT ESCALATING: The field incident is worsening.";
}

int RouteEventWeatherWeightBonus(std::string_view routeEventType, const GameState& gameState) {
    if (gameState.weather == WeatherAnomaly::AcidRain) {
        if (routeEventType == "service_call") {
            return 3;
        }
        if (routeEventType == "field_refuel") {
            return 2;
        }
        if (routeEventType == "blocked_route") {
            return 1;
        }
    } else if (gameState.weather == WeatherAnomaly::EtherFog) {
        if (routeEventType == "relay_instability") {
            return 4;
        }
        if (routeEventType == "blocked_route") {
            return 1;
        }
        if (routeEventType == "damaged_convoy") {
            return 1;
        }
    }
    return 0;
}

bool RouteEventBlockedByWeather(std::string_view routeEventType, const GameState& gameState) {
    if (routeEventType == "merchant_window") {
        return gameState.weather == WeatherAnomaly::AcidRain ||
            gameState.weather == WeatherAnomaly::EtherFog;
    }
    return false;
}

int RouteEventProgress(const SessionProfile& profile, const WorldFieldState& worldState) {
    if (worldState.activeRouteEventType == "service_call") {
        int progress = 0;
        if (profile.character.awakening.fieldServiceUses > 0 ||
            worldState.serviceCyclesCompleted > 0 ||
            worldState.serviceBayActive) {
            progress += 1;
        }
        if (profile.partnerTank.damage.hull >= 70.0f &&
            profile.partnerTank.energyReserve >= 55.0f &&
            profile.partnerTank.ammoReserve >= 45.0f) {
            progress += 1;
        }
        return progress;
    }
    if (worldState.activeRouteEventType == "field_refuel") {
        int progress = 0;
        if (profile.partnerTank.energyReserve >= 70.0f ||
            InventoryCount(profile, "power_cell") > 0) {
            progress += 1;
        }
        if (worldState.caravanRouteActive ||
            worldState.droneStationsActive ||
            worldState.tradeNetworkActive ||
            worldState.railFreightActive) {
            progress += 1;
        }
        return progress;
    }
    if (worldState.activeRouteEventType == "relay_instability") {
        int progress = 0;
        if (worldState.localRelayAvailable ||
            worldState.regionalGridOnline ||
            worldState.towerSyncRecovered) {
            progress += 1;
        }
        if (worldState.relaySubstationActive ||
            worldState.relaySyncCycles > 0 ||
            worldState.routeContamination < 12.0f) {
            progress += 1;
        }
        return progress;
    }
    if (worldState.activeRouteEventType == "blocked_route") {
        int progress = 0;
        if (!worldState.routeOverrun && worldState.routeContamination < 18.0f) {
            progress += 1;
        }
        if (worldState.campFortificationLevel >= 1 ||
            worldState.railFortressActive ||
            worldState.caravanRouteActive ||
            worldState.droneStationsActive) {
            progress += 1;
        }
        return progress;
    }
    if (worldState.activeRouteEventType == "damaged_convoy") {
        int progress = 0;
        if (worldState.caravanRouteActive ||
            worldState.droneStationsActive ||
            worldState.tradeNetworkActive ||
            worldState.railFreightActive) {
            progress += 1;
        }
        if (worldState.serviceBayActive ||
            InventoryCount(profile, "repair_patch") > 0 ||
            profile.firstPlayableRoute.firstServicePerformed) {
            progress += 1;
        }
        return progress;
    }
    if (worldState.activeRouteEventType == "merchant_window") {
        return worldState.tradeCyclesCompleted > 0 ? 1 : 0;
    }
    return 0;
}

bool RouteEventHardFail(const SessionProfile& profile, const WorldFieldState& worldState) {
    if (worldState.activeRouteEventType == "service_call") {
        return profile.partnerTank.damage.hull < 32.0f || profile.partnerTank.energyReserve < 16.0f;
    }
    if (worldState.activeRouteEventType == "field_refuel") {
        return profile.partnerTank.energyReserve < 12.0f && worldState.routeContamination >= 22.0f;
    }
    if (worldState.activeRouteEventType == "relay_instability") {
        return worldState.routeContamination >= 28.0f && !worldState.localRelayAvailable;
    }
    if (worldState.activeRouteEventType == "blocked_route") {
        return worldState.routeOverrun && worldState.routeContamination >= 24.0f;
    }
    if (worldState.activeRouteEventType == "damaged_convoy") {
        return worldState.infrastructureDecay >= 30.0f &&
            !worldState.caravanRouteActive &&
            !worldState.droneStationsActive &&
            !worldState.tradeNetworkActive &&
            !worldState.railFreightActive;
    }
    if (worldState.activeRouteEventType == "merchant_window") {
        return worldState.routeOverrun || worldState.routeContamination >= 20.0f;
    }
    return false;
}

void ApplyRouteEventSuccess(SessionProfile& profile, WorldFieldState& worldState, GameState& gameState) {
    const std::string routeEventType = worldState.activeRouteEventType;
    if (routeEventType == "service_call") {
        AddInventoryItem(profile, "repair_patch", 1, 0.2f);
        AddInventoryItem(profile, "cryo_medkit", 1, 0.5f);
        worldState.serviceCyclesCompleted = std::max(worldState.serviceCyclesCompleted, 1);
        worldState.infrastructureDecay = std::max(0.0f, worldState.infrastructureDecay - 1.5f);
        profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 6.0f);
        profile.partnerTank.ammoReserve = std::min(100.0f, profile.partnerTank.ammoReserve + 4.0f);
        gameState.lastEvent = "ROUTE EVENT RESOLVED: Service call stabilized. Shelter 17 logged fresh repair stock and emergency med support.";
    } else if (routeEventType == "field_refuel") {
        AddInventoryItem(profile, "power_cell", 1, 0.3f);
        AddInventoryItem(profile, "trade_voucher", 1, 0.0f);
        profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 14.0f);
        profile.partnerTank.ammoReserve = std::min(100.0f, profile.partnerTank.ammoReserve + 6.0f);
        worldState.infrastructureDecay = std::max(0.0f, worldState.infrastructureDecay - 0.6f);
        gameState.lastEvent = "ROUTE EVENT RESOLVED: Field refuel window secured. BT-72 reserves and supply vouchers were pushed back into the route ledger.";
    } else if (routeEventType == "relay_instability") {
        AddInventoryItem(profile, "copper_wire", 1, 0.2f);
        worldState.localRelayAvailable = worldState.localRelayAvailable || profile.story.relayRecovered;
        worldState.routeContamination = std::max(0.0f, worldState.routeContamination - 6.0f);
        worldState.relayCreditsEarned += 20;
        gameState.lastEvent = "ROUTE EVENT RESOLVED: Relay instability contained. The local packet stabilized and fresh relay credit was logged.";
    } else if (routeEventType == "blocked_route") {
        AddInventoryItem(profile, "steel_scrap", 2, 0.5f);
        AddInventoryItem(profile, "old_plate", 1, 0.5f);
        worldState.routeOverrun = false;
        worldState.routeContamination = std::max(0.0f, worldState.routeContamination - 10.0f);
        worldState.infrastructureDecay = std::max(0.0f, worldState.infrastructureDecay - 1.5f);
        gameState.lastEvent = "ROUTE EVENT RESOLVED: The blocked route was reopened and the salvage lane is readable again.";
    } else if (routeEventType == "damaged_convoy") {
        AddInventoryItem(profile, "trade_voucher", 1, 0.0f);
        AddInventoryItem(profile, "repair_patch", 1, 0.2f);
        AddInventoryItem(profile, "steel_scrap", 2, 0.5f);
        worldState.tradeCyclesCompleted = std::max(worldState.tradeCyclesCompleted, 1);
        gameState.lastEvent = "ROUTE EVENT RESOLVED: The damaged convoy was recovered and its cargo folded back into Shelter 17 logistics.";
    } else if (routeEventType == "merchant_window") {
        worldState.tradeCyclesCompleted = std::max(worldState.tradeCyclesCompleted, 1);
        worldState.infrastructureDecay = std::max(0.0f, worldState.infrastructureDecay - 0.8f);
        gameState.lastEvent = "ROUTE EVENT RESOLVED: The merchant window closed cleanly after a discreet field exchange and the broker trace folded back into the recovery ledger.";
    } else {
        gameState.lastEvent = "ROUTE EVENT RESOLVED: The field incident was contained.";
    }

    worldState.routeEventsResolved += 1;
    gameState.lastEvent += " " + CurrentRecoveryHandoffSummary(profile);
    AppendRecoveryBackboneReadabilityHint(gameState.lastEvent, profile);
}

void ApplyRouteEventFailure(SessionProfile& profile, WorldFieldState& worldState, GameState& gameState, bool expired) {
    const std::string routeEventType = worldState.activeRouteEventType;
    if (routeEventType == "service_call") {
        worldState.infrastructureDecay = std::min(100.0f, worldState.infrastructureDecay + 2.5f);
        profile.partnerTank.energyReserve = std::max(0.0f, profile.partnerTank.energyReserve - 5.0f);
        gameState.lastEvent = expired
            ? "ROUTE EVENT EXPIRED: The service call window closed. BT-72 reserve pressure climbed and field upkeep drifted upward."
            : "ROUTE EVENT FAILED: The service call slipped. BT-72 reserve pressure climbed and field upkeep drifted upward.";
    } else if (routeEventType == "field_refuel") {
        worldState.routeContamination = std::min(100.0f, worldState.routeContamination + 2.0f);
        profile.partnerTank.energyReserve = std::max(0.0f, profile.partnerTank.energyReserve - 10.0f);
        gameState.lastEvent = expired
            ? "ROUTE EVENT EXPIRED: The refuel window collapsed and BT-72 lost precious reserve stock."
            : "ROUTE EVENT FAILED: The refuel window collapsed under pressure and BT-72 lost precious reserve stock.";
    } else if (routeEventType == "relay_instability") {
        worldState.routeContamination = std::min(100.0f, worldState.routeContamination + 6.0f);
        worldState.infrastructureDecay = std::min(100.0f, worldState.infrastructureDecay + 2.0f);
        gameState.lastEvent = expired
            ? "ROUTE EVENT EXPIRED: Relay instability spilled back into the route and signal decay worsened."
            : "ROUTE EVENT FAILED: Relay instability broke containment and signal decay worsened.";
    } else if (routeEventType == "blocked_route") {
        worldState.routeOverrun = true;
        worldState.routeContamination = std::min(100.0f, worldState.routeContamination + 8.0f);
        worldState.infrastructureDecay = std::min(100.0f, worldState.infrastructureDecay + 2.0f);
        gameState.lastEvent = expired
            ? "ROUTE EVENT EXPIRED: The blocked lane folded back under debris pressure and the route is drifting toward overrun."
            : "ROUTE EVENT FAILED: The blocked lane broke cleanly and the route is drifting toward overrun.";
    } else if (routeEventType == "damaged_convoy") {
        worldState.routeContamination = std::min(100.0f, worldState.routeContamination + 3.0f);
        worldState.infrastructureDecay = std::min(100.0f, worldState.infrastructureDecay + 1.5f);
        gameState.lastEvent = expired
            ? "ROUTE EVENT EXPIRED: The convoy broke apart before recovery crews could stabilize it."
            : "ROUTE EVENT FAILED: The convoy collapsed before recovery crews could stabilize it.";
    } else if (routeEventType == "merchant_window") {
        worldState.routeContamination = std::min(100.0f, worldState.routeContamination + (expired ? 1.0f : 2.0f));
        gameState.lastEvent = expired
            ? "ROUTE EVENT EXPIRED: The merchant trace folded before the exchange could be made."
            : "ROUTE EVENT FAILED: The merchant trace panicked and cut the channel before the exchange landed.";
    } else {
        gameState.lastEvent = expired
            ? "ROUTE EVENT EXPIRED: The field incident closed unresolved."
            : "ROUTE EVENT FAILED: The field incident expired unresolved.";
    }

    if (expired) {
        worldState.routeEventsExpired += 1;
    } else {
        worldState.routeEventsFailed += 1;
    }
    AppendRecoveryBackboneReadabilityHint(gameState.lastEvent, profile);
}

struct RouteEventCandidate {
    const char* type = "";
    int weight = 0;
};

std::string PickRouteEventType(const SessionProfile& profile, const WorldFieldState& worldState, const GameState& gameState) {
    std::vector<RouteEventCandidate> candidates;
    if (profile.firstPlayableRoute.firstServicePerformed &&
        !worldState.serviceBayActive &&
        (TankNeedsRepair(profile) || worldState.serviceCyclesCompleted == 0)) {
        candidates.push_back({"service_call", 5 + RouteEventWeatherWeightBonus("service_call", gameState)});
    }
    if (profile.story.tankLinked &&
        (profile.partnerTank.energyReserve < 80.0f ||
            profile.partnerTank.ammoReserve < 70.0f ||
            InventoryCount(profile, "power_cell") == 0)) {
        candidates.push_back({"field_refuel", 4 + RouteEventWeatherWeightBonus("field_refuel", gameState)});
    }
    if (profile.firstPlayableRoute.firstRecoveryNodeActivated &&
        !worldState.relaySubstationActive &&
        (worldState.routeContamination >= 10.0f ||
            !worldState.localRelayAvailable ||
            !worldState.regionalGridOnline)) {
        candidates.push_back({"relay_instability", 3 + RouteEventWeatherWeightBonus("relay_instability", gameState)});
    }
    if (profile.story.outerRoadCleared &&
        (worldState.routeOverrun ||
            worldState.routeContamination >= 16.0f ||
            worldState.infrastructureDecay >= 18.0f)) {
        candidates.push_back({"blocked_route", 3 + RouteEventWeatherWeightBonus("blocked_route", gameState)});
    }
    if ((worldState.caravanRouteActive ||
            worldState.droneStationsActive ||
            worldState.tradeNetworkActive ||
            worldState.railFreightActive) &&
        worldState.serviceCyclesCompleted == 0) {
        candidates.push_back({"damaged_convoy", 2 + RouteEventWeatherWeightBonus("damaged_convoy", gameState)});
    }
    if (!worldState.routeOverrun &&
        worldState.routeContamination < 10.0f &&
        worldState.infrastructureDecay < 24.0f &&
        (worldState.tradeNetworkActive ||
            worldState.caravanRouteActive ||
            worldState.droneStationsActive ||
            worldState.railFreightActive ||
            worldState.serviceBayActive) &&
        (profile.lanlineServices.relayCredits >= 90 ||
            InventoryCount(profile, "trade_voucher") > 0) &&
        !RouteEventBlockedByWeather("merchant_window", gameState)) {
        candidates.push_back({"merchant_window", 1 + RouteEventWeatherWeightBonus("merchant_window", gameState)});
    }
    if (candidates.empty() && profile.story.tankLinked && TankNeedsRepair(profile)) {
        candidates.push_back({"service_call", 1 + RouteEventWeatherWeightBonus("service_call", gameState)});
    }
    if (candidates.empty()) {
        return {};
    }

    int totalWeight = 0;
    for (const auto& candidate : candidates) {
        totalWeight += candidate.weight;
    }
    const int pick = totalWeight > 0 ? (worldState.routeEventSerial % totalWeight) : 0;
    int cumulative = 0;
    for (const auto& candidate : candidates) {
        cumulative += candidate.weight;
        if (pick < cumulative) {
            return candidate.type;
        }
    }
    return candidates.front().type;
}

void BeginRouteEvent(WorldFieldState& worldState, std::string routeEventType, GameState& gameState) {
    worldState.activeRouteEventType = std::move(routeEventType);
    worldState.routeEventTimeRemaining = RouteEventDuration(worldState.activeRouteEventType);
    worldState.routeEventOfferTimeRemaining = RouteEventOfferDuration(worldState.activeRouteEventType);
    worldState.routeEventProgress = 0;
    worldState.routeEventStage = 0;
    worldState.routeEventCooldown = 0.0f;
    worldState.lastRouteEventType = worldState.activeRouteEventType;
    worldState.lastRouteEventOutcome.clear();
    worldState.routeEventSerial += 1;
    gameState.lastEvent = RouteEventStartText(worldState.activeRouteEventType) +
        RouteEventWeatherText(worldState.activeRouteEventType, gameState.weather);
}

void EndRouteEvent(WorldFieldState& worldState, bool success, std::string_view outcome) {
    const std::string routeEventType = worldState.activeRouteEventType;
    worldState.lastRouteEventType = routeEventType;
    worldState.lastRouteEventOutcome = std::string(outcome);
    worldState.activeRouteEventType.clear();
    worldState.routeEventTimeRemaining = 0.0f;
    worldState.routeEventOfferTimeRemaining = 0.0f;
    worldState.routeEventProgress = 0;
    worldState.routeEventStage = 0;
    worldState.routeEventCooldown = RouteEventCooldown(routeEventType, success);
}

}  // namespace

void UpdateRouteRandomEvents(SessionProfile& profile, GameState& gameState, float dt) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr) {
        return;
    }

    if (!RouteEventOnboardingComplete(profile)) {
        if (!HasActiveRouteEvent(*worldState)) {
            worldState->routeEventCooldown = 0.0f;
            worldState->routeEventOfferTimeRemaining = 0.0f;
            worldState->lastRouteEventType.clear();
            worldState->lastRouteEventOutcome.clear();
        }
        return;
    }

    if (!RouteEventLayerUnlocked(profile)) {
        if (!HasActiveRouteEvent(*worldState)) {
            worldState->routeEventCooldown = 0.0f;
            worldState->routeEventOfferTimeRemaining = 0.0f;
        }
        return;
    }

    if (HasActiveRouteEvent(*worldState)) {
        if (worldState->routeEventOfferTimeRemaining > 0.0f) {
            worldState->routeEventOfferTimeRemaining = std::max(0.0f, worldState->routeEventOfferTimeRemaining - dt);
            if (worldState->routeEventOfferTimeRemaining <= 0.0f) {
                gameState.lastEvent = RouteEventActivationText(worldState->activeRouteEventType);
            }
        }

        const float eventDuration = RouteEventDuration(worldState->activeRouteEventType);
        worldState->routeEventTimeRemaining = std::max(0.0f, worldState->routeEventTimeRemaining - dt);
        worldState->routeEventProgress = std::max(worldState->routeEventProgress, RouteEventProgress(profile, *worldState));
        if (worldState->routeEventOfferTimeRemaining <= 0.0f &&
            worldState->routeEventStage == 0 &&
            worldState->routeEventTimeRemaining <= eventDuration * 0.45f) {
            worldState->routeEventStage = 1;
            gameState.lastEvent = RouteEventEscalationText(worldState->activeRouteEventType);
        }
        if (worldState->routeEventProgress >= RouteEventGoal(worldState->activeRouteEventType)) {
            ApplyRouteEventSuccess(profile, *worldState, gameState);
            EndRouteEvent(*worldState, true, "success");
            return;
        }
        if (worldState->routeEventOfferTimeRemaining <= 0.0f &&
            RouteEventHardFail(profile, *worldState)) {
            ApplyRouteEventFailure(profile, *worldState, gameState, false);
            EndRouteEvent(*worldState, false, "failed");
            return;
        }
        if (worldState->routeEventTimeRemaining <= 0.0f) {
            ApplyRouteEventFailure(profile, *worldState, gameState, true);
            EndRouteEvent(*worldState, false, "expired");
        }
        return;
    }

    worldState->routeEventCooldown = std::max(0.0f, worldState->routeEventCooldown - dt);
    if (worldState->routeEventCooldown > 0.0f) {
        return;
    }

    const std::string routeEventType = PickRouteEventType(profile, *worldState, gameState);
    if (routeEventType.empty()) {
        worldState->routeEventCooldown = 30.0f;
        return;
    }

    BeginRouteEvent(*worldState, routeEventType, gameState);
}

bool TryResolveMerchantRouteEvent(SessionProfile& profile, GameState& gameState) {
    auto* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr) {
        gameState.lastEvent = "Merchant window could not resolve without selected-world state.";
        return false;
    }
    if (worldState->activeRouteEventType != "merchant_window" || !HasActiveRouteEvent(*worldState)) {
        gameState.lastEvent = "No merchant route event is currently open.";
        return false;
    }
    if (worldState->routeEventOfferTimeRemaining > 0.0f) {
        worldState->routeEventOfferTimeRemaining = 0.0f;
    }

    bool paidWithVoucher = ConsumeInventoryItem(profile, "trade_voucher", 1);
    if (!paidWithVoucher) {
        const int creditCost = 90;
        if (profile.lanlineServices.relayCredits < creditCost) {
            gameState.lastEvent = "Merchant window needs 1 trade voucher or 90 relay credits before the broker will commit.";
            return false;
        }
        profile.lanlineServices.relayCredits -= creditCost;
        worldState->relayCreditsSpent += creditCost;
    }

    switch (worldState->routeEventSerial % 3) {
        case 0:
            AddInventoryItem(profile, "repair_patch", 1, 0.2f);
            AddInventoryItem(profile, "filter_media", 1, 0.3f);
            break;
        case 1:
            AddInventoryItem(profile, "power_cell", 1, 0.3f);
            AddInventoryItem(profile, "field_battery", 1, 0.2f);
            break;
        default:
            AddInventoryItem(profile, "cryo_medkit", 1, 0.5f);
            AddInventoryItem(profile, "sealant_roll", 1, 0.2f);
            break;
    }

    worldState->tradeCyclesCompleted += 1;
    worldState->routeEventProgress = RouteEventGoal(worldState->activeRouteEventType);
    ApplyRouteEventSuccess(profile, *worldState, gameState);
    EndRouteEvent(*worldState, true, "success");
    return true;
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

    const float playerNoise = PlayerCombatNoise(player);
    for (auto& object : world.objects) {
        if (object.interaction != InteractionType::Hostile) {
            continue;
        }

        const float dx = player.x - object.x;
        const float dy = player.y - object.y;
        const float distanceSq = (dx * dx) + (dy * dy);
        const float distance = std::sqrt(distanceSq);

        const HostileRole role = ClassifyHostileRole(object);
        const MechanicalHostileDamageState* mechanicalDamage = FindMechanicalHostileDamageState(gameState, object.registryId);
        HostileAwarenessState& awareness = EnsureHostileAwarenessState(gameState, object.registryId);
        const float visualRadius = HostileVisualRadius(role, player, gameState) * HostileVisualScale(role, mechanicalDamage);
        const float hearingRadius = HostileHearingRadius(role, player, gameState, playerNoise);
        const bool visualContact = distance > 0.2f &&
            distance <= visualRadius &&
            !HasBlockingGeometryOnLine(world, object, player.x, player.y);
        const bool heardNearby = distance > 0.2f &&
            distance <= hearingRadius &&
            playerNoise >= 0.55f;
        UpdateHostileAwareness(awareness, role, visualContact, heardNearby, dt);

        const bool suspicious = awareness.awareness >= 18.0f;
        const bool engaged = awareness.awareness >= 55.0f;
        const float preferredMinRange = HostilePreferredMinRange(role) * HostileWeaponRangeScale(role, mechanicalDamage);
        const float preferredMaxRange = HostilePreferredMaxRange(role) * HostileWeaponRangeScale(role, mechanicalDamage);
        const bool usesStandoffMovement = preferredMaxRange > preferredMinRange;
        const bool rangedDiscipline = IsRangedDisciplineRole(role);
        bool shotLineBlocked = rangedDiscipline && HasBlockingGeometryOnLine(world, object, player.x, player.y);
        bool friendlyInLine = rangedDiscipline && HasFriendlyOnShotLine(world, object, player.x, player.y);
        const bool shouldRetreat =
            role == HostileRole::HumanTactical &&
            (object.health <= HostileMaxHealthHint(role) * 0.45f ||
                (player.insideTank && distance < preferredMaxRange + 0.35f));
        float coverTargetX = 0.0f;
        float coverTargetY = 0.0f;
        const bool shouldSeekCover =
            role == HostileRole::HumanTactical &&
            engaged &&
            shouldRetreat &&
            FindHumanCoverTarget(world, object, player.x, player.y, coverTargetX, coverTargetY);
        if (suspicious && distance > 0.2f) {
            const float step = dt * HostileAdvanceSpeed(role) * HostileMovementScale(role, mechanicalDamage) * (engaged ? 1.0f : 0.72f);
            float moveX = 0.0f;
            float moveY = 0.0f;
            bool adaptiveCoverMove = false;

            if (shouldSeekCover) {
                const float coverDx = coverTargetX - object.x;
                const float coverDy = coverTargetY - object.y;
                const float coverDistance = std::sqrt(coverDx * coverDx + coverDy * coverDy);
                if (coverDistance > 0.001f) {
                    const float stepScale = std::min(step, coverDistance) / coverDistance;
                    moveX = coverDx * stepScale;
                    moveY = coverDy * stepScale;
                    adaptiveCoverMove = true;
                }
            } else if (usesStandoffMovement && (distance < preferredMinRange || shouldRetreat)) {
                moveX = -(dx / distance) * step;
                moveY = -(dy / distance) * step;
            } else if (usesStandoffMovement && (shotLineBlocked || friendlyInLine)) {
                const float bias = HostileLateralBias(object);
                moveX = (-dy / distance) * step * bias;
                moveY = (dx / distance) * step * bias;
            } else if (usesStandoffMovement && distance <= preferredMaxRange && engaged) {
                const float bias = HostileLateralBias(object);
                moveX = (-dy / distance) * step * 0.7f * bias;
                moveY = (dx / distance) * step * 0.7f * bias;
            } else {
                moveX = (dx / distance) * step;
                moveY = (dy / distance) * step;
            }

            if (adaptiveCoverMove) {
                if (!TryMoveHostileAdaptive(world, object, moveX, moveY)) {
                    TryMoveHostile(world, object, moveX, moveY);
                }
            } else {
                TryMoveHostile(world, object, moveX, moveY);
            }
        }

        const float updatedDx = player.x - object.x;
        const float updatedDy = player.y - object.y;
        const float updatedDistance = std::sqrt((updatedDx * updatedDx) + (updatedDy * updatedDy));
        const float attackRange = HostileAttackRange(role, player.insideTank) * HostileWeaponRangeScale(role, mechanicalDamage);
        const bool lineBlockedAfterMove = HasBlockingGeometryOnLine(world, object, player.x, player.y);
        const bool postMoveVisualContact = updatedDistance > 0.2f &&
            updatedDistance <= visualRadius &&
            !lineBlockedAfterMove;
        shotLineBlocked = rangedDiscipline && lineBlockedAfterMove;
        friendlyInLine = rangedDiscipline && HasFriendlyOnShotLine(world, object, player.x, player.y);
        const bool attackOpportunity = updatedDistance < attackRange &&
            engaged &&
            gameState.damageCooldown <= 0.0f &&
            profile.character.hp > 0.0f &&
            (!rangedDiscipline || (!shotLineBlocked && !friendlyInLine && postMoveVisualContact));
        if (!attackOpportunity) {
            awareness.attackWindup = std::max(0.0f, awareness.attackWindup - dt * (rangedDiscipline ? 2.2f : 2.8f));
            continue;
        }

        const float attackWindupDuration = HostileAttackWindupDuration(role, player.insideTank);
        if (awareness.attackWindup < attackWindupDuration) {
            awareness.attackWindup = std::min(attackWindupDuration, awareness.attackWindup + dt);
            continue;
        }

        awareness.attackWindup = 0.0f;
        if (updatedDistance < attackRange &&
            engaged &&
            gameState.damageCooldown <= 0.0f &&
            profile.character.hp > 0.0f) {
            if (player.insideTank) {
                const bool bulwarkSync = TankHasBulwarkSync(profile);
                const bool ramShield = TankUsesRamShield(profile);
                float tankDamage = HostileTankDamage(role) * HostileWeaponDamageScale(role, mechanicalDamage);
                if (bulwarkSync) {
                    tankDamage *= 0.82f;
                }
                if (ramShield) {
                    tankDamage *= 0.85f;
                }
                const float hullBefore = profile.partnerTank.damage.hull;
                const float sensorsBefore = profile.partnerTank.damage.sensors;
                const float cockpitBefore = profile.partnerTank.damage.cockpit;
                profile.partnerTank.damage.hull = std::max(0.0f, profile.partnerTank.damage.hull - tankDamage);
                profile.partnerTank.damage.sensors = std::max(0.0f, profile.partnerTank.damage.sensors - (tankDamage * 0.35f));
                profile.partnerTank.damage.cockpit = std::max(0.0f, profile.partnerTank.damage.cockpit - (tankDamage * 0.2f));
                gameState.lastEvent = DescribeHostileImpact(
                    object,
                    role,
                    true,
                    bulwarkSync || ramShield);
                gameState.lastEvent += DescribeBt72DefenseImpact(
                    profile,
                    hullBefore,
                    profile.partnerTank.damage.hull,
                    sensorsBefore,
                    profile.partnerTank.damage.sensors,
                    cockpitBefore,
                    profile.partnerTank.damage.cockpit);
            } else {
                const float incomingDamage = HostileFootDamage(role, profile, gameState) * HostileWeaponDamageScale(role, mechanicalDamage);
                profile.character.hp = std::max(0.0f, profile.character.hp - incomingDamage);
                gameState.lastEvent = DescribeHostileImpact(object, role, false, false);
            }
            gameState.damageCooldown = HostileAttackCooldown(role);
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

    gameState.hostileAwareness.erase(
        std::remove_if(gameState.hostileAwareness.begin(), gameState.hostileAwareness.end(),
            [&](const HostileAwarenessState& state) { return !world.HasObject(state.registryId); }),
        gameState.hostileAwareness.end());
    gameState.mechanicalHostileDamage.erase(
        std::remove_if(gameState.mechanicalHostileDamage.begin(), gameState.mechanicalHostileDamage.end(),
            [&](const MechanicalHostileDamageState& state) { return !world.HasObject(state.registryId); }),
        gameState.mechanicalHostileDamage.end());
}

void HandleAttack(World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState) {
    const bool gunnerSeat = player.insideTank && player.bt72GunnerSeat;
    const float momentum = std::sqrt((player.velocityX * player.velocityX) + (player.velocityY * player.velocityY));
    const MapObject* hostile = FindNearestHostile(world, player.x, player.y, player.insideTank ? (gunnerSeat ? 6.6f : 2.6f) : 1.8f);
    const MapObject* reactiveBreakable = FindNearestReactiveBreakable(
        world,
        player,
        player.insideTank ? (gunnerSeat ? 7.2f : 2.8f) : 1.9f,
        true);
    if (hostile == nullptr && reactiveBreakable == nullptr) {
        gameState.lastEvent = player.insideTank
            ? (gunnerSeat ? "No hostile target in gunner range." : "No hostile target in ram range.")
            : "No hostile target in melee range.";
        return;
    }

    if (gunnerSeat) {
        const float ammoCost = std::max(1.6f, 3.0f - Bt72AttackAmmoDiscount(profile, true));
        const float energyCost = std::max(1.0f, 2.0f - Bt72AttackEnergyDiscount(profile, gameState, true));
        if (profile.partnerTank.ammoReserve < ammoCost || profile.partnerTank.energyReserve < energyCost) {
            gameState.lastEvent = "BT-72 gunner seat lacks ammo or power for a support burst.";
            return;
        }
        profile.partnerTank.ammoReserve = std::max(0.0f, profile.partnerTank.ammoReserve - ammoCost);
        profile.partnerTank.energyReserve = std::max(0.0f, profile.partnerTank.energyReserve - energyCost);
        player.recoilOffset = std::min(0.5f, player.recoilOffset + 0.12f);
        gameState.tankThermalLoad = std::min(100.0f, gameState.tankThermalLoad + 4.0f);
    }

    if (hostile == nullptr && reactiveBreakable != nullptr) {
        if (gunnerSeat) {
            TriggerCombatFeedback(player, 0.24f, 0.52f, 0.0f, 0.0f);
        } else if (player.insideTank) {
            TriggerCombatFeedback(player, 0.0f, 0.0f, 0.18f, std::min(0.85f, 0.32f + momentum * 0.14f));
        }
        ApplyReactiveBreakableHit(
            world,
            *reactiveBreakable,
            player,
            staticEraser,
            gameState,
            profile.selectedWorld,
            false,
            false,
            momentum);
        return;
    }

    if (gunnerSeat) {
        if (MapObject* lineBreakable = FindReactiveBreakableOnShotLine(world, player, hostile->x, hostile->y);
            lineBreakable != nullptr) {
            TriggerCombatFeedback(player, 0.24f, 0.52f, 0.0f, 0.0f);
            ApplyReactiveBreakableHit(
                world,
                *lineBreakable,
                player,
                staticEraser,
                gameState,
                profile.selectedWorld,
                false,
                true,
                0.0f);
            return;
        }
    }

    if (auto* mutableObject = const_cast<MapObject*>(hostile); mutableObject != nullptr) {
        const HostileRole role = ClassifyHostileRole(*mutableObject);
        const MechanicalHostileDamageState* mechanicalDamage =
            FindMechanicalHostileDamageState(gameState, mutableObject->registryId);
        const bool hasEmergencyMeleeTool = HasEmergencyMeleeTool(profile);
        const bool fieldReflexEquipped = !player.insideTank && HasEquippedPassiveSkill(profile, "skill_field_reflex");
        const bool crewSupportReadable = player.insideTank &&
            HasEquippedPassiveSkill(profile, "skill_pilot_sync") &&
            HasBt72CrewSupport(profile);
        const float footReflexBoost = player.insideTank ? 1.0f : FootReflexCombatBoost(profile, gameState);
        const float crewCoordinationBoost = player.insideTank ? Bt72CrewCoordinationBoost(profile, gameState, gunnerSeat) : 1.0f;
        const float linkFactor = player.insideTank ? (0.88f + profile.partnerTank.trustLink * (gunnerSeat ? 0.24f : 0.18f)) : 1.0f;
        const float sensorFactor = player.insideTank ? std::clamp(profile.partnerTank.damage.sensors / 100.0f, 0.58f, 1.0f) : 1.0f;
        const float thermalFactor = player.insideTank
            ? (gameState.tankThermalLoad >= 80.0f ? 0.88f
                : (gameState.tankThermalLoad >= 55.0f ? 0.94f
                    : (gameState.tankThermalLoad >= 25.0f ? 1.03f : 1.0f)))
            : 1.0f;
        float damage = gunnerSeat
            ? (24.0f + static_cast<float>(EffectiveStatValue(profile, gameState, 'P')) * 1.8f)
            : (player.insideTank
            ? ((TankUsesRamShield(profile) ? 36.0f : 28.0f) +
                static_cast<float>(EffectiveStatValue(profile, gameState, 'P')) * 1.5f +
                momentum * (TankUsesRamShield(profile) ? 4.0f : 2.7f))
            : ((hasEmergencyMeleeTool ? 20.0f : 14.0f) + static_cast<float>(EffectiveStatValue(profile, gameState, 'S')) * (hasEmergencyMeleeTool ? 1.05f : 0.9f)));
        if (player.insideTank) {
            damage *= linkFactor * sensorFactor * thermalFactor * crewCoordinationBoost;
        } else {
            damage *= footReflexBoost;
        }
        if (gunnerSeat && role == HostileRole::HumanTactical) {
            damage += 4.0f;
        } else if (player.insideTank && !gunnerSeat && role == HostileRole::GhoulRush) {
            damage += 5.0f;
        }
        const CombatDamageWindow damageWindow = EvaluateCombatDamageWindow(
            *mutableObject,
            role,
            mechanicalDamage,
            player,
            profile,
            gameState,
            false,
            momentum);
        damage = damage * damageWindow.multiplier + damageWindow.flatDamage;
        const std::string mechanicalFeedback = ApplyMechanicalHostileDamage(
            *mutableObject,
            role,
            player,
            profile,
            gameState,
            false,
            momentum);
        const std::string windowFeedback = DescribeCombatDamageWindow(damageWindow, player.insideTank, gunnerSeat, false);
        mutableObject->health -= damage;
        if (player.insideTank) {
            RegisterTankSyncStyle(profile, !gunnerSeat);
        }
        if (gunnerSeat) {
            TriggerCombatFeedback(player, 0.24f, 0.62f, 0.0f, 0.0f);
        } else if (player.insideTank) {
            TriggerCombatFeedback(player, 0.0f, 0.0f, 0.24f, std::min(1.0f, 0.38f + momentum * 0.12f));
        }
        if (mutableObject->health <= 0.0f) {
            std::string progressionEvent;
            AwardExperience(profile, player.insideTank ? 45 : 30, &progressionEvent);
            std::string skillEvent;
            if (player.insideTank) RegisterTankAction(profile, &skillEvent);
            else RegisterFootKill(profile, &skillEvent);
            AddInventoryItem(profile, player.insideTank ? "wreck_scrap" : "ether_tissue", 1, 0.4f);
            const std::string routeEvent = ResolveFirstPlayableRouteKill(*mutableObject, player.insideTank, profile);
            staticEraser.Erase(mutableObject->registryId);
            staticEraser.Save(profile.selectedWorld);
            gameState.lastEvent = mutableObject->displayName + " neutralized. " + progressionEvent;
            gameState.lastEvent += windowFeedback;
            if (!skillEvent.empty()) {
                gameState.lastEvent += " " + skillEvent;
            }
            if (!routeEvent.empty()) {
                gameState.lastEvent += " " + routeEvent;
                AppendObjectiveRuntimeHint(gameState.lastEvent, profile, staticEraser);
                AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
            }
        } else {
            gameState.lastEvent = gunnerSeat
                ? (crewSupportReadable
                    ? "BT-72 gunner burst landed. Pilot Sync and crew discipline tightened the burst and armor fragments peeled off the target."
                    : "BT-72 gunner burst landed. Muzzle flash lit the lane and armor fragments peeled off the target.")
                : (player.insideTank
                ? "Ram impact landed. Shock rolled through the lane as BT-72 drove the contact back."
                : (fieldReflexEquipped
                    ? (hasEmergencyMeleeTool
                        ? "Emergency baton strike landed. Field Reflex kept the operator tight on the opening exchange."
                        : "Strike landed cleanly. Field Reflex kept the operator tight on the opening exchange.")
                    : (hasEmergencyMeleeTool ? "Emergency baton strike landed." : "Strike landed on hostile target.")));
            gameState.lastEvent += windowFeedback;
            gameState.lastEvent += mechanicalFeedback;
        }
    }
}

void HandleSpecialAttack(World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState) {
    const bool gunnerSeat = player.insideTank && player.bt72GunnerSeat;
    const float radius = player.insideTank ? (gunnerSeat ? 10.5f : 9.0f) : 6.5f;
    const MapObject* hostile = FindNearestHostile(world, player.x, player.y, radius);
    const MapObject* reactiveBreakable = FindNearestReactiveBreakable(world, player, radius, true);
    if (hostile == nullptr && reactiveBreakable == nullptr) {
        gameState.lastEvent = player.insideTank
            ? (gunnerSeat ? "No hostile target in the BT-72 gunner arc." : "No hostile target in cannon range.")
            : "No hostile target in firing range.";
        return;
    }

    if (player.insideTank) {
        const float ammoCost = std::max(3.5f, (gunnerSeat ? 6.0f : 8.0f) - Bt72AttackAmmoDiscount(profile, gunnerSeat));
        const float energyCost = std::max(2.0f,
            (gunnerSeat ? 4.0f : (TankHasStabilizerSync(profile) ? 5.0f : 6.0f)) -
            Bt72AttackEnergyDiscount(profile, gameState, gunnerSeat));
        if (profile.partnerTank.ammoReserve < ammoCost || profile.partnerTank.energyReserve < energyCost) {
            gameState.lastEvent = gunnerSeat
                ? "BT-72 gunner seat lacks ammo or power for a support cannon cycle."
                : "BT-72 lacks ammo or energy for a cannon strike.";
            return;
        }
        profile.partnerTank.ammoReserve = std::max(0.0f, profile.partnerTank.ammoReserve - ammoCost);
        profile.partnerTank.energyReserve = std::max(0.0f, profile.partnerTank.energyReserve - energyCost);
        float recoilPush = gunnerSeat ? 0.08f : (TankHasStabilizerSync(profile) ? 0.32f : 0.55f);
        float recoilOffset = gunnerSeat ? 0.16f : (TankHasStabilizerSync(profile) ? 0.18f : 0.32f);
        if (HasEquippedPassiveSkill(profile, "skill_muscle_memory")) {
            const float stabilityFactor = std::max(0.72f, 1.0f - static_cast<float>(EffectiveStatValue(profile, gameState, 'S')) * 0.018f);
            recoilPush *= stabilityFactor;
            recoilOffset *= stabilityFactor;
        }
        player.velocityX -= std::cos(player.facingRadians) * recoilPush;
        player.velocityY -= std::sin(player.facingRadians) * recoilPush;
        player.recoilOffset = std::min(0.75f, player.recoilOffset + recoilOffset);
        gameState.tankThermalLoad = std::min(100.0f, gameState.tankThermalLoad + (gunnerSeat ? 8.0f : (TankHasStabilizerSync(profile) ? 10.0f : 14.0f)));
    } else {
        if (!ConsumeInventoryItem(profile, "#%it_ptrs_ammo", 1)) {
            gameState.lastEvent = "No PTRS ammo available for a ranged shot.";
            return;
        }
        const float precisionCost = std::max(4.0f,
            8.0f -
            static_cast<float>(std::max(0, EffectiveStatValue(profile, gameState, 'A') - 5)) * 0.35f -
            (HasEquippedPassiveSkill(profile, "skill_field_reflex") ? 1.4f : 0.0f));
        if (profile.character.mp < precisionCost) {
            AddInventoryItem(profile, "#%it_ptrs_ammo", 1, 0.7f);
            gameState.lastEvent = "Not enough MP to stabilize a precision shot.";
            return;
        }
        profile.character.mp = std::max(0.0f, profile.character.mp - precisionCost);
    }

    auto triggerSpecialFeedback = [&]() {
        if (player.insideTank) {
            TriggerCombatFeedback(
                player,
                gunnerSeat ? 0.34f : 0.48f,
                gunnerSeat ? 0.82f : 1.0f,
                gunnerSeat ? 0.24f : 0.38f,
                gunnerSeat ? 0.62f : 1.0f);
        } else {
            TriggerCombatFeedback(player, 0.18f, 0.34f, 0.0f, 0.0f);
        }
    };

    if (hostile == nullptr && reactiveBreakable != nullptr) {
        triggerSpecialFeedback();
        ApplyReactiveBreakableHit(
            world,
            *reactiveBreakable,
            player,
            staticEraser,
            gameState,
            profile.selectedWorld,
            true,
            false,
            0.0f);
        return;
    }

    if (MapObject* lineBreakable = FindReactiveBreakableOnShotLine(world, player, hostile->x, hostile->y);
        lineBreakable != nullptr) {
        triggerSpecialFeedback();
        ApplyReactiveBreakableHit(
            world,
            *lineBreakable,
            player,
            staticEraser,
            gameState,
            profile.selectedWorld,
            true,
            true,
            0.0f);
        return;
    }

    if (auto* mutableObject = const_cast<MapObject*>(hostile); mutableObject != nullptr) {
        const HostileRole role = ClassifyHostileRole(*mutableObject);
        const MechanicalHostileDamageState* mechanicalDamage =
            FindMechanicalHostileDamageState(gameState, mutableObject->registryId);
        const bool fieldReflexEquipped = !player.insideTank && HasEquippedPassiveSkill(profile, "skill_field_reflex");
        const bool crewSupportReadable = player.insideTank &&
            HasEquippedPassiveSkill(profile, "skill_pilot_sync") &&
            HasBt72CrewSupport(profile);
        const float footReflexBoost = player.insideTank ? 1.0f : FootReflexCombatBoost(profile, gameState);
        const float crewCoordinationBoost = player.insideTank ? Bt72CrewCoordinationBoost(profile, gameState, gunnerSeat) : 1.0f;
        const float linkFactor = player.insideTank ? (0.9f + profile.partnerTank.trustLink * 0.2f) : 1.0f;
        const float sensorFactor = player.insideTank ? std::clamp(profile.partnerTank.damage.sensors / 100.0f, 0.55f, 1.0f) : 1.0f;
        const float thermalFactor = player.insideTank
            ? (gameState.tankThermalLoad >= 80.0f ? 0.86f
                : (gameState.tankThermalLoad >= 55.0f ? 0.93f
                    : (gameState.tankThermalLoad >= 25.0f ? 1.05f : 1.0f)))
            : 1.0f;
        float damage = player.insideTank
            ? ((gunnerSeat ? 58.0f : (TankHasStabilizerSync(profile) ? 50.0f : 45.0f)) +
                static_cast<float>(EffectiveStatValue(profile, gameState, 'P')) * (gunnerSeat ? 2.2f : 2.0f))
            : (24.0f + static_cast<float>(EffectiveStatValue(profile, gameState, 'P')) * 1.1f);
        if (player.insideTank) {
            damage *= linkFactor * sensorFactor * thermalFactor * crewCoordinationBoost;
        } else {
            damage *= footReflexBoost;
        }
        if (player.insideTank && role == HostileRole::RobotControl) {
            damage += gunnerSeat ? 6.0f : 9.0f;
        }
        const CombatDamageWindow damageWindow = EvaluateCombatDamageWindow(
            *mutableObject,
            role,
            mechanicalDamage,
            player,
            profile,
            gameState,
            true,
            0.0f);
        damage = damage * damageWindow.multiplier + damageWindow.flatDamage;
        const std::string mechanicalFeedback = ApplyMechanicalHostileDamage(
            *mutableObject,
            role,
            player,
            profile,
            gameState,
            true,
            0.0f);
        const std::string windowFeedback = DescribeCombatDamageWindow(damageWindow, player.insideTank, gunnerSeat, true);
        mutableObject->health -= damage;
        if (player.insideTank) {
            RegisterTankSyncStyle(profile, false);
        }
        triggerSpecialFeedback();
        if (mutableObject->health <= 0.0f) {
            std::string progressionEvent;
            AwardExperience(profile, player.insideTank ? 60 : 40, &progressionEvent);
            std::string skillEvent;
            if (player.insideTank) RegisterTankAction(profile, &skillEvent);
            else RegisterFootKill(profile, &skillEvent);
            AddInventoryItem(profile, player.insideTank ? "wreck_scrap" : "ether_tissue", 1, 0.4f);
            const std::string routeEvent = ResolveFirstPlayableRouteKill(*mutableObject, player.insideTank, profile);
            staticEraser.Erase(mutableObject->registryId);
            staticEraser.Save(profile.selectedWorld);
            gameState.lastEvent = mutableObject->displayName + " destroyed by special attack. " + progressionEvent;
            gameState.lastEvent += windowFeedback;
            if (!skillEvent.empty()) {
                gameState.lastEvent += " " + skillEvent;
            }
            if (!routeEvent.empty()) {
                gameState.lastEvent += " " + routeEvent;
                AppendObjectiveRuntimeHint(gameState.lastEvent, profile, staticEraser);
                AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
            }
        } else {
            gameState.lastEvent = player.insideTank
                ? (gunnerSeat
                    ? (crewSupportReadable
                        ? "BT-72 gunner cannon strike landed. Pilot Sync and crew discipline kept the support flash tight across the lane."
                        : "BT-72 gunner cannon strike landed. Support flash and shock rippled through the lane.")
                        : "Cannon strike landed. Heavy muzzle flash and shock wave rolled off the hull.")
                : (fieldReflexEquipped
                    ? "Precision shot landed. Field Reflex steadied the opening shot."
                    : "Precision shot landed.");
            gameState.lastEvent += windowFeedback;
            gameState.lastEvent += mechanicalFeedback;
        }
    }
}

void HandleInteraction(const MapObject* nearest,
    World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState,
    const WorldExecutionContext* executionContext) {
    if (nearest == nullptr) {
        gameState.lastEvent = "No actionable target nearby.";
        return;
    }

    if (HandleScriptTagInteraction(nearest, world, player, profile, gameState) &&
        NormalizeGameplayDescriptorTag(nearest->scriptTag) != "workshop_service") {
        if (NormalizeGameplayDescriptorTag(nearest->scriptTag) == "specialist_cryo") {
            staticEraser.Erase(nearest->registryId);
            staticEraser.Save(profile.selectedWorld);
            world.RemoveObject(nearest->registryId);
            SaveSessionProfile(profile, DefaultSessionProfilePath());
        }
        return;
    }

    if (nearest->registryId == "[%cryo_0001]") {
        if (profile.story.awakenedFromCryo) {
            gameState.lastEvent = "Cryostasis already terminated. Emergency route markers remain on the capsule shell.";
            return;
        }
        profile.firstPlayableRoute.introSeen = true;
        profile.story.awakenedFromCryo = true;
        profile.firstPlayableRoute.prePipPadClueCount = std::max(profile.firstPlayableRoute.prePipPadClueCount, 1);
        std::string continuityDiagnostic;
        SeedContinuityAnchorAfterBunkerAnomaly(profile, &continuityDiagnostic);
        if (!HasEmergencyMeleeTool(profile)) {
            AddInventoryItem(profile, "#%it_emergency_baton", 1, 1.2f);
            profile.firstPlayableRoute.emergencyMeleeRecovered = true;
            gameState.lastEvent = "Cryostasis terminated. Adjacent pods are split open, several berths stand empty, and an emergency baton plus stained service sketch were recovered from the capsule tray.";
        } else {
            gameState.lastEvent = "Cryostasis terminated across the shared cryo tier. Memory loss remains, but movement is stable.";
        }
        if (!continuityDiagnostic.empty()) {
            gameState.lastEvent += " " + continuityDiagnostic;
        }
        return;
    }

    if (nearest->registryId == "[%core_0001]") {
        if (!profile.story.pipPadRecovered) {
            profile.firstPlayableRoute.prePipPadClueCount = std::max(profile.firstPlayableRoute.prePipPadClueCount, 2);
            if (!profile.firstPlayableRoute.accessCardRecovered) {
                profile.firstPlayableRoute.accessCardRecovered = true;
                AddInventoryItem(profile, "bunker_access_card", 1, 0.1f);
                gameState.lastEvent = "Core-rack clipboard still holds a bunker access card. The card opens the archive wing and the Pip-Pad locker, while the papers point to a missing BT-72 starter core.";
                return;
            }
            gameState.lastEvent = "Paper maintenance sheets mention a missing BT-72 starter core and a lockout that only a Pip-Pad can clear.";
            return;
        }
        if (profile.firstPlayableRoute.bt72CoreRecovered) {
            gameState.lastEvent = "Central core rack already stripped for the BT-72 starter assembly.";
            return;
        }
        profile.firstPlayableRoute.bt72CoreRecovered = true;
        AddInventoryItem(profile, "power_cell", 1, 0.3f);
        AddCollectedTapeIfMissing(profile, "bt72_core_service_001", "BT-72 Starter Core Ledger");
        gameState.lastEvent = "Starter core package recovered from the rack. BT-72 can be powered once the hull and service notes are ready.";
        return;
    }

    if (nearest->registryId == "[%garage_0001]") {
        if (!profile.story.pipPadRecovered) {
            profile.firstPlayableRoute.prePipPadClueCount = std::max(profile.firstPlayableRoute.prePipPadClueCount, 2);
            gameState.lastEvent = "Old lift diagrams and grease-pencil notes point to a dormant BT-72 berth deeper in the garage.";
            return;
        }
        if (profile.firstPlayableRoute.bt72HullInspected) {
            gameState.lastEvent = profile.firstPlayableRoute.bt72Restored
                ? "Garage lift remains locked around a partially restored BT-72 berth."
                : "Garage lift survey already mirrored. BT-72 still needs its core and service notes.";
            return;
        }
        profile.firstPlayableRoute.bt72HullInspected = true;
        AddInventoryItem(profile, "old_plate", 1, 0.5f);
        AddCollectedTapeIfMissing(profile, "bt72_hull_survey_001", "BT-72 Hull Survey Sheet");
        gameState.lastEvent = "Garage lift survey mirrored. One serviceable hull plate recovered for BT-72 restoration.";
        return;
    }

    if (nearest->registryId == "[%hangar_power_0001]") {
        if (profile.hangarPowerRestored) {
            gameState.lastEvent = "Hangar power bus already restored. Crane controls still require a separate online cycle.";
            return;
        }
        profile.hangarPowerRestored = true;
        gameState.lastEvent = "Hangar power bus restored. Crane control can now be brought online.";
        return;
    }

    if (nearest->registryId == "[%bt72_crane_control_0001]") {
        if (!profile.hangarPowerRestored) {
            gameState.lastEvent = "BT-72 crane control is dark. Restore hangar power first.";
            return;
        }
        if (profile.bt72CraneControlOnline) {
            gameState.lastEvent = "BT-72 crane control is already online.";
            return;
        }
        profile.bt72CraneControlOnline = true;
        gameState.lastEvent = "BT-72 crane control online. Clear the crane path before attaching the hull.";
        return;
    }

    if (nearest->registryId == "[%bt72_crane_path_0001]") {
        if (!profile.hangarPowerRestored || !profile.bt72CraneControlOnline) {
            gameState.lastEvent = "BT-72 crane path cannot be cleared until hangar power and crane control are online.";
            return;
        }
        if (profile.bt72CranePathClear) {
            gameState.lastEvent = "BT-72 crane path is already clear.";
            return;
        }
        profile.bt72CranePathClear = true;
        gameState.lastEvent = "BT-72 crane path cleared. Hook can now attach the surveyed hull.";
        return;
    }

    if (nearest->registryId == "[%bt72_crane_hook_0001]") {
        if (!profile.firstPlayableRoute.bt72HullInspected) {
            gameState.lastEvent = "BT-72 crane hook has no verified lift points. Inspect the hull berth first.";
            return;
        }
        if (!profile.hangarPowerRestored) {
            gameState.lastEvent = "BT-72 crane hook is unpowered. Restore hangar power first.";
            return;
        }
        if (!profile.bt72CraneControlOnline) {
            gameState.lastEvent = "BT-72 crane hook is locked out. Bring crane control online first.";
            return;
        }
        if (!profile.bt72CranePathClear) {
            gameState.lastEvent = "BT-72 crane hook path is blocked. Clear the crane path first.";
            return;
        }
        if (!AttachBt72HullToCrane(profile)) {
            gameState.lastEvent = profile.bt72HullLockedInRestorationCradle
                ? "BT-72 hull is already locked in the restoration cradle."
                : "BT-72 crane hook failed to attach the hull.";
            return;
        }
        gameState.lastEvent = "BT-72 hull attached to the hangar crane hook. Move it to the service lift cradle.";
        return;
    }

    if (nearest->registryId == "[%bt72_service_lift_0001]") {
        if (!profile.bt72HullAttachedToCrane) {
            gameState.lastEvent = "BT-72 service lift is waiting for a crane-attached hull.";
            return;
        }
        if (!MoveBt72HullToServiceLift(profile)) {
            gameState.lastEvent = "BT-72 service lift could not receive the hull. Check crane power, control, and path state.";
            return;
        }
        gameState.lastEvent = "BT-72 hull moved to the service lift and locked in the restoration cradle.";
        return;
    }

    if (nearest->registryId == "[%pip_0001]") {
        if (profile.story.pipPadRecovered) {
            BeginPipDeviceReselect(profile);
            gameState.lastEvent = "Pip-Boy cradle opened. Current device is being returned to the rack; choose another model when the swap cycle completes.";
            return;
        }
        if (!profile.firstPlayableRoute.accessCardRecovered) {
            gameState.lastEvent = "The Pip-Boy/Pip-Pad selection station is still under mechanical card-lock. Pull a bunker access card from the core service racks first.";
            return;
        }

        SelectPipDevice(profile, "#%it_pippad");
        AddInventoryItem(profile, "cryo_medkit", 1, 0.5f);
        gameState.lastEvent = profile.firstPlayableRoute.prePipPadClueCount >= 2
            ? "Pip-Boy/Pip-Pad station selected the default Pip-Pad 3500. Press TAB to open the Pip-Pad; the bunker paper trail now makes sense."
            : "Pip-Boy/Pip-Pad station selected the default Pip-Pad 3500. Press TAB to open the Pip-Pad; the bunker paper trail is still incomplete.";
        return;
    }

    if (nearest->registryId == "[%bluelink_module_0001]") {
        if (!HasPipPad(profile)) {
            gameState.lastEvent = "BlueLink module handshake rejected. Recover the Pip-Pad before collecting field media modules.";
            return;
        }
        if (profile.blueLinkModuleRecovered) {
            gameState.lastEvent = "BlueLink Media Module already recovered. Install it through the Pip-Pad expansion bay.";
            return;
        }
        profile.blueLinkModuleRecovered = true;
        gameState.lastEvent = "BlueLink Media Module recovered. Pip-Pad media index remains locked until the expansion bay cover is removed and the module is installed.";
        return;
    }

    if (nearest->registryId == "[%pippad_expansion_bay_0001]") {
        if (!HasPipPad(profile)) {
            gameState.lastEvent = "Pip-Pad expansion bay unavailable. Recover the Pip-Pad first.";
            return;
        }
        if (!HasBlueLinkModule(profile)) {
            gameState.lastEvent = "Pip-Pad expansion bay still has its dummy cover. Recover the BlueLink Media Module before installation.";
            return;
        }
        if (IsBlueLinkInstalled(profile)) {
            gameState.lastEvent = "BlueLink Media Module already installed. Media index is available.";
            return;
        }
        if (InstallBlueLinkModule(profile)) {
            gameState.lastEvent = "BlueLink Media Module installed. Media index, audio, transcript, and translation channels are now available for recovered media.";
        } else {
            gameState.lastEvent = "BlueLink installation failed. Check Pip-Pad recovery and module state.";
        }
        return;
    }

    if (nearest->registryId == "[%archive_0001]") {
        if (!profile.firstPlayableRoute.accessCardRecovered) {
            gameState.lastEvent = "Archive wing is still under a dead mechanical interlock. Recover a bunker access card first.";
            return;
        }
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
        gameState.lastEvent = "Archive sync complete. One reactor core and one body are still missing, and BT-72 service traces terminate near the garage.";
        if (!skillEvent.empty()) {
            gameState.lastEvent += " " + skillEvent;
        }
        return;
    }

    if (nearest->registryId == "[%bt72_service_notes_0001]") {
        if (!HasPipPad(profile)) {
            gameState.lastEvent = "BT-72 service notes require Pip-Pad recovery before holo-records can be mirrored.";
            return;
        }
        if (!profile.story.archiveRecovered) {
            gameState.lastEvent = "BT-72 service notes remain encrypted until archive personnel records are mirrored.";
            return;
        }
        if (profile.firstPlayableRoute.bt72ServiceNotesRecovered) {
            gameState.lastEvent = "BT-72 service notes already mirrored to Pip-Pad.";
            return;
        }
        profile.firstPlayableRoute.bt72ServiceNotesRecovered = true;
        AddCollectedTapeIfMissing(profile, "bt72_service_reel_001", "BT-72 Service Reel");
        gameState.lastEvent = "BT-72 service notes and holo-records mirrored to Pip-Pad.";
        return;
    }

    if (nearest->registryId == "[%bt72_repair_patch_0001]") {
        if (!HasPipPad(profile)) {
            gameState.lastEvent = "Repair patch locker requires Pip-Pad recovery before the restoration manifest can be checked.";
            return;
        }
        if (HasInventoryItem(profile, "repair_patch")) {
            gameState.lastEvent = "BT-72 repair patch already recovered for restoration.";
            return;
        }
        AddInventoryItem(profile, "repair_patch", 1, 0.2f);
        gameState.lastEvent = "Repair patch recovered for BT-72 restoration.";
        return;
    }

    if (nearest->registryId == "[#tr_hull_0001]") {
        if (!profile.story.pipPadRecovered) {
            gameState.lastEvent = "The tank berth is locked behind dead diagnostics. Recover the Pip-Pad first.";
            return;
        }
        if (!profile.firstPlayableRoute.bt72Restored) {
            if (!HasBt72RestorationPrerequisites(profile) || !HasBt72RestorationMaterials(profile)) {
                gameState.lastEvent = DescribeBt72RestorationNeeds(profile);
                return;
            }
            ConsumeInventoryItem(profile, "power_cell", 1);
            ConsumeInventoryItem(profile, "repair_patch", 1);
            ConsumeInventoryItem(profile, "old_plate", 1);
            ApplyStarterBt72Restoration(profile);
            profile.partnerTank.secondSeatUnlocked = true;
            player.bucketRaised = false;
            gameState.lastEvent = "BT-72 restored to partial field condition. Cockpit link and training HUD are now available.";
            AppendObjectivePreviewHint(gameState.lastEvent, profile);
            AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
            return;
        }
        if (!player.insideTank && profile.partnerTank.damage.hull <= 0.0f) {
            gameState.lastEvent = "BT-72 hull is disabled. Run workshop service before attempting another link.";
            return;
        }

        const bool firstLink = !profile.story.tankLinked;
        player.insideTank = !player.insideTank;
        player.bt72GunnerSeat = false;
        profile.partnerTank.deployed = player.insideTank;
        profile.story.tankLinked = profile.story.tankLinked || player.insideTank;
        profile.partnerTank.secondSeatUnlocked = profile.partnerTank.secondSeatUnlocked || profile.story.tankLinked;
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
            if (profile.partnerTank.assignedGunnerHandle == profile.character.displayName) {
                profile.partnerTank.assignedGunnerHandle.clear();
            }
            profile.partnerTank.worldX = player.x;
            profile.partnerTank.worldY = player.y;
            profile.partnerTank.worldPositionKnown = true;
        }
        gameState.lastEvent = player.insideTank
            ? (firstLink
                    ? "BT-72 link established. Cockpit channel synchronized and first-drive diagnostics are live."
                    : "BT-72 link re-established. Cockpit channel synchronized.")
            : "BT-72 link suspended. Returning to foot movement.";
        if (player.insideTank) {
            AppendObjectivePreviewHint(gameState.lastEvent, profile);
            AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
        }
        return;
    }

    if (nearest->registryId == "#%it_bucket_0001") {
        if (!profile.story.tankLinked) {
            gameState.lastEvent = "Bucket-rack work is pointless without a live BT-72 link.";
            return;
        }
        if (player.bt72GunnerSeat) {
            gameState.lastEvent = "Return to BT-72 pilot controls before fitting the clearance rack.";
            return;
        }
        if (profile.firstPlayableRoute.clearanceModuleInstalled) {
            gameState.lastEvent = "Clearance module already installed on BT-72.";
            return;
        }
        if (!profile.firstPlayableRoute.clearanceBlueprintRecovered) {
            gameState.lastEvent = "The rack makes no sense yet. Decode the clearance module blueprint from the maintenance echo first.";
            return;
        }
        if (!profile.firstPlayableRoute.clearanceMaterialsRecovered) {
            GrantContainerLoot(profile, *nearest, 0.4f);
            profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
            gameState.lastEvent = "Bucket rack broken down into usable clearance parts. Bring BT-72 in for final install.";
            return;
        }
        if (!IsTankNearServicePoint(profile, *nearest, 4.2f)) {
            gameState.lastEvent = "Bring BT-72 to the rack before installing the clearance module.";
            return;
        }
        if (!HasClearanceInstallMaterials(profile)) {
            gameState.lastEvent = DescribeClearanceModuleNeeds(profile);
            return;
        }
        auto* module = FindTankModule(profile, TankModuleSlotType::Bucket);
        if (module != nullptr) {
            module->moduleId = "bucket_shield_a";
            module->displayName = "Bucket Rig Mk.I";
            module->installed = true;
        }
        ConsumeInventoryItem(profile, "scrap_steel", 1);
        ConsumeInventoryItem(profile, "hydraulic_seal", 1);
        player.bucketRaised = true;
        profile.firstPlayableRoute.clearanceModuleInstalled = true;
        profile.story.bucketRecovered = true;
        staticEraser.Erase(nearest->registryId);
        staticEraser.Save(profile.selectedWorld);
        world.RemoveObject(nearest->registryId);
        gameState.lastEvent = "Clearance module installed on BT-72. Heavy debris routes can now be challenged.";
        AppendObjectivePreviewHint(gameState.lastEvent, profile);
        AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
        return;
    }

    if (nearest->registryId == "[%bulkhead_0001]") {
        if (!profile.story.bucketRecovered) {
            gameState.lastEvent = "The outer bulkhead cycles, but heavy debris beyond it still demands a BT-72 clearance module.";
        } else {
            profile.story.exitedBunker = true;
            gameState.lastEvent = "Outer bulkhead cycled and the lift route unlocks to the surface approach. BT-72 can now enter the first blocked recovery corridor.";
            AppendObjectiveRuntimeHint(gameState.lastEvent, profile, staticEraser);
            AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
        }
        return;
    }

    if (nearest->registryId == "#%res_scrap_0001") {
        if (!player.insideTank) {
            gameState.lastEvent = "Debris too dense for manual clearing. Link with BT-72 first.";
            return;
        }
        if (player.bt72GunnerSeat) {
            gameState.lastEvent = "Return to BT-72 pilot controls before attempting heavy clearance.";
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
                GrantContainerLoot(profile, *mutableObject, 0.5f);
                staticEraser.Erase(mutableObject->registryId);
                staticEraser.Save(profile.selectedWorld);
                world.RemoveObject(mutableObject->registryId);
                profile.story.outerRoadCleared = true;
                std::string progressionEvent;
                AwardExperience(profile, 50, &progressionEvent);
                gameState.lastEvent = "Outer debris barrier cleared. BT-72 punched open the combat lane toward the first recovery node. " + progressionEvent;
                AppendObjectiveRuntimeHint(gameState.lastEvent, profile, staticEraser);
                AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
            } else {
                gameState.lastEvent = "Debris impact registered. Keep pushing the barrier.";
            }
        }
        return;
    }

    if (nearest->registryId == "#%term_0001") {
        if (!profile.story.outerRoadCleared || !(profile.firstPlayableRoute.firstTankCombatResolved || staticEraser.IsErased("[%enemy_ghoul_0001]"))) {
            gameState.lastEvent = "Relay sync is unsafe. Secure the outer route first.";
            return;
        }
        if (!profile.firstPlayableRoute.firstServicePerformed) {
            gameState.lastEvent = "BT-72 is still hot off first contact. Run one service/rest cycle before syncing the node.";
            return;
        }
        if (!profile.story.relayRecovered) {
            auto& worldState = EnsureSelectedWorldFieldState(profile);
            profile.story.relayRecovered = true;
            profile.firstPlayableRoute.firstRecoveryNodeActivated = true;
            std::string skillEvent;
            RegisterArchiveSync(profile, &skillEvent);
            AddInventoryItem(profile, "relay_reconstruction_data", 1, 0.2f);
            if (std::none_of(profile.character.collectedTapes.begin(),
                    profile.character.collectedTapes.end(),
                    [&](const TapeEntry& tape) { return tape.tapeId == "relay_reconstruction_data"; })) {
                profile.character.collectedTapes.push_back({"relay_reconstruction_data", "Relay Reconstruction Packet", false});
            }
            worldState.localRelayAvailable = true;
            worldState.routeContamination = std::max(0.0f, worldState.routeContamination - 10.0f);
            worldState.infrastructureDecay = std::max(0.0f, worldState.infrastructureDecay - 6.0f);
            std::string progressionEvent;
            AwardExperience(profile, HasEquippedPassiveSkill(profile, "skill_data_miner") ? 50 : 40, &progressionEvent);
            gameState.lastEvent = "Recovery node synchronized. Shelter 17 now reads a live route toward the city fringe. Return for debrief. " + progressionEvent;
            if (!skillEvent.empty()) {
                gameState.lastEvent += " " + skillEvent;
            }
            AppendObjectiveRuntimeHint(gameState.lastEvent, profile, staticEraser);
            AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
            AppendRecoveryBackboneReadabilityHint(gameState.lastEvent, profile);
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
                profile.firstPlayableRoute.debriefSummaryViewed = true;
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
            gameState.lastEvent = "Debrief uploaded. The first route is closed, city approach hooks are tagged, and industrial recovery planning is now live for Shelter 17. " + progressionEvent;
                if (!skillEvent.empty()) {
                    gameState.lastEvent += " " + skillEvent;
                }
                AppendObjectiveRuntimeHint(gameState.lastEvent, profile, staticEraser);
                AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
                AppendRecoveryBackboneReadabilityHint(gameState.lastEvent, profile);
            } else {
                gameState.lastEvent = "Debrief log already archived. Shelter 17 recovery planning remains on file.";
            }
            return;
        }

    if (nearest->registryId == "[%camp_0001]") {
        const bool firstSurfaceArrival = profile.story.exitedBunker && !profile.firstPlayableRoute.surfaceArrivalReached;
        if (profile.story.exitedBunker) {
            profile.firstPlayableRoute.surfaceArrivalReached = true;
        }
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
        if (firstSurfaceArrival) {
            gameState.lastEvent = "Surface foothold secured. " + gameState.lastEvent;
        }
        if (erosionCleared > 0) {
            gameState.lastEvent += " Ether bloom reduced by " + std::to_string(erosionCleared) + "%.";
        }
        AppendObjectiveRuntimeHint(gameState.lastEvent, profile, staticEraser);
        AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
        return;
    }

    switch (nearest->interaction) {
        case InteractionType::Hostile:
            gameState.lastEvent = "This target is hostile. Attack instead of interacting.";
            break;
        case InteractionType::Terminal:
            AddCollectedTapeIfMissing(profile, nearest->registryId, nearest->displayName);
            if (executionContext != nullptr) {
                std::string bridgeStatus;
                if (TryExecuteCompiledScript(*nearest, *executionContext, bridgeStatus)) {
                    gameState.lastEvent = bridgeStatus;
                    break;
                }
            }
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
            const bool gridOnline = IsRegionalGridOnline(profile);
            const bool manualStarterService = !gridOnline && profile.firstPlayableRoute.firstTankCombatResolved;
            if (!gridOnline && !manualStarterService) {
                gameState.lastEvent = "Workshop grid is offline. Use field service or return after first combat for a manual recovery pass.";
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
                profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + (manualStarterService ? 7.0f : 14.0f));
                gameState.lastEvent = manualStarterService
                    ? "Manual workshop coupler connected. BT-72 batteries boosted from local reserves."
                    : "Workshop power coupler connected. BT-72 batteries boosted.";
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
            const float doctrineBoost = DoctrineWorkshopBoost(profile) * PylonGridBoost(profile) * (manualStarterService ? 0.84f : 1.0f);
            const auto* worldState = FindWorldFieldState(profile, profile.selectedWorld);
            const float infrastructurePenalty = worldState != nullptr
                ? std::max(0.68f, 1.0f - worldState->infrastructureDecay / 150.0f)
                : 1.0f;
            const float repairBoost = engineerBoost * doctrineBoost * infrastructurePenalty * Bt72ServiceSkillBoost(profile, gameState);
            profile.partnerTank.damage.hull = std::min(100.0f, profile.partnerTank.damage.hull + (18.0f + intelligence * 0.9f) * repairBoost);
            profile.partnerTank.damage.bucket = std::min(100.0f, profile.partnerTank.damage.bucket + (20.0f + intelligence * 0.7f) * repairBoost);
            profile.partnerTank.damage.sensors = std::min(100.0f, profile.partnerTank.damage.sensors + (16.0f + intelligence * 0.8f) * repairBoost);
            profile.partnerTank.damage.turret = std::min(100.0f, profile.partnerTank.damage.turret + (12.0f + intelligence * 0.6f) * repairBoost);
            profile.partnerTank.damage.cockpit = std::min(100.0f, profile.partnerTank.damage.cockpit + (10.0f + intelligence * 0.5f) * repairBoost);
            profile.partnerTank.damage.powerCore = std::min(100.0f, profile.partnerTank.damage.powerCore + (14.0f + intelligence * 0.6f) * repairBoost);
            profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + (manualStarterService ? 12.0f : 22.0f));
            profile.partnerTank.ammoReserve = std::min(100.0f, profile.partnerTank.ammoReserve + (manualStarterService ? 6.0f : 12.0f));
            gameState.workshopServiceCooldown = 120.0f;
            {
                std::string progressionEvent;
                AwardExperience(profile, 25, &progressionEvent);
                if (manualStarterService) {
                    gameState.lastEvent = hasEngineer
                        ? "Manual workshop cycle complete. Resident engineer stabilized the first BT-72 recovery pass. " + progressionEvent
                        : "Manual workshop cycle complete. BT-72 patched and resupplied off bunker reserves. " + progressionEvent;
                } else {
                    gameState.lastEvent = hasEngineer
                        ? "Workshop cycle complete. Resident engineer boosted BT-72 repair output. " + progressionEvent
                        : "Workshop cycle complete. BT-72 repaired and resupplied. " + progressionEvent;
                }
                ApplyServiceChoiceBonuses(profile, gameState, false, gameState.lastEvent);
                if (profile.firstPlayableRoute.firstTankCombatResolved && !profile.firstPlayableRoute.firstServicePerformed) {
                    profile.firstPlayableRoute.firstServicePerformed = true;
                    gameState.lastEvent += " First service halt logged for the route.";
                    AppendObjectivePreviewHint(gameState.lastEvent, profile);
                    AppendRouteBeatReadabilityHint(gameState.lastEvent, profile);
                }
            }
            break;
        }
        case InteractionType::Container:
            GrantContainerLoot(profile, *nearest, 0.4f);
            staticEraser.Erase(nearest->registryId);
            staticEraser.Save(profile.selectedWorld);
            world.RemoveObject(nearest->registryId);
            gameState.lastEvent = "Container secured and contents transferred.";
            break;
        case InteractionType::Resource:
        case InteractionType::VehicleAnchor:
        case InteractionType::Static:
            if (executionContext != nullptr) {
                std::string bridgeStatus;
                if (TryExecuteCompiledScript(*nearest, *executionContext, bridgeStatus)) {
                    gameState.lastEvent = bridgeStatus;
                    break;
                }
            }
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

#if 0
void DrawPipPadTabBar(int& activeTab) {
    const char* tabs[] = {"STAT", "INV", "DATA", "MAP", "QUEST", "NET", "SERV"};
    for (int index = 0; index < IM_ARRAYSIZE(tabs); ++index) {
        if (index > 0) {
            ImGui::SameLine();
        }
        if (ImGui::Button(tabs[index], ImVec2(132.0f, 34.0f))) {
            activeTab = index;
        }
    }
}

void DrawPipPadStatTab(const SessionProfile& profile, const GameState& gameState) {
    const int statS = EffectiveStatValue(profile, gameState, 'S');
    const int statP = EffectiveStatValue(profile, gameState, 'P');
    const int statE = EffectiveStatValue(profile, gameState, 'E');
    const int statC = EffectiveStatValue(profile, gameState, 'C');
    const int statI = EffectiveStatValue(profile, gameState, 'I');
    const int statA = EffectiveStatValue(profile, gameState, 'A');
    const int statL = EffectiveStatValue(profile, gameState, 'L');

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
}

void DrawPipPadInventoryTab(SessionProfile& profile, GameState& gameState) {
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
}

void DrawPipPadDataTab(PlayerState& player, SessionProfile& profile, GameState& gameState) {
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
    ImGui::BulletText("Defensive State: %s", Bt72DefenseStatusLabel(profile).c_str());
    ImGui::BulletText("Return-Fire Cut: %.0f%%", Bt72ReturnFireMitigation(profile) * 100.0f);
    ImGui::BulletText("Hull Band: %s", TankIntegrityBand(profile.partnerTank.damage.hull));
    ImGui::ProgressBar(profile.partnerTank.damage.hull / 100.0f, ImVec2(-1.0f, 16.0f), "Hull");
    ImGui::ProgressBar(profile.partnerTank.damage.bucket / 100.0f, ImVec2(-1.0f, 16.0f), "Bucket");
    ImGui::ProgressBar(profile.partnerTank.damage.sensors / 100.0f, ImVec2(-1.0f, 16.0f), "Sensors");
    ImGui::ProgressBar(profile.partnerTank.damage.cockpit / 100.0f, ImVec2(-1.0f, 16.0f), "Cockpit");
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
}

void DrawPipPadMapTab(const World& world) {
    ImGui::Text("Local Survey");
    ImGui::Text("Zone: %s", world.metadata.name.c_str());
    ImGui::Text("Objective: %s", world.metadata.objective.c_str());
    ImGui::Separator();
    ImGui::BeginChild("MapList", ImVec2(0.0f, 300.0f), true);
    for (const auto& object : world.objects) {
        ImGui::BulletText("%s | %.1f %.1f | HP %.0f", object.displayName.c_str(), object.x, object.y, object.health);
    }
    ImGui::EndChild();
}

void DrawPipPadNetTab(const SessionProfile& profile, GameState& gameState) {
    LanlineSessionState sessionState;
    const bool hasSessionState = LoadLanlineSessionState(sessionState);
    const auto& runtimeNotifications = UpdateLanlineRuntimeNotifications(
        hasSessionState ? &sessionState : nullptr,
        profile.selectedWorld,
        gameState);
    ImGui::Text("Lanline - optime");
    if (!hasSessionState) {
        ImGui::TextDisabled("No active Lanline session state found. Launch through BunkerLauncher to seed roster and snapshot data.");
        if (!runtimeNotifications.empty()) {
            ImGui::Separator();
            ImGui::Text("Runtime Notifications");
            for (const auto& notification : runtimeNotifications) {
                ImGui::BulletText("%s", notification.c_str());
            }
        }
        return;
    }

    const std::string sessionWorldReference = NormalizeWorldReference(sessionState.worldName);
    const bool worldMatchesRuntime = sessionWorldReference == profile.selectedWorld;
    const auto& diagnostics = CachedLanlineDiagnostics(sessionState, profile.selectedWorld);
    ImGui::Text("Session ID: %s", sessionState.sessionId.c_str());
    ImGui::Text("Mode: %s", sessionState.mode.c_str());
    ImGui::Text("Lifecycle: %s", sessionState.lifecycleStage.c_str());
    ImGui::Text("Active Actor: %s", sessionState.activeActor.c_str());
    ImGui::Text("Pending Peer: %s", sessionState.pendingPeer.empty() ? "none" : sessionState.pendingPeer.c_str());
    ImGui::Text("Connected Peer: %s", sessionState.connectedPeer.empty() ? "none" : sessionState.connectedPeer.c_str());
    ImGui::Text("Reserved Slots: %d", bunker::ReservedLanlineSessionSlots(sessionState));
    ImGui::Text("Pending Slots: %d", bunker::PendingLanlineSessionSlots(sessionState));
    ImGui::Text("Accepted Client Slots: %d", bunker::AcceptedLanlineSessionSlots(sessionState));
    ImGui::Text("Ready Seats: %d", bunker::ReadyLanlineSessionSlots(sessionState));
    ImGui::Text("World: %s", sessionWorldReference.c_str());
    ImGui::Text("Host: %s", sessionState.hostEndpoint.c_str());
    ImGui::Text("Updated: %s", sessionState.updatedAt.c_str());
    ImGui::Text("Runtime World Match: %s", worldMatchesRuntime ? "yes" : "no");
    ImGui::BulletText("Host reachable: %s", diagnostics.hostReachable ? "yes" : "no");
    ImGui::BulletText("Ping: %s",
        diagnostics.pingMs >= 0 ? (std::to_string(diagnostics.pingMs) + " ms").c_str() : "n/a");
    ImGui::BulletText("Snapshot freshness: %s", diagnostics.snapshotFresh ? "fresh" : "stale");
    ImGui::BulletText("Presence: %d / %d online", diagnostics.onlinePlayers, diagnostics.totalPlayers);
    if (!diagnostics.lastError.empty()) {
        ImGui::TextDisabled("%s", diagnostics.lastError.c_str());
    }
    ImGui::Separator();
    ImGui::Text("Session Roster");
    for (const auto& playerEntry : sessionState.players) {
        ImGui::BulletText("%s | %s | %s | %s | %s",
            playerEntry.displayName.c_str(),
            playerEntry.role.c_str(),
            playerEntry.online ? "Online" : "Offline",
            bunker::LanlineSlotStateLabel(playerEntry),
            bunker::LanlineReadyLabel(playerEntry));
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
    ImGui::Text("Relay Chat Mirror");
    if (sessionState.relayMessages.empty()) {
        ImGui::TextDisabled("No relay chat mirrored into this session yet.");
    } else {
        const std::size_t startIndex = sessionState.relayMessages.size() > 6
            ? sessionState.relayMessages.size() - 6
            : 0;
        for (std::size_t index = startIndex; index < sessionState.relayMessages.size(); ++index) {
            const auto& relayMessage = sessionState.relayMessages[index];
            ImGui::BulletText("[%s] %s @ %s: %s",
                relayMessage.channelId.c_str(),
                relayMessage.author.c_str(),
                relayMessage.timeLabel.c_str(),
                relayMessage.body.c_str());
        }
    }
    ImGui::Separator();
    ImGui::Text("Voice Presence");
    if (sessionState.voicePresence.empty()) {
        ImGui::TextDisabled("No Lanline voice presence mirrored into this session yet.");
    } else {
        for (const auto& voicePresence : sessionState.voicePresence) {
            ImGui::BulletText("%s | %s | PTT %s | peak %d%% | %s",
                voicePresence.handle.c_str(),
                voicePresence.speaking ? "transmitting" : "idle",
                voicePresence.pushToTalk ? "on" : "off",
                static_cast<int>(voicePresence.peakLevel * 100.0f),
                voicePresence.timeLabel.c_str());
        }
    }
    ImGui::Separator();
    ImGui::Text("Runtime Notifications");
    if (runtimeNotifications.empty()) {
        ImGui::TextDisabled("No runtime Lanline notifications yet.");
    } else {
        for (const auto& notification : runtimeNotifications) {
            ImGui::BulletText("%s", notification.c_str());
        }
    }
    ImGui::Separator();
    const auto knownSessions = DiscoverLanlineSessionSnapshots();
    ImGui::Text("Known Sessions");
    if (knownSessions.empty()) {
        ImGui::TextDisabled("No session snapshots discovered.");
    } else {
        for (const auto& knownSession : knownSessions) {
            const auto normalizedKnownWorld = NormalizeWorldReference(knownSession.worldName);
            const auto& knownDiagnostics = CachedLanlineDiagnostics(knownSession, profile.selectedWorld);
            ImGui::BulletText("%s | %s | %s | %s | %s",
                knownSession.sessionId.c_str(),
                knownSession.mode.c_str(),
                knownSession.lifecycleStage.c_str(),
                JoinabilityLabel(knownSession),
                normalizedKnownWorld.c_str(),
                knownSession.hostEndpoint.c_str());
            ImGui::TextDisabled("  Slots %d/%d | Host %s | Ping %s | Match %s | Snapshot %s | Presence %d/%d",
                bunker::OccupiedLanlineSessionSlots(knownSession),
                bunker::MaxLanlineSessionSlots(knownSession),
                knownDiagnostics.hostReachable ? "reachable" : "offline",
                knownDiagnostics.pingMs >= 0 ? (std::to_string(knownDiagnostics.pingMs) + " ms").c_str() : "n/a",
                knownDiagnostics.worldMatch ? "yes" : "no",
                knownDiagnostics.snapshotFresh ? "fresh" : "stale",
                knownDiagnostics.onlinePlayers,
                knownDiagnostics.totalPlayers);
            if (bunker::IsJoinableLanlineSession(knownSession)) {
                ImGui::TextDisabled("  Open slots: %d | Pending: %d | Reserved: %d | Accepted: %d | Ready: %d",
                    bunker::AvailableLanlineSessionSlots(knownSession),
                    bunker::PendingLanlineSessionSlots(knownSession),
                    bunker::ReservedLanlineSessionSlots(knownSession),
                    bunker::AcceptedLanlineSessionSlots(knownSession),
                    bunker::ReadyLanlineSessionSlots(knownSession));
            }
        }
    }
    ImGui::Separator();
    ImGui::TextWrapped("Lanline - optime keeps a visible session roster and snapshot trail even without Steam/Xbox auth.");
}

void DrawPipPadServicesTab(const World& world, const PlayerState& player, SessionProfile& profile, GameState& gameState) {
    const auto* currentWorldFieldState = FindWorldFieldState(profile, profile.selectedWorld);
    static bool lanlineServicesLoaded = false;
    static LanlineServicesState lanlineServices = MakeDefaultLanlineServicesState(std::time(nullptr));
    if (!lanlineServicesLoaded) {
        LanlineServicesSave lanlineSave{};
        if (LoadLanlineServicesSave(DefaultLanlineServicesSavePath(), lanlineSave)) {
            lanlineServices = MakeLanlineServicesStateFromSave(lanlineSave, std::time(nullptr));
        }
        ApplyLanlineServicesProfileSnapshot(lanlineServices, profile.lanlineServices);
        lanlineServicesLoaded = true;
    }

    LanlineSessionState sessionState;
    const bool hasSessionState = LoadLanlineSessionState(sessionState);
    const auto servicesUnlock = BuildServicesUnlockState(profile, currentWorldFieldState);
    SyncLanlineServicesPresence(lanlineServices, hasSessionState ? &sessionState : nullptr, servicesUnlock);
    gameState.supportTerminalNearby = IsNearTaggedObject(world, player.x, player.y, "lanline_service_hub", 4.0f);
    gameState.tankServiceNearby = IsNearTaggedObject(world, player.x, player.y, "tank_service", 4.0f);
    gameState.medicalSupportNearby = IsNearTaggedObject(world, player.x, player.y, "medical_support", 4.0f);
    gameState.feyRingScheduleVisible = gameState.feyRingScheduleVisible ||
        IsNearTaggedObject(world, player.x, player.y, "fey_ring", 4.0f);
    ImGui::Text("Lanline Services");
    ImGui::BulletText("Service hub nearby: %s", gameState.supportTerminalNearby ? "yes" : "no");
    ImGui::BulletText("Tank service nearby: %s", gameState.tankServiceNearby ? "yes" : "no");
    ImGui::BulletText("Medical support nearby: %s", gameState.medicalSupportNearby ? "yes" : "no");
    ImGui::BulletText("Fey schedule visible: %s", gameState.feyRingScheduleVisible ? "yes" : "no");
    if (!gameState.lastSupportAction.empty()) {
        ImGui::TextWrapped("Support action: %s", gameState.lastSupportAction.c_str());
    }
    if (!gameState.lastPortalAction.empty()) {
        ImGui::TextWrapped("Portal action: %s", gameState.lastPortalAction.c_str());
    }
    DrawLanlineServicesPanel(lanlineServices, servicesUnlock, static_cast<std::int64_t>(std::time(nullptr)));
    SyncLanlineServicesProfileSnapshot(profile.lanlineServices, lanlineServices);
    SaveLanlineServicesSave(BuildLanlineServicesSave(lanlineServices), DefaultLanlineServicesSavePath());
}
#endif

void DrawPipPad(const World& world,
    PlayerState& player,
    SessionProfile& profile,
    StaticEraser& staticEraser,
    GameState& gameState) {
    static int activeTab = 0;

    if (!PlayerHasPipPadAccess(profile)) {
        player.uiVisible = false;
        return;
    }

    if (!player.uiVisible) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(740.0f, 560.0f), ImGuiCond_FirstUseEver);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.03f, 0.06f, 0.04f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.7f, 0.3f, 0.7f));
    ImGui::Begin("PIP-PAD // Recovery Shell", nullptr, ImGuiWindowFlags_NoCollapse);

    DrawPipPadTabBar(activeTab);

    ImGui::Separator();

    if (activeTab == 0) {
        DrawPipPadStatTab(profile, gameState);
    } else if (activeTab == 1) {
        DrawPipPadInventoryTab(profile, gameState);
    } else if (activeTab == 2) {
        DrawPipPadDataTab(player, profile, gameState);
    } else if (activeTab == 3) {
        DrawPipPadMapTab(world);
    } else if (activeTab == 4) {
        const float recoveryIndex = ShelterRecoveryIndex(profile);
        auto* worldFieldState = FindWorldFieldState(profile, profile.selectedWorld, true);
        const bool stableRecoveryBackbone = worldFieldState != nullptr &&
            IsStableRecoveryBackbone(profile, *worldFieldState);
        const auto routeReadout = BuildFirstPlayableRouteReadout(profile);
        const auto backboneStatus = CurrentRecoveryBackboneStatus(profile);
        ImGui::Text("Mission Log");
        ImGui::BulletText("Checkpoint: %s", routeReadout.checkpoint.c_str());
        ImGui::BulletText("%s", CurrentStoryObjective(profile, staticEraser).c_str());
        ImGui::BulletText("Route Beat: %s", routeReadout.beat.c_str());
        ImGui::BulletText("Vertical Slice: %d / %d", routeReadout.completedSteps, routeReadout.totalSteps);
        ImGui::BulletText("Surface Lane: %s", routeReadout.surfaceStatus.c_str());
        ImGui::TextWrapped("Recovery Handoff: %s", CurrentRecoveryHandoffSummary(profile).c_str());
        ImGui::BulletText("Industrial Backbone: %s", backboneStatus.stage.c_str());
        ImGui::TextWrapped("Backbone status: %s", backboneStatus.status.c_str());
        ImGui::TextWrapped("Backbone payoff: %s", backboneStatus.payoff.c_str());
        ImGui::TextWrapped("Route Event Layer: %s", ActiveRouteEventSummary(profile).c_str());
        ImGui::TextWrapped("Vertical Slice Brief: %s", routeReadout.brief.c_str());
        if (worldFieldState != nullptr) {
            ImGui::BulletText("Route events resolved/failed/expired: %d / %d / %d",
                worldFieldState->routeEventsResolved,
                worldFieldState->routeEventsFailed,
                worldFieldState->routeEventsExpired);
            if (worldFieldState->activeRouteEventType == "merchant_window" && HasActiveRouteEvent(*worldFieldState)) {
                ImGui::Separator();
                ImGui::Text("Merchant Window");
                ImGui::TextWrapped("One discreet broker exchange is open on the recovery fringe. Spend 1 trade voucher or 90 relay credits before the trace folds.");
                if (ImGui::Button("Broker Field Exchange")) {
                    TryResolveMerchantRouteEvent(profile, gameState);
                }
            }
        }
        ImGui::TextWrapped("Next Payoff: %s", routeReadout.nextPayoff.c_str());
        if (!profile.firstPlayableRoute.bt72Restored || !profile.firstPlayableRoute.clearanceModuleInstalled) {
            ImGui::Separator();
            ImGui::Text("BT-72 Restore Route");
            for (const auto& entry : BuildBt72RestorationRoute(profile)) {
                if (entry.completed) {
                    ImGui::TextDisabled("%s", entry.text.c_str());
                } else {
                    ImGui::BulletText("%s", entry.text.c_str());
                }
            }
        }
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
        DrawPipPadNetTab(profile, gameState);
    } else if (activeTab == 6) {
        DrawPipPadServicesTab(world, player, profile, gameState);
    }

    ImGui::Separator();
    ImGui::TextWrapped("COMMS: %s", gameState.lastEvent.c_str());
    ImGui::End();
    ImGui::PopStyleColor(2);
}

}  // namespace bunker
