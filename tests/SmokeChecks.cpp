#include "../include/AppPaths.hpp"
#include "../include/AtomicPersistence.hpp"
#include "../include/BuildAnnouncement.hpp"
#include "../include/GameplayDescriptorRegistry.hpp"
#include "../include/HangarSystem.hpp"
#include "../include/LanlineServices.hpp"
#include "../include/LaunchSession.hpp"
#include "../include/PrefabLibrary.hpp"
#include "../include/SessionProfiles.hpp"
#include "../include/World.hpp"
#include "../include/WorldEditorUndo.hpp"
#include "../include/WorldExport.hpp"
#include "../include/WorldSemanticAuthoring.hpp"
#include "../include/WorldValidation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {

struct WorkingDirectoryGuard {
    explicit WorkingDirectoryGuard(const fs::path& target)
        : original(fs::current_path()) {
        fs::current_path(target);
    }

    ~WorkingDirectoryGuard() {
        std::error_code ec;
        fs::current_path(original, ec);
    }

    fs::path original;
};

bool Check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "[smoke] " << message << '\n';
        return false;
    }
    return true;
}

void WriteRawString(std::ofstream& file, const std::string& value) {
    const auto length = static_cast<std::uint32_t>(value.size());
    file.write(reinterpret_cast<const char*>(&length), sizeof(length));
    file.write(value.data(), static_cast<std::streamsize>(value.size()));
}

bool RunWorldRoundtrip() {
    bunker::World savedWorld;
    savedWorld.GeneratePrototypeZone();
    savedWorld.metadata.name = "Smoke Test World";
    savedWorld.metadata.objective = "Roundtrip world persistence";
    savedWorld.EnsureStarterInfrastructure();
    if (auto* archiveTerminal = savedWorld.FindObjectByRegistryId("[%archive_0001]")) {
        archiveTerminal->editorLayer = "Archive";
        archiveTerminal->prefabSourceId = "prefab_archive_sync";
    }

    const auto saveStatus = bunker::SaveWorldAtomically(savedWorld, bunker::DefaultWorldPath());
    if (!Check(saveStatus.ok, "world save failed: " + saveStatus.message)) {
        return false;
    }

    bunker::World loadedWorld;
    if (!Check(loadedWorld.Load(bunker::DefaultWorldPath().string()), "world load failed")) {
        return false;
    }

    return Check(loadedWorld.metadata.name == savedWorld.metadata.name, "world metadata name mismatch") &&
        Check(loadedWorld.metadata.objective == savedWorld.metadata.objective, "world objective mismatch") &&
        Check(loadedWorld.objects.size() == savedWorld.objects.size(), "world object count mismatch") &&
        Check(loadedWorld.HasScriptTag("echo_trace"), "starter infrastructure missing after roundtrip") &&
        Check(loadedWorld.HasScriptTag("water_reclaimer"), "water reclaimer missing after roundtrip") &&
        Check(loadedWorld.HasLinkTarget("inner_spur_assembly"), "assembly link target missing after roundtrip") &&
        Check(loadedWorld.HasLinkTarget("inner_spur_foundry"), "foundry link target missing after roundtrip") &&
        Check(loadedWorld.HasLinkTarget("inner_spur_reactor"), "reactor link target missing after roundtrip") &&
        Check(loadedWorld.HasLinkTarget("inner_spur_capacitor"), "capacitor link target missing after roundtrip") &&
        Check(loadedWorld.HasLinkTarget("shelter17_backbone"), "relay substation link target missing after roundtrip") &&
        Check(loadedWorld.HasLinkTarget("inner_spur_service"), "service bay link target missing after roundtrip") &&
        Check(loadedWorld.HasLinkTarget("inner_spur_water"), "water reclaimer link target missing after roundtrip") &&
        Check(loadedWorld.CountObjectsInEditorLayer("Service") >= 1, "world roundtrip should preserve inferred service layers") &&
        Check(loadedWorld.CountObjectsInEditorLayer("Archive") == 1, "world roundtrip should preserve custom editor layer assignment") &&
        Check(loadedWorld.FindObjectByRegistryId("[%archive_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%archive_0001]")->prefabSourceId == "prefab_archive_sync",
            "world roundtrip should preserve prefab source linkage");
}

bool RunProfileRoundtrip() {
    bunker::SessionProfile savedProfile = bunker::MakeDefaultSessionProfile();
    savedProfile.account.username = "smoke_user";
    savedProfile.selectedWorld = "smoke_zone.bwld";
    savedProfile.fieldCheckpointKnown = true;
    savedProfile.fieldCheckpointWorld.clear();
    savedProfile.lanlineServices.relayCredits = 1337;
    savedProfile.launcherAnnouncements.lastSeenBuildNumber = bunker::kCurrentBuildNumber;
    savedProfile.launcherAnnouncements.lastSeenAnnouncementId = bunker::CurrentBuildAnnouncement().announcementId;
    savedProfile.launcherAnnouncements.lastSeenVersionLabel = std::string(bunker::kCurrentVersionLabel);

    const auto saveStatus = bunker::SaveProfileAtomically(savedProfile, bunker::DefaultSessionProfilePath());
    if (!Check(saveStatus.ok, "profile save failed: " + saveStatus.message)) {
        return false;
    }

    bunker::SessionProfile loadedProfile;
    if (!Check(bunker::LoadSessionProfile(bunker::DefaultSessionProfilePath(), loadedProfile), "profile load failed")) {
        return false;
    }

    return Check(loadedProfile.account.username == savedProfile.account.username, "profile username mismatch") &&
        Check(loadedProfile.selectedWorld == savedProfile.selectedWorld, "profile selected world mismatch") &&
        Check(loadedProfile.fieldCheckpointWorld == savedProfile.selectedWorld, "profile migration/normalize checkpoint mismatch") &&
        Check(loadedProfile.lanlineServices.relayCredits == savedProfile.lanlineServices.relayCredits, "profile relay credits mismatch") &&
        Check(loadedProfile.launcherAnnouncements.lastSeenBuildNumber == savedProfile.launcherAnnouncements.lastSeenBuildNumber,
            "profile launcher last-seen build mismatch") &&
        Check(loadedProfile.launcherAnnouncements.lastSeenAnnouncementId == savedProfile.launcherAnnouncements.lastSeenAnnouncementId,
            "profile launcher last-seen announcement mismatch");
}

bool RunLauncherAnnouncementSmoke() {
    bunker::SessionProfile unseenProfile = bunker::MakeDefaultSessionProfile();
    if (!Check(
            bunker::ShouldShowBuildAnnouncement(
                unseenProfile.launcherAnnouncements.lastSeenBuildNumber,
                unseenProfile.launcherAnnouncements.lastSeenAnnouncementId),
            "launcher announcement smoke expected unseen profile to show current announcement")) {
        return false;
    }

    unseenProfile.launcherAnnouncements.lastSeenBuildNumber = bunker::kCurrentBuildNumber;
    unseenProfile.launcherAnnouncements.lastSeenAnnouncementId = bunker::CurrentBuildAnnouncement().announcementId;
    unseenProfile.launcherAnnouncements.lastSeenVersionLabel = std::string(bunker::kCurrentVersionLabel);
    if (!Check(
            !bunker::ShouldShowBuildAnnouncement(
                unseenProfile.launcherAnnouncements.lastSeenBuildNumber,
                unseenProfile.launcherAnnouncements.lastSeenAnnouncementId),
            "launcher announcement smoke expected dismissed current announcement to stay hidden")) {
        return false;
    }

    unseenProfile.launcherAnnouncements.lastSeenAnnouncementId = "older_announcement";
    return Check(
        bunker::ShouldShowBuildAnnouncement(
            unseenProfile.launcherAnnouncements.lastSeenBuildNumber,
            unseenProfile.launcherAnnouncements.lastSeenAnnouncementId),
        "launcher announcement smoke expected newer local announcement id to resurface widget");
}

bool RunLanlineServicesRoundtripSmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.selectedWorld = "lanline_smoke_world.bwld";
    profile.fieldCheckpointKnown = true;
    profile.fieldCheckpointWorld = profile.selectedWorld;
    profile.story.outerRoadCleared = true;
    profile.character.collectedTapes.push_back({"tower_pylon_alpha", "Pylon Alpha", true, false, true});
    profile.character.collectedTapes.push_back({"tower_pylon_beta", "Pylon Beta", true, false, true});
    bunker::WorldFieldState* worldState = bunker::FindWorldFieldState(profile, profile.selectedWorld, true);
    if (!Check(worldState != nullptr, "lanline services smoke failed to create world field state")) {
        return false;
    }

    worldState->towerSyncRecovered = true;
    worldState->localRelayAvailable = true;
    worldState->regionalGridOnline = true;
    worldState->caravanRouteActive = true;
    worldState->industrialSurveyActive = true;
    worldState->industrialGateUnlocked = true;
    worldState->industrialOutpostActive = true;
    worldState->tradeNetworkActive = true;
    worldState->railFreightActive = true;
    worldState->railFortressActive = true;
    worldState->recoveryFabricatorActive = true;
    worldState->assemblyCellActive = true;
    worldState->foundryLineActive = true;
    worldState->reactorYardActive = true;
    worldState->capacitorBankActive = true;
    worldState->relaySubstationActive = true;
    worldState->serviceBayActive = true;
    worldState->waterReclaimerActive = true;
    worldState->orbitalUplinkActive = true;
    worldState->feyRingIntercityUnlocked = true;
    worldState->feyRingInterserverUnlocked = true;

    const auto unlockState = bunker::BuildServicesUnlockState(profile, worldState);
    if (!Check(unlockState.towerSyncRecovered, "lanline services smoke expected tower sync recovery flag")) {
        return false;
    }
    if (!Check(unlockState.relaySubstationActive, "lanline services smoke expected relay substation unlock flag")) {
        return false;
    }
    if (!Check(unlockState.serviceBayActive, "lanline services smoke expected service bay unlock flag")) {
        return false;
    }
    if (!Check(unlockState.waterReclaimerActive, "lanline services smoke expected water reclaimer unlock flag")) {
        return false;
    }
    if (!Check(unlockState.backboneStable, "lanline services smoke expected stable backbone")) {
        return false;
    }
    if (!Check(unlockState.feyRingIntercityUnlocked, "lanline services smoke expected inter-city Fey unlock")) {
        return false;
    }
    if (!Check(unlockState.feyRingInterserverUnlocked, "lanline services smoke expected inter-server Fey unlock")) {
        return false;
    }
    if (!Check(bunker::IsTankServiceUnlocked(unlockState), "lanline services smoke expected tank service unlock")) {
        return false;
    }
    if (!Check(bunker::IsMedicalSupportUnlocked(unlockState), "lanline services smoke expected medical support unlock")) {
        return false;
    }

    bunker::LanlineServicesState state = bunker::MakeDefaultLanlineServicesState(1000);
    state.relayCredits = 777;
    state.ownedCosmetics = {"skin_bt72_ashgray", "cosmetic_relay_badge"};
    bunker::SupportOrder order;
    order.orderId = "order_service_01";
    order.itemId = "tank_engine_kit";
    order.itemLabel = "BT-72 Engine Service Kit";
    order.destinationNode = "Shelter 17";
    order.state = bunker::SupportOrderState::Queued;
    order.paymentCurrency = bunker::StoreCurrency::InGame;
    order.priceCredits = 250;
    order.createdAtUnix = 1000;
    order.etaUnix = 1360;
    state.supportOrders.push_back(order);

    bunker::LanlineServicesProfile profileSnapshot{};
    bunker::SyncLanlineServicesProfileSnapshot(profileSnapshot, state);
    if (!Check(profileSnapshot.relayCredits == 777, "lanline services smoke expected profile relay credits sync")) {
        return false;
    }
    if (!Check(profileSnapshot.ownedCosmetics.size() == 2, "lanline services smoke expected cosmetic snapshot sync")) {
        return false;
    }
    if (!Check(profileSnapshot.pendingSupportOrders.size() == 1 &&
            profileSnapshot.pendingSupportOrders[0] == "order_service_01",
            "lanline services smoke expected pending support order sync")) {
        return false;
    }

    const bunker::LanlineServicesSave save = bunker::BuildLanlineServicesSave(state);
    if (!Check(bunker::SaveLanlineServicesSave(save, bunker::DefaultLanlineServicesSavePath()),
            "lanline services smoke failed to save services state")) {
        return false;
    }

    bunker::LanlineServicesSave loadedSave{};
    if (!Check(bunker::LoadLanlineServicesSave(bunker::DefaultLanlineServicesSavePath(), loadedSave),
            "lanline services smoke failed to load services state")) {
        return false;
    }

    bunker::LanlineServicesState loadedState = bunker::MakeLanlineServicesStateFromSave(loadedSave, 1200);
    bunker::ApplyLanlineServicesProfileSnapshot(loadedState, profileSnapshot);
    bunker::AdvanceLanlineSupportOrders(loadedState, 1500);
    if (!Check(bunker::CountSupportOrdersInState(loadedState, bunker::SupportOrderState::Delivered) == 1,
            "lanline services smoke expected one delivered support order after time advance")) {
        return false;
    }

    std::string claimSummary;
    if (!Check(bunker::ClaimDeliveredSupportOrders(loadedState, profile, &claimSummary) == 1,
            "lanline services smoke expected delivered support order claim")) {
        return false;
    }

    profile.lanlineServices.relayCredits = loadedState.relayCredits + 23;
    bunker::SyncLanlineServicesSessionProfile(profile, loadedState);
    bunker::SyncLanlineServicesProfileSnapshot(profileSnapshot, loadedState);
    return Check(loadedState.relayCredits == 777, "lanline services smoke expected relay credits after roundtrip") &&
        Check(loadedState.ownedCosmetics.size() == 2, "lanline services smoke expected owned cosmetics after roundtrip") &&
        Check(loadedState.supportOrders.size() == 1, "lanline services smoke expected one support order after roundtrip") &&
        Check(loadedState.supportOrders[0].destinationNode == "Shelter 17",
            "lanline services smoke expected destination node after roundtrip") &&
        Check(loadedState.supportOrders[0].state == bunker::SupportOrderState::Claimed,
            "lanline services smoke expected claimed support order after runtime claim") &&
        Check(profileSnapshot.pendingSupportOrders.empty(),
            "lanline services smoke expected pending support orders to clear after claim") &&
        Check(worldState->relayCreditsSpent == 23,
            "lanline services smoke expected world relay credit spend mirror after session sync") &&
        Check(claimSummary.find("BT-72 Engine Service Kit") != std::string::npos,
            "lanline services smoke expected claim summary to mention delivered order") &&
        Check(std::any_of(
                profile.character.inventory.begin(),
                profile.character.inventory.end(),
                [](const bunker::InventoryEntry& item) {
                    return item.itemId == "engine_seal" && item.count >= 1;
                }),
            "lanline services smoke expected engine service kit delivery to grant engine_seal inventory");
}

