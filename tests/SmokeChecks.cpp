#include "../include/AppPaths.hpp"
#include "../include/AtomicPersistence.hpp"
#include "../include/GameplayDescriptorRegistry.hpp"
#include "../include/LaunchSession.hpp"
#include "../include/SessionProfiles.hpp"
#include "../include/World.hpp"
#include "../include/WorldValidation.hpp"

#include <cstdlib>
#include <filesystem>
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
        Check(loadedWorld.HasScriptTag("echo_trace"), "starter infrastructure missing after roundtrip");
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
        RunLegacyWorldAliasMigrationSmoke();

    fs::remove_all(sandboxRoot, ec);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
