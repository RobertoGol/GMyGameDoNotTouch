#include "../include/LanlineServices.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

#include "imgui.h"

#include "../include/AppPaths.hpp"
#include "../include/StoryRoute.hpp"

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

const char* ToLabel(StoreCurrency currency) {
    switch (currency) {
        case StoreCurrency::InGame: return "Recovery Scrip";
        case StoreCurrency::SymbolicSupport: return "Symbolic Support";
    }
    return "Unknown";
}

const char* ToLabel(SupportOrderState state) {
    switch (state) {
        case SupportOrderState::Draft: return "Draft";
        case SupportOrderState::Queued: return "Queued";
        case SupportOrderState::Routed: return "Routed";
        case SupportOrderState::Delivered: return "Delivered";
        case SupportOrderState::Claimed: return "Claimed";
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

const char* DescribeFeyLoad(int queueSize, int capacity) {
    if (capacity <= 0) {
        return "Unknown";
    }
    const float loadRatio = static_cast<float>(queueSize) / static_cast<float>(capacity);
    if (loadRatio >= 0.85f) {
        return "Heavy";
    }
    if (loadRatio >= 0.5f) {
        return "Moderate";
    }
    return "Light";
}

std::string CurrentChatTimeLabel() {
    const std::time_t now = std::time(nullptr);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &now);
#else
    localTime = *std::localtime(&now);
#endif
    std::ostringstream out;
    if (localTime.tm_hour < 10) out << '0';
    out << localTime.tm_hour << ':';
    if (localTime.tm_min < 10) out << '0';
    out << localTime.tm_min;
    return out.str();
}

std::string CurrentLanlineTimestamp() {
    const std::time_t now = std::time(nullptr);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &now);
#else
    localTime = *std::localtime(&now);
#endif
    std::ostringstream out;
    out << (localTime.tm_year + 1900) << '-';
    if (localTime.tm_mon + 1 < 10) out << '0';
    out << (localTime.tm_mon + 1) << '-';
    if (localTime.tm_mday < 10) out << '0';
    out << localTime.tm_mday << ' ';
    if (localTime.tm_hour < 10) out << '0';
    out << localTime.tm_hour << ':';
    if (localTime.tm_min < 10) out << '0';
    out << localTime.tm_min << ':';
    if (localTime.tm_sec < 10) out << '0';
    out << localTime.tm_sec;
    return out.str();
}