bool RunTankServiceKitSmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.character.inventory.push_back({"track_patch", 1, 0.2f});
    profile.character.inventory.push_back({"servo_patch", 1, 0.2f});
    profile.character.inventory.push_back({"engine_seal", 1, 0.2f});
    profile.character.inventory.push_back({"lens_pack", 1, 0.2f});
    profile.partnerTank.damage.hull = 52.0f;
    profile.partnerTank.damage.bucket = 48.0f;
    profile.partnerTank.damage.turret = 41.0f;
    profile.partnerTank.damage.sensors = 36.0f;
    profile.partnerTank.damage.cockpit = 74.0f;
    profile.partnerTank.damage.powerCore = 43.0f;
    profile.partnerTank.energyReserve = 58.0f;
    profile.partnerTank.inRepair = true;

    const auto countItem = [&](std::string_view itemId) {
        const auto it = std::find_if(
            profile.character.inventory.begin(),
            profile.character.inventory.end(),
            [&](const bunker::InventoryEntry& entry) { return entry.itemId == itemId; });
        return it == profile.character.inventory.end() ? 0 : it->count;
    };

    std::string eventText;
    if (!Check(bunker::TryConsumeBestTankServiceKit(profile, &eventText),
            "tank service smoke expected engine kit to apply first")) {
        return false;
    }
    if (!Check(countItem("engine_seal") == 0, "tank service smoke expected engine seal to be consumed")) {
        return false;
    }
    if (!Check(profile.partnerTank.damage.powerCore == 75.0f && profile.partnerTank.energyReserve == 76.0f,
            "tank service smoke expected engine kit to restore power core and reserve charge")) {
        return false;
    }
    if (!Check(eventText.find("Engine service kit applied") != std::string::npos,
            "tank service smoke expected engine kit event text")) {
        return false;
    }

    eventText.clear();
    if (!Check(bunker::TryConsumeBestTankServiceKit(profile, &eventText),
            "tank service smoke expected suspension kit to apply second")) {
        return false;
    }
    if (!Check(countItem("track_patch") == 0, "tank service smoke expected track patch to be consumed")) {
        return false;
    }
    if (!Check(profile.partnerTank.damage.hull == 74.0f && profile.partnerTank.damage.bucket == 76.0f,
            "tank service smoke expected suspension kit to restore hull carriage and bucket rig")) {
        return false;
    }
    if (!Check(eventText.find("Suspension repair kit applied") != std::string::npos,
            "tank service smoke expected suspension kit event text")) {
        return false;
    }

    eventText.clear();
    if (!Check(bunker::TryConsumeBestTankServiceKit(profile, &eventText),
            "tank service smoke expected sensor kit to apply third")) {
        return false;
    }
    if (!Check(countItem("lens_pack") == 0, "tank service smoke expected lens pack to be consumed")) {
        return false;
    }
    if (!Check(profile.partnerTank.damage.sensors == 70.0f && profile.partnerTank.damage.cockpit == 84.0f,
            "tank service smoke expected sensor kit to restore optics and cockpit feed")) {
        return false;
    }
    if (!Check(eventText.find("Sensor recovery kit applied") != std::string::npos,
            "tank service smoke expected sensor kit event text")) {
        return false;
    }

    eventText.clear();
    if (!Check(bunker::TryConsumeBestTankServiceKit(profile, &eventText),
            "tank service smoke expected turret kit to apply fourth")) {
        return false;
    }
    if (!Check(countItem("servo_patch") == 0, "tank service smoke expected servo patch to be consumed")) {
        return false;
    }
    if (!Check(profile.partnerTank.damage.turret == 73.0f,
            "tank service smoke expected turret kit to restore turret integrity")) {
        return false;
    }
    if (!Check(eventText.find("Turret service kit applied") != std::string::npos,
            "tank service smoke expected turret kit event text")) {
        return false;
    }
    if (!Check(!profile.partnerTank.inRepair,
            "tank service smoke expected successful service to clear in-repair flag")) {
        return false;
    }

    eventText.clear();
    if (!Check(!bunker::TryConsumeBestTankServiceKit(profile, &eventText),
            "tank service smoke expected missing kit fallback after all kits are consumed")) {
        return false;
    }
    if (!Check(eventText.find("Compatible tank service kit not found.") != std::string::npos,
            "tank service smoke expected missing kit message")) {
        return false;
    }

    bunker::SessionProfile idleProfile = bunker::MakeDefaultSessionProfile();
    idleProfile.character.inventory.push_back({"track_patch", 1, 0.2f});
    std::string idleEvent;
    return Check(!bunker::TryConsumeBestTankServiceKit(idleProfile, &idleEvent),
            "tank service smoke expected idle tank not to consume a valid kit") &&
        Check(idleProfile.character.inventory.back().count == 1,
            "tank service smoke expected idle tank to keep the suspension kit") &&
        Check(idleEvent.find("no damaged subsystem") != std::string::npos,
            "tank service smoke expected explicit no-damage guidance");
}

bool RunLaunchTicketFlow() {
    bunker::LaunchTicketInfo issuedTicket;
    issuedTicket.accountId = "#10077";
    issuedTicket.sessionMode = "lanline";
    issuedTicket.characterName = "Smoke Scout";
    issuedTicket.selectedWorld = "smoke_zone.bwld";
    issuedTicket.lanlineSessionId = "relay-test-01";
    issuedTicket.hostEndpoint = "127.0.0.1:4100";

    if (!Check(bunker::IssueLaunchTicket(issuedTicket), "launch ticket issue failed")) {
        return false;
    }

    bunker::LaunchTicketInfo consumedTicket;
    std::string failureReason;
    if (!Check(bunker::ConsumeLaunchTicket(consumedTicket, failureReason), "launch ticket consume failed: " + failureReason)) {
        return false;
    }

    return Check(consumedTicket.accountId == issuedTicket.accountId, "launch ticket account mismatch") &&
        Check(consumedTicket.sessionMode == issuedTicket.sessionMode, "launch ticket mode mismatch") &&
        Check(consumedTicket.selectedWorld == issuedTicket.selectedWorld, "launch ticket world mismatch") &&
        Check(!fs::exists(bunker::LaunchTicketPath()), "launch ticket file was not removed after consume");
}

bool RunGameplayDescriptorValidationSmoke() {
    if (!Check(bunker::NormalizeGameplayDescriptorTag("radio_tower") == "tower_sync", "tower alias normalization failed")) {
        return false;
    }
    if (!Check(bunker::NormalizeGameplayDescriptorTag("workshop_field_service") == "workshop_service", "workshop alias normalization failed")) {
        return false;
    }

    bunker::World world;
    world.metadata.name = "Validation Smoke";

    bunker::MapObject tower;
    tower.registryId = "[%tower_0001]";
    tower.displayName = "Tower";
    tower.interaction = bunker::InteractionType::Terminal;
    tower.category = bunker::ObjectCategory::Terminal;
    tower.scriptTag = "radio_tower";
    tower.linkTarget = "regional_grid";
    world.objects.push_back(tower);

    bunker::MapObject gate;
    gate.registryId = "[%gate_0001]";
    gate.displayName = "Industrial Gate";
    gate.interaction = bunker::InteractionType::Transition;
    gate.category = bunker::ObjectCategory::Landmark;
    gate.scriptTag = "industrial_gate";
    gate.linkTarget = "[%route_anchor_0001]";
    world.objects.push_back(gate);

    const auto issues = bunker::ValidateWorldForRuntime(world);
    const int errors = bunker::CountValidationErrors(issues);
    const int warnings = bunker::CountValidationWarnings(issues);

    return Check(errors == 1, "expected one validation error for missing registry link target") &&
        Check(warnings == 1, "expected one validation warning for legacy alias");
}

bool RunSemanticDependencyValidationSmoke() {
    bunker::World world;
    world.metadata.name = "Semantic Dependency Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0001]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "wrong_water_target";
    world.objects.push_back(waterReclaimer);

    const auto issues = bunker::ValidateWorldForRuntime(world);
    int dependencyWarnings = 0;
    bool sawLinkTargetMismatch = false;
    bool sawStructuredDependency = false;
    for (const auto& issue : issues) {
        if (issue.code == "missing_authored_dependency") {
            ++dependencyWarnings;
            if (issue.scriptTag == "water_reclaimer" &&
                (issue.relatedValue == "service_bay" ||
                 issue.relatedValue == "relay_substation" ||
                 issue.relatedValue == "recovery_fabricator")) {
                sawStructuredDependency = true;
            }
        }
        if (issue.code == "descriptor_link_target_mismatch") {
            sawLinkTargetMismatch = issue.scriptTag == "water_reclaimer" &&
                issue.relatedValue == "inner_spur_water";
        }
    }

    return Check(dependencyWarnings == 3, "expected three authored dependency warnings for water_reclaimer") &&
        Check(sawLinkTargetMismatch, "expected canonical link target mismatch warning for water_reclaimer") &&
        Check(sawStructuredDependency, "expected structured dependency context for water_reclaimer");
}

bool RunSemanticDependencyGraphSmoke() {
    const auto dependencyTags = bunker::RequiredSemanticDependencyTags("water_reclaimer");
    if (!Check(dependencyTags.size() == 3, "water_reclaimer should expose three shared semantic dependencies")) {
        return false;
    }
    if (!Check(dependencyTags[0] == "service_bay" &&
            dependencyTags[1] == "relay_substation" &&
            dependencyTags[2] == "recovery_fabricator",
            "water_reclaimer shared dependency ordering mismatch")) {
        return false;
    }

    bunker::World world;
    world.metadata.name = "Semantic Dependency Graph Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0001]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    world.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    if (!Check(batch.createdCount == 7, "dependency graph smoke expected cascade setup to create seven anchors")) {
        return false;
    }

    const auto graph = bunker::BuildSemanticDependencyGraph(world, 0);
    const auto objectIndices = bunker::CollectSemanticDependencyObjectIndices(world, 0, true);

    int presentEdges = 0;
    bool sawWaterToService = false;
    bool sawWaterToRelay = false;
    bool sawWaterToFabricator = false;
    bool sawServiceToFoundry = false;
    bool sawRelayToCapacitor = false;
    for (const auto& edge : graph) {
        if (edge.dependencyPresent) {
            ++presentEdges;
        }
        if (edge.sourceScriptTag == "water_reclaimer" && edge.dependencyScriptTag == "service_bay") {
            sawWaterToService = edge.dependencyPresent;
        }
        if (edge.sourceScriptTag == "water_reclaimer" && edge.dependencyScriptTag == "relay_substation") {
            sawWaterToRelay = edge.dependencyPresent;
        }
        if (edge.sourceScriptTag == "water_reclaimer" && edge.dependencyScriptTag == "recovery_fabricator") {
            sawWaterToFabricator = edge.dependencyPresent;
        }
        if (edge.sourceScriptTag == "service_bay" && edge.dependencyScriptTag == "foundry_line") {
            sawServiceToFoundry = edge.dependencyPresent;
        }
        if (edge.sourceScriptTag == "relay_substation" && edge.dependencyScriptTag == "capacitor_bank") {
            sawRelayToCapacitor = edge.dependencyPresent;
        }
    }

    return Check(graph.size() == 9, "semantic dependency graph should expose nine recursive edges for water_reclaimer chain") &&
        Check(presentEdges == 9, "semantic dependency graph edges should all be resolved after cascade creation") &&
        Check(objectIndices.size() == 8, "semantic dependency object collection should include root plus seven anchors") &&
        Check(sawWaterToService, "semantic dependency graph missing water_reclaimer -> service_bay edge") &&
        Check(sawWaterToRelay, "semantic dependency graph missing water_reclaimer -> relay_substation edge") &&
        Check(sawWaterToFabricator, "semantic dependency graph missing water_reclaimer -> recovery_fabricator edge") &&
        Check(sawServiceToFoundry, "semantic dependency graph missing service_bay -> foundry_line edge") &&
        Check(sawRelayToCapacitor, "semantic dependency graph missing relay_substation -> capacitor_bank edge");
}

