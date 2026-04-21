#include "GameRuntimePipPad.hpp"

#include <algorithm>
#include <ctime>

#include "imgui.h"

#include "../include/HangarSystem.hpp"
#include "../include/LanlineLobbyLogic.hpp"
#include "../include/LanlineServices.hpp"
#include "../include/SkillSystem.hpp"

namespace bunker {

namespace {

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
        if (IsLanlineReadyEligibleSlot(playerEntry) && playerEntry.ready != previousIt->ready) {
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
        const auto previousVoiceIt = std::find_if(previousSession.voicePresence.begin(), previousSession.voicePresence.end(),
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
        const bool peakChangedMeaningfully = std::abs(voicePresence.peakLevel - previousVoiceIt->peakLevel) >= 0.2f;
        if (voicePresence.speaking && peakChangedMeaningfully) {
            pushNotification(voicePresence.handle + " voice peak now at " +
                std::to_string(static_cast<int>(voicePresence.peakLevel * 100.0f)) + "%.");
        }
    }

    for (const auto& previousVoice : previousSession.voicePresence) {
        const auto currentVoiceIt = std::find_if(session->voicePresence.begin(), session->voicePresence.end(),
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

bool TankUsesRamShield(const SessionProfile& profile) {
    const auto* module = FindTankModule(profile, TankModuleSlotType::Bucket);
    return module != nullptr && module->moduleId == "ram_shield_mk1";
}

bool TankUsesTowCoupler(const SessionProfile& profile) {
    const auto* module = FindTankModule(profile, TankModuleSlotType::Bucket);
    return module != nullptr && module->moduleId == "tow_coupler_mk1";
}

const char* CurrentUtilityModuleLabel(const SessionProfile& profile) {
    if (TankUsesRamShield(profile)) {
        return "Ram Shield Mk.I";
    }
    if (TankUsesTowCoupler(profile)) {
        return "Tow Coupler Mk.I";
    }
    return "Bucket Rig Mk.I";
}

void ToggleTankUtilityModule(SessionProfile& profile, PlayerState& player, GameState& gameState) {
    auto* module = FindTankModule(profile, TankModuleSlotType::Bucket);
    if (module == nullptr) {
        gameState.lastEvent = "No utility hardpoint found on BT-72.";
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

bool ConsumeAnyRepairMaterial(SessionProfile& profile) {
    const char* repairItems[] = {"scrap_steel", "hydraulic_seal", "wreck_scrap", "old_plate", "copper_wire", "repair_patch"};
    for (const char* itemId : repairItems) {
        if (ConsumeInventoryItem(profile, itemId, 1)) {
            return true;
        }
    }
    return false;
}

bool TryRunFieldWorkbench(PlayerState& player,
    SessionProfile& profile,
    GameState& gameState) {
    if (!player.insideTank) {
        gameState.lastEvent = "Field service requires an active BT-72 cockpit link.";
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
    const float boost = hasEngineer ? 1.12f : 1.0f;
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
    if (!recipeEvent.empty()) {
        gameState.lastEvent += " " + recipeEvent;
    }
    return true;
}

bool HasCollectedTape(const SessionProfile& profile, const std::string& tapeId) {
    return std::any_of(profile.character.collectedTapes.begin(), profile.character.collectedTapes.end(),
        [&](const TapeEntry& tape) { return tape.tapeId == tapeId; });
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

}  // namespace

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
    ImGui::ProgressBar(profile.partnerTank.damage.hull / 100.0f, ImVec2(-1.0f, 16.0f), "Hull");
    ImGui::ProgressBar(profile.partnerTank.damage.bucket / 100.0f, ImVec2(-1.0f, 16.0f), "Bucket");
    ImGui::ProgressBar(profile.partnerTank.damage.sensors / 100.0f, ImVec2(-1.0f, 16.0f), "Sensors");
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
    ImGui::Text("Reserved Slots: %d", ReservedLanlineSessionSlots(sessionState));
    ImGui::Text("Pending Slots: %d", PendingLanlineSessionSlots(sessionState));
    ImGui::Text("Accepted Client Slots: %d", AcceptedLanlineSessionSlots(sessionState));
    ImGui::Text("Ready Seats: %d", ReadyLanlineSessionSlots(sessionState));
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
            LanlineSlotStateLabel(playerEntry),
            LanlineReadyLabel(playerEntry));
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
                OccupiedLanlineSessionSlots(knownSession),
                MaxLanlineSessionSlots(knownSession),
                knownDiagnostics.hostReachable ? "reachable" : "offline",
                knownDiagnostics.pingMs >= 0 ? (std::to_string(knownDiagnostics.pingMs) + " ms").c_str() : "n/a",
                knownDiagnostics.worldMatch ? "yes" : "no",
                knownDiagnostics.snapshotFresh ? "fresh" : "stale",
                knownDiagnostics.onlinePlayers,
                knownDiagnostics.totalPlayers);
            if (IsJoinableLanlineSession(knownSession)) {
                ImGui::TextDisabled("  Open slots: %d | Pending: %d | Reserved: %d | Accepted: %d | Ready: %d",
                    AvailableLanlineSessionSlots(knownSession),
                    PendingLanlineSessionSlots(knownSession),
                    ReservedLanlineSessionSlots(knownSession),
                    AcceptedLanlineSessionSlots(knownSession),
                    ReadyLanlineSessionSlots(knownSession));
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

}  // namespace bunker