void NormalizeStringInventory(std::vector<std::string>& values) {
    values.erase(
        std::remove_if(
            values.begin(),
            values.end(),
            [](const std::string& value) { return value.empty(); }),
        values.end());
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

std::vector<std::string> CollectPendingSupportOrderIds(const std::vector<SupportOrder>& orders) {
    std::vector<std::string> ids;
    ids.reserve(orders.size());
    for (const auto& order : orders) {
        if (order.orderId.empty() ||
            order.state == SupportOrderState::Delivered ||
            order.state == SupportOrderState::Claimed) {
            continue;
        }
        ids.push_back(order.orderId);
    }
    NormalizeStringInventory(ids);
    return ids;
}

std::string JoinLabels(const std::vector<std::string>& labels) {
    std::ostringstream out;
    for (std::size_t index = 0; index < labels.size(); ++index) {
        if (index > 0) {
            out << ", ";
        }
        out << labels[index];
    }
    return out.str();
}

void AddProfileInventoryItem(SessionProfile& profile, const std::string& itemId, int count, float unitWeight) {
    if (itemId.empty() || count <= 0) {
        return;
    }

    for (auto& entry : profile.character.inventory) {
        if (entry.itemId == itemId) {
            entry.count += count;
            if (entry.unitWeight <= 0.0f && unitWeight > 0.0f) {
                entry.unitWeight = unitWeight;
            }
            return;
        }
    }

    profile.character.inventory.push_back({itemId, count, unitWeight});
}

const SupportCatalogItem* FindSupportCatalogItem(const LanlineServicesState& state, std::string_view itemId) {
    for (const auto& item : state.supportCatalog) {
        if (item.id == itemId) {
            return &item;
        }
    }
    return nullptr;
}

void GrantSupportDeliveryItem(SessionProfile& profile,
    const std::string& itemId,
    int count,
    float unitWeight,
    std::vector<std::string>& grantedLabels) {
    AddProfileInventoryItem(profile, itemId, count, unitWeight);
    grantedLabels.push_back(itemId + " x" + std::to_string(count));
}

void GrantSupportDeliveryContent(SessionProfile& profile,
    std::string_view contentId,
    std::vector<std::string>& grantedLabels) {
    if (contentId == "bulk_salvage") {
        GrantSupportDeliveryItem(profile, "steel_scrap", 3, 0.5f, grantedLabels);
        GrantSupportDeliveryItem(profile, "old_plate", 1, 0.5f, grantedLabels);
        return;
    }
    if (contentId == "repair_parts" || contentId == "sealant_roll") {
        GrantSupportDeliveryItem(profile, "repair_patch", 1, 0.2f, grantedLabels);
        return;
    }
    if (contentId == "circuit_scrap") {
        GrantSupportDeliveryItem(profile, "copper_wire", 2, 0.2f, grantedLabels);
        return;
    }
    if (contentId == "filter_media") {
        GrantSupportDeliveryItem(profile, "clean_water", 1, 0.4f, grantedLabels);
        return;
    }
    if (contentId == "field_battery") {
        GrantSupportDeliveryItem(profile, "power_cell", 1, 0.3f, grantedLabels);
        return;
    }
    if (contentId == "track_patch") {
        GrantSupportDeliveryItem(profile, "track_patch", 1, 0.2f, grantedLabels);
        return;
    }
    if (contentId == "servo_patch") {
        GrantSupportDeliveryItem(profile, "servo_patch", 1, 0.2f, grantedLabels);
        return;
    }
    if (contentId == "engine_seal") {
        GrantSupportDeliveryItem(profile, "engine_seal", 1, 0.2f, grantedLabels);
        return;
    }
    if (contentId == "lens_pack") {
        GrantSupportDeliveryItem(profile, "lens_pack", 1, 0.2f, grantedLabels);
        return;
    }
    if (contentId == "medkit") {
        GrantSupportDeliveryItem(profile, "cryo_medkit", 1, 0.5f, grantedLabels);
        return;
    }
    if (contentId == "trauma_kit") {
        GrantSupportDeliveryItem(profile, "cryo_medkit", 2, 0.5f, grantedLabels);
        return;
    }
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

std::vector<ChatChannel> MakeDefaultChatChannels() {
    return {
        {"session", "Session", {{"Lanline", "Session relay ready. Presence log restored.", "boot"}}},
        {"relay", "Relay", {{"Shelter 17 Relay", "Tower sync can reopen broader relay services.", "boot"}}},
        {"support", "Support", {{"Dispatch", "Support catalog mirrors recovery-safe requests only.", "boot"}}},
    };
}

FriendEntry* FindFriendEntry(LanlineServicesState& state, const std::string& handle) {
    for (auto& friendEntry : state.friends) {
        if (friendEntry.handle == handle) {
            return &friendEntry;
        }
    }
    return nullptr;
}

bool PublishRelayChatMessage(const std::string& channelId, const std::string& author, const std::string& body, std::string* errorText) {
    LanlineSessionState sessionState;
    if (!LoadLanlineSessionState(sessionState)) {
        if (errorText != nullptr) {
            *errorText = "No active Lanline session is available for relay transport.";
        }
        return false;
    }

    sessionState.updatedAt = CurrentLanlineTimestamp();
    sessionState.relayMessages.push_back({channelId, author, CurrentChatTimeLabel(), body});
    if (sessionState.relayMessages.size() > 32) {
        sessionState.relayMessages.erase(
            sessionState.relayMessages.begin(),
            sessionState.relayMessages.begin() + static_cast<std::vector<LanlineRelayMessage>::difference_type>(sessionState.relayMessages.size() - 32));
    }

    if (!SaveLanlineSessionState(sessionState) ||
        !SaveLanlineSessionState(sessionState, LanlineSessionSnapshotPath(sessionState.sessionId))) {
        if (errorText != nullptr) {
            *errorText = "Relay transport failed to write Lanline session state.";
        }
        return false;
    }

    return true;
}

bool PublishVoicePresence(const VoiceSettings& voice,
    const std::string& handle,
    bool speaking,
    float peakLevel,
    std::string* errorText) {
    LanlineSessionState sessionState;
    if (!LoadLanlineSessionState(sessionState)) {
        if (errorText != nullptr) {
            *errorText = "No active Lanline session is available for voice relay.";
        }
        return false;
    }

    sessionState.updatedAt = CurrentLanlineTimestamp();
    auto presenceIt = std::find_if(
        sessionState.voicePresence.begin(),
        sessionState.voicePresence.end(),
        [&](const LanlineVoicePresence& entry) { return entry.handle == handle; });
    if (presenceIt == sessionState.voicePresence.end()) {
        sessionState.voicePresence.push_back({handle, voice.enabled, voice.pushToTalk, speaking, peakLevel, CurrentChatTimeLabel()});
    } else {
        presenceIt->voiceEnabled = voice.enabled;
        presenceIt->pushToTalk = voice.pushToTalk;
        presenceIt->speaking = speaking;
        presenceIt->peakLevel = peakLevel;
        presenceIt->timeLabel = CurrentChatTimeLabel();
    }

    if (sessionState.voicePresence.size() > 16) {
        sessionState.voicePresence.erase(
            sessionState.voicePresence.begin(),
            sessionState.voicePresence.begin() + static_cast<std::vector<LanlineVoicePresence>::difference_type>(sessionState.voicePresence.size() - 16));
    }

    if (!SaveLanlineSessionState(sessionState) ||
        !SaveLanlineSessionState(sessionState, LanlineSessionSnapshotPath(sessionState.sessionId))) {
        if (errorText != nullptr) {
            *errorText = "Voice relay failed to write Lanline session state.";
        }
        return false;
    }

    return true;
}

void DrawLanlineServicesLockedScreen(const ServicesUnlockState& unlockState) {
    ImGui::Text("Lanline Services: offline");
    ImGui::Separator();
    if (!unlockState.towerSyncRecovered) {
        ImGui::TextWrapped("Relay access unavailable. Restore and synchronize the first tower to open Lanline Services.");
        ImGui::BulletText("Requirement: first tower restored");
        ImGui::BulletText("Requirement: tower sync completed");
    } else {
        ImGui::TextWrapped("Relay node detected, but services are still restricted while Shelter 17 stabilizes.");
        if (!unlockState.relaySubstationActive) {
            ImGui::BulletText("Requirement: relay substation operational");
        }
        if (!unlockState.serviceBayActive) {
            ImGui::BulletText("Requirement: service bay operational");
        }
        if (!unlockState.waterReclaimerActive) {
            ImGui::BulletText("Requirement: water reclaimer operational");
        }
    }
    ImGui::BulletText("Unlock tier: %s", ToLabel(unlockState.tier));
    if (!unlockState.backboneStage.empty()) {
        ImGui::BulletText("Industrial backbone: %s", unlockState.backboneStage.c_str());
    }
    if (!unlockState.backbonePayoff.empty()) {
        ImGui::TextWrapped("Backbone payoff: %s", unlockState.backbonePayoff.c_str());
    }
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
    std::string relayStatus;
    if (ImGui::Button("Transmit")) {
        if (messageBuffer[0] != '\0') {
            relayStatus.clear();
            if (PublishRelayChatMessage(channel.id, "Operator", messageBuffer, &relayStatus)) {
                AddChatMessage(channel, "Operator", messageBuffer, CurrentChatTimeLabel());
            } else {
                AddChatMessage(channel, "Operator", messageBuffer, "local");
            }
            std::memset(messageBuffer, 0, messageBufferSize);
        }
    }
    if (!relayStatus.empty()) {
        ImGui::TextDisabled("%s", relayStatus.c_str());
    } else {
        ImGui::TextDisabled("Relay chat rides on the active Lanline session state and snapshot mirror.");
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
    static float testPeakLevel = 0.72f;
    std::string voiceStatus;
    ImGui::SliderFloat("Test Peak Level", &testPeakLevel, 0.0f, 1.0f);
    if (ImGui::Button("Send Voice Test Pulse")) {
        PublishVoicePresence(voice, "Operator", true, testPeakLevel, &voiceStatus);
    }
    ImGui::SameLine();
    if (ImGui::Button("Send Idle Voice State")) {
        PublishVoicePresence(voice, "Operator", false, 0.0f, &voiceStatus);
    }
    if (!voiceStatus.empty()) {
        ImGui::TextDisabled("%s", voiceStatus.c_str());
    } else {
        ImGui::TextDisabled("Voice activity now relays through the active Lanline session state; raw audio transport remains future work.");
    }

    bool showedPresence = false;
    for (const auto& entry : state.friends) {
        if (!entry.inCurrentSession && entry.statusText.find("Voice") == std::string::npos) {
            continue;
        }
        if (!showedPresence) {
            ImGui::Separator();
            ImGui::Text("Voice Presence");
            showedPresence = true;
        }
        ImGui::BulletText("%s | %s | %s",
            entry.handle.c_str(),
            entry.statusText.c_str(),
            entry.online ? "online" : "offline");
    }
}

void DrawOperationalSupportCategory(LanlineServicesState& state,
    const ServicesUnlockState& unlockState,
    SupportCategory category,
    const char* title,
    const char* subtitle) {
    ImGui::Text("%s", title);
    ImGui::TextDisabled("%s", subtitle);
    ImGui::Separator();

    bool drewAnyItem = false;
    for (const auto& item : state.supportCatalog) {
        if (!IsAllowedSupportItem(item) || item.currency != StoreCurrency::InGame || item.category != category) {
            continue;
        }
        if (item.category == SupportCategory::TankService && !IsTankServiceUnlocked(unlockState)) {
            continue;
        }
        if (item.category == SupportCategory::Medical && !IsMedicalSupportUnlocked(unlockState)) {
            continue;
        }

        drewAnyItem = true;
        ImGui::PushID(item.id.c_str());
        ImGui::Text("%s", item.label.c_str());
        ImGui::TextWrapped("%s", item.description.c_str());
        ImGui::BulletText("Cost: %d %s", item.priceCredits, ToLabel(item.currency));
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
            order.paymentCurrency = item.currency;
            order.priceCredits = item.priceCredits;
            order.createdAtUnix = static_cast<std::int64_t>(std::time(nullptr));
            order.etaUnix = order.createdAtUnix + (item.category == SupportCategory::TankService ? 420 : 240);
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

    if (!drewAnyItem) {
        if (category == SupportCategory::TankService && !IsTankServiceUnlocked(unlockState)) {
            ImGui::TextDisabled("Tank service remains locked until backbone stability is restored.");
        } else if (category == SupportCategory::Medical && !IsMedicalSupportUnlocked(unlockState)) {
            ImGui::TextDisabled("Medical support remains locked until the first tower sync is restored.");
        } else {
            ImGui::TextDisabled("No relay requests are currently listed in this category.");
        }
    }
}

void DrawCosmeticSupportCategory(LanlineServicesState& state, SupportCategory category, const char* title) {
    ImGui::Text("%s", title);
    ImGui::TextDisabled("Symbolic support only. Cosmetic ownership never grants gameplay advantage.");
    ImGui::Separator();

    bool drewAnyItem = false;
    for (const auto& item : state.supportCatalog) {
        if (!IsAllowedSupportItem(item) || item.currency != StoreCurrency::SymbolicSupport || item.category != category) {
            continue;
        }

        drewAnyItem = true;
        ImGui::PushID(item.id.c_str());
        const bool alreadyOwned =
            std::find(state.ownedCosmetics.begin(), state.ownedCosmetics.end(), item.id) != state.ownedCosmetics.end();
        ImGui::Text("%s", item.label.c_str());
        ImGui::TextWrapped("%s", item.description.c_str());
        ImGui::BulletText("Support tier: %s", item.supportLabel.empty() ? "Symbolic Support" : item.supportLabel.c_str());
        if (ImGui::TreeNode("Includes")) {
            for (const auto& content : item.contents) {
                ImGui::BulletText("%s", content.c_str());
            }
            ImGui::TreePop();
        }
        ImGui::BeginDisabled(alreadyOwned);
        if (ImGui::Button(alreadyOwned ? "Already Owned" : "Support Project")) {
            SupportOrder order;
            order.orderId = "support_" + item.id + "_" + std::to_string(state.supportOrders.size() + 1);
            order.itemId = item.id;
            order.itemLabel = item.label;
            order.paymentCurrency = item.currency;
            order.createdAtUnix = static_cast<std::int64_t>(std::time(nullptr));
            order.state = SupportOrderState::Draft;
            state.supportOrders.push_back(order);
            state.ownedCosmetics.push_back(item.id);
        }
        ImGui::EndDisabled();
        ImGui::Separator();
        ImGui::PopID();
    }

    if (!drewAnyItem) {
        ImGui::TextDisabled("No cosmetic support items are currently listed in this category.");
    }
}

void DrawSupportOrdersSummary(const LanlineServicesState& state, std::int64_t nowUnix) {
    if (state.supportOrders.empty()) {
        return;
    }

    ImGui::Separator();
    ImGui::BulletText("Queued: %d", CountSupportOrdersInState(state, SupportOrderState::Queued));
    ImGui::BulletText("Routed: %d", CountSupportOrdersInState(state, SupportOrderState::Routed));
    ImGui::BulletText("Delivered: %d", CountSupportOrdersInState(state, SupportOrderState::Delivered));
    ImGui::BulletText("Claimed: %d", CountSupportOrdersInState(state, SupportOrderState::Claimed));
    if (ImGui::TreeNode("Active Orders")) {
        for (const auto& order : state.supportOrders) {
            ImGui::BulletText("%s | %s | %s | ETA %s",
                order.orderId.c_str(),
                order.itemLabel.c_str(),
                ToLabel(order.state),
                order.etaUnix > 0 ? FormatCountdown(order.etaUnix - nowUnix).c_str() : "n/a");
        }
        ImGui::TreePop();
    }
}

void DrawSupportOverview(LanlineServicesState& state, std::int64_t nowUnix) {
    ImGui::TextWrapped("Relay support requests. No weapons, no completed tanks, no direct combat advantages.");
    ImGui::BulletText("Recovery Scrip: %d", state.relayCredits);
    ImGui::Separator();
    ImGui::TextDisabled("Operational requests stay on in-game currency. Cosmetic support stays separate and symbolic.");
    DrawSupportOrdersSummary(state, nowUnix);
}

void DrawFeyGateTab(LanlineServicesState& state, const ServicesUnlockState& unlockState, std::int64_t nowUnix) {
    ImGui::TextWrapped("Fey Ring Network: transit windows and relay routes.");
    ImGui::Separator();

    auto drawGateSection = [&](bool interServer, const char* title, bool unlocked, const char* lockedMessage) {
        ImGui::Text("%s", title);
        if (!unlocked) {
            ImGui::TextDisabled("%s", lockedMessage);
            ImGui::Separator();
            return;
        }

        bool drewAnyGate = false;
        for (auto& gate : state.feyGateCycles) {
            if (gate.interServer != interServer) {
                continue;
            }

            drewAnyGate = true;
            gate.state = ComputeFeyGateState(gate, nowUnix);
            ImGui::PushID(gate.id.c_str());
            ImGui::Text("%s -> %s", gate.originLabel.c_str(), gate.destinationLabel.c_str());
            ImGui::BulletText("State: %s", ToLabel(gate.state));
            ImGui::BulletText("Route: %s", gate.routeType.c_str());
            ImGui::BulletText("Load: %s", DescribeFeyLoad(gate.queueSize, gate.capacity));
            ImGui::BulletText("Queue: %d / %d", gate.queueSize, gate.capacity);
            ImGui::BulletText("Stability: %s", gate.unstable ? "unstable transit" : "stable window");
            if (gate.state == FeyGateState::Open || gate.state == FeyGateState::Transit) {
                ImGui::BulletText("Closes in: %s", FormatCountdown(gate.closesAtUnix - nowUnix).c_str());
            } else {
                ImGui::BulletText("Opens in: %s", FormatCountdown(gate.opensAtUnix - nowUnix).c_str());
            }
            if (gate.nextWindowUnix > 0) {
                ImGui::BulletText("Next cycle in: %s", FormatCountdown(gate.nextWindowUnix - nowUnix).c_str());
            }
            ImGui::TextDisabled(gate.interServer ? "Inter-server portal window" : "Inter-city transit window");
            ImGui::Separator();
            ImGui::PopID();
        }

        if (!drewAnyGate) {
            ImGui::TextDisabled("No Fey Ring routes are currently authored for this section.");
            ImGui::Separator();
        }
    };

    drawGateSection(false,
        "Inter-city Routes",
        IsIntercityPortalScheduleUnlocked(unlockState),
        "Inter-city routes unlock after the Shelter 17 backbone stabilizes.");
    drawGateSection(true,
        "Inter-server Portals",
        IsInterserverPortalScheduleUnlocked(unlockState),
        "Inter-server portal windows unlock only after relay expansion and wider network recovery.");

    if (unlockState.intercityPortalsUnlocked || unlockState.interserverPortalsUnlocked) {
        ImGui::TextDisabled("Transit windows remain authored-world schedules, not a public internet browser.");
    }
}

}  // namespace

ServicesUnlockState BuildServicesUnlockState(const SessionProfile& profile, const WorldFieldState* worldState) {
    const WorldFieldState* resolvedWorldState = worldState != nullptr
        ? worldState
        : FindWorldFieldState(profile, profile.selectedWorld);
    SessionProfile scopedProfile = profile;
    if (resolvedWorldState != nullptr && !resolvedWorldState->worldName.empty()) {
        scopedProfile.selectedWorld = NormalizeWorldReference(resolvedWorldState->worldName);
    }
    ServicesUnlockState state{};
    state.towerSyncRecovered = resolvedWorldState != nullptr && resolvedWorldState->towerSyncRecovered;
    state.firstTowerActivated = state.towerSyncRecovered;
    state.localRelayAvailable = resolvedWorldState != nullptr &&
        (resolvedWorldState->localRelayAvailable || state.towerSyncRecovered);
    state.relaySubstationActive = resolvedWorldState != nullptr &&
        IsRelaySubstationOperational(*resolvedWorldState);
    state.serviceBayActive = resolvedWorldState != nullptr &&
        IsServiceBayOperational(*resolvedWorldState);
    state.waterReclaimerActive = resolvedWorldState != nullptr &&
        IsWaterReclaimerOperational(*resolvedWorldState);
    state.backboneStable = resolvedWorldState != nullptr &&
        IsStableRecoveryBackbone(scopedProfile, *resolvedWorldState);
    state.feyRingIntercityUnlocked = state.backboneStable &&
        resolvedWorldState != nullptr &&
        resolvedWorldState->feyRingIntercityUnlocked;
    state.feyRingInterserverUnlocked = state.backboneStable &&
        resolvedWorldState != nullptr &&
        resolvedWorldState->feyRingInterserverUnlocked &&
        IsOrbitalUplinkOperational(scopedProfile, *resolvedWorldState) &&
        IsTradeNetworkOperational(scopedProfile, *resolvedWorldState);
    state.intercityPortalsUnlocked = state.feyRingIntercityUnlocked;
    state.interserverPortalsUnlocked = state.feyRingInterserverUnlocked;
    const auto backboneStatus = CurrentRecoveryBackboneStatus(scopedProfile);
    state.backboneStage = backboneStatus.stage;
    state.backboneStatus = backboneStatus.status;
    state.backbonePayoff = backboneStatus.payoff;
    state.routeEventSummary = ActiveRouteEventSummary(scopedProfile);
    if (resolvedWorldState != nullptr) {
        state.merchantWindowActive =
            resolvedWorldState->activeRouteEventType == "merchant_window" &&
            HasActiveRouteEvent(*resolvedWorldState);
        state.routeEventsResolved = resolvedWorldState->routeEventsResolved;
        state.routeEventsFailed = resolvedWorldState->routeEventsFailed;
        state.routeEventsExpired = resolvedWorldState->routeEventsExpired;
    }

    if (!state.towerSyncRecovered) {
        state.tier = ServicesUnlockTier::Locked;
    } else if (!state.backboneStable) {
        state.tier = ServicesUnlockTier::TowerLinked;
    } else if (!state.feyRingInterserverUnlocked) {
        state.tier = ServicesUnlockTier::BackboneStable;
    } else {
        state.tier = ServicesUnlockTier::RelayExpanded;
    }
    return state;
}

bool IsAllowedSupportItem(const SupportCatalogItem& item) {
    if (item.grantsWeapon || item.grantsCompletedTank || item.combatAdvantage) {
        return false;
    }
    if (item.currency == StoreCurrency::SymbolicSupport) {
        return item.category == SupportCategory::Skins || item.category == SupportCategory::Cosmetics;
    }
    if (item.currency == StoreCurrency::InGame) {
        return item.category != SupportCategory::Skins && item.category != SupportCategory::Cosmetics;
    }
    return false;
}

std::vector<SupportCatalogItem> MakeDefaultSupportCatalog() {
    return {
        {"support_salvage_small", "Salvage Crate / Small", "Basic recovery materials for shelter upkeep and workshop stock.", SupportCategory::Materials, StoreCurrency::InGame, 120, "", false, false, false, TankSubsystem::None, {"bulk_salvage", "repair_parts", "circuit_scrap"}},
        {"relay_filter_bundle", "Filter Media Bundle", "Purification media and relay-safe consumables for continued recovery operations.", SupportCategory::Utility, StoreCurrency::InGame, 95, "", false, false, false, TankSubsystem::None, {"filter_media", "sealant_roll", "field_battery"}},
        {"skin_bt72_ashgray", "BT-72 Hull Livery: Ash Gray", "Cosmetic paint set for BT-72. No gameplay effect.", SupportCategory::Skins, StoreCurrency::SymbolicSupport, 0, "Support Tier A", false, false, false, TankSubsystem::None, {"skin_bt72_ashgray"}},
        {"cosmetic_relay_badge", "Relay Operator Badge", "Terminal insignia and profile marker set. Cosmetic only.", SupportCategory::Cosmetics, StoreCurrency::SymbolicSupport, 0, "Support Tier B", false, false, false, TankSubsystem::None, {"badge_relay_operator", "terminal_theme_relay"}},
        {"tank_suspension_kit", "BT-72 Suspension Repair Kit", "Service bundle for suspension and track maintenance.", SupportCategory::TankService, StoreCurrency::InGame, 210, "", false, false, false, TankSubsystem::Suspension, {"track_patch", "bearing_set", "grease_pack", "alignment_tools"}},
        {"tank_turret_kit", "BT-72 Turret Service Kit", "Turret servo, stabilization and bearing maintenance bundle.", SupportCategory::TankService, StoreCurrency::InGame, 230, "", false, false, false, TankSubsystem::Turret, {"servo_patch", "turret_bearing", "stabilizer_fluid"}},
        {"tank_engine_kit", "BT-72 Engine Service Kit", "Field maintenance kit for engine and cooling assembly.", SupportCategory::TankService, StoreCurrency::InGame, 250, "", false, false, false, TankSubsystem::Engine, {"engine_seal", "coolant_pack", "injector_cleanser"}},
        {"tank_sensor_kit", "BT-72 Sensor Recovery Kit", "Optics, relay and calibration tools for damaged sensor arrays.", SupportCategory::TankService, StoreCurrency::InGame, 185, "", false, false, false, TankSubsystem::Sensors, {"lens_pack", "sensor_relay", "calibration_spool"}},
        {"medkit_standard", "Field Medkit", "Standard expedition med supply for operator recovery.", SupportCategory::Medical, StoreCurrency::InGame, 75, "", false, false, false, TankSubsystem::None, {"medkit", "bandage_roll", "sterile_patch"}},
        {"medkit_trauma", "Trauma Response Pack", "Advanced trauma bundle for severe field damage recovery.", SupportCategory::Medical, StoreCurrency::InGame, 120, "", false, false, false, TankSubsystem::None, {"trauma_kit", "injector", "coagulant_pack"}},
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
    state.chatChannels = MakeDefaultChatChannels();
    state.supportCatalog = MakeDefaultSupportCatalog();
    state.feyGateCycles = MakeDefaultFeyGateCycles(nowUnix);
    return state;
}

LanlineServicesState MakeLanlineServicesStateFromSave(const LanlineServicesSave& save, std::int64_t nowUnix) {
    LanlineServicesState state = MakeDefaultLanlineServicesState(nowUnix);
    state.relayCredits = std::max(0, save.relayCredits);
    state.voice = save.voice;
    state.supportOrders = save.supportOrders;
    state.ownedCosmetics = save.ownedCosmetics;
    NormalizeStringInventory(state.ownedCosmetics);
    return state;
}

LanlineServicesSave BuildLanlineServicesSave(const LanlineServicesState& state) {
    LanlineServicesSave save;
    save.relayCredits = std::max(0, state.relayCredits);
    save.voice = state.voice;
    save.supportOrders = state.supportOrders;
    save.ownedCosmetics = state.ownedCosmetics;
    NormalizeStringInventory(save.ownedCosmetics);
    return save;
}

void ApplyLanlineServicesProfileSnapshot(LanlineServicesState& state, const LanlineServicesProfile& profile) {
    state.relayCredits = std::max(0, profile.relayCredits);
    if (!profile.ownedCosmetics.empty()) {
        state.ownedCosmetics = profile.ownedCosmetics;
        NormalizeStringInventory(state.ownedCosmetics);
    }
}

void SyncLanlineServicesProfileSnapshot(LanlineServicesProfile& profile, const LanlineServicesState& state) {
    profile.relayCredits = std::max(0, state.relayCredits);
    profile.ownedCosmetics = state.ownedCosmetics;
    NormalizeStringInventory(profile.ownedCosmetics);
    profile.pendingSupportOrders = CollectPendingSupportOrderIds(state.supportOrders);
    profile.cosmeticsShopSeen = profile.cosmeticsShopSeen || !profile.ownedCosmetics.empty();
}

void SyncLanlineServicesSessionProfile(SessionProfile& profile, const LanlineServicesState& state) {
    const int previousCredits = std::max(0, profile.lanlineServices.relayCredits);
    SyncLanlineServicesProfileSnapshot(profile.lanlineServices, state);
    if (profile.selectedWorld.empty()) {
        return;
    }

    WorldFieldState* worldState = FindWorldFieldState(profile, profile.selectedWorld, true);
    if (worldState == nullptr) {
        return;
    }

    const int currentCredits = std::max(0, profile.lanlineServices.relayCredits);
    if (currentCredits > previousCredits) {
        worldState->relayCreditsEarned += currentCredits - previousCredits;
    } else if (previousCredits > currentCredits) {
        worldState->relayCreditsSpent += previousCredits - currentCredits;
    }
}

void AdvanceLanlineSupportOrders(LanlineServicesState& state, std::int64_t nowUnix) {
    for (auto& order : state.supportOrders) {
        if (order.state == SupportOrderState::Queued && order.etaUnix > 0 && nowUnix >= (order.createdAtUnix + 120)) {
            order.state = SupportOrderState::Routed;
        }
        if ((order.state == SupportOrderState::Queued || order.state == SupportOrderState::Routed) &&
            order.etaUnix > 0 && nowUnix >= order.etaUnix) {
            order.state = SupportOrderState::Delivered;
        }
    }
}

int CountSupportOrdersInState(const LanlineServicesState& state, SupportOrderState orderState) {
    return static_cast<int>(std::count_if(
        state.supportOrders.begin(),
        state.supportOrders.end(),
        [&](const SupportOrder& order) { return order.state == orderState; }));
}

int ClaimDeliveredSupportOrders(LanlineServicesState& state, SessionProfile& profile, std::string* summary) {
    int claimedCount = 0;
    std::vector<std::string> claimedOrderSummaries;

    for (auto& order : state.supportOrders) {
        if (order.state != SupportOrderState::Delivered) {
            continue;
        }

        std::vector<std::string> grantedLabels;
        if (const auto* catalogItem = FindSupportCatalogItem(state, order.itemId); catalogItem != nullptr) {
            for (const auto& contentId : catalogItem->contents) {
                GrantSupportDeliveryContent(profile, contentId, grantedLabels);
            }
            if (catalogItem->currency == StoreCurrency::SymbolicSupport) {
                profile.lanlineServices.ownedCosmetics.push_back(catalogItem->id);
            }
        } else if (order.paymentCurrency == StoreCurrency::SymbolicSupport) {
            profile.lanlineServices.ownedCosmetics.push_back(order.itemId);
        }

        NormalizeStringInventory(profile.lanlineServices.ownedCosmetics);
        order.state = SupportOrderState::Claimed;
        ++claimedCount;

        if (!grantedLabels.empty()) {
            claimedOrderSummaries.push_back(order.itemLabel + " -> " + JoinLabels(grantedLabels));
        } else if (order.paymentCurrency == StoreCurrency::SymbolicSupport) {
            claimedOrderSummaries.push_back(order.itemLabel + " -> symbolic support logged");
        } else {
            claimedOrderSummaries.push_back(order.itemLabel + " -> depot manifest received");
        }
    }

    if (summary != nullptr) {
        summary->clear();
        if (claimedCount == 1) {
            *summary = "Lanline delivery received: " + claimedOrderSummaries.front() + ".";
        } else if (claimedCount > 1) {
            *summary = "Lanline deliveries received: " + std::to_string(claimedCount) +
                " parcels. Latest: " + claimedOrderSummaries.back() + ".";
        }
    }

    return claimedCount;
}

ServiceHubMode ResolveLanlineServicesMode(const ServicesUnlockState& unlockState, const LanlineSessionState* sessionState) {
    if (!IsLanlineServicesUnlocked(unlockState)) {
        return ServiceHubMode::OfflineLocal;
    }
    if (sessionState != nullptr && sessionState->mode != "Solo") {
        return ServiceHubMode::LanlineLocal;
    }
    return unlockState.backboneStable ? ServiceHubMode::RelayOnline : ServiceHubMode::LanlineLocal;
}

void SyncLanlineServicesPresence(LanlineServicesState& state,
    const LanlineSessionState* sessionState,
    const ServicesUnlockState& unlockState) {
    state.mode = ResolveLanlineServicesMode(unlockState, sessionState);
    state.chatChannels = MakeDefaultChatChannels();
    for (auto& friendEntry : state.friends) {
        friendEntry.inCurrentSession = false;
    }

    if (sessionState == nullptr) {
        if (auto* sessionChannel = FindChatChannel(state, "session")) {
            AddChatMessage(*sessionChannel, "Lanline", "No active session mirror is currently available.", "now");
        }
        return;
    }

    if (auto* sessionChannel = FindChatChannel(state, "session")) {
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

    for (const auto& relayMessage : sessionState->relayMessages) {
        ChatChannel* channel = FindChatChannel(state, relayMessage.channelId);
        if (channel == nullptr) {
            state.chatChannels.push_back({relayMessage.channelId, relayMessage.channelId, {}});
            channel = &state.chatChannels.back();
        }
        AddChatMessage(*channel, relayMessage.author, relayMessage.body, relayMessage.timeLabel);
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

    for (const auto& voicePresence : sessionState->voicePresence) {
        FriendEntry* friendEntry = FindFriendEntry(state, voicePresence.handle);
        const std::string voiceStatus = voicePresence.speaking
            ? ("Voice transmitting | peak " + std::to_string(static_cast<int>(voicePresence.peakLevel * 100.0f)) + "% | " + voicePresence.timeLabel)
            : ("Voice idle | " + voicePresence.timeLabel);
        if (friendEntry == nullptr) {
            state.friends.push_back({
                voicePresence.handle,
                "Lanline Voice",
                voiceStatus,
                true,
                true,
                false,
                std::clamp(voicePresence.peakLevel, 0.0f, 1.5f)});
            continue;
        }
        friendEntry->nodeLabel = "Lanline Voice";
        friendEntry->statusText = voiceStatus;
        friendEntry->online = voicePresence.voiceEnabled;
        friendEntry->inCurrentSession = true;
        friendEntry->voiceVolume = std::clamp(voicePresence.peakLevel, 0.0f, 1.5f);
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

std::filesystem::path DefaultLanlineServicesSavePath() {
    return ProfilesDirectory() / "lanline_services.state";
}

bool SaveLanlineServicesSave(const LanlineServicesSave& save, const std::filesystem::path& path) {
    EnsureProjectDirectories();
    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }

    out << "relay_credits=" << save.relayCredits << '\n';
    out << "voice_enabled=" << (save.voice.enabled ? 1 : 0) << '\n';
    out << "voice_ptt=" << (save.voice.pushToTalk ? 1 : 0) << '\n';
    out << "voice_key=" << save.voice.pushToTalkKey << '\n';
    out << "voice_input_sensitivity=" << save.voice.inputSensitivity << '\n';
    out << "voice_input_gain=" << save.voice.inputGain << '\n';
    out << "voice_output_gain=" << save.voice.outputGain << '\n';
    out << "voice_input_device=" << save.voice.selectedInputDevice << '\n';
    out << "voice_output_device=" << save.voice.selectedOutputDevice << '\n';
    for (const auto& order : save.supportOrders) {
        out << "support_order="
            << order.orderId << ','
            << order.itemId << ','
            << order.itemLabel << ','
            << order.destinationNode << ','
            << static_cast<int>(order.state) << ','
            << static_cast<int>(order.paymentCurrency) << ','
            << order.priceCredits << ','
            << order.createdAtUnix << ','
            << order.etaUnix << '\n';
    }
    out << "owned_cosmetics=" << save.ownedCosmetics.size() << '\n';
    for (const auto& cosmetic : save.ownedCosmetics) {
        out << "cosmetic=" << cosmetic << '\n';
    }
    return true;
}

bool LoadLanlineServicesSave(const std::filesystem::path& path, LanlineServicesSave& outSave) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    outSave = {};
    std::string line;
    while (std::getline(in, line)) {
        const auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, pos);
        const std::string value = line.substr(pos + 1);
        if (key == "relay_credits") outSave.relayCredits = std::stoi(value);
        else if (key == "voice_enabled") outSave.voice.enabled = std::stoi(value) != 0;
        else if (key == "voice_ptt") outSave.voice.pushToTalk = std::stoi(value) != 0;
        else if (key == "voice_key") std::snprintf(outSave.voice.pushToTalkKey, sizeof(outSave.voice.pushToTalkKey), "%s", value.c_str());
        else if (key == "voice_input_sensitivity") outSave.voice.inputSensitivity = std::stof(value);
        else if (key == "voice_input_gain") outSave.voice.inputGain = std::stof(value);
        else if (key == "voice_output_gain") outSave.voice.outputGain = std::stof(value);
        else if (key == "voice_input_device") outSave.voice.selectedInputDevice = std::stoi(value);
        else if (key == "voice_output_device") outSave.voice.selectedOutputDevice = std::stoi(value);
        else if (key == "support_order") {
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (start <= value.size()) {
                const auto next = value.find(',', start);
                parts.push_back(value.substr(start, next == std::string::npos ? std::string::npos : next - start));
                if (next == std::string::npos) {
                    break;
                }
                start = next + 1;
            }
            if (parts.size() >= 9) {
                SupportOrder order;
                order.orderId = parts[0];
                order.itemId = parts[1];
                order.itemLabel = parts[2];
                order.destinationNode = parts[3];
                order.state = static_cast<SupportOrderState>(std::stoi(parts[4]));
                order.paymentCurrency = static_cast<StoreCurrency>(std::stoi(parts[5]));
                order.priceCredits = std::stoi(parts[6]);
                order.createdAtUnix = static_cast<std::int64_t>(std::stoll(parts[7]));
                order.etaUnix = static_cast<std::int64_t>(std::stoll(parts[8]));
                outSave.supportOrders.push_back(order);
            }
        }
        else if (key == "cosmetic") {
            outSave.ownedCosmetics.push_back(value);
        }
    }
    outSave.relayCredits = std::max(0, outSave.relayCredits);
    NormalizeStringInventory(outSave.ownedCosmetics);
    return true;
}

void DrawLanlineServicesPanel(LanlineServicesState& state,
    const ServicesUnlockState& unlockState,
    std::int64_t nowUnix) {
    AdvanceLanlineSupportOrders(state, nowUnix);

    if (!IsLanlineServicesUnlocked(unlockState)) {
        DrawLanlineServicesLockedScreen(unlockState);
        return;
    }

    static char friendSearch[128] = "";
    static char messageInput[256] = "";
    static int selectedChannelIndex = 0;

    ImGui::Text("Lanline Services Online");
    ImGui::BulletText("Unlock tier: %s", ToLabel(unlockState.tier));
    ImGui::BulletText("Tower sync recovered: %s", unlockState.towerSyncRecovered ? "yes" : "no");
    ImGui::BulletText("Local relay available: %s", unlockState.localRelayAvailable ? "yes" : "no");
    ImGui::BulletText("Relay substation: %s", unlockState.relaySubstationActive ? "online" : "offline");
    ImGui::BulletText("Service bay: %s", unlockState.serviceBayActive ? "online" : "offline");
    ImGui::BulletText("Water reclaimer: %s", unlockState.waterReclaimerActive ? "online" : "offline");
    ImGui::BulletText("Backbone stable: %s", unlockState.backboneStable ? "yes" : "no");
    ImGui::BulletText("Fey inter-city: %s", unlockState.feyRingIntercityUnlocked ? "unlocked" : "locked");
    ImGui::BulletText("Fey inter-server: %s", unlockState.feyRingInterserverUnlocked ? "unlocked" : "locked");
    if (!unlockState.backboneStage.empty()) {
        ImGui::BulletText("Industrial backbone: %s", unlockState.backboneStage.c_str());
    }
    if (!unlockState.backboneStatus.empty()) {
        ImGui::TextWrapped("Backbone status: %s", unlockState.backboneStatus.c_str());
    }
    if (!unlockState.backbonePayoff.empty()) {
        ImGui::TextWrapped("Backbone payoff: %s", unlockState.backbonePayoff.c_str());
    }
    if (!unlockState.routeEventSummary.empty()) {
        ImGui::TextWrapped("Route event layer: %s", unlockState.routeEventSummary.c_str());
    }
    ImGui::BulletText("Route events resolved/failed/expired: %d / %d / %d",
        unlockState.routeEventsResolved,
        unlockState.routeEventsFailed,
        unlockState.routeEventsExpired);
    if (unlockState.merchantWindowActive) {
        ImGui::TextWrapped("Merchant window: one discreet broker exchange is currently open on the active recovery route.");
    }
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
        if (ImGui::BeginTabItem("Supplies")) {
            DrawSupportOverview(state, nowUnix);
            DrawOperationalSupportCategory(
                state,
                unlockState,
                SupportCategory::Materials,
                "Supplies",
                "Recovery materials and basic workshop stock on in-game currency.");
            DrawOperationalSupportCategory(
                state,
                unlockState,
                SupportCategory::Utility,
                "Utility",
                "Relay-safe consumables and support bundles on in-game currency.");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Tank Service")) {
            DrawSupportOverview(state, nowUnix);
            DrawOperationalSupportCategory(
                state,
                unlockState,
                SupportCategory::TankService,
                "Tank Service",
                "BT-72 recovery and maintenance requests. Unlocks after backbone stability.");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Medical")) {
            DrawSupportOverview(state, nowUnix);
            DrawOperationalSupportCategory(
                state,
                unlockState,
                SupportCategory::Medical,
                "Medical",
                "Operator recovery supplies. Unlocks after the first tower sync.");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Cosmetics")) {
            DrawSupportOverview(state, nowUnix);
            DrawCosmeticSupportCategory(state, SupportCategory::Skins, "Skins");
            DrawCosmeticSupportCategory(state, SupportCategory::Cosmetics, "Cosmetics");
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