bool RunSemanticLayoutSmoke() {
    bunker::World world;
    world.metadata.name = "Semantic Layout Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0001]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    waterReclaimer.x = 12.0f;
    waterReclaimer.y = -3.0f;
    world.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    if (!Check(batch.createdCount == 7, "semantic layout smoke expected cascade setup to create seven anchors")) {
        return false;
    }

    const int movedObjects = bunker::AutoLayoutSemanticDependencyChain(world, 0, statusText);
    if (!Check(movedObjects == 7, "semantic layout should reposition all seven dependency anchors")) {
        return false;
    }

    const auto* serviceBay = world.FindObjectByScriptTag("service_bay");
    const auto* relaySubstation = world.FindObjectByScriptTag("relay_substation");
    const auto* recoveryFabricator = world.FindObjectByScriptTag("recovery_fabricator");
    const auto* capacitorBank = world.FindObjectByScriptTag("capacitor_bank");
    const auto* reactorYard = world.FindObjectByScriptTag("reactor_yard");
    const auto* industrialOutpost = world.FindObjectByScriptTag("industrial_outpost");
    const auto* foundryLine = world.FindObjectByScriptTag("foundry_line");

    if (!Check(serviceBay != nullptr &&
            relaySubstation != nullptr &&
            recoveryFabricator != nullptr &&
            capacitorBank != nullptr &&
            reactorYard != nullptr &&
            industrialOutpost != nullptr &&
            foundryLine != nullptr,
            "semantic layout smoke expected all anchors to exist after cascade")) {
        return false;
    }

    const float rootX = world.objects[0].x;
    const float rootY = world.objects[0].y;
    const float firstLayerX = serviceBay->x;
    const float secondLayerX = capacitorBank->x;
    auto almostEqual = [](float lhs, float rhs) {
        return std::abs(lhs - rhs) < 0.001f;
    };

    return Check(almostEqual(rootX, 12.0f) && almostEqual(rootY, -3.0f), "semantic layout should not move root object") &&
        Check(firstLayerX > rootX, "semantic layout should push first dependency layer away from root") &&
        Check(secondLayerX > firstLayerX, "semantic layout should push second dependency layer beyond first layer") &&
        Check(almostEqual(relaySubstation->x, firstLayerX) && almostEqual(recoveryFabricator->x, firstLayerX),
            "first dependency layer should share one x-column") &&
        Check(almostEqual(reactorYard->x, secondLayerX) &&
            almostEqual(industrialOutpost->x, secondLayerX) &&
            almostEqual(foundryLine->x, secondLayerX),
            "second dependency layer should share one x-column") &&
        Check(serviceBay->y < relaySubstation->y && relaySubstation->y < recoveryFabricator->y,
            "first dependency layer should be ordered into semantic lanes") &&
        Check(capacitorBank->y < reactorYard->y && reactorYard->y < industrialOutpost->y && industrialOutpost->y < foundryLine->y,
            "second dependency layer should be ordered into semantic lanes");
}

bool RunSemanticLayoutPreserveManualSmoke() {
    bunker::World world;
    world.metadata.name = "Semantic Layout Preserve Manual Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0001]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    waterReclaimer.x = 10.0f;
    waterReclaimer.y = -4.0f;
    world.objects.push_back(waterReclaimer);

    bunker::MapObject serviceBay;
    serviceBay.registryId = "[%service_manual_0001]";
    serviceBay.displayName = "Service Bay";
    serviceBay.interaction = bunker::InteractionType::Terminal;
    serviceBay.category = bunker::ObjectCategory::Terminal;
    serviceBay.scriptTag = "service_bay";
    serviceBay.linkTarget = "inner_spur_service";
    serviceBay.x = 27.0f;
    serviceBay.y = -17.0f;
    world.objects.push_back(serviceBay);

    bunker::MapObject relaySubstation;
    relaySubstation.registryId = "[%relay_manual_0001]";
    relaySubstation.displayName = "Relay Substation";
    relaySubstation.interaction = bunker::InteractionType::Terminal;
    relaySubstation.category = bunker::ObjectCategory::Terminal;
    relaySubstation.scriptTag = "relay_substation";
    relaySubstation.linkTarget = "shelter17_backbone";
    relaySubstation.x = 30.0f;
    relaySubstation.y = 12.0f;
    world.objects.push_back(relaySubstation);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    if (!Check(batch.createdCount == 5, "semantic preserve-manual smoke expected cascade setup to create five anchors")) {
        return false;
    }

    const float authoredServiceX = world.objects[1].x;
    const float authoredServiceY = world.objects[1].y;
    const float authoredRelayX = world.objects[2].x;
    const float authoredRelayY = world.objects[2].y;
    auto* pinnedRecoveryFabricator = world.FindObjectByScriptTag("recovery_fabricator");
    if (!Check(pinnedRecoveryFabricator != nullptr, "preserve-manual layout smoke expected recovery_fabricator after cascade")) {
        return false;
    }
    pinnedRecoveryFabricator->semanticLayoutPinned = true;
    const float pinnedRecoveryX = pinnedRecoveryFabricator->x;
    const float pinnedRecoveryY = pinnedRecoveryFabricator->y;

    bunker::SemanticLayoutOptions preserveOptions;
    preserveOptions.preserveManualPlacement = true;
    const int movedAutoAnchors = bunker::AutoLayoutSemanticDependencyChain(world, 0, statusText, preserveOptions);
    if (!Check(movedAutoAnchors == 4, "preserve-manual layout should reposition only four movable auto-created anchors")) {
        return false;
    }

    const auto* authoredServiceBay = world.FindObjectByScriptTag("service_bay");
    const auto* authoredRelaySubstation = world.FindObjectByScriptTag("relay_substation");
    const auto* recoveryFabricator = world.FindObjectByScriptTag("recovery_fabricator");
    const auto* capacitorBank = world.FindObjectByScriptTag("capacitor_bank");
    const auto* reactorYard = world.FindObjectByScriptTag("reactor_yard");
    const auto* industrialOutpost = world.FindObjectByScriptTag("industrial_outpost");
    const auto* foundryLine = world.FindObjectByScriptTag("foundry_line");
    auto almostEqual = [](float lhs, float rhs) {
        return std::abs(lhs - rhs) < 0.001f;
    };

    if (!Check(authoredServiceBay != nullptr &&
            authoredRelaySubstation != nullptr &&
            recoveryFabricator != nullptr &&
            capacitorBank != nullptr &&
            reactorYard != nullptr &&
            industrialOutpost != nullptr &&
            foundryLine != nullptr,
            "preserve-manual layout smoke expected full dependency chain after cascade")) {
        return false;
    }

    if (!Check(almostEqual(authoredServiceBay->x, authoredServiceX) && almostEqual(authoredServiceBay->y, authoredServiceY),
            "preserve-manual layout should not move authored service bay")) {
        return false;
    }
    if (!Check(almostEqual(authoredRelaySubstation->x, authoredRelayX) && almostEqual(authoredRelaySubstation->y, authoredRelayY),
            "preserve-manual layout should not move authored relay substation")) {
        return false;
    }
    if (!Check(almostEqual(recoveryFabricator->x, pinnedRecoveryX) && almostEqual(recoveryFabricator->y, pinnedRecoveryY),
            "preserve-manual layout should not move explicitly pinned auto anchor")) {
        return false;
    }
    if (!Check(!bunker::IsAutoGeneratedSemanticAnchor(*authoredServiceBay) &&
            !bunker::IsAutoGeneratedSemanticAnchor(*authoredRelaySubstation),
            "preserve-manual smoke expected authored anchors to remain non-auto")) {
        return false;
    }
    if (!Check(bunker::IsAutoGeneratedSemanticAnchor(*recoveryFabricator) && bunker::IsPinnedSemanticAnchor(*recoveryFabricator),
            "preserve-manual smoke expected pinned recovery_fabricator to remain auto but explicit pinned")) {
        return false;
    }
    if (!Check(bunker::IsAutoGeneratedSemanticAnchor(*recoveryFabricator) &&
            bunker::IsAutoGeneratedSemanticAnchor(*capacitorBank) &&
            bunker::IsAutoGeneratedSemanticAnchor(*reactorYard) &&
            bunker::IsAutoGeneratedSemanticAnchor(*industrialOutpost) &&
            bunker::IsAutoGeneratedSemanticAnchor(*foundryLine),
            "preserve-manual smoke expected cascade-created anchors to be marked auto")) {
        return false;
    }
    if (!Check(statusText.find("preserved 3 authored/pinned anchors") != std::string::npos,
            "preserve-manual layout status should mention preserved authored and pinned anchors")) {
        return false;
    }

    bunker::SemanticLayoutOptions fullReflowOptions;
    fullReflowOptions.preserveManualPlacement = false;
    const int movedAuthoredAnchors = bunker::AutoLayoutSemanticDependencyChain(world, 0, statusText, fullReflowOptions);
    const float firstLayerX = authoredServiceBay->x;
    return Check(movedAuthoredAnchors == 3, "full semantic reflow should move the two authored anchors and one pinned auto anchor that were previously preserved") &&
        Check(firstLayerX > world.objects[0].x, "full semantic reflow should still place first layer beyond root") &&
        Check(almostEqual(authoredServiceBay->x, firstLayerX),
            "full semantic reflow should align authored service bay into the first-layer column") &&
        Check(almostEqual(authoredRelaySubstation->x, firstLayerX) && almostEqual(recoveryFabricator->x, firstLayerX),
            "full semantic reflow should align authored and auto anchors into one first-layer column") &&
        Check(authoredServiceBay->y < authoredRelaySubstation->y && authoredRelaySubstation->y < recoveryFabricator->y,
            "full semantic reflow should restore first-layer semantic lane ordering");
}

bool RunSemanticAuthoringStateRoundtripSmoke() {
    bunker::World savedWorld;
    savedWorld.metadata.name = "Semantic Authoring State Roundtrip";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0001]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    savedWorld.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(savedWorld, statusText);
    if (!Check(batch.createdCount == 7, "semantic state roundtrip smoke expected seven created dependency anchors")) {
        return false;
    }

    const int adoptedAnchors = bunker::AdoptSemanticDependencyChainAsAuthored(savedWorld, 0, false, statusText);
    if (!Check(adoptedAnchors == 7, "semantic state roundtrip smoke expected adopt-chain to convert seven auto anchors")) {
        return false;
    }

    if (!Check(savedWorld.Save(bunker::DefaultWorldPath().string()), "semantic state roundtrip world save failed")) {
        return false;
    }

    bunker::World loadedWorld;
    if (!Check(loadedWorld.Load(bunker::DefaultWorldPath().string()), "semantic state roundtrip world load failed")) {
        return false;
    }

    const char* expectedAdoptedTags[] = {
        "service_bay",
        "relay_substation",
        "recovery_fabricator",
        "foundry_line",
        "industrial_outpost",
        "capacitor_bank",
        "reactor_yard"
    };
    for (const char* scriptTag : expectedAdoptedTags) {
        const auto* object = loadedWorld.FindObjectByScriptTag(scriptTag);
        if (!Check(object != nullptr, std::string("loaded world missing adopted semantic anchor: ") + scriptTag)) {
            return false;
        }
        if (!Check(!bunker::IsAutoGeneratedSemanticAnchor(*object),
                std::string("adopted semantic anchor should persist as authored: ") + scriptTag)) {
            return false;
        }
        if (!Check(bunker::IsPinnedSemanticAnchor(*object),
                std::string("adopted semantic anchor should persist pinned placement: ") + scriptTag)) {
            return false;
        }
    }

    const auto* loadedRoot = loadedWorld.FindObjectByScriptTag("water_reclaimer");
    return Check(loadedRoot != nullptr, "loaded world missing semantic root after roundtrip") &&
        Check(!bunker::IsAutoGeneratedSemanticAnchor(*loadedRoot), "semantic root should not become auto-created after roundtrip") &&
        Check(!bunker::IsPinnedSemanticAnchor(*loadedRoot), "semantic root should remain unpinned after roundtrip");
}

