#include "../include/LanlineServices.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "imgui.h"

namespace bunker {

namespace {

const char* ToLabel(SupportCategory category) {
    switch (category) {
        case SupportCategory::Materials: return "Materials";
        case SupportCategory::Skins: return "Skins";
        case SupportCategory::Cosmetics: return "Cosmetics";
        case SupportCategory::Utility: return "Utility";
        case SupportCategory::TankService: return "Tank Service";
        case SupportCategory::Medical: return "Medical";
    }
    return "Unknown";
}

const char* ToLabel(SupportOrderState state) {
    switch (state) {
        case SupportOrderState::Draft: return "Draft";
        case SupportOrderState::Queued: return "Queued";
        case SupportOrderState::Routed: return "Routed";
        case SupportOrderState::Delivered: return "Delivered";
    }
    return "Unknown";
}

const char* ToLabel(FeyGateState state) {
    switch (state) {
        case FeyGateState::Closed: return "Closed";
        case FeyGateState::Open: return "Open";
        case FeyGateState::Transit: return "Transit";
    }
    return "Unknown";
}

const char* ToLabel(ServicesUnlockTier tier) {
    switch (tier) {
        case ServicesUnlockTier::Locked: return "Locked";
        case ServicesUnlockTier::TowerLinked: return "Tower Linked";
        case ServicesUnlockTier::BackboneStable: return "Backbone Stable";
        case ServicesUnlockTier::RelayExpanded: return "Relay Expanded";
    }
    return "Unknown";
}

void AddChatMessage(ChatChannel& channel, const std::string& author, const std::string& body, const std::string& timeLabel) {
    channel.messages.push_back({author, body, timeLabel});
}

ChatChannel* FindChatChannel(LanlineServicesState& state, const std::string& channelId) {
    for (auto& channel : state.chatChannels) {
        if (channel.id == channelId) {
            return &channel;
        }
    }
    return nullptr;
}

FriendEntry* FindFriendEntry(LanlineServicesState& state, const std::string& handle) {
    for (auto& friendEntry : state.friends) {
        if (friendEntry.handle == handle) {
            return &friendEntry;
        }
    }
    return nullptr;
}

void DrawLanlineServicesLockedScreen(const ServicesUnlockState& unlockState) {
    ImGui::Text("Lanline Services: offline");
    ImGui::Separator();
    if (!unlockState.firstTowerActivated) {
        ImGui::TextWrapped("Relay access unavailable. Restore and synchronize the first tower to open Lanline Services.");
        ImGui::BulletText("Requirement: first tower restored");
        ImGui::BulletText("Requirement: tower sync completed");
    } else {
        ImGui::TextWrapped("Relay node detected, but services are still restricted while Shelter 17 stabilizes.");
    }
    ImGui::BulletText("Unlock tier: %s", ToLabel(unlockState.tier));
}

void DrawFriendsTab(LanlineServicesState& state, char* searchBuffer, std::size_t searchBufferSize) {
    ImGui::InputText("Relay Search", searchBuffer, searchBufferSize);
    ImGui::Separator();
    for (auto& entry : state.friends) {
        if (searchBuffer[0] != '\0') {
            const std::string loweredHandle = entry.handle;
            const std::string loweredSearch = searchBuffer;
            if (loweredHandle.find(loweredSearch) == std::string::npos) {
                continue;
            }
        }

        ImGui::PushID(entry.handle.c_str());
        ImGui::Text("%s", entry.handle.c_str());
        ImGui::BulletText("Node: %s", entry.nodeLabel.c_str());
        ImGui::BulletText("Presence: %s", entry.online ? "online" : "offline");
        ImGui::BulletText("Session: %s", entry.inCurrentSession ? "linked" : "remote");
        ImGui::TextWrapped("%s", entry.statusText.c_str());
        if (ImGui::Button(entry.voiceMuted ? "Unmute Voice" : "Mute Voice")) {
            entry.voiceMuted = !entry.voiceMuted;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("Voice Volume", &entry.voiceVolume, 0.0f, 1.5f);
        ImGui::Separator();
        ImGui::PopID();
    }
}

void DrawChatTab(LanlineServicesState& state, int& selectedChannelIndex, char* messageBuffer, std::size_t messageBufferSize) {
    if (state.chatChannels.empty()) {
        ImGui::TextDisabled("Relay chat channels are still empty.");
        return;
    }

    selectedChannelIndex = std::clamp(selectedChannelIndex, 0, static_cast<int>(state.chatChannels.size()) - 1);
    if (ImGui::BeginCombo("Channel", state.chatChannels[static_cast<std::size_t>(selectedChannelIndex)].label.c_str())) {
        for (int index = 0; index < static_cast<int>(state.chatChannels.size()); ++index) {
            const bool selected = selectedChannelIndex == index;
            if (ImGui::Selectable(state.chatChannels[static_cast<std::size_t>(index)].label.c_str(), selected)) {
                selectedChannelIndex = index;
            }
        }
        ImGui::EndCombo();
    }

    auto& channel = state.chatChannels[static_cast<std::size_t>(selectedChannelIndex)];
    ImGui::BeginChild("LanlineChatHistory", ImVec2(0.0f, 180.0f), true);
    for (const auto& message : channel.messages) {
        ImGui::TextWrapped("[%s] %s: %s", message.timeLabel.c_str(), message.author.c_str(), message.body.c_str());
    }
    ImGui::EndChild();

    ImGui::InputText("Message", messageBuffer, messageBufferSize);
    if (ImGui::Button("Transmit")) {
        if (messageBuffer[0] != '\0') {
            AddChatMessage(channel, "Operator", messageBuffer, "now");
            std::memset(messageBuffer, 0, messageBufferSize);
        }
    }
}

void DrawVoiceTab(LanlineServicesState& state) {
    auto& voice = state.voice;
    ImGui::Checkbox("Enable Voice", &voice.enabled);
    ImGui::Checkbox("Push To Talk", &voice.pushToTalk);
    ImGui::InputText("PTT Key", voice.pushToTalkKey, IM_ARRAYSIZE(voice.pushToTalkKey));
    ImGui::SliderFloat("Input Sensitivity", &voice.inputSensitivity, 0.0f, 1.0f);
    ImGui::SliderFloat("Input Gain", &voice.inputGain, 0.0f, 2.0f);
    ImGui::SliderFloat("Output Gain", &voice.outputGain, 0.0f, 2.0f);
    ImGui::InputInt("Input Device", &voice.selectedInputDevice);
    ImGui::InputInt("Output Device", &voice.selectedOutputDevice);
    ImGui::TextDisabled("Voice transport remains future work; this is the shell/settings layer from Lanline Services.");
}

void DrawSupportTab(LanlineServicesState& state, const ServicesUnlockState& unlockState) {
    ImGui::TextWrapped("Relay support requests. No weapons, no completed tanks, no direct combat advantages.");
    ImGui::BulletText("Recovery Scrip: %d", state.relayCredits);
    ImGui::Separator();

    for (const auto& item : state.supportCatalog) {
        if (!IsAllowedSupportItem(item)) {
            continue;
        }
        if (item.category == SupportCategory::TankService && !IsTankServiceUnlocked(unlockState)) {
            continue;
        }
        if (item.category == SupportCategory::Medical && !IsMedicalSupportUnlocked(unlockState)) {
            continue;
        }

        ImGui::PushID(item.id.c_str());
        ImGui::Text("[%s] %s", ToLabel(item.category), item.label.c_str());
        ImGui::TextWrapped("%s", item.description.c_str());
        ImGui::BulletText("Cost: %d Recovery Scrip", item.priceCredits);
        if (ImGui::TreeNode("Contents")) {
            for (const auto& content : item.contents) {
                ImGui::BulletText("%s", content.c_str());
            }
            ImGui::TreePop();
        }
        const bool affordable = state.relayCredits >= item.priceCredits;
        ImGui::BeginDisabled(!affordable);
        if (ImGui::Button("Request")) {
            state.relayCredits -= item.priceCredits;
            SupportOrder order;
            order.orderId = "order_" + item.id + "_" + std::to_string(state.supportOrders.size() + 1);
            order.itemId = item.id;
            order.itemLabel = item.label;
            order.state = SupportOrderState::Queued;
            state.supportOrders.push_back(order);
        }
        ImGui::EndDisabled();
        if (!affordable) {
            ImGui::TextDisabled("Insufficient Recovery Scrip.");
        }
        ImGui::Separator();
        ImGui::PopID();
    }

    if (!state.supportOrders.empty() && ImGui::TreeNode("Active Orders")) {
        for (const auto& order : state.supportOrders) {
            ImGui::BulletText("%s | %s | %s",
                order.orderId.c_str(),
                order.itemLabel.c_str(),
                ToLabel(order.state));
        }
        ImGui::TreePop();
    }
}

void DrawFeyGateTab(LanlineServicesState& state, const ServicesUnlockState& unlockState, std::int64_t nowUnix) {
    ImGui::TextWrapped("Fey Ring Network: transit windows and relay routes.");
    ImGui::Separator();

    for (auto& gate : state.feyGateCycles) {
        if (gate.interServer && !IsInterserverPortalScheduleUnlocked(unlockState)) {
            continue;
        }
        if (!gate.interServer && !IsIntercityPortalScheduleUnlocked(unlockState)) {
            continue;
        }

        gate.state = ComputeFeyGateState(gate, nowUnix);
        ImGui::PushID(gate.id.c_str());
        ImGui::Text("%s -> %s", gate.originLabel.c_str(), gate.destinationLabel.c_str());
        ImGui::BulletText("State: %s", ToLabel(gate.state));
        ImGui::BulletText("Queue: %d / %d", gate.queueSize, gate.capacity);
        if (gate.state == FeyGateState::Open) {
            ImGui::BulletText("Closes in: %s", FormatCountdown(gate.closesAtUnix - nowUnix).c_str());
        } else {
            ImGui::BulletText("Opens in: %s", FormatCountdown(gate.opensAtUnix - nowUnix).c_str());
        }
        ImGui::TextDisabled(gate.interServer ? "Inter-server portal" : "Inter-city portal");
        ImGui::Separator();
        ImGui::PopID();
    }
}

}  // namespace

ServicesUnlockState BuildServicesUnlockState(const SessionProfile& profile, const WorldFieldState* worldState) {
    ServicesUnlockState state{};
    state.firstTowerActivated = HasRegionalGridOnline(profile);
    state.localRelayAvailable = state.firstTowerActivated;
    state.backboneStable = worldState != nullptr && IsStableRecoveryBackbone(profile, *worldState);
    state.intercityPortalsUnlocked = state.backboneStable;
    state.interserverPortalsUnlocked = state.backboneStable &&
        worldState != nullptr &&
        IsOrbitalUplinkOperational(profile, *worldState) &&
        IsTradeNetworkOperational(profile, *worldState);

    if (!state.firstTowerActivated) {
        state.tier = ServicesUnlockTier::Locked;
    } else if (!state.backboneStable) {
        state.tier = ServicesUnlockTier::TowerLinked;
    } else if (!state.interserverPortalsUnlocked) {
        state.tier = ServicesUnlockTier::BackboneStable;
    } else {
        state.tier = ServicesUnlockTier::RelayExpanded;
    }
    return state;
}

bool IsAllowedSupportItem(const SupportCatalogItem& item) {
    return !item.grantsWeapon && !item.grantsCompletedTank && !item.combatAdvantage;
}

std::vector<SupportCatalogItem> MakeDefaultSupportCatalog() {
    return {
        {"support_salvage_small", "Salvage Crate / Small", "Basic recovery materials for shelter upkeep and workshop stock.", SupportCategory::Materials, 120, false, false, false, TankSubsystem::None, {"bulk_salvage", "repair_parts", "circuit_scrap"}},
        {"relay_filter_bundle", "Filter Media Bundle", "Purification media and relay-safe consumables for continued recovery operations.", SupportCategory::Utility, 95, false, false, false, TankSubsystem::None, {"filter_media", "sealant_roll", "field_battery"}},
        {"skin_bt72_ashgray", "BT-72 Hull Livery: Ash Gray", "Cosmetic paint set for BT-72. No gameplay effect.", SupportCategory::Skins, 160, false, false, false, TankSubsystem::None, {"skin_bt72_ashgray"}},
        {"cosmetic_relay_badge", "Relay Operator Badge", "Terminal insignia and profile marker set. Cosmetic only.", SupportCategory::Cosmetics, 140, false, false, false, TankSubsystem::None, {"badge_relay_operator", "terminal_theme_relay"}},
        {"tank_suspension_kit", "BT-72 Suspension Repair Kit", "Service bundle for suspension and track maintenance.", SupportCategory::TankService, 210, false, false, false, TankSubsystem::Suspension, {"track_patch", "bearing_set", "grease_pack", "alignment_tools"}},
        {"tank_turret_kit", "BT-72 Turret Service Kit", "Turret servo, stabilization and bearing maintenance bundle.", SupportCategory::TankService, 230, false, false, false, TankSubsystem::Turret, {"servo_patch", "turret_bearing", "stabilizer_fluid"}},
        {"tank_engine_kit", "BT-72 Engine Service Kit", "Field maintenance kit for engine and cooling assembly.", SupportCategory::TankService, 250, false, false, false, TankSubsystem::Engine, {"engine_seal", "coolant_pack", "injector_cleanser"}},
        {"tank_sensor_kit", "BT-72 Sensor Recovery Kit", "Optics, relay and calibration tools for damaged sensor arrays.", SupportCategory::TankService, 185, false, false, false, TankSubsystem::Sensors, {"lens_pack", "sensor_relay", "calibration_spool"}},
        {"medkit_standard", "Field Medkit", "Standard expedition med supply for operator recovery.", SupportCategory::Medical, 75, false, false, false, TankSubsystem::None, {"medkit", "bandage_roll", "sterile_patch"}},
        {"medkit_trauma", "Trauma Response Pack", "Advanced trauma bundle for severe field damage recovery.", SupportCategory::Medical, 120, false, false, false, TankSubsystem::None, {"trauma_kit", "injector", "coagulant_pack"}},
    };
}

std::vector<FeyGateCycle> MakeDefaultFeyGateCycles(std::int64_t nowUnix) {
    return {
        {"ring_shelter17_ironspan", "Shelter 17 Ring", "Iron Span", "City", FeyGateState::Closed, nowUnix + 240, nowUnix + 720, nowUnix + 1080, 2, 12, false, false},
        {"portal_shelter17_relay_shard", "Shelter 17 Relay", "Relay Shard Node", "Server", FeyGateState::Closed, nowUnix + 900, nowUnix + 1260, nowUnix + 1800, 0, 24, true, false},
    };
}

LanlineServicesState MakeDefaultLanlineServicesState(std::int64_t nowUnix) {
    LanlineServicesState state;
    state.friends = {
        {"relay.operator.kite", "Shelter 17 Relay", "Watching relay recovery traffic.", true, false, false, 1.0f},
        {"spur.mechanic.ira", "Inner Spur Depot", "Waiting for stable backbone before sending heavier service loads.", false, false, false, 1.0f},
        {"fortress.dispatch", "Rail Fortress Net", "Patrol routing and support manifests mirrored through Lanline.", true, false, false, 1.0f},
    };
    state.chatChannels = {
        {"session", "Session", {{"Lanline", "Session relay ready. Presence log restored.", "boot"}}},
        {"relay", "Relay", {{"Shelter 17 Relay", "Tower sync can reopen broader relay services.", "boot"}}},
        {"support", "Support", {{"Dispatch", "Support catalog mirrors recovery-safe requests only.", "boot"}}},
    };
    state.supportCatalog = MakeDefaultSupportCatalog();
    state.feyGateCycles = MakeDefaultFeyGateCycles(nowUnix);
    return state;
}

void SyncLanlineServicesPresence(LanlineServicesState& state, const LanlineSessionState* sessionState) {
    for (auto& friendEntry : state.friends) {
        friendEntry.inCurrentSession = false;
    }
    ChatChannel* sessionChannel = FindChatChannel(state, "session");
    if (sessionChannel != nullptr) {
        sessionChannel->messages.clear();
    }

    if (sessionState == nullptr) {
        if (sessionChannel != nullptr) {
            AddChatMessage(*sessionChannel, "Lanline", "No active session mirror is currently available.", "now");
        }
        return;
    }

    if (sessionChannel != nullptr) {
        char presenceBuffer[96] = {};
        std::snprintf(presenceBuffer, sizeof(presenceBuffer), "%s | %s | %s",
            sessionState->sessionId.c_str(),
            sessionState->mode.c_str(),
            NormalizeWorldReference(sessionState->worldName).c_str());
        AddChatMessage(*sessionChannel, "Lanline", presenceBuffer, sessionState->updatedAt);
        for (const auto& eventLine : sessionState->eventLog) {
            AddChatMessage(*sessionChannel, "Session Log", eventLine, sessionState->updatedAt);
        }
    }

    for (const auto& player : sessionState->players) {
        const std::string handle = player.displayName;
        FriendEntry* friendEntry = FindFriendEntry(state, handle);
        if (friendEntry == nullptr) {
            state.friends.push_back({handle, "Lanline Session", player.role, player.online, true, false, 1.0f});
            friendEntry = &state.friends.back();
        }
        friendEntry->nodeLabel = "Lanline Session";
        friendEntry->statusText = player.role;
        friendEntry->online = player.online;
        friendEntry->inCurrentSession = true;
    }
}

FeyGateState ComputeFeyGateState(const FeyGateCycle& gate, std::int64_t nowUnix) {
    if (nowUnix >= gate.opensAtUnix && nowUnix <= gate.closesAtUnix) {
        return gate.unstable ? FeyGateState::Transit : FeyGateState::Open;
    }
    return FeyGateState::Closed;
}

std::string FormatCountdown(std::int64_t secondsRemaining) {
    const std::int64_t clamped = std::max<std::int64_t>(0, secondsRemaining);
    const std::int64_t minutes = clamped / 60;
    const std::int64_t seconds = clamped % 60;
    return std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
}

void DrawLanlineServicesPanel(LanlineServicesState& state,
    const ServicesUnlockState& unlockState,
    std::int64_t nowUnix) {
    if (!IsLanlineServicesUnlocked(unlockState)) {
        DrawLanlineServicesLockedScreen(unlockState);
        return;
    }

    static char friendSearch[128] = "";
    static char messageInput[256] = "";
    static int selectedChannelIndex = 0;

    ImGui::Text("Lanline Services Online");
    ImGui::BulletText("Unlock tier: %s", ToLabel(unlockState.tier));
    ImGui::BulletText("Backbone stable: %s", unlockState.backboneStable ? "yes" : "no");
    ImGui::Separator();

    if (ImGui::BeginTabBar("LanlineServicesTabs")) {
        if (ImGui::BeginTabItem("Friends")) {
            DrawFriendsTab(state, friendSearch, sizeof(friendSearch));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Chat")) {
            DrawChatTab(state, selectedChannelIndex, messageInput, sizeof(messageInput));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Voice")) {
            DrawVoiceTab(state);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Support")) {
            DrawSupportTab(state, unlockState);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Fey Rings")) {
            DrawFeyGateTab(state, unlockState, nowUnix);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

}  // namespace bunker
