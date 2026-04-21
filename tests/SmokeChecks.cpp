#include "../include/AppPaths.hpp"
#include "../include/AtomicPersistence.hpp"
#include "../include/GameplayDescriptorRegistry.hpp"
#include "../include/LaunchSession.hpp"
#include "../include/PrefabLibrary.hpp"
#include "../include/SessionProfiles.hpp"
#include "../include/World.hpp"
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
        Check(loadedWorld.HasLinkTarget("inner_spur_water"), "water reclaimer link target missing after roundtrip");
}

bool RunProfileRoundtrip() {
    bunker::SessionProfile savedProfile = bunker::MakeDefaultSessionProfile();
    savedProfile.account.username = "smoke_user";
    savedProfile.selectedWorld = "smoke_zone.bwld";
    savedProfile.fieldCheckpointKnown = true;
    savedProfile.fieldCheckpointWorld.clear();
    savedProfile.lanlineServices.relayCredits = 1337;

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
        Check(loadedProfile.lanlineServices.relayCredits == savedProfile.lanlineServices.relayCredits, "profile relay credits mismatch");
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
    authoredPrefab.label = "Authored Relay";
    authoredPrefab.object.registryId = "[%relay_prefab_0001]";
    authoredPrefab.object.displayName = "Relay Prefab";
    authoredPrefab.object.interaction = bunker::InteractionType::Terminal;
    authoredPrefab.object.category = bunker::ObjectCategory::Terminal;
    authoredPrefab.object.scriptTag = "relay_substation";
    authoredPrefab.object.linkTarget = "shelter17_backbone";
    authoredPrefab.object.semanticAutoCreated = false;
    authoredPrefab.object.semanticLayoutPinned = true;
    savedPrefabs.push_back(authoredPrefab);

    bunker::PrefabRecord autoPrefab;
    autoPrefab.label = "Auto Water";
    autoPrefab.object.registryId = "[%water_reclaimer_auto_0001]";
    autoPrefab.object.displayName = "Water Auto Prefab";
    autoPrefab.object.interaction = bunker::InteractionType::Terminal;
    autoPrefab.object.category = bunker::ObjectCategory::Terminal;
    autoPrefab.object.scriptTag = "water_reclaimer";
    autoPrefab.object.linkTarget = "inner_spur_water";
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
        Check(loadedPrefabs[0].object.semanticLayoutPinned, "prefab library should preserve authored pinned semantic state") &&
        Check(!loadedPrefabs[0].object.semanticAutoCreated, "prefab library should preserve authored semantic origin") &&
        Check(loadedPrefabs[1].label == autoPrefab.label, "prefab library should preserve auto prefab label") &&
        Check(loadedPrefabs[1].object.semanticAutoCreated, "prefab library should preserve auto semantic origin") &&
        Check(!loadedPrefabs[1].object.semanticLayoutPinned, "prefab library should preserve unpinned auto semantic state");
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

    serviceBay->linkTarget = "service_bay_history_drift";
    const auto firstExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::AllowWarnings);
    if (!Check(firstExport.ok, "export history selection smoke expected first prototype export to pass")) {
        return false;
    }
    if (!Check(std::filesystem::exists(firstExport.validationSnapshotPath),
            "export history selection smoke expected first export snapshot archive")) {
        return false;
    }

    serviceBay->linkTarget = "inner_spur_service";
    currentWaterReclaimer->linkTarget = "water_reclaimer_history_drift";
    const auto secondExport = bunker::ExportWorldWithValidation(
        world,
        exportPath,
        bunker::ExportValidationPolicy::AllowWarnings);
    if (!Check(secondExport.ok, "export history selection smoke expected second prototype export to pass")) {
        return false;
    }
    if (!Check(std::filesystem::exists(secondExport.validationSnapshotPath),
            "export history selection smoke expected second export snapshot archive")) {
        return false;
    }

    std::vector<bunker::WorldExportHistoryEntry> historyEntries;
    if (!Check(bunker::LoadWorldExportHistory(exportPath, historyEntries),
            "export history selection smoke expected audit history to load")) {
        return false;
    }
    if (!Check(historyEntries.size() >= 2, "export history selection smoke expected at least two history entries")) {
        return false;
    }
    if (!Check(historyEntries[0].validationSnapshotPath == secondExport.validationSnapshotPath,
            "export history selection smoke expected newest history entry to match second export snapshot")) {
        return false;
    }
    if (!Check(historyEntries[1].validationSnapshotPath == firstExport.validationSnapshotPath,
            "export history selection smoke expected older history entry to match first export snapshot")) {
        return false;
    }

    const auto currentIssues = bunker::ValidateWorldForRuntime(world);
    const auto latestDelta = bunker::CompareValidationToSnapshot(currentIssues, historyEntries[0].validationSnapshotPath);
    if (!Check(latestDelta.hasBaseline, "export history selection smoke expected latest history snapshot to load")) {
        return false;
    }
    if (!Check(latestDelta.issueRegressions.empty() && latestDelta.issueImprovements.empty(),
            "export history selection smoke expected current world to match latest history checkpoint")) {
        return false;
    }

    const auto olderDelta = bunker::CompareValidationToSnapshot(currentIssues, historyEntries[1].validationSnapshotPath);
    const std::string olderReport =
        bunker::BuildValidationSnapshotDeltaReport(olderDelta, "Historical export checkpoint");
    if (!Check(olderDelta.hasBaseline, "export history selection smoke expected older history snapshot to load")) {
        return false;
    }
    if (!Check(olderDelta.regressions.empty() && olderDelta.improvements.empty(),
            "export history selection smoke expected same warning count versus older checkpoint")) {
        return false;
    }
    if (!Check(olderDelta.issueRegressions.size() == 1, "export history selection smoke expected one historical regression")) {
        return false;
    }
    if (!Check(olderDelta.issueImprovements.size() == 1, "export history selection smoke expected one historical improvement")) {
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
            "export history selection smoke expected parsed policy label in audit history") &&
        Check(historyEntries[0].decisionLabel == "exported",
            "export history selection smoke expected parsed decision label in audit history");
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
        RunLaunchTicketFlow() &&
        RunGameplayDescriptorValidationSmoke() &&
        RunSemanticDependencyValidationSmoke() &&
        RunSemanticDependencyGraphSmoke() &&
        RunSemanticLayoutSmoke() &&
        RunSemanticLayoutPreserveManualSmoke() &&
        RunSemanticAuthoringStateRoundtripSmoke() &&
        RunSemanticAutoAnchorValidationSmoke() &&
        RunPrefabLibrarySemanticStateSmoke() &&
        RunStrictSemanticExportPolicySmoke() &&
        RunValidatedWorldExportArtifactSmoke() &&
        RunWorldExportAuditTrailSmoke() &&
        RunShippingBaselineDiffSmoke() &&
        RunShippingBaselineObjectAwareDriftSmoke() &&
        RunExportHistoryCheckpointSelectionSmoke() &&
        RunLegacySemanticAutoInferenceSmoke() &&
        RunSemanticAuthoringCascadeSmoke() &&
        RunStarterSemanticLinkTargetSmoke() &&
        RunLegacyWorldAliasMigrationSmoke();

    fs::remove_all(sandboxRoot, ec);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