bool RunSemanticAutoAnchorValidationSmoke() {
    bunker::World world;
    world.metadata.name = "Semantic Auto Anchor Validation Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0001]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    world.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    if (!Check(batch.createdCount == 7, "semantic auto-anchor validation smoke expected seven created dependency anchors")) {
        return false;
    }

    auto countIssuesByCode = [](const std::vector<bunker::ValidationIssue>& issues, std::string_view code) {
        int count = 0;
        for (const auto& issue : issues) {
            if (issue.code == code) {
                ++count;
            }
        }
        return count;
    };

    const auto issuesBeforeAdopt = bunker::ValidateWorldForRuntime(world);
    if (!Check(countIssuesByCode(issuesBeforeAdopt, "auto_created_semantic_anchor") == 7,
            "semantic auto-anchor validation should warn about all seven cascade-created anchors")) {
        return false;
    }

    const int adoptedAnchors = bunker::AdoptAllAutoCreatedSemanticAnchors(world, statusText);
    if (!Check(adoptedAnchors == 7, "semantic auto-anchor validation smoke expected adopt-all to convert seven anchors")) {
        return false;
    }

    const auto issuesAfterAdopt = bunker::ValidateWorldForRuntime(world);
    return Check(countIssuesByCode(issuesAfterAdopt, "auto_created_semantic_anchor") == 0,
            "semantic auto-anchor validation warnings should clear after adopt-all") &&
        Check(bunker::CountValidationErrors(issuesAfterAdopt) == 0, "semantic auto-anchor validation should not create errors after adopt-all") &&
        Check(bunker::CountValidationWarnings(issuesAfterAdopt) == 0, "semantic auto-anchor validation should fully clear warnings after adopt-all");
}

bool RunPrefabLibrarySemanticStateSmoke() {
    std::vector<bunker::PrefabRecord> savedPrefabs;

    bunker::PrefabRecord authoredPrefab;
    authoredPrefab.id = "prefab_authored_relay";
    authoredPrefab.label = "Authored Relay";
    authoredPrefab.targetType = "Structure";
    authoredPrefab.sourceLabel = "Smoke capture";
    authoredPrefab.completionMode = "Captured";
    authoredPrefab.object.registryId = "[%relay_prefab_0001]";
    authoredPrefab.object.displayName = "Relay Prefab";
    authoredPrefab.object.interaction = bunker::InteractionType::Terminal;
    authoredPrefab.object.category = bunker::ObjectCategory::Terminal;
    authoredPrefab.object.scriptTag = "relay_substation";
    authoredPrefab.object.linkTarget = "shelter17_backbone";
    authoredPrefab.object.editorLayer = "Service";
    authoredPrefab.object.semanticAutoCreated = false;
    authoredPrefab.object.semanticLayoutPinned = true;
    savedPrefabs.push_back(authoredPrefab);

    bunker::PrefabRecord autoPrefab;
    autoPrefab.id = "prefab_auto_water";
    autoPrefab.label = "Auto Water";
    autoPrefab.targetType = "Environment";
    autoPrefab.sourceLabel = "Concept import";
    autoPrefab.completionMode = "Keep as partial shell";
    autoPrefab.object.registryId = "[%water_reclaimer_auto_0001]";
    autoPrefab.object.displayName = "Water Auto Prefab";
    autoPrefab.object.interaction = bunker::InteractionType::Terminal;
    autoPrefab.object.category = bunker::ObjectCategory::Terminal;
    autoPrefab.object.scriptTag = "water_reclaimer";
    autoPrefab.object.linkTarget = "inner_spur_water";
    autoPrefab.object.editorLayer = "Waterworks";
    autoPrefab.object.semanticAutoCreated = true;
    autoPrefab.object.semanticLayoutPinned = false;
    autoPrefab.object.manualLoot = false;
    savedPrefabs.push_back(autoPrefab);

    if (!Check(bunker::SavePrefabLibrary(savedPrefabs), "prefab library semantic state smoke failed to save prefab library")) {
        return false;
    }

    std::vector<bunker::PrefabRecord> loadedPrefabs;
    if (!Check(bunker::LoadPrefabLibrary(loadedPrefabs), "prefab library semantic state smoke failed to load prefab library")) {
        return false;
    }

    if (!Check(loadedPrefabs.size() == 2, "prefab library semantic state smoke expected two prefabs after roundtrip")) {
        return false;
    }

    return Check(loadedPrefabs[0].label == authoredPrefab.label, "prefab library should preserve authored prefab label") &&
        Check(loadedPrefabs[0].id == authoredPrefab.id, "prefab library should preserve authored prefab id") &&
        Check(loadedPrefabs[0].targetType == authoredPrefab.targetType, "prefab library should preserve authored prefab target type") &&
        Check(loadedPrefabs[0].sourceLabel == authoredPrefab.sourceLabel, "prefab library should preserve authored prefab source label") &&
        Check(loadedPrefabs[0].completionMode == authoredPrefab.completionMode, "prefab library should preserve authored prefab completion mode") &&
        Check(loadedPrefabs[0].object.editorLayer == authoredPrefab.object.editorLayer, "prefab library should preserve authored prefab layer") &&
        Check(loadedPrefabs[0].object.semanticLayoutPinned, "prefab library should preserve authored pinned semantic state") &&
        Check(!loadedPrefabs[0].object.semanticAutoCreated, "prefab library should preserve authored semantic origin") &&
        Check(loadedPrefabs[1].label == autoPrefab.label, "prefab library should preserve auto prefab label") &&
        Check(loadedPrefabs[1].id == autoPrefab.id, "prefab library should preserve auto prefab id") &&
        Check(loadedPrefabs[1].targetType == autoPrefab.targetType, "prefab library should preserve auto prefab target type") &&
        Check(loadedPrefabs[1].sourceLabel == autoPrefab.sourceLabel, "prefab library should preserve auto prefab source label") &&
        Check(loadedPrefabs[1].completionMode == autoPrefab.completionMode, "prefab library should preserve auto prefab completion mode") &&
        Check(loadedPrefabs[1].object.editorLayer == autoPrefab.object.editorLayer, "prefab library should preserve custom prefab layer") &&
        Check(loadedPrefabs[1].object.semanticAutoCreated, "prefab library should preserve auto semantic origin") &&
        Check(!loadedPrefabs[1].object.semanticLayoutPinned, "prefab library should preserve unpinned auto semantic state");
}

bool RunPrefabUsageAndExportReportSmoke() {
    std::vector<bunker::PrefabRecord> prefabs;

    bunker::PrefabRecord relayPrefab;
    relayPrefab.id = "prefab_relay_service";
    relayPrefab.label = "Relay Service";
    relayPrefab.targetType = "Structure";
    relayPrefab.sourceLabel = "Smoke prefab usage";
    relayPrefab.completionMode = "Captured";
    relayPrefab.object.registryId = "[%relay_prefab_usage_0001]";
    relayPrefab.object.displayName = "Relay Service Seed";
    relayPrefab.object.interaction = bunker::InteractionType::Terminal;
    relayPrefab.object.category = bunker::ObjectCategory::Terminal;
    relayPrefab.object.scriptTag = "relay_substation";
    relayPrefab.object.linkTarget = "shelter17_backbone";
    relayPrefab.object.editorLayer = "Service";
    prefabs.push_back(relayPrefab);

    if (!Check(bunker::SavePrefabLibrary(prefabs), "prefab usage smoke failed to save prefab library")) {
        return false;
    }

    bunker::World world;
    world.metadata.name = "Prefab Usage Smoke";
    world.metadata.biome = "Industrial Interior";
    world.metadata.objective = "Verify prefab usage tracking and export report.";

    bunker::MapObject relayObject;
    relayObject.registryId = "[%relay_usage_0001]";
    relayObject.displayName = "Relay Usage";
    relayObject.interaction = bunker::InteractionType::Terminal;
    relayObject.category = bunker::ObjectCategory::Terminal;
    relayObject.prefabSourceId = relayPrefab.id;
    relayObject.scriptTag = "relay_substation";
    relayObject.linkTarget = "shelter17_backbone";
    world.AddObject(relayObject);

    bunker::MapObject brokenObject = relayObject;
    brokenObject.registryId = "[%relay_usage_0002]";
    brokenObject.displayName = "Broken Prefab Usage";
    brokenObject.prefabSourceId = "prefab_missing_service";
    world.AddObject(brokenObject);

    const auto usageIndices = bunker::CollectPrefabUsageObjectIndices(world, relayPrefab.id);
    const auto brokenIndices = bunker::CollectBrokenPrefabReferenceObjectIndices(world, prefabs);
    if (!Check(usageIndices.size() == 1, "prefab usage smoke expected one valid prefab-derived object")) {
        return false;
    }
    if (!Check(brokenIndices.size() == 1, "prefab usage smoke expected one broken prefab reference")) {
        return false;
    }
    if (!Check(bunker::CountPrefabUsageInWorld(world, relayPrefab.id) == 1, "prefab usage smoke expected usage count of one")) {
        return false;
    }
    if (!Check(bunker::CountPrefabDerivedObjects(world) == 2, "prefab usage smoke expected two prefab-derived objects")) {
        return false;
    }

    const auto exportResult = bunker::ExportWorldWithValidation(world, bunker::DefaultWorldPath());
    if (!Check(exportResult.ok, "prefab usage smoke expected export to succeed")) {
        return false;
    }

    const std::string report = bunker::LoadTextArtifactPreview(exportResult.validationReportPath, 8000);
    return Check(report.find("Format: BWL5") != std::string::npos, "prefab usage smoke expected export report to mention BWL5 format") &&
        Check(report.find("Prefab-derived objects: 2") != std::string::npos, "prefab usage smoke expected prefab-derived object count in report") &&
        Check(report.find("Broken prefab references: 1") != std::string::npos, "prefab usage smoke expected broken prefab reference count in report");
}

bool RunStrictSemanticExportPolicySmoke() {
    bunker::World world;
    world.metadata.name = "Strict Semantic Export Policy Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0001]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    world.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    if (!Check(batch.createdCount == 7, "strict export policy smoke expected seven auto-created dependency anchors")) {
        return false;
    }

    const auto issues = bunker::ValidateWorldForRuntime(world);
    std::string policyReason;
    const bool strictBlocks = bunker::ExportPolicyBlocksWorld(
        issues,
        bunker::ExportValidationPolicy::BlockAutoCreatedSemanticAnchors,
        policyReason);
    if (!Check(strictBlocks, "strict export policy should block auto-created semantic anchors")) {
        return false;
    }
    if (!Check(policyReason.find("7") != std::string::npos, "strict export policy reason should mention seven auto-created anchors")) {
        return false;
    }
    if (!Check(!bunker::ExportPolicyBlocksWorld(
                issues,
                bunker::ExportValidationPolicy::AllowWarnings,
                policyReason),
            "allow-warnings export policy should not block auto-created semantic anchors")) {
        return false;
    }

    const int adoptedAnchors = bunker::AdoptAllAutoCreatedSemanticAnchors(world, statusText);
    if (!Check(adoptedAnchors == 7, "strict export policy smoke expected adopt-all to convert seven anchors")) {
        return false;
    }

    const auto issuesAfterAdopt = bunker::ValidateWorldForRuntime(world);
    return Check(!bunker::ExportPolicyBlocksWorld(
                issuesAfterAdopt,
                bunker::ExportValidationPolicy::BlockAutoCreatedSemanticAnchors,
                policyReason),
            "strict export policy should clear after adopting auto-created semantic anchors") &&
        Check(bunker::CountValidationIssuesByCode(issuesAfterAdopt, "auto_created_semantic_anchor") == 0,
            "strict export policy smoke expected no auto-created anchor warnings after adopt-all");
}

bool RunValidatedWorldExportArtifactSmoke() {
    bunker::World world;
    world.metadata.name = "Validated World Export Artifact Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0001]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    world.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    if (!Check(batch.createdCount == 7, "validated export artifact smoke expected seven auto-created dependency anchors")) {
        return false;
    }

    const auto prototypeExportPath = bunker::WorldDirectory() / "prototype_export_smoke.bwld";
    const auto prototypeExport = bunker::ExportWorldWithValidation(
        world,
        prototypeExportPath,
        bunker::ExportValidationPolicy::AllowWarnings);
    if (!Check(prototypeExport.ok, "prototype export should allow warning-only auto semantic anchors")) {
        return false;
    }
    if (!Check(std::filesystem::exists(prototypeExport.worldPath), "prototype export should write world file")) {
        return false;
    }
    if (!Check(std::filesystem::exists(prototypeExport.validationReportPath), "prototype export should write validation report")) {
        return false;
    }
    if (!Check(std::filesystem::exists(prototypeExport.auditTrailPath), "prototype export should append export audit trail")) {
        return false;
    }

    std::ifstream prototypeReportFile(prototypeExport.validationReportPath);
    std::string prototypeReport(
        (std::istreambuf_iterator<char>(prototypeReportFile)),
        std::istreambuf_iterator<char>());
    if (!Check(prototypeReport.find("auto_created_semantic_anchor") != std::string::npos,
            "prototype export validation report should list auto-created semantic anchor debt")) {
        return false;
    }
    if (!Check(prototypeReport.find("Policy: prototype / allow warnings") != std::string::npos,
            "prototype export validation report should record prototype policy")) {
        return false;
    }
    if (!Check(prototypeReport.find("Decision: exported") != std::string::npos,
            "prototype export validation report should record exported decision")) {
        return false;
    }

    const auto shippingExportPath = bunker::WorldDirectory() / "shipping_export_smoke.bwld";
    const auto shippingBlockedExport = bunker::ExportWorldWithValidation(
        world,
        shippingExportPath,
        bunker::ExportValidationPolicy::BlockAutoCreatedSemanticAnchors);
    if (!Check(!shippingBlockedExport.ok, "shipping export should block auto-created semantic anchors")) {
        return false;
    }
    if (!Check(!std::filesystem::exists(shippingBlockedExport.worldPath), "blocked shipping export should not write world file")) {
        return false;
    }
    if (!Check(std::filesystem::exists(shippingBlockedExport.validationReportPath), "blocked shipping export should still write validation report")) {
        return false;
    }

    std::ifstream blockedReportFile(shippingBlockedExport.validationReportPath);
    std::string blockedReport(
        (std::istreambuf_iterator<char>(blockedReportFile)),
        std::istreambuf_iterator<char>());
    if (!Check(blockedReport.find("Policy: shipping-safe") != std::string::npos,
            "blocked shipping validation report should record shipping-safe policy")) {
        return false;
    }
    if (!Check(blockedReport.find("Decision: blocked by policy") != std::string::npos,
            "blocked shipping validation report should record policy block")) {
        return false;
    }

    const int adoptedAnchors = bunker::AdoptAllAutoCreatedSemanticAnchors(world, statusText);
    if (!Check(adoptedAnchors == 7, "validated export artifact smoke expected adopt-all to convert seven anchors")) {
        return false;
    }

    const auto shippingPassedExport = bunker::ExportWorldWithValidation(
        world,
        shippingExportPath,
        bunker::ExportValidationPolicy::BlockAutoCreatedSemanticAnchors);
    if (!Check(shippingPassedExport.ok, "shipping export should pass after adopting auto-created semantic anchors")) {
        return false;
    }
    if (!Check(std::filesystem::exists(shippingPassedExport.worldPath), "shipping export should write world file after adopt-all")) {
        return false;
    }

    std::ifstream shippingReportFile(shippingPassedExport.validationReportPath);
    std::string shippingReport(
        (std::istreambuf_iterator<char>(shippingReportFile)),
        std::istreambuf_iterator<char>());
    return Check(shippingReport.find("Auto-created semantic anchors: 0") != std::string::npos,
            "shipping export validation report should show zero remaining auto-created semantic anchors") &&
        Check(shippingReport.find("Decision: exported") != std::string::npos,
            "shipping export validation report should record exported decision") &&
        Check(shippingPassedExport.autoCreatedSemanticAnchorCount == 0,
            "shipping export result should report zero auto-created semantic anchors after adopt-all");
}

bool RunWorldExportAuditTrailSmoke() {
    bunker::World world;
    world.metadata.name = "World Export Audit Trail Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0002]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    world.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    if (!Check(batch.createdCount == 7, "export audit trail smoke expected seven auto-created dependency anchors")) {
        return false;
    }

    const auto exportPath = bunker::WorldDirectory() / "audit_history_smoke.bwld";
    const auto prototypeExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::AllowWarnings);
    if (!Check(prototypeExport.ok, "export audit trail smoke expected prototype export to pass")) {
        return false;
    }

    const auto blockedExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::BlockAutoCreatedSemanticAnchors);
    if (!Check(!blockedExport.ok && blockedExport.blockedByPolicy,
            "export audit trail smoke expected strict export to block by policy")) {
        return false;
    }

    const int adoptedAnchors = bunker::AdoptAllAutoCreatedSemanticAnchors(world, statusText);
    if (!Check(adoptedAnchors == 7, "export audit trail smoke expected adopt-all to convert seven anchors")) {
        return false;
    }

    const auto shippingExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::BlockAutoCreatedSemanticAnchors);
    if (!Check(shippingExport.ok, "export audit trail smoke expected shipping export to pass after adopt-all")) {
        return false;
    }
    if (!Check(std::filesystem::exists(shippingExport.auditTrailPath), "export audit trail smoke expected audit trail file")) {
        return false;
    }

    const std::string auditTrail = bunker::LoadTextArtifactPreview(shippingExport.auditTrailPath, 20000);
    return Check(auditTrail.find("Policy: prototype / allow warnings") != std::string::npos,
            "export audit trail should include prototype export entry") &&
        Check(auditTrail.find("Policy: shipping-safe") != std::string::npos,
            "export audit trail should include shipping export entries") &&
        Check(auditTrail.find("Decision: blocked by policy") != std::string::npos,
            "export audit trail should record blocked shipping export") &&
        Check(auditTrail.find("Auto-created semantic anchors: 0") != std::string::npos,
            "export audit trail should record zero semantic debt after adopt-all") &&
        Check(auditTrail.find("Decision: exported") != std::string::npos,
            "export audit trail should record successful exports");
}

bool RunShippingBaselineDiffSmoke() {
    bunker::World world;
    world.metadata.name = "Shipping Baseline Diff Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0003]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    world.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    if (!Check(batch.createdCount == 7, "shipping baseline diff smoke expected seven auto-created dependency anchors")) {
        return false;
    }

    const int adoptedAnchors = bunker::AdoptAllAutoCreatedSemanticAnchors(world, statusText);
    if (!Check(adoptedAnchors == 7, "shipping baseline diff smoke expected adopt-all to convert seven anchors")) {
        return false;
    }

    const auto exportPath = bunker::WorldDirectory() / "shipping_baseline_diff_smoke.bwld";
    const auto baselineExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::BlockAutoCreatedSemanticAnchors);
    if (!Check(baselineExport.ok, "shipping baseline diff smoke expected baseline export to pass")) {
        return false;
    }
    if (!Check(baselineExport.baselineUpdated, "shipping baseline diff smoke expected shipping export to update baseline snapshot")) {
        return false;
    }
    if (!Check(std::filesystem::exists(baselineExport.baselineSnapshotPath), "shipping baseline diff smoke expected baseline snapshot file")) {
        return false;
    }

    const auto cleanDelta = bunker::CompareValidationToBaseline(bunker::ValidateWorldForRuntime(world), exportPath);
    if (!Check(cleanDelta.hasBaseline, "shipping baseline diff smoke expected baseline snapshot to load")) {
        return false;
    }
    if (!Check(cleanDelta.regressions.empty(), "shipping baseline diff smoke expected no regressions for unchanged world")) {
        return false;
    }

    auto* brokenWaterReclaimer = world.FindObjectByScriptTag("water_reclaimer");
    if (!Check(brokenWaterReclaimer != nullptr, "shipping baseline diff smoke expected water_reclaimer object")) {
        return false;
    }
    brokenWaterReclaimer->linkTarget = "broken_water_target";

    const auto driftIssues = bunker::ValidateWorldForRuntime(world);
    const auto driftDelta = bunker::CompareValidationToBaseline(driftIssues, exportPath);
    const std::string driftReport = bunker::BuildValidationBaselineDeltaReport(driftDelta);
    if (!Check(!driftDelta.regressions.empty(), "shipping baseline diff smoke expected regression after link target drift")) {
        return false;
    }
    if (!Check(!driftDelta.issueRegressions.empty(), "shipping baseline diff smoke expected object-level regression after link target drift")) {
        return false;
    }
    if (!Check(driftReport.find("descriptor_link_target_mismatch") != std::string::npos,
            "shipping baseline diff report should mention descriptor_link_target_mismatch regression")) {
        return false;
    }
    if (!Check(driftReport.find("[%water_0003]") != std::string::npos,
            "shipping baseline diff report should mention the concrete regressed water_reclaimer object")) {
        return false;
    }
    if (!Check(driftReport.find("(+1)") != std::string::npos,
            "shipping baseline diff report should show +1 regression")) {
        return false;
    }
    if (!Check(driftDelta.issueRegressions[0].objectId == "[%water_0003]",
            "shipping baseline diff smoke expected object-level regression to point at water_reclaimer registry")) {
        return false;
    }
    if (!Check(driftDelta.issueRegressions[0].scriptTag == "water_reclaimer",
            "shipping baseline diff smoke expected object-level regression to point at water_reclaimer scriptTag")) {
        return false;
    }

    const int appliedFixes = bunker::AutoFixSafeValidationIssues(world, statusText);
    if (!Check(appliedFixes >= 1, "shipping baseline diff smoke expected safe autofix to repair drift")) {
        return false;
    }

    const auto repairedDelta = bunker::CompareValidationToBaseline(bunker::ValidateWorldForRuntime(world), exportPath);
    return Check(repairedDelta.regressions.empty(), "shipping baseline diff smoke expected regressions to clear after autofix") &&
        Check(repairedDelta.issueRegressions.empty(), "shipping baseline diff smoke expected object-level regressions to clear after autofix") &&
        Check(repairedDelta.improvements.empty(), "shipping baseline diff smoke expected no drift after autofix");
}

bool RunShippingBaselineObjectAwareDriftSmoke() {
    bunker::World world;
    world.metadata.name = "Shipping Baseline Object-Aware Drift Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0004]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    world.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    if (!Check(batch.createdCount == 7, "shipping baseline object-aware smoke expected seven auto-created dependency anchors")) {
        return false;
    }

    const int adoptedAnchors = bunker::AdoptAllAutoCreatedSemanticAnchors(world, statusText);
    if (!Check(adoptedAnchors == 7, "shipping baseline object-aware smoke expected adopt-all to convert seven anchors")) {
        return false;
    }

    auto* serviceBay = world.FindObjectByScriptTag("service_bay");
    auto* currentWaterReclaimer = world.FindObjectByScriptTag("water_reclaimer");
    if (!Check(serviceBay != nullptr, "shipping baseline object-aware smoke expected service_bay")) {
        return false;
    }
    if (!Check(currentWaterReclaimer != nullptr, "shipping baseline object-aware smoke expected water_reclaimer")) {
        return false;
    }

    const std::string serviceBayRegistryId = serviceBay->registryId;
    serviceBay->linkTarget = "service_bay_drift";

    const auto exportPath = bunker::WorldDirectory() / "shipping_baseline_object_drift_smoke.bwld";
    const auto baselineExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::BlockAutoCreatedSemanticAnchors);
    if (!Check(baselineExport.ok, "shipping baseline object-aware smoke expected baseline export to pass")) {
        return false;
    }

    serviceBay->linkTarget = "inner_spur_service";
    currentWaterReclaimer->linkTarget = "water_reclaimer_drift";

    const auto objectAwareDelta = bunker::CompareValidationToBaseline(bunker::ValidateWorldForRuntime(world), exportPath);
    const std::string objectAwareReport = bunker::BuildValidationBaselineDeltaReport(objectAwareDelta);
    if (!Check(objectAwareDelta.hasBaseline, "shipping baseline object-aware smoke expected baseline snapshot to load")) {
        return false;
    }
    if (!Check(objectAwareDelta.regressions.empty() && objectAwareDelta.improvements.empty(),
            "shipping baseline object-aware smoke expected no count-level drift when warning count stays flat")) {
        return false;
    }
    if (!Check(objectAwareDelta.issueRegressions.size() == 1, "shipping baseline object-aware smoke expected one object-level regression")) {
        return false;
    }
    if (!Check(objectAwareDelta.issueImprovements.size() == 1, "shipping baseline object-aware smoke expected one object-level improvement")) {
        return false;
    }
    if (!Check(objectAwareDelta.issueRegressions[0].objectId == "[%water_0004]",
            "shipping baseline object-aware smoke expected water_reclaimer to become the new regressed object")) {
        return false;
    }
    if (!Check(objectAwareDelta.issueImprovements[0].objectId == serviceBayRegistryId,
            "shipping baseline object-aware smoke expected service_bay baseline drift to be marked as improved")) {
        return false;
    }
    return Check(objectAwareReport.find("No issue-count drift against shipping baseline.") != std::string::npos,
            "shipping baseline object-aware report should explicitly mention zero count-level drift") &&
        Check(objectAwareReport.find("[%water_0004]") != std::string::npos,
            "shipping baseline object-aware report should mention regressed water_reclaimer object") &&
        Check(objectAwareReport.find(serviceBayRegistryId) != std::string::npos,
            "shipping baseline object-aware report should mention improved service_bay object");
}

bool RunExportHistoryCheckpointSelectionSmoke() {
    bunker::World world;
    world.metadata.name = "Export History Checkpoint Selection Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0005]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "inner_spur_water";
    world.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    if (!Check(batch.createdCount == 7, "export history selection smoke expected seven auto-created dependency anchors")) {
        return false;
    }

    const int adoptedAnchors = bunker::AdoptAllAutoCreatedSemanticAnchors(world, statusText);
    if (!Check(adoptedAnchors == 7, "export history selection smoke expected adopt-all to convert seven anchors")) {
        return false;
    }

    auto* serviceBay = world.FindObjectByScriptTag("service_bay");
    auto* currentWaterReclaimer = world.FindObjectByScriptTag("water_reclaimer");
    if (!Check(serviceBay != nullptr, "export history selection smoke expected service_bay")) {
        return false;
    }
    if (!Check(currentWaterReclaimer != nullptr, "export history selection smoke expected water_reclaimer")) {
        return false;
    }

    const std::string serviceBayRegistryId = serviceBay->registryId;
    const auto exportPath = bunker::WorldDirectory() / "export_history_selection_smoke.bwld";
    std::error_code cleanupEc;
    std::filesystem::remove(exportPath, cleanupEc);
    std::filesystem::remove(bunker::ValidationReportPathForWorld(exportPath), cleanupEc);
    std::filesystem::remove(bunker::ExportAuditTrailPathForWorld(exportPath), cleanupEc);
    std::filesystem::remove(bunker::ValidationBaselinePathForWorld(exportPath), cleanupEc);
    const std::string snapshotPrefix = exportPath.stem().string() + ".validation-snapshot-";
    for (const auto& entry : std::filesystem::directory_iterator(exportPath.parent_path(), cleanupEc)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto filePath = entry.path();
        const std::string fileName = filePath.filename().string();
        if (fileName.rfind(snapshotPrefix, 0) == 0 && filePath.extension() == ".txt") {
            std::filesystem::remove(filePath, cleanupEc);
        }
    }

    const auto shippingExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::BlockAutoCreatedSemanticAnchors);
    if (!Check(shippingExport.ok, "export history selection smoke expected shipping export to pass")) {
        return false;
    }
    if (!Check(shippingExport.baselineUpdated, "export history selection smoke expected shipping export to update baseline")) {
        return false;
    }
    if (!Check(std::filesystem::exists(shippingExport.validationSnapshotPath),
            "export history selection smoke expected shipping export snapshot archive")) {
        return false;
    }

    serviceBay->linkTarget = "service_bay_history_drift";
    const auto firstPrototypeExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::AllowWarnings);
    if (!Check(firstPrototypeExport.ok, "export history selection smoke expected first prototype export to pass")) {
        return false;
    }
    if (!Check(std::filesystem::exists(firstPrototypeExport.validationSnapshotPath),
            "export history selection smoke expected first export snapshot archive")) {
        return false;
    }

    serviceBay->linkTarget = "inner_spur_service";
    currentWaterReclaimer->linkTarget = "water_reclaimer_history_drift";
    const auto secondPrototypeExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::AllowWarnings);
    if (!Check(secondPrototypeExport.ok, "export history selection smoke expected second prototype export to pass")) {
        return false;
    }
    if (!Check(std::filesystem::exists(secondPrototypeExport.validationSnapshotPath),
            "export history selection smoke expected second export snapshot archive")) {
        return false;
    }

    currentWaterReclaimer->linkTarget = "[%missing_history_target]";
    const auto blockedExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::AllowWarnings);
    if (!Check(!blockedExport.ok && blockedExport.blockedByValidation,
            "export history selection smoke expected blocked export to fail validation")) {
        return false;
    }
    if (!Check(std::filesystem::exists(blockedExport.validationSnapshotPath),
            "export history selection smoke expected blocked export snapshot archive")) {
        return false;
    }

    currentWaterReclaimer->linkTarget = "water_reclaimer_history_drift";

    std::vector<bunker::WorldExportHistoryEntry> historyEntries;
    if (!Check(bunker::LoadWorldExportHistory(exportPath, historyEntries),
            "export history selection smoke expected audit history to load")) {
        return false;
    }
    if (!Check(historyEntries.size() >= 4, "export history selection smoke expected at least four history entries")) {
        return false;
    }
    if (!Check(historyEntries[0].validationSnapshotPath == blockedExport.validationSnapshotPath,
            "export history selection smoke expected newest history entry to match blocked export snapshot")) {
        return false;
    }
    if (!Check(historyEntries[1].validationSnapshotPath == secondPrototypeExport.validationSnapshotPath,
            "export history selection smoke expected second history entry to match second prototype export snapshot")) {
        return false;
    }
    if (!Check(historyEntries[2].validationSnapshotPath == firstPrototypeExport.validationSnapshotPath,
            "export history selection smoke expected third history entry to match first prototype export snapshot")) {
        return false;
    }
    if (!Check(historyEntries[3].validationSnapshotPath == shippingExport.validationSnapshotPath,
            "export history selection smoke expected shipping snapshot to remain in history")) {
        return false;
    }

    bunker::WorldExportHistoryQuery latestShippingQuery;
    latestShippingQuery.filter = bunker::WorldExportHistoryFilter::ShippingOnly;
    latestShippingQuery.requireSuccessful = true;
    latestShippingQuery.requireValidationSnapshot = true;
    const auto latestShippingSelection = bunker::FindLatestMatchingHistoryEntry(historyEntries, latestShippingQuery);
    if (!Check(latestShippingSelection.found && latestShippingSelection.historyIndex == 3,
            "export history selection smoke expected latest successful shipping selection to resolve shipping checkpoint")) {
        return false;
    }

    bunker::WorldExportHistoryQuery latestPrototypeQuery;
    latestPrototypeQuery.filter = bunker::WorldExportHistoryFilter::PrototypeOnly;
    latestPrototypeQuery.requireSuccessful = true;
    latestPrototypeQuery.requireValidationSnapshot = true;
    const auto latestPrototypeSelection = bunker::FindLatestMatchingHistoryEntry(historyEntries, latestPrototypeQuery);
    if (!Check(latestPrototypeSelection.found && latestPrototypeSelection.historyIndex == 1,
            "export history selection smoke expected latest successful prototype selection to resolve newest successful prototype")) {
        return false;
    }

    bunker::WorldExportHistoryQuery latestBlockedQuery;
    latestBlockedQuery.filter = bunker::WorldExportHistoryFilter::Blocked;
    latestBlockedQuery.requireValidationSnapshot = true;
    const auto latestBlockedSelection = bunker::FindLatestMatchingHistoryEntry(historyEntries, latestBlockedQuery);
    if (!Check(latestBlockedSelection.found && latestBlockedSelection.historyIndex == 0,
            "export history selection smoke expected latest blocked selection to resolve blocked checkpoint")) {
        return false;
    }

    bunker::WorldExportHistoryQuery baselineUpdatedQuery;
    baselineUpdatedQuery.filter = bunker::WorldExportHistoryFilter::BaselineUpdatedOnly;
    baselineUpdatedQuery.requireValidationSnapshot = true;
    const auto baselineUpdatedSelections =
        bunker::FilterWorldExportHistoryEntries(historyEntries, baselineUpdatedQuery);
    if (!Check(baselineUpdatedSelections.size() == 1,
            "export history selection smoke expected baseline-updated filter to return one checkpoint")) {
        return false;
    }
    if (!Check(baselineUpdatedSelections[0].historyIndex == 3,
            "export history selection smoke expected baseline-updated filter to resolve shipping checkpoint")) {
        return false;
    }

    bunker::WorldExportHistoryQuery saveFailedQuery;
    saveFailedQuery.filter = bunker::WorldExportHistoryFilter::SaveFailed;
    saveFailedQuery.requireValidationSnapshot = true;
    const auto noMatchSelection = bunker::FindLatestMatchingHistoryEntry(historyEntries, saveFailedQuery);
    if (!Check(!noMatchSelection.found && !noMatchSelection.fallbackMessage.empty(),
            "export history selection smoke expected no-match fallback for missing save-failed checkpoint")) {
        return false;
    }

    const auto shippingPresetSelection = bunker::ResolveComparePresetTarget(
        historyEntries,
        bunker::WorldExportComparePreset::LatestSuccessfulShipping,
        0);
    const auto prototypePresetSelection = bunker::ResolveComparePresetTarget(
        historyEntries,
        bunker::WorldExportComparePreset::LatestSuccessfulPrototype,
        0);
    const auto blockedPresetSelection = bunker::ResolveComparePresetTarget(
        historyEntries,
        bunker::WorldExportComparePreset::LatestBlocked,
        0);
    const auto baselinePresetSelection = bunker::ResolveComparePresetTarget(
        historyEntries,
        bunker::WorldExportComparePreset::LatestBaselineUpdated,
        0);
    const auto manualPresetSelection = bunker::ResolveComparePresetTarget(
        historyEntries,
        bunker::WorldExportComparePreset::ManualSelection,
        2);
    if (!Check(shippingPresetSelection.found && shippingPresetSelection.historyIndex == latestShippingSelection.historyIndex,
            "export history selection smoke expected shipping preset resolution to match latest shipping selection")) {
        return false;
    }
    if (!Check(prototypePresetSelection.found && prototypePresetSelection.historyIndex == latestPrototypeSelection.historyIndex,
            "export history selection smoke expected prototype preset resolution to match latest prototype selection")) {
        return false;
    }
    if (!Check(blockedPresetSelection.found && blockedPresetSelection.historyIndex == latestBlockedSelection.historyIndex,
            "export history selection smoke expected blocked preset resolution to match latest blocked selection")) {
        return false;
    }
    if (!Check(baselinePresetSelection.found && baselinePresetSelection.historyIndex == baselineUpdatedSelections[0].historyIndex,
            "export history selection smoke expected baseline preset resolution to match baseline-updated selection")) {
        return false;
    }
    if (!Check(manualPresetSelection.found && manualPresetSelection.historyIndex == 2,
            "export history selection smoke expected manual preset resolution to preserve manual checkpoint")) {
        return false;
    }

    const auto currentIssues = bunker::ValidateWorldForRuntime(world);
    const auto latestPrototypeDelta =
        bunker::CompareValidationToSnapshot(currentIssues, historyEntries[1].validationSnapshotPath);
    if (!Check(latestPrototypeDelta.hasBaseline, "export history selection smoke expected latest prototype snapshot to load")) {
        return false;
    }
    if (!Check(latestPrototypeDelta.issueRegressions.empty() && latestPrototypeDelta.issueImprovements.empty(),
            "export history selection smoke expected current world to match latest successful prototype checkpoint")) {
        return false;
    }

    const auto olderDelta = bunker::CompareValidationToSnapshot(currentIssues, historyEntries[2].validationSnapshotPath);
    const std::string olderReport =
        bunker::BuildValidationSnapshotDeltaReport(olderDelta, "Historical export checkpoint");
    if (!Check(olderDelta.hasBaseline, "export history selection smoke expected older prototype snapshot to load")) {
        return false;
    }
    if (!Check(olderDelta.regressions.empty() && olderDelta.improvements.empty(),
            "export history selection smoke expected same warning count versus older prototype checkpoint")) {
        return false;
    }
    if (!Check(olderDelta.issueRegressions.size() == 1, "export history selection smoke expected one historical regression")) {
        return false;
    }
    if (!Check(olderDelta.issueImprovements.size() == 1, "export history selection smoke expected one historical improvement")) {
        return false;
    }
    const auto blockedDelta =
        bunker::CompareValidationToSnapshot(currentIssues, historyEntries[0].validationSnapshotPath);
    if (!Check(blockedDelta.hasBaseline, "export history selection smoke expected blocked snapshot to load")) {
        return false;
    }
    return Check(olderDelta.issueRegressions[0].objectId == "[%water_0005]",
            "export history selection smoke expected older checkpoint compare to point at water_reclaimer regression") &&
        Check(olderDelta.issueImprovements[0].objectId == serviceBayRegistryId,
            "export history selection smoke expected older checkpoint compare to point at service_bay improvement") &&
        Check(olderReport.find("Historical export checkpoint diff") != std::string::npos,
            "export history selection smoke expected labeled historical diff report") &&
        Check(olderReport.find("[%water_0005]") != std::string::npos,
            "export history selection smoke expected report to mention regressed water_reclaimer object") &&
        Check(olderReport.find(serviceBayRegistryId) != std::string::npos,
            "export history selection smoke expected report to mention improved service_bay object") &&
        Check(historyEntries[0].policyLabel == "prototype / allow warnings",
            "export history selection smoke expected parsed policy label in blocked audit history") &&
        Check(historyEntries[0].decisionLabel == "blocked by validation",
            "export history selection smoke expected parsed blocked decision label in audit history") &&
        Check(historyEntries[3].policyLabel == "shipping-safe",
            "export history selection smoke expected parsed shipping policy label in audit history") &&
        Check(historyEntries[3].baselineUpdated,
            "export history selection smoke expected shipping audit entry to carry baseline update metadata") &&
        Check(blockedDelta.issueImprovements.size() == 1,
            "export history selection smoke expected blocked checkpoint compare to show one repaired issue") &&
        Check(blockedDelta.issueImprovements[0].code == "broken_link_target",
            "export history selection smoke expected blocked checkpoint compare to identify repaired broken-link issue") &&
        Check(noMatchSelection.fallbackMessage.find("save-failed export") != std::string::npos,
            "export history selection smoke expected no-match fallback to mention save-failed export") &&
        Check(shippingPresetSelection.summaryLabel.find("[shipping]") != std::string::npos,
            "export history selection smoke expected compact history summary to carry shipping badge");
}

bool RunLegacySemanticAutoInferenceSmoke() {
    std::ofstream file(bunker::DefaultWorldPath(), std::ios::binary);
    if (!Check(file.is_open(), "legacy semantic inference smoke failed to open world file for write")) {
        return false;
    }

    file.write("BWL2", 4);
    WriteRawString(file, "Legacy Semantic Auto Inference");
    WriteRawString(file, "Bunker Interior");
    WriteRawString(file, "Load legacy BWL2 auto semantic anchors.");
    const float spawnX = 0.0f;
    const float spawnY = 0.0f;
    file.write(reinterpret_cast<const char*>(&spawnX), sizeof(spawnX));
    file.write(reinterpret_cast<const char*>(&spawnY), sizeof(spawnY));

    const std::uint32_t count = 1;
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    WriteRawString(file, "[%service_bay_auto_0001]");
    WriteRawString(file, "Legacy Auto Service Bay");
    WriteRawString(file, "service_bay");
    WriteRawString(file, "inner_spur_service");

    const std::uint32_t interaction = static_cast<std::uint32_t>(bunker::InteractionType::Terminal);
    const std::uint32_t category = static_cast<std::uint32_t>(bunker::ObjectCategory::Terminal);
    const float x = 4.0f;
    const float y = 5.0f;
    const float z = 0.0f;
    const float width = 3.3f;
    const float depth = 2.8f;
    const float height = 3.0f;
    const float health = 100.0f;
    const bool blocksMovement = false;
    const bool discovered = true;
    const bool manualLoot = false;

    file.write(reinterpret_cast<const char*>(&interaction), sizeof(interaction));
    file.write(reinterpret_cast<const char*>(&category), sizeof(category));
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
    file.write(reinterpret_cast<const char*>(&y), sizeof(y));
    file.write(reinterpret_cast<const char*>(&z), sizeof(z));
    file.write(reinterpret_cast<const char*>(&width), sizeof(width));
    file.write(reinterpret_cast<const char*>(&depth), sizeof(depth));
    file.write(reinterpret_cast<const char*>(&height), sizeof(height));
    file.write(reinterpret_cast<const char*>(&health), sizeof(health));
    file.write(reinterpret_cast<const char*>(&blocksMovement), sizeof(blocksMovement));
    file.write(reinterpret_cast<const char*>(&discovered), sizeof(discovered));
    file.write(reinterpret_cast<const char*>(&manualLoot), sizeof(manualLoot));
    WriteRawString(file, "");
    WriteRawString(file, "");
    WriteRawString(file, "");
    WriteRawString(file, "");
    file.close();

    bunker::World loadedWorld;
    if (!Check(loadedWorld.Load(bunker::DefaultWorldPath().string()), "legacy semantic inference world load failed")) {
        return false;
    }

    const auto* serviceBay = loadedWorld.FindObjectByScriptTag("service_bay");
    return Check(serviceBay != nullptr, "legacy semantic inference should load service_bay") &&
        Check(bunker::IsAutoGeneratedSemanticAnchor(*serviceBay), "legacy BWL2 auto anchor should infer semanticAutoCreated from registryId") &&
        Check(!bunker::IsPinnedSemanticAnchor(*serviceBay), "legacy BWL2 auto anchor should not infer pinned semantic placement");
}

bool RunLegacyWorldEditorLayerInferenceSmoke() {
    std::ofstream file(bunker::DefaultWorldPath(), std::ios::binary);
    if (!Check(file.is_open(), "legacy editor-layer inference smoke failed to open world file for write")) {
        return false;
    }

    file.write("BWL3", 4);
    WriteRawString(file, "Legacy Layer Inference");
    WriteRawString(file, "Bunker Interior");
    WriteRawString(file, "Load BWL3 worlds without explicit editor layers.");
    const float spawnX = 1.0f;
    const float spawnY = -2.0f;
    file.write(reinterpret_cast<const char*>(&spawnX), sizeof(spawnX));
    file.write(reinterpret_cast<const char*>(&spawnY), sizeof(spawnY));

    const std::uint32_t count = 1;
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    WriteRawString(file, "[%services_0001]");
    WriteRawString(file, "Lanline Services Relay");
    WriteRawString(file, "lanline_service_hub");
    WriteRawString(file, "shelter17_services");

    const std::uint32_t interaction = static_cast<std::uint32_t>(bunker::InteractionType::Terminal);
    const std::uint32_t category = static_cast<std::uint32_t>(bunker::ObjectCategory::Terminal);
    const float x = 6.0f;
    const float y = 4.0f;
    const float z = 0.0f;
    const float width = 2.2f;
    const float depth = 1.8f;
    const float height = 2.4f;
    const float health = 100.0f;
    const bool blocksMovement = false;
    const bool discovered = true;
    const bool manualLoot = false;
    const bool semanticAutoCreated = false;
    const bool semanticLayoutPinned = false;

    file.write(reinterpret_cast<const char*>(&interaction), sizeof(interaction));
    file.write(reinterpret_cast<const char*>(&category), sizeof(category));
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
    file.write(reinterpret_cast<const char*>(&y), sizeof(y));
    file.write(reinterpret_cast<const char*>(&z), sizeof(z));
    file.write(reinterpret_cast<const char*>(&width), sizeof(width));
    file.write(reinterpret_cast<const char*>(&depth), sizeof(depth));
    file.write(reinterpret_cast<const char*>(&height), sizeof(height));
    file.write(reinterpret_cast<const char*>(&health), sizeof(health));
    file.write(reinterpret_cast<const char*>(&blocksMovement), sizeof(blocksMovement));
    file.write(reinterpret_cast<const char*>(&discovered), sizeof(discovered));
    file.write(reinterpret_cast<const char*>(&manualLoot), sizeof(manualLoot));
    file.write(reinterpret_cast<const char*>(&semanticAutoCreated), sizeof(semanticAutoCreated));
    file.write(reinterpret_cast<const char*>(&semanticLayoutPinned), sizeof(semanticLayoutPinned));
    WriteRawString(file, "");
    WriteRawString(file, "");
    WriteRawString(file, "");
    WriteRawString(file, "");
    file.close();

    bunker::World loadedWorld;
    if (!Check(loadedWorld.Load(bunker::DefaultWorldPath().string()), "legacy editor-layer inference world load failed")) {
        return false;
    }

    const auto* servicesHub = loadedWorld.FindObjectByScriptTag("lanline_service_hub");
    const auto collectedLayers = loadedWorld.CollectEditorLayerNames();
    return Check(servicesHub != nullptr, "legacy editor-layer inference should load service hub") &&
        Check(servicesHub->editorLayer == "Service", "legacy BWL3 object should infer Service editor layer from scriptTag") &&
        Check(loadedWorld.CountObjectsInEditorLayer("Service") == 1, "legacy BWL3 object should count inside inferred Service layer") &&
        Check(std::find(collectedLayers.begin(), collectedLayers.end(), "Service") != collectedLayers.end(),
            "legacy BWL3 object should surface inferred Service layer in layer list");
}

bool RunWorldEditorUndoSmoke() {
    bunker::World world;
    world.metadata.name = "Undo Smoke";
    world.metadata.objective = "Verify editor undo stack.";

    bunker::MapObject rootObject;
    rootObject.registryId = "[%root_0001]";
    rootObject.displayName = "Root Anchor";
    rootObject.interaction = bunker::InteractionType::Terminal;
    rootObject.category = bunker::ObjectCategory::Terminal;
    world.AddObject(rootObject);

    bunker::WorldEditorUndoStack undoStack;
    undoStack.MarkSaved();
    if (!Check(!undoStack.IsDirty(), "undo smoke should start clean after mark-saved")) {
        return false;
    }

    bunker::MapObject addedObject;
    addedObject.registryId = "[%service_0001]";
    addedObject.displayName = "Service Relay";
    addedObject.interaction = bunker::InteractionType::Terminal;
    addedObject.category = bunker::ObjectCategory::Terminal;
    addedObject.scriptTag = "lanline_service_hub";
    addedObject.linkTarget = "shelter17_services";
    world.AddObject(addedObject);
    undoStack.PushObjectAdded("Add service relay", world.objects.back(), 1, addedObject.registryId);

    if (!Check(undoStack.CanUndo(), "undo smoke expected add-object action to be undoable")) {
        return false;
    }
    const auto addUndo = undoStack.Undo(world);
    if (!Check(addUndo.changed, "undo smoke expected add-object undo to change world")) {
        return false;
    }
    if (!Check(!world.HasObject(addedObject.registryId), "undo smoke expected add-object undo to remove added object")) {
        return false;
    }
    const auto addRedo = undoStack.Redo(world);
    if (!Check(addRedo.changed, "undo smoke expected add-object redo to change world")) {
        return false;
    }
    if (!Check(world.HasObject(addedObject.registryId), "undo smoke expected redo to restore added object")) {
        return false;
    }

    const bunker::MapObject beforeFirstUpdate = world.objects[1];
    world.objects[1].displayName = "Service Relay Updated";
    undoStack.PushObjectUpdated("Edit service relay", beforeFirstUpdate, world.objects[1], 1);
    const bunker::MapObject beforeSecondUpdate = world.objects[1];
    world.objects[1].x = 14.0f;
    undoStack.PushObjectUpdated("Edit service relay", beforeSecondUpdate, world.objects[1], 1);

    if (!Check(undoStack.UndoCount() == 2, "undo smoke expected update coalescing to keep a single object-edit record")) {
        return false;
    }
    const auto updateUndo = undoStack.Undo(world);
    if (!Check(updateUndo.changed, "undo smoke expected object-edit undo to apply")) {
        return false;
    }
    if (!Check(world.objects[1].displayName == beforeFirstUpdate.displayName &&
            world.objects[1].x == beforeFirstUpdate.x,
            "undo smoke expected coalesced object-edit undo to restore original object")) {
        return false;
    }
    const auto updateRedo = undoStack.Redo(world);
    if (!Check(updateRedo.changed, "undo smoke expected object-edit redo to apply")) {
        return false;
    }
    if (!Check(world.objects[1].displayName == "Service Relay Updated" && world.objects[1].x == 14.0f,
            "undo smoke expected coalesced object-edit redo to restore latest object state")) {
        return false;
    }

    const bunker::WorldMetadata metadataBefore = world.metadata;
    world.metadata.objective = "Updated objective";
    undoStack.PushWorldMetadataUpdated("Update world objective", metadataBefore, world.metadata);
    const bunker::WorldMetadata metadataMid = world.metadata;
    world.metadata.playerSpawnX = 9.0f;
    undoStack.PushWorldMetadataUpdated("Update world objective", metadataMid, world.metadata);

    if (!Check(undoStack.UndoCount() == 3, "undo smoke expected metadata updates to coalesce into one record")) {
        return false;
    }
    const auto metadataUndo = undoStack.Undo(world);
    if (!Check(metadataUndo.changed, "undo smoke expected metadata undo to apply")) {
        return false;
    }
    if (!Check(world.metadata.objective == metadataBefore.objective &&
            world.metadata.playerSpawnX == metadataBefore.playerSpawnX,
            "undo smoke expected metadata undo to restore original metadata")) {
        return false;
    }
    const auto metadataRedo = undoStack.Redo(world);
    if (!Check(metadataRedo.changed, "undo smoke expected metadata redo to apply")) {
        return false;
    }
    if (!Check(world.metadata.objective == "Updated objective" && world.metadata.playerSpawnX == 9.0f,
            "undo smoke expected metadata redo to restore latest metadata state")) {
        return false;
    }

    const bunker::MapObject removedObject = world.objects[1];
    world.RemoveObject(removedObject.registryId);
    undoStack.PushObjectRemoved("Delete service relay", removedObject, 1, removedObject.registryId, rootObject.registryId);
    const auto removeUndo = undoStack.Undo(world);
    if (!Check(removeUndo.changed && world.HasObject(removedObject.registryId),
            "undo smoke expected removed object to return after undo")) {
        return false;
    }
    const auto removeRedo = undoStack.Redo(world);
    if (!Check(removeRedo.changed && !world.HasObject(removedObject.registryId),
            "undo smoke expected redo to remove object again")) {
        return false;
    }

    const bunker::World beforeBatchWorld = world;
    bunker::MapObject batchObject;
    batchObject.registryId = "[%batch_0001]";
    batchObject.displayName = "Batch Anchor";
    batchObject.interaction = bunker::InteractionType::Transition;
    batchObject.category = bunker::ObjectCategory::Landmark;
    world.AddObject(batchObject);
    world.metadata.biome = "Industrial Test";
    undoStack.PushBatchWorldEdit("Batch semantic action", beforeBatchWorld, world, rootObject.registryId, batchObject.registryId);

    const auto batchUndo = undoStack.Undo(world);
    if (!Check(batchUndo.changed && !world.HasObject(batchObject.registryId) && world.metadata.biome == beforeBatchWorld.metadata.biome,
            "undo smoke expected batch undo to restore pre-batch world")) {
        return false;
    }
    const auto batchRedo = undoStack.Redo(world);
    if (!Check(batchRedo.changed && world.HasObject(batchObject.registryId) && world.metadata.biome == "Industrial Test",
            "undo smoke expected batch redo to restore post-batch world")) {
        return false;
    }

    undoStack.MarkSaved();
    return Check(!undoStack.IsDirty(), "undo smoke expected mark-saved to clear dirty state") &&
        Check(undoStack.PeekUndo() != nullptr, "undo smoke expected undo stack to retain history after mark-saved");
}

bool RunWorldReferenceGraphSmoke() {
    bunker::World world;
    world.metadata.name = "World Reference Graph Smoke";

    bunker::MapObject target;
    target.registryId = "[%target_0001]";
    target.displayName = "Target Anchor";
    target.interaction = bunker::InteractionType::Terminal;
    target.category = bunker::ObjectCategory::Terminal;
    world.objects.push_back(target);

    bunker::MapObject source;
    source.registryId = "[%source_0001]";
    source.displayName = "Source Anchor";
    source.interaction = bunker::InteractionType::Terminal;
    source.category = bunker::ObjectCategory::Terminal;
    source.scriptTag = "remote_link";
    source.linkTarget = target.registryId;
    world.objects.push_back(source);

    bunker::MapObject brokenSource;
    brokenSource.registryId = "[%broken_source_0001]";
    brokenSource.displayName = "Broken Source";
    brokenSource.interaction = bunker::InteractionType::Transition;
    brokenSource.category = bunker::ObjectCategory::Landmark;
    brokenSource.linkTarget = "[%missing_target_0001]";
    world.objects.push_back(brokenSource);

    const auto references = world.BuildObjectReferences();
    if (!Check(references.size() == 2, "world reference graph smoke expected two registry-style references")) {
        return false;
    }

    const auto incomingReferences = world.FindIncomingObjectReferences(target.registryId);
    if (!Check(incomingReferences.size() == 1, "world reference graph smoke expected one incoming reference")) {
        return false;
    }
    if (!Check(incomingReferences[0].sourceObjectId == source.registryId,
            "world reference graph smoke expected incoming reference to come from source anchor")) {
        return false;
    }
    if (!Check(incomingReferences[0].resolved && incomingReferences[0].targetObjectIndex == 0,
            "world reference graph smoke expected incoming reference to resolve to target object")) {
        return false;
    }

    const auto outgoingResolvedReferences = world.FindOutgoingObjectReferences(source.registryId);
    if (!Check(outgoingResolvedReferences.size() == 1,
            "world reference graph smoke expected one outgoing reference for resolved source")) {
        return false;
    }
    if (!Check(outgoingResolvedReferences[0].resolved &&
            outgoingResolvedReferences[0].targetObjectId == target.registryId,
            "world reference graph smoke expected resolved outgoing reference to target anchor")) {
        return false;
    }

    const auto outgoingBrokenReferences = world.FindOutgoingObjectReferences(brokenSource.registryId);
    if (!Check(outgoingBrokenReferences.size() == 1,
            "world reference graph smoke expected one outgoing reference for broken source")) {
        return false;
    }
    if (!Check(!outgoingBrokenReferences[0].resolved &&
            outgoingBrokenReferences[0].targetObjectIndex < 0,
            "world reference graph smoke expected broken outgoing reference to remain unresolved")) {
        return false;
    }

    const auto* resolvedTarget = world.FindObjectByRegistryId(target.registryId);
    if (!Check(resolvedTarget != nullptr && resolvedTarget->displayName == "Target Anchor",
            "world reference graph smoke expected find-by-registry to resolve target")) {
        return false;
    }

    return Check(world.HasIncomingObjectReferences(target.registryId),
            "world reference graph smoke expected incoming reference flag on target") &&
        Check(!world.HasIncomingObjectReferences(source.registryId),
            "world reference graph smoke expected no incoming reference flag on source") &&
        Check(std::string(bunker::WorldObjectReferenceFieldLabel(references[0].field)) == "linkTarget",
            "world reference graph smoke expected linkTarget field label");
}

bool RunSemanticAuthoringCascadeSmoke() {
    bunker::World world;
    world.metadata.name = "Semantic Authoring Cascade Smoke";

    bunker::MapObject waterReclaimer;
    waterReclaimer.registryId = "[%water_0001]";
    waterReclaimer.displayName = "Water Reclaimer";
    waterReclaimer.interaction = bunker::InteractionType::Terminal;
    waterReclaimer.category = bunker::ObjectCategory::Terminal;
    waterReclaimer.scriptTag = "water_reclaimer";
    waterReclaimer.linkTarget = "wrong_water_target";
    world.objects.push_back(waterReclaimer);

    std::string statusText;
    const auto batch = bunker::CreateMissingDependencyAnchorsCascade(world, statusText);
    auto batchContainsScriptTag = [&](std::string_view scriptTag) {
        return std::find(batch.createdScriptTags.begin(), batch.createdScriptTags.end(), scriptTag) !=
            batch.createdScriptTags.end();
    };

    if (!Check(batch.createdCount == 7, "expected seven created dependency anchors in cascade")) {
        return false;
    }
    if (!Check(batch.lastCreatedObjectIndex >= 0 &&
            batch.lastCreatedObjectIndex < static_cast<int>(world.objects.size()),
            "expected valid last created anchor index")) {
        return false;
    }
    if (!Check(batch.createdObjectIndices.size() == static_cast<std::size_t>(batch.createdCount),
            "created object index list should match created count")) {
        return false;
    }

    const char* expectedCreatedTags[] = {
        "service_bay",
        "relay_substation",
        "recovery_fabricator",
        "foundry_line",
        "industrial_outpost",
        "capacitor_bank",
        "reactor_yard"
    };
    for (const char* scriptTag : expectedCreatedTags) {
        if (!Check(batchContainsScriptTag(scriptTag),
                std::string("cascade result missing created scriptTag: ") + scriptTag)) {
            return false;
        }
        const auto* object = world.FindObjectByScriptTag(scriptTag);
        if (!Check(object != nullptr, std::string("world missing cascaded anchor: ") + scriptTag)) {
            return false;
        }
        const char* expectedLinkTarget = bunker::DefaultGameplayDescriptorLinkTarget(scriptTag);
        if (!Check(expectedLinkTarget != nullptr,
                std::string("missing canonical link target for cascaded anchor: ") + scriptTag)) {
            return false;
        }
        if (!Check(object->linkTarget == expectedLinkTarget,
                std::string("cascaded anchor link target mismatch for ") + scriptTag)) {
            return false;
        }
        if (!Check(bunker::IsAutoGeneratedSemanticAnchor(*object),
                std::string("cascaded anchor should persist as auto-created before adopt: ") + scriptTag)) {
            return false;
        }
        if (!Check(!bunker::IsPinnedSemanticAnchor(*object),
                std::string("cascaded anchor should not start pinned: ") + scriptTag)) {
            return false;
        }
    }

    const int appliedFixes = bunker::AutoFixSafeValidationIssues(world, statusText);
    if (!Check(appliedFixes == 1, "expected one safe fix after cascade for water_reclaimer link target drift")) {
        return false;
    }

    const auto issuesAfterSafeFix = bunker::ValidateWorldForRuntime(world);
    int autoCreatedWarnings = 0;
    for (const auto& issue : issuesAfterSafeFix) {
        if (issue.code == "auto_created_semantic_anchor") {
            ++autoCreatedWarnings;
        }
    }
    if (!Check(autoCreatedWarnings == 7, "expected seven auto-created semantic anchor warnings after cascade safe-fix pass")) {
        return false;
    }

    const int adoptedAnchors = bunker::AdoptAllAutoCreatedSemanticAnchors(world, statusText);
    if (!Check(adoptedAnchors == 7, "expected adopt-all to convert seven auto-created semantic anchors after cascade")) {
        return false;
    }

    const auto remainingIssues = bunker::ValidateWorldForRuntime(world);
    const auto* repairedWaterReclaimer = world.FindObjectByScriptTag("water_reclaimer");
    return Check(repairedWaterReclaimer != nullptr, "water_reclaimer missing after cascade") &&
        Check(repairedWaterReclaimer->linkTarget == "inner_spur_water", "water_reclaimer canonical link target was not restored") &&
        Check(bunker::CountValidationErrors(remainingIssues) == 0, "expected zero validation errors after cascade autofix") &&
        Check(bunker::CountValidationWarnings(remainingIssues) == 0, "expected zero validation warnings after cascade autofix");
}

bool RunStarterSemanticLinkTargetSmoke() {
    bunker::World world;
    world.GeneratePrototypeZone();
    world.EnsureStarterInfrastructure();

    const char* expectedTags[] = {
        "rail_depot",
        "orbital_uplink",
        "rail_fortress_hub",
        "recovery_fabricator",
        "industrial_gate",
        "industrial_survey",
        "industrial_outpost",
        "assembly_cell",
        "foundry_line",
        "reactor_yard",
        "capacitor_bank",
        "relay_substation",
        "service_bay",
        "water_reclaimer",
        "lanline_service_hub",
        "fey_ring",
        "medical_support",
        "tank_service",
        "echo_trace",
        "specialist_cryo"
    };

    for (const char* scriptTag : expectedTags) {
        const auto* object = world.FindObjectByScriptTag(scriptTag);
        if (!Check(object != nullptr, std::string("starter semantic missing: ") + scriptTag)) {
            return false;
        }
        const char* expectedLinkTarget = bunker::DefaultGameplayDescriptorLinkTarget(scriptTag);
        if (!Check(expectedLinkTarget != nullptr, std::string("missing canonical default link target for: ") + scriptTag)) {
            return false;
        }
        if (!Check(object->linkTarget == expectedLinkTarget,
                std::string("starter semantic link target mismatch for ") + scriptTag +
                ": expected " + expectedLinkTarget + ", got " + object->linkTarget)) {
            return false;
        }
    }

    return true;
}

bool RunLegacyWorldAliasMigrationSmoke() {
    bunker::World savedWorld;
    savedWorld.metadata.name = "Legacy Alias World";

    bunker::MapObject tower;
    tower.registryId = "[%tower_legacy_0001]";
    tower.displayName = "Legacy Tower";
    tower.interaction = bunker::InteractionType::Terminal;
    tower.category = bunker::ObjectCategory::Terminal;
    tower.scriptTag = "radio_tower";
    savedWorld.objects.push_back(tower);

    if (!Check(savedWorld.Save(bunker::DefaultWorldPath().string()), "legacy world save failed")) {
        return false;
    }

    bunker::World loadedWorld;
    if (!Check(loadedWorld.Load(bunker::DefaultWorldPath().string()), "legacy world load failed")) {
        return false;
    }

    const auto* normalizedObject = loadedWorld.FindObjectByScriptTag("tower_sync");
    return Check(normalizedObject != nullptr, "legacy alias world did not normalize to tower_sync") &&
        Check(normalizedObject->scriptTag == "tower_sync", "loaded legacy object tag was not canonicalized");
}

}  // namespace

int main() {
    const fs::path sandboxRoot = fs::temp_directory_path() / "bunker_smoke_checks";
    std::error_code ec;
    fs::remove_all(sandboxRoot, ec);
    fs::create_directories(sandboxRoot, ec);
    if (ec) {
        std::cerr << "[smoke] failed to create sandbox: " << ec.message() << '\n';
        return EXIT_FAILURE;
    }

    WorkingDirectoryGuard guard(sandboxRoot);
    bunker::EnsureProjectDirectories();

    const bool ok = RunWorldRoundtrip() &&
        RunProfileRoundtrip() &&
        RunLauncherAnnouncementSmoke() &&
        RunLanlineServicesRoundtripSmoke() &&
        RunTankServiceKitSmoke() &&
        RunLaunchTicketFlow() &&
        RunGameplayDescriptorValidationSmoke() &&
        RunSemanticDependencyValidationSmoke() &&
        RunSemanticDependencyGraphSmoke() &&
        RunSemanticLayoutSmoke() &&
        RunSemanticLayoutPreserveManualSmoke() &&
        RunSemanticAuthoringStateRoundtripSmoke() &&
        RunSemanticAutoAnchorValidationSmoke() &&
        RunPrefabLibrarySemanticStateSmoke() &&
        RunPrefabUsageAndExportReportSmoke() &&
        RunStrictSemanticExportPolicySmoke() &&
        RunValidatedWorldExportArtifactSmoke() &&
        RunWorldExportAuditTrailSmoke() &&
        RunShippingBaselineDiffSmoke() &&
        RunShippingBaselineObjectAwareDriftSmoke() &&
        RunExportHistoryCheckpointSelectionSmoke() &&
        RunLegacySemanticAutoInferenceSmoke() &&
        RunLegacyWorldEditorLayerInferenceSmoke() &&
        RunWorldEditorUndoSmoke() &&
        RunWorldReferenceGraphSmoke() &&
        RunSemanticAuthoringCascadeSmoke() &&
        RunStarterSemanticLinkTargetSmoke() &&
        RunLegacyWorldAliasMigrationSmoke();

    fs::remove_all(sandboxRoot, ec);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
