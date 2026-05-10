#include "../include/AppPaths.hpp"
#include "../include/AtomicPersistence.hpp"
#include "../include/BuildAnnouncement.hpp"
#include "../include/GameExecution.hpp"
#include "../include/GameRuntime.hpp"
#include "../include/GameplayDescriptorRegistry.hpp"
#include "../include/HangarSystem.hpp"
#include "../include/LanlineServices.hpp"
#include "../include/LaunchSession.hpp"
#include "../include/PrefabLibrary.hpp"
#include "../include/SessionProfiles.hpp"
#include "../include/StoryRoute.hpp"
#include "../include/World.hpp"
#include "../include/WorldEditorUndo.hpp"
#include "../include/WorldEvents.hpp"
#include "../include/WorldExport.hpp"
#include "../include/WorldSemanticAuthoring.hpp"
#include "../include/WorldValidation.hpp"
#include "../Editor/src/EditorSupport.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

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

bool WriteTextFile(const fs::path& path, const std::string& text) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file << text;
    return true;
}

std::string ReadTextFile(const fs::path& path) {
    std::ifstream file(path);
    std::string text;
    std::string line;
    while (std::getline(file, line)) {
        text += line;
        text += '\n';
    }
    return text;
}

bool ContainsText(const std::string& text, const std::string& needle) {
    return text.find(needle) != std::string::npos;
}

void WriteRawString(std::ofstream& file, const std::string& value) {
    const auto length = static_cast<std::uint32_t>(value.size());
    file.write(reinterpret_cast<const char*>(&length), sizeof(length));
    file.write(value.data(), static_cast<std::streamsize>(value.size()));
}

void UnlockAndEquipSkill(bunker::SessionProfile& profile, const std::string& skillId) {
    for (auto& skill : profile.character.passiveSkills) {
        if (skill.skillId == skillId) {
            skill.unlocked = true;
            skill.equipped = true;
            return;
        }
    }
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
    if (auto* recoveryLocker = savedWorld.FindObjectByRegistryId("[%pip_0001]")) {
        recoveryLocker->lootMode = bunker::LootMode::RandomTable;
        recoveryLocker->lootEntries.clear();
        recoveryLocker->lootEntries.reserve(405);
        for (int index = 0; index < 405; ++index) {
            recoveryLocker->lootEntries.push_back({
                "roundtrip_loot_" + std::to_string(index),
                1,
                1 + (index % 3),
                1.0f + static_cast<float>(index % 5)
            });
        }
    }
    bunker::MapObject sparseContainer;
    sparseContainer.registryId = "[%sparse_loot_0001]";
    sparseContainer.displayName = "Sparse Loot Container";
    sparseContainer.interaction = bunker::InteractionType::Container;
    sparseContainer.category = bunker::ObjectCategory::Container;
    sparseContainer.manualLoot = true;
    sparseContainer.lootMode = bunker::LootMode::RandomTable;
    sparseContainer.lootEntries = {
        {"sparse_loot_real", 2, 5, 0.0f},
        {"", 1, 1, 1.0f},
        {"", 1, 1, 1.0f},
    };
    savedWorld.AddObject(sparseContainer);
    if (auto* sparse = savedWorld.FindObjectByRegistryId(sparseContainer.registryId)) {
        sparse->lootEntries.push_back({});
        sparse->lootEntries.push_back({});
    }
    bunker::MapObject highRowContainer = sparseContainer;
    highRowContainer.registryId = "[%high_row_loot_0001]";
    highRowContainer.displayName = "High Row Loot Container";
    highRowContainer.lootEntries.assign(20, {});
    highRowContainer.lootEntries[19] = {"high_row_loot_real", 1, 1, 1.0f};
    savedWorld.AddObject(highRowContainer);
    bunker::MapObject tenthRowContainer = sparseContainer;
    tenthRowContainer.registryId = "[%tenth_row_loot_0001]";
    tenthRowContainer.displayName = "Tenth Row Loot Container";
    tenthRowContainer.lootEntries.assign(20, {});
    tenthRowContainer.lootEntries[9] = {"tenth_row_loot_real", 1, 2, 1.0f};
    savedWorld.AddObject(tenthRowContainer);
    bunker::MapObject emptyRowsOnlyContainer = sparseContainer;
    emptyRowsOnlyContainer.registryId = "[%empty_rows_only_0001]";
    emptyRowsOnlyContainer.displayName = "Empty Rows Only Container";
    emptyRowsOnlyContainer.lootEntries.assign(20, {});
    savedWorld.AddObject(emptyRowsOnlyContainer);

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
            "world roundtrip should preserve prefab source linkage") &&
        Check(loadedWorld.FindObjectByRegistryId("[%pip_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%pip_0001]")->lootMode == bunker::LootMode::RandomTable,
            "world roundtrip should preserve random loot chest mode") &&
        Check(loadedWorld.FindObjectByRegistryId("[%pip_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%pip_0001]")->lootEntries.size() == 405,
            "world roundtrip should preserve 400+ scalable loot entries") &&
        Check(loadedWorld.FindObjectByRegistryId("[%pip_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%pip_0001]")->lootEntries[404].itemId == "roundtrip_loot_404",
            "world roundtrip should preserve high-index loot entry IDs") &&
        Check(loadedWorld.FindObjectByRegistryId("[%sparse_loot_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%sparse_loot_0001]")->lootEntries.size() == 1,
            "world roundtrip should not invent persisted UI-only virtual loot rows") &&
        Check(loadedWorld.FindObjectByRegistryId("[%sparse_loot_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%sparse_loot_0001]")->lootEntries[0].weight == 0.0f,
            "world roundtrip should preserve zero random loot weight for runtime skip semantics") &&
        Check(loadedWorld.FindObjectByRegistryId("[%high_row_loot_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%high_row_loot_0001]")->lootEntries.size() == 20,
            "world roundtrip should preserve interior empty rows before a filled high-index loot row") &&
        Check(loadedWorld.FindObjectByRegistryId("[%high_row_loot_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%high_row_loot_0001]")->lootEntries[19].itemId == "high_row_loot_real",
            "world roundtrip should preserve filled high-index virtual-row loot entry") &&
        Check(loadedWorld.FindObjectByRegistryId("[%tenth_row_loot_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%tenth_row_loot_0001]")->lootEntries.size() == 10,
            "world roundtrip should trim trailing empty rows after a filled row 10 entry") &&
        Check(loadedWorld.FindObjectByRegistryId("[%tenth_row_loot_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%tenth_row_loot_0001]")->lootEntries[9].itemId == "tenth_row_loot_real",
            "world roundtrip should preserve filled row 10 loot entry") &&
        Check(loadedWorld.FindObjectByRegistryId("[%empty_rows_only_0001]") != nullptr &&
                loadedWorld.FindObjectByRegistryId("[%empty_rows_only_0001]")->lootEntries.empty(),
            "world roundtrip should trim all-empty trailing loot rows to zero persisted entries");
}

bool RunEditorWorldFileHelpersSmoke() {
    const fs::path tempRoot = fs::temp_directory_path() / "bunker_editor_file_helpers_smoke";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);
    if (!Check(!ec, "editor file helper smoke failed to create temp root")) {
        return false;
    }

    WorkingDirectoryGuard workingDirectoryGuard(tempRoot);
    bunker::EnsureProjectDirectories();

    bunker::World existingWorld;
    existingWorld.GeneratePrototypeZone();
    existingWorld.metadata.name = "Existing Authoring World";
    const auto existingSave = bunker::SaveWorldAtomically(existingWorld, bunker::DefaultWorldPath());
    if (!Check(existingSave.ok, "editor file helper smoke failed to seed existing world: " + existingSave.message)) {
        return false;
    }

    bunker::World newWorld;
    std::string statusText;
    if (!Check(editor_support::CreateNewEditorWorld(newWorld, statusText),
            "editor file helper smoke expected CreateNewEditorWorld to succeed")) {
        return false;
    }

    bunker::World reloadedExistingWorld;
    if (!Check(reloadedExistingWorld.Load(bunker::DefaultWorldPath().string()),
            "editor file helper smoke failed to reload seeded world after New World")) {
        return false;
    }
    if (!Check(reloadedExistingWorld.metadata.name == "Existing Authoring World",
            "editor file helper smoke expected New World helper not to overwrite existing .bwld")) {
        return false;
    }

    newWorld.metadata.name = "Helper Save World";
    std::filesystem::path saveAsPath;
    if (!Check(editor_support::TrySaveEditorWorldAtPath(newWorld, "helper_save_world", statusText, &saveAsPath),
            "editor file helper smoke expected Save helper to write .bwld")) {
        return false;
    }
    if (!Check(saveAsPath.extension() == ".bwld",
            "editor file helper smoke expected Save helper to normalize missing extension to .bwld")) {
        return false;
    }

    bunker::World loadedSavedWorld;
    std::filesystem::path loadedPath;
    if (!Check(editor_support::TryLoadEditorWorldAtPath(saveAsPath, loadedSavedWorld, statusText, &loadedPath),
            "editor file helper smoke expected Load helper to reload saved world")) {
        return false;
    }

    const auto worldFiles = editor_support::ListNativeWorldFiles();
    const bool foundSavedWorld = std::any_of(worldFiles.begin(), worldFiles.end(), [&](const fs::path& path) {
        return path.filename() == saveAsPath.filename();
    });

    return Check(loadedSavedWorld.metadata.name == "Helper Save World",
            "editor file helper smoke expected saved world metadata to roundtrip") &&
        Check(foundSavedWorld, "editor file helper smoke expected saved world to appear in .bwld file list") &&
        Check(!editor_support::TryLoadEditorWorldAtPath("not_a_world.esp", loadedSavedWorld, statusText, nullptr) &&
                statusText.find(".bwld") != std::string::npos,
            "editor file helper smoke expected non-.bwld native world load to be rejected");
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
    savedProfile.continuityAnchorSeeded = true;
    savedProfile.continuityAnchorVariance = 0.23f;
    savedProfile.story.awakenedFromCryo = true;
    savedProfile.story.pipPadRecovered = true;
    savedProfile.story.archiveRecovered = true;
    savedProfile.firstPlayableRoute.introSeen = true;
    savedProfile.firstPlayableRoute.emergencyMeleeRecovered = true;
    savedProfile.firstPlayableRoute.accessCardRecovered = true;
    savedProfile.firstPlayableRoute.earlyVerminEncounterResolved = true;
    savedProfile.firstPlayableRoute.prePipPadClueCount = 2;
    savedProfile.firstPlayableRoute.bt72HullInspected = true;
    savedProfile.firstPlayableRoute.bt72CoreRecovered = true;
    savedProfile.firstPlayableRoute.bt72ServiceNotesRecovered = true;
    savedProfile.firstPlayableRoute.bt72Restored = true;
    savedProfile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    savedProfile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    savedProfile.firstPlayableRoute.surfaceArrivalReached = true;
    savedProfile.partnerTank.secondSeatUnlocked = true;
    savedProfile.partnerTank.secondSeatPolicy = "trusted_only";
    savedProfile.partnerTank.trustedGunnerHandle = "Lan Buddy";
    savedProfile.partnerTank.assignedGunnerHandle = "Lan Buddy";
    savedProfile.partnerTank.gunnerDrillSeen = true;
    auto* savedWorldState = bunker::FindWorldFieldState(savedProfile, savedProfile.selectedWorld, true);
    if (!Check(savedWorldState != nullptr, "profile roundtrip expected selected world state")) {
        return false;
    }
    savedWorldState->activeRouteEventType = "service_call";
    savedWorldState->routeEventTimeRemaining = 92.0f;
    savedWorldState->routeEventCooldown = 17.0f;
    savedWorldState->routeEventOfferTimeRemaining = 11.0f;
    savedWorldState->routeEventProgress = 1;
    savedWorldState->routeEventStage = 1;
    savedWorldState->routeEventSerial = 3;
    savedWorldState->routeEventsResolved = 2;
    savedWorldState->routeEventsFailed = 1;
    savedWorldState->routeEventsExpired = 4;
    savedWorldState->lastRouteEventType = "damaged_convoy";
    savedWorldState->lastRouteEventOutcome = "failed";

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
            "profile launcher last-seen announcement mismatch") &&
        Check(loadedProfile.continuityAnchorSeeded, "profile continuity anchor seeded mismatch") &&
        Check(std::abs(loadedProfile.continuityAnchorVariance - 0.23f) < 0.01f,
            "profile continuity anchor variance mismatch") &&
        Check(loadedProfile.firstPlayableRoute.accessCardRecovered, "profile first route access card mismatch") &&
        Check(loadedProfile.firstPlayableRoute.prePipPadClueCount == 2, "profile first route clue count mismatch") &&
        Check(loadedProfile.firstPlayableRoute.bt72Restored, "profile first route restore flag mismatch") &&
        Check(loadedProfile.firstPlayableRoute.clearanceMaterialsRecovered, "profile first route clearance material flag mismatch") &&
        Check(loadedProfile.firstPlayableRoute.surfaceArrivalReached, "profile first route surface arrival flag mismatch") &&
        Check(loadedProfile.partnerTank.secondSeatUnlocked, "profile second seat unlock mismatch") &&
        Check(loadedProfile.partnerTank.secondSeatPolicy == "trusted_only", "profile second seat policy mismatch") &&
        Check(loadedProfile.partnerTank.trustedGunnerHandle == "Lan Buddy", "profile trusted gunner mismatch") &&
        Check(bunker::FindWorldFieldState(loadedProfile, loadedProfile.selectedWorld) != nullptr,
            "profile roundtrip expected loaded selected world state") &&
        Check(bunker::FindWorldFieldState(loadedProfile, loadedProfile.selectedWorld)->activeRouteEventType == "service_call",
            "profile roundtrip active route event mismatch") &&
        Check(std::abs(bunker::FindWorldFieldState(loadedProfile, loadedProfile.selectedWorld)->routeEventTimeRemaining - 92.0f) < 0.01f,
            "profile roundtrip active route event timer mismatch") &&
        Check(std::abs(bunker::FindWorldFieldState(loadedProfile, loadedProfile.selectedWorld)->routeEventOfferTimeRemaining - 11.0f) < 0.01f,
            "profile roundtrip route event offer timer mismatch") &&
        Check(bunker::FindWorldFieldState(loadedProfile, loadedProfile.selectedWorld)->routeEventProgress == 1,
            "profile roundtrip route event progress mismatch") &&
        Check(bunker::FindWorldFieldState(loadedProfile, loadedProfile.selectedWorld)->routeEventsResolved == 2,
            "profile roundtrip resolved route-event count mismatch") &&
        Check(bunker::FindWorldFieldState(loadedProfile, loadedProfile.selectedWorld)->routeEventsFailed == 1,
            "profile roundtrip failed route-event count mismatch") &&
        Check(bunker::FindWorldFieldState(loadedProfile, loadedProfile.selectedWorld)->routeEventsExpired == 4,
            "profile roundtrip expired route-event count mismatch") &&
        Check(bunker::FindWorldFieldState(loadedProfile, loadedProfile.selectedWorld)->lastRouteEventType == "damaged_convoy",
            "profile roundtrip last route-event type mismatch") &&
        Check(bunker::FindWorldFieldState(loadedProfile, loadedProfile.selectedWorld)->lastRouteEventOutcome == "failed",
            "profile roundtrip last route-event outcome mismatch");
}

bool RunProfileMigrationContractSmoke() {
    const fs::path tempRoot = fs::current_path() / "profile_migration_contract_smoke";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);
    if (!Check(!ec, "profile migration smoke failed to create temp directory")) {
        return false;
    }

    const fs::path v9Path = tempRoot / "legacy_v9_profile.txt";
    if (!Check(WriteTextFile(v9Path,
            "profile_format=BPF1\n"
            "profile_version=9\n"
            "story_awakened=1\n"
            "story_pippad=1\n"
            "story_archive=1\n"
            "route_access_card=1\n"
            "route_pre_pippad_clues=2\n"
            "route_bt72_hull=1\n"
            "route_bt72_core=1\n"
            "route_bt72_notes=1\n"
            "route_bt72_restored=0\n"
            "story_tank=0\n"),
            "profile migration smoke failed to write v9 profile")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    bunker::SessionProfile v9Profile;
    if (!Check(bunker::LoadSessionProfile(v9Path, v9Profile),
            "profile_migration_v9_defaults_bluelink_and_crane_locked expected load success")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    if (!Check(v9Profile.story.pipPadRecovered &&
            bunker::HasPipPad(v9Profile) &&
            v9Profile.pipPadExpansionCoverPresent &&
            !v9Profile.blueLinkModuleRecovered &&
            !v9Profile.blueLinkModuleInstalled &&
            !bunker::CanUsePipPadMediaIndex(v9Profile) &&
            !v9Profile.hangarPowerRestored &&
            !v9Profile.bt72CraneControlOnline &&
            !v9Profile.bt72CranePathClear &&
            !v9Profile.bt72HullMovedToServiceLift &&
            !v9Profile.bt72HullLockedInRestorationCradle &&
            !v9Profile.firstPlayableRoute.bt72Restored &&
            !v9Profile.story.tankLinked,
            "profile_migration_v9_defaults_bluelink_and_crane_locked expected locked defaults")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    const fs::path v10Path = tempRoot / "legacy_v10_bluelink_profile.txt";
    if (!Check(WriteTextFile(v10Path,
            "profile_format=BPF1\n"
            "profile_version=10\n"
            "story_awakened=1\n"
            "story_pippad=1\n"
            "pippad_expansion_cover_present=0\n"
            "bluelink_module_recovered=1\n"
            "bluelink_module_installed=1\n"
            "story_archive=1\n"
            "route_bt72_hull=1\n"
            "route_bt72_core=1\n"
            "route_bt72_notes=1\n"
            "route_bt72_restored=0\n"
            "story_tank=0\n"),
            "profile migration smoke failed to write v10 profile")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    bunker::SessionProfile v10Profile;
    if (!Check(bunker::LoadSessionProfile(v10Path, v10Profile),
            "profile_migration_v10_preserves_bluelink_but_requires_cradle expected load success")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    if (!Check(bunker::IsBlueLinkInstalled(v10Profile) &&
            bunker::CanUsePipPadMediaIndex(v10Profile) &&
            !v10Profile.pipPadExpansionCoverPresent &&
            !v10Profile.hangarPowerRestored &&
            !v10Profile.bt72CraneControlOnline &&
            !v10Profile.bt72CranePathClear &&
            !v10Profile.bt72HullMovedToServiceLift &&
            !v10Profile.bt72HullLockedInRestorationCradle &&
            !bunker::CanCompleteBt72StagedRestoration(v10Profile) &&
            !v10Profile.firstPlayableRoute.bt72Restored &&
            !v10Profile.story.tankLinked,
            "profile_migration_v10_preserves_bluelink_but_requires_cradle expected media only and crane locked")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    const fs::path restoredPath = tempRoot / "legacy_restored_bt72_profile.txt";
    if (!Check(WriteTextFile(restoredPath,
            "profile_format=BPF1\n"
            "profile_version=9\n"
            "story_pippad=1\n"
            "story_archive=1\n"
            "route_bt72_hull=1\n"
            "route_bt72_core=1\n"
            "route_bt72_notes=1\n"
            "route_bt72_restored=1\n"
            "story_tank=1\n"),
            "profile migration smoke failed to write legacy restored BT-72 profile")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    bunker::SessionProfile restoredProfile;
    if (!Check(bunker::LoadSessionProfile(restoredPath, restoredProfile),
            "profile_migration_legacy_restored_bt72_implies_crane_path expected load success")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    if (!Check(restoredProfile.firstPlayableRoute.bt72Restored &&
            restoredProfile.story.tankLinked &&
            restoredProfile.hangarPowerRestored &&
            restoredProfile.bt72CraneControlOnline &&
            restoredProfile.bt72CranePathClear &&
            restoredProfile.bt72HullMovedToServiceLift &&
            restoredProfile.bt72HullLockedInRestorationCradle &&
            !restoredProfile.bt72HullAttachedToCrane &&
            bunker::CanCompleteBt72StagedRestoration(restoredProfile),
            "profile_migration_legacy_restored_bt72_implies_crane_path expected crane compatibility flags")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    bunker::SessionProfile currentProfile = bunker::MakeDefaultSessionProfile();
    currentProfile.story.awakenedFromCryo = true;
    currentProfile.continuityAnchorSeeded = true;
    currentProfile.story.pipPadRecovered = true;
    bunker::AddInventoryItem(currentProfile, "#%it_pippad", 1, 0.8f);
    currentProfile.blueLinkModuleRecovered = true;
    currentProfile.blueLinkModuleInstalled = true;
    currentProfile.pipPadExpansionCoverPresent = false;
    currentProfile.story.archiveRecovered = true;
    currentProfile.firstPlayableRoute.accessCardRecovered = true;
    currentProfile.firstPlayableRoute.prePipPadClueCount = 2;
    currentProfile.firstPlayableRoute.bt72HullInspected = true;
    currentProfile.firstPlayableRoute.bt72CoreRecovered = true;
    currentProfile.firstPlayableRoute.bt72ServiceNotesRecovered = true;
    currentProfile.firstPlayableRoute.bt72Restored = true;
    currentProfile.hangarPowerRestored = true;
    currentProfile.bt72CraneControlOnline = true;
    currentProfile.bt72CranePathClear = true;
    currentProfile.bt72HullAttachedToCrane = false;
    currentProfile.bt72HullMovedToServiceLift = true;
    currentProfile.bt72HullLockedInRestorationCradle = true;
    currentProfile.story.tankLinked = true;

    const fs::path currentPath = tempRoot / "current_profile.txt";
    const auto saveStatus = bunker::SaveProfileAtomically(currentProfile, currentPath);
    if (!Check(saveStatus.ok, "profile_migration_current_roundtrip_preserves_v936_route_state failed to save profile: " + saveStatus.message)) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    bunker::SessionProfile loadedCurrentProfile;
    if (!Check(bunker::LoadSessionProfile(currentPath, loadedCurrentProfile),
            "profile_migration_current_roundtrip_preserves_v936_route_state failed to load profile")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    if (!Check(loadedCurrentProfile.story.pipPadRecovered &&
            bunker::IsBlueLinkInstalled(loadedCurrentProfile) &&
            bunker::CanUsePipPadMediaIndex(loadedCurrentProfile) &&
            loadedCurrentProfile.firstPlayableRoute.bt72Restored &&
            loadedCurrentProfile.story.tankLinked &&
            loadedCurrentProfile.bt72HullLockedInRestorationCradle &&
            loadedCurrentProfile.continuityAnchorSeeded &&
            bunker::CurrentStoryObjectivePreview(loadedCurrentProfile).find("Restore BT-72") == std::string::npos,
            "profile_migration_current_roundtrip_preserves_v936_route_state expected current final state")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    const std::string savedText = ReadTextFile(currentPath);
    const bool ok = Check(ContainsText(savedText, "profile_format=BPF1\n") &&
            ContainsText(savedText, "profile_version=11\n") &&
            ContainsText(savedText, "pippad_expansion_cover_present=") &&
            ContainsText(savedText, "bluelink_module_recovered=") &&
            ContainsText(savedText, "bluelink_module_installed=") &&
            ContainsText(savedText, "hangar_power_restored=") &&
            ContainsText(savedText, "bt72_crane_control_online=") &&
            ContainsText(savedText, "bt72_crane_path_clear=") &&
            ContainsText(savedText, "bt72_hull_attached_to_crane=") &&
            ContainsText(savedText, "bt72_hull_moved_to_service_lift=") &&
            ContainsText(savedText, "bt72_hull_locked_in_restoration_cradle="),
            "profile_migration_current_save_writes_bluelink_and_crane_keys expected current keys");

    fs::remove_all(tempRoot, ec);
    return ok;
}

bool RunContinuityAnchorContractSmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    bunker::World world;
    bunker::MapObject cryo;
    cryo.registryId = "[%cryo_0001]";
    cryo.displayName = "Cryo Capsule";
    world.AddObject(cryo);
    bunker::PlayerState player;
    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;

    bunker::HandleInteraction(world.FindObjectByRegistryId("[%cryo_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.continuityAnchorSeeded,
            "continuity_anchor_seeded_after_bunker_anomaly expected Continuity Anchor to seed")) {
        return false;
    }
    if (!Check(profile.continuityAnchorVariance > 0.0f &&
            gameState.lastEvent.find("Continuity Anchor variance detected.") != std::string::npos &&
            gameState.lastEvent.find("Identity continuity profile recovered.") != std::string::npos,
            "continuity_anchor_seeded_after_bunker_anomaly expected diagnostic copy")) {
        return false;
    }

    const fs::path tempRoot = fs::current_path() / "continuity_anchor_contract_smoke";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);
    if (!Check(!ec, "continuity anchor smoke failed to create temp directory")) {
        return false;
    }

    const fs::path profilePath = tempRoot / "profile.txt";
    const auto saveStatus = bunker::SaveProfileAtomically(profile, profilePath);
    if (!Check(saveStatus.ok, "continuity_anchor_persists_after_save_load failed to save profile: " + saveStatus.message)) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    bunker::SessionProfile loadedProfile;
    if (!Check(bunker::LoadSessionProfile(profilePath, loadedProfile),
            "continuity_anchor_persists_after_save_load failed to load profile")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    if (!Check(loadedProfile.continuityAnchorSeeded &&
            std::abs(loadedProfile.continuityAnchorVariance - profile.continuityAnchorVariance) < 0.01f,
            "continuity_anchor_persists_after_save_load expected anchor state to roundtrip")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    const fs::path legacyPath = tempRoot / "legacy_soulline_profile.txt";
    {
        std::ofstream legacyProfile(legacyPath);
        legacyProfile << "profile_format=BPF1\n";
        legacyProfile << "profile_version=8\n";
        legacyProfile << "soulline_seeded=1\n";
        legacyProfile << "soulline_variance=0.42\n";
    }

    bunker::SessionProfile legacyLoaded;
    if (!Check(bunker::LoadSessionProfile(legacyPath, legacyLoaded),
            "continuity_anchor_aliases_soulline_legacy_field failed to load legacy profile")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    const bool ok = Check(legacyLoaded.continuityAnchorSeeded && bunker::SoulLineSeeded(legacyLoaded),
            "continuity_anchor_aliases_soulline_legacy_field expected seeded alias") &&
        Check(std::abs(legacyLoaded.continuityAnchorVariance - 0.42f) < 0.01f &&
                std::abs(bunker::SoulLineVariance(legacyLoaded) - 0.42f) < 0.01f,
            "continuity_anchor_aliases_soulline_legacy_field expected variance alias");

    fs::remove_all(tempRoot, ec);
    return ok;
}

bool RunFirstPlayableRouteStorySmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    if (!Check(bunker::CurrentStoryCheckpointLabel(profile) == "Intro // Cryo Wake",
            "first route smoke expected intro checkpoint label")) {
        return false;
    }
    if (!Check(bunker::CurrentStoryObjectivePreview(profile).find("Wake from the cryo capsule") != std::string::npos,
            "first route smoke expected intro objective preview")) {
        return false;
    }

    profile.story.awakenedFromCryo = true;
    profile.firstPlayableRoute.accessCardRecovered = true;
    profile.firstPlayableRoute.prePipPadClueCount = 2;
    if (!Check(bunker::CurrentStoryCheckpointLabel(profile) == "Pip-Pad Recovery",
            "first route smoke expected Pip-Pad checkpoint label")) {
        return false;
    }

    profile.story.pipPadRecovered = true;
    profile.firstPlayableRoute.earlyVerminEncounterResolved = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72HullInspected = true;
    profile.firstPlayableRoute.bt72CoreRecovered = true;
    profile.firstPlayableRoute.bt72ServiceNotesRecovered = true;
    profile.hangarPowerRestored = true;
    profile.bt72CraneControlOnline = true;
    profile.bt72CranePathClear = true;
    profile.bt72HullMovedToServiceLift = true;
    profile.bt72HullLockedInRestorationCradle = true;
    profile.character.inventory.push_back({"power_cell", 1, 0.3f});
    profile.character.inventory.push_back({"repair_patch", 1, 0.2f});
    profile.character.inventory.push_back({"old_plate", 1, 0.5f});
    const auto restoreRoute = bunker::BuildBt72RestorationRoute(profile);
    if (!Check(restoreRoute.size() == 10, "first route smoke expected BT-72 restore checklist to have ten entries")) {
        return false;
    }
    if (!Check(!restoreRoute[5].completed, "first route smoke expected BT-72 restore step to remain incomplete before restore")) {
        return false;
    }
    if (!Check(bunker::CurrentStoryObjectivePreview(profile).find("Restore BT-72") != std::string::npos,
            "first route smoke expected restore objective preview")) {
        return false;
    }
    const auto initialSliceRoute = bunker::BuildFirstPlayableRouteSlice(profile);
    if (!Check(initialSliceRoute.size() == 14, "first route smoke expected vertical slice checklist to have fourteen entries")) {
        return false;
    }
    if (!Check(!initialSliceRoute[4].completed && initialSliceRoute[3].completed,
            "first route smoke expected BT-72 restoration to be the next incomplete slice step")) {
        return false;
    }

    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    if (!Check(bunker::CurrentStoryCheckpointLabel(profile) == "Surface Arrival",
            "first route smoke expected surface arrival checkpoint label")) {
        return false;
    }
    if (!Check(bunker::CurrentStoryObjectivePreview(profile).find("exterior foothold") != std::string::npos,
            "first route smoke expected surface arrival objective preview")) {
        return false;
    }
    const auto surfaceArrivalSliceRoute = bunker::BuildFirstPlayableRouteSlice(profile);
    if (!Check(surfaceArrivalSliceRoute[7].completed && !surfaceArrivalSliceRoute[8].completed,
            "first route smoke expected surface arrival to gate the post-bulkhead payoff")) {
        return false;
    }
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;
    profile.firstPlayableRoute.firstTankCombatResolved = true;
    profile.firstPlayableRoute.firstServicePerformed = true;
    profile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    if (!Check(bunker::CurrentStoryCheckpointLabel(profile) == "Debrief",
            "first route smoke expected debrief checkpoint label")) {
        return false;
    }
    if (!Check(bunker::CurrentStoryObjectivePreview(profile).find("Return for debrief") != std::string::npos,
            "first route smoke expected debrief objective preview")) {
        return false;
    }

    profile.firstPlayableRoute.debriefSummaryViewed = true;
    profile.story.returnedToBase = true;
    auto* worldState = bunker::FindWorldFieldState(profile, profile.selectedWorld, true);
    if (!Check(worldState != nullptr, "first route smoke expected world field state")) {
        return false;
    }
    return Check(bunker::CurrentStoryCheckpointLabel(profile) == "Industrial Expansion",
            "first route smoke expected industrial expansion checkpoint label") &&
        Check(bunker::CurrentStoryObjectivePreview(profile).find("rail depot") != std::string::npos,
            "first route smoke expected industrial follow-up objective preview");
}

bool RunBt72RestorationObjectiveReadoutSmoke() {
    const auto hasAny = [](const std::string& text, std::initializer_list<const char*> needles) {
        return std::any_of(needles.begin(), needles.end(), [&](const char* needle) {
            return text.find(needle) != std::string::npos;
        });
    };
    const auto setRouteOpen = [](bunker::SessionProfile& profile) {
        profile.story.awakenedFromCryo = true;
        profile.story.pipPadRecovered = true;
        profile.story.archiveRecovered = true;
        profile.firstPlayableRoute.accessCardRecovered = true;
    };
    const auto setKnowledgeComplete = [](bunker::SessionProfile& profile) {
        profile.firstPlayableRoute.bt72HullInspected = true;
        profile.firstPlayableRoute.bt72CoreRecovered = true;
        profile.firstPlayableRoute.bt72ServiceNotesRecovered = true;
    };
    const auto setCradleComplete = [](bunker::SessionProfile& profile) {
        profile.hangarPowerRestored = true;
        profile.bt72CraneControlOnline = true;
        profile.bt72CranePathClear = true;
        profile.bt72HullAttachedToCrane = false;
        profile.bt72HullMovedToServiceLift = true;
        profile.bt72HullLockedInRestorationCradle = true;
    };
    const auto addRestorationMaterials = [](bunker::SessionProfile& profile) {
        bunker::AddInventoryItem(profile, "power_cell", 1, 0.3f);
        bunker::AddInventoryItem(profile, "repair_patch", 1, 0.2f);
        bunker::AddInventoryItem(profile, "old_plate", 1, 0.5f);
    };

    bunker::StaticEraser staticEraser;

    bunker::SessionProfile knowledgeMissing = bunker::MakeDefaultSessionProfile();
    setRouteOpen(knowledgeMissing);
    const std::string missingPreview = bunker::CurrentStoryObjectivePreview(knowledgeMissing);
    if (!Check(missingPreview.find("hull") != std::string::npos &&
            missingPreview.find("starter core") != std::string::npos &&
            missingPreview.find("service notes") != std::string::npos,
            "bt72_objective_requires_knowledge_before_cradle expected hull/core/service notes objective")) {
        return false;
    }

    bunker::SessionProfile cradleMissing = bunker::MakeDefaultSessionProfile();
    setRouteOpen(cradleMissing);
    setKnowledgeComplete(cradleMissing);
    addRestorationMaterials(cradleMissing);
    const std::string cradlePreview = bunker::CurrentStoryObjectivePreview(cradleMissing);
    const std::string cradleObjective = bunker::CurrentStoryObjective(cradleMissing, staticEraser);
    if (!Check(hasAny(cradlePreview, {"crane", "service lift", "cradle"}) &&
            hasAny(cradleObjective, {"crane", "service lift", "cradle"}) &&
            cradlePreview.find("Survey the BT-72 hull, recover the starter core, and decode the service notes") == std::string::npos &&
            cradleObjective.find("Survey the BT-72 hull, recover the starter core, and decode the service notes") == std::string::npos &&
            cradlePreview.find("Restore BT-72 to partial operating condition") == std::string::npos &&
            cradleObjective.find("Restore BT-72 to partial operating condition") == std::string::npos,
            "bt72_objective_points_to_cradle_after_knowledge expected crane/cradle objective")) {
        return false;
    }

    bunker::SessionProfile materialMissing = bunker::MakeDefaultSessionProfile();
    setRouteOpen(materialMissing);
    setKnowledgeComplete(materialMissing);
    setCradleComplete(materialMissing);
    bunker::AddInventoryItem(materialMissing, "power_cell", 1, 0.3f);
    bunker::AddInventoryItem(materialMissing, "old_plate", 1, 0.5f);
    const std::string materialObjective = bunker::CurrentStoryObjective(materialMissing, staticEraser);
    if (!Check(materialObjective.find("repair patch") != std::string::npos &&
            materialObjective.find("Survey the BT-72 hull") == std::string::npos &&
            !hasAny(materialObjective, {"crane", "cradle"}),
            "bt72_objective_points_to_materials_after_cradle expected repair patch objective")) {
        return false;
    }

    bunker::SessionProfile restoreReady = bunker::MakeDefaultSessionProfile();
    setRouteOpen(restoreReady);
    setKnowledgeComplete(restoreReady);
    setCradleComplete(restoreReady);
    addRestorationMaterials(restoreReady);
    const std::string restoreReadyObjective = bunker::CurrentStoryObjectivePreview(restoreReady);
    if (!Check(restoreReadyObjective.find("Restore BT-72") != std::string::npos &&
            restoreReadyObjective.find("partial operating condition") != std::string::npos,
            "bt72_objective_restore_ready_after_cradle_and_materials expected restore-ready objective")) {
        return false;
    }

    restoreReady.firstPlayableRoute.bt72Restored = true;
    const std::string syncObjective = bunker::CurrentStoryObjective(restoreReady, staticEraser);
    if (!Check(syncObjective.find("cockpit") != std::string::npos &&
            syncObjective.find("link") != std::string::npos &&
            syncObjective.find("Restore BT-72") == std::string::npos,
            "bt72_objective_moves_to_sync_after_restore expected cockpit sync objective")) {
        return false;
    }

    const auto cradleMissingRoute = bunker::BuildBt72RestorationRoute(cradleMissing);
    const auto missingCradleEntry = std::find_if(cradleMissingRoute.begin(), cradleMissingRoute.end(), [](const bunker::StoryRouteEntry& entry) {
        return entry.text.find("cradle") != std::string::npos || entry.text.find("service lift") != std::string::npos;
    });
    bunker::SessionProfile cradleComplete = cradleMissing;
    setCradleComplete(cradleComplete);
    const auto cradleCompleteRoute = bunker::BuildBt72RestorationRoute(cradleComplete);
    const auto completeCradleEntry = std::find_if(cradleCompleteRoute.begin(), cradleCompleteRoute.end(), [](const bunker::StoryRouteEntry& entry) {
        return entry.text.find("cradle") != std::string::npos || entry.text.find("service lift") != std::string::npos;
    });
    if (!Check(missingCradleEntry != cradleMissingRoute.end() &&
            completeCradleEntry != cradleCompleteRoute.end() &&
            !missingCradleEntry->completed &&
            completeCradleEntry->completed,
            "bt72_readout_tracks_restoration_cradle_step expected cradle checklist state")) {
        return false;
    }

    bunker::SessionProfile blueLinkOnly = bunker::MakeDefaultSessionProfile();
    blueLinkOnly.story.awakenedFromCryo = true;
    bunker::RecoverPipPad(blueLinkOnly);
    blueLinkOnly.blueLinkModuleRecovered = true;
    blueLinkOnly.blueLinkModuleInstalled = true;
    blueLinkOnly.pipPadExpansionCoverPresent = false;
    const std::string blueLinkObjective = bunker::CurrentStoryObjectivePreview(blueLinkOnly);
    return Check(bunker::CanUsePipPadMediaIndex(blueLinkOnly) &&
            !blueLinkOnly.story.archiveRecovered &&
            !blueLinkOnly.story.tankLinked &&
            !blueLinkOnly.firstPlayableRoute.bt72Restored &&
            blueLinkObjective.find("archive") != std::string::npos,
            "bt72_objective_bluelink_does_not_skip_route expected normal archive objective");
}

bool RunRouteBeatPresentationSmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    if (!Check(bunker::CurrentFirstPlayableRouteBeat(profile).label == "Bunker Trace",
            "route beat smoke expected bunker-trace opening label")) {
        return false;
    }

    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    if (!Check(bunker::CurrentFirstPlayableRouteBeat(profile).label == "Bulkhead Breakout",
            "route beat smoke expected bulkhead-breakout label before exit")) {
        return false;
    }

    profile.story.exitedBunker = true;
    const auto surfaceAscentBeat = bunker::CurrentFirstPlayableRouteBeat(profile);
    if (!Check(surfaceAscentBeat.label == "Surface Ascent",
            "route beat smoke expected surface-ascent label after bunker exit")) {
        return false;
    }
    if (!Check(surfaceAscentBeat.cue.find("exterior foothold") != std::string::npos,
            "route beat smoke expected exterior-foothold cue for surface-ascent beat")) {
        return false;
    }

    profile.firstPlayableRoute.surfaceArrivalReached = true;
    if (!Check(bunker::CurrentFirstPlayableRouteBeat(profile).label == "Exterior Exposure",
            "route beat smoke expected exterior-exposure label after surface arrival")) {
        return false;
    }

    profile.story.outerRoadCleared = true;
    if (!Check(bunker::CurrentFirstPlayableRouteBeat(profile).label == "First Contact",
            "route beat smoke expected first-contact label after debris clear")) {
        return false;
    }

    profile.firstPlayableRoute.firstTankCombatResolved = true;
    if (!Check(bunker::CurrentFirstPlayableRouteBeat(profile).label == "Service Halt",
            "route beat smoke expected service-halt label after first combat")) {
        return false;
    }

    profile.firstPlayableRoute.firstServicePerformed = true;
    if (!Check(bunker::CurrentFirstPlayableRouteBeat(profile).label == "Recovery Sync",
            "route beat smoke expected recovery-sync label after first service")) {
        return false;
    }

    profile.story.relayRecovered = true;
    profile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    if (!Check(bunker::CurrentFirstPlayableRouteBeat(profile).label == "Debrief Window",
            "route beat smoke expected debrief-window label after relay sync")) {
        return false;
    }

    profile.story.returnedToBase = true;
    profile.firstPlayableRoute.debriefSummaryViewed = true;
    (void)bunker::FindWorldFieldState(profile, profile.selectedWorld, true);
    const auto industrialBeat = bunker::CurrentFirstPlayableRouteBeat(profile);
    return Check(industrialBeat.label == "Industrial Handoff",
            "route beat smoke expected industrial-handoff label after debrief") &&
        Check(industrialBeat.cue.find("rail freight") != std::string::npos,
            "route beat smoke expected industrial handoff cue to point at rail freight");
}

bool RunFirstPlayableRouteReadoutSmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    const auto introReadout = bunker::BuildFirstPlayableRouteReadout(profile);
    if (!Check(introReadout.checkpoint == "Intro // Cryo Wake",
            "route readout smoke expected intro checkpoint")) {
        return false;
    }
    if (!Check(introReadout.completedSteps == 0 && introReadout.totalSteps == 14,
            "route readout smoke expected empty intro progress")) {
        return false;
    }
    if (!Check(introReadout.surfaceStatus == "bunker-bound",
            "route readout smoke expected bunker-bound surface status before exit")) {
        return false;
    }
    if (!Check(introReadout.nextPayoff == "Wake from cryostasis.",
            "route readout smoke expected cryostasis as first payoff")) {
        return false;
    }
    if (!Check(introReadout.brief.find("Pip-Pad") != std::string::npos,
            "route readout smoke expected bunker-trace brief to mention Pip-Pad recovery")) {
        return false;
    }

    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    const auto ascentReadout = bunker::BuildFirstPlayableRouteReadout(profile);
    if (!Check(ascentReadout.beat == "Surface Ascent",
            "route readout smoke expected surface-ascent beat after bunker exit")) {
        return false;
    }
    if (!Check(ascentReadout.surfaceStatus == "ascent corridor live",
            "route readout smoke expected ascent-corridor surface status after exit")) {
        return false;
    }
    if (!Check(ascentReadout.nextPayoff == "Reach the first surface arrival foothold.",
            "route readout smoke expected surface foothold next payoff")) {
        return false;
    }
    if (!Check(ascentReadout.brief.find("Skyline, exposure") != std::string::npos,
            "route readout smoke expected surface-ascent payoff in the compact brief")) {
        return false;
    }

    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;
    profile.firstPlayableRoute.firstTankCombatResolved = true;
    const auto serviceReadout = bunker::BuildFirstPlayableRouteReadout(profile);
    if (!Check(serviceReadout.surfaceStatus == "contact resolved, service halt pending",
            "route readout smoke expected post-contact service-halt surface status")) {
        return false;
    }
    if (!Check(serviceReadout.nextPayoff == "Take the first service/rest halt.",
            "route readout smoke expected first service as the next payoff")) {
        return false;
    }

    profile.firstPlayableRoute.firstServicePerformed = true;
    profile.story.relayRecovered = true;
    profile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    profile.firstPlayableRoute.debriefSummaryViewed = true;
    if (!Check(bunker::FindWorldFieldState(profile, profile.selectedWorld, true) != nullptr,
            "route readout smoke expected selected world state for industrial handoff")) {
        return false;
    }
    const auto industrialReadout = bunker::BuildFirstPlayableRouteReadout(profile);
    return Check(industrialReadout.completedSteps == 14 && industrialReadout.totalSteps == 14,
            "route readout smoke expected full slice progress after debrief") &&
        Check(industrialReadout.surfaceStatus == "route closed, industrial handoff live",
            "route readout smoke expected closed-route surface status after debrief") &&
        Check(industrialReadout.nextPayoff.find("rail freight") != std::string::npos,
            "route readout smoke expected industrial handoff to point at rail freight");
}

bool RunSurfaceArrivalWorldEventSmoke() {
    bunker::World world;

    bunker::MapObject cryo;
    cryo.registryId = "[%cryo_0001]";
    cryo.displayName = "Cryo Bay";
    world.AddObject(cryo);

    bunker::MapObject pipPad;
    pipPad.registryId = "[%pip_0001]";
    pipPad.displayName = "Pip-Pad Locker";
    world.AddObject(pipPad);

    bunker::MapObject archive;
    archive.registryId = "[%archive_0001]";
    archive.displayName = "Archive Terminal";
    world.AddObject(archive);

    bunker::MapObject hull;
    hull.registryId = "[#tr_hull_0001]";
    hull.displayName = "BT-72 Hull";
    world.AddObject(hull);

    bunker::MapObject camp;
    camp.registryId = "[%camp_0001]";
    camp.displayName = "Forward Camp";
    camp.x = 20.0f;
    camp.y = 4.0f;
    world.AddObject(camp);

    if (!Check(world.IsStarterScenarioWorld(), "surface arrival smoke expected starter scenario world")) {
        return false;
    }

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;

    bunker::PlayerState player;
    player.x = camp.x;
    player.y = camp.y;

    bunker::GameState gameState;
    bunker::ProcessScriptedWorldEvents(world, player, profile, gameState);

    return Check(profile.firstPlayableRoute.surfaceArrivalReached, "surface arrival smoke expected route flag to flip at camp anchor") &&
        Check(gameState.zoneEventExterior, "surface arrival smoke expected exterior world event to trigger") &&
        Check(gameState.lastEvent.find("SURFACE ARRIVAL") != std::string::npos,
            "surface arrival smoke expected exterior event copy") &&
        Check(gameState.lastEvent.find("first exterior foothold") != std::string::npos,
            "surface arrival smoke expected foothold payoff text") &&
        Check(gameState.lastEvent.find("Next: Use the clearance module to break the outer debris barrier.") != std::string::npos,
            "surface arrival smoke expected explicit next-objective handoff") &&
        Check(gameState.lastEvent.find("Route beat: Exterior Exposure") != std::string::npos,
            "surface arrival smoke expected exterior-exposure beat copy");
}

bool RunFirstCombatWorldEventSmoke() {
    bunker::World world;
    world.GeneratePrototypeZone();

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;

    bunker::PlayerState player;
    player.insideTank = true;
    if (const auto* ghoul = world.FindObjectByRegistryId("[%enemy_ghoul_0001]"); ghoul != nullptr) {
        player.x = ghoul->x;
        player.y = ghoul->y;
    } else {
        return Check(false, "first combat smoke expected starter ghoul anchor");
    }

    bunker::GameState gameState;
    bunker::ProcessScriptedWorldEvents(world, player, profile, gameState);

    return Check(gameState.zoneEventFirstCombat, "first combat smoke expected first-contact event to trigger") &&
        Check(gameState.lastEvent.find("CONTACT:") != std::string::npos,
            "first combat smoke expected contact event copy") &&
        Check(gameState.lastEvent.find("service halt") != std::string::npos,
            "first combat smoke expected service-halt cue in contact event") &&
        Check(gameState.lastEvent.find("Route beat: First Contact") != std::string::npos,
            "first combat smoke expected first-contact route beat");
}

bool RunFirstCombatResolutionHandoffSmoke() {
    bunker::World world;

    bunker::MapObject ghoul;
    ghoul.registryId = "[%enemy_ghoul_0001]";
    ghoul.displayName = "Outer Ghoul";
    ghoul.interaction = bunker::InteractionType::Hostile;
    ghoul.category = bunker::ObjectCategory::Hostile;
    ghoul.health = 1.0f;
    world.AddObject(ghoul);

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.selectedWorld = "first_contact_handoff_smoke.bwld";
    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.partnerTank.deployed = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;

    bunker::PlayerState player;
    player.insideTank = true;
    player.x = ghoul.x;
    player.y = ghoul.y;

    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    bunker::HandleAttack(world, player, profile, staticEraser, gameState);

    return Check(profile.firstPlayableRoute.firstTankCombatResolved,
            "first combat handoff smoke expected combat-resolution flag") &&
        Check(gameState.lastEvent.find("Service halt") != std::string::npos,
            "first combat handoff smoke expected service-halt resolution cue") &&
        Check(gameState.lastEvent.find("Run a first service cycle before pushing the relay node.") != std::string::npos,
            "first combat handoff smoke expected first-service runtime objective hint") &&
        Check(gameState.lastEvent.find("Route beat: Service Halt") != std::string::npos,
            "first combat handoff smoke expected service-halt route beat");
}

bool RunWorkshopServiceRouteHandoffSmoke() {
    bunker::World world;
    world.GeneratePrototypeZone();
    const bunker::MapObject* workshop = world.FindObjectByRegistryId("[%workshop_0001]");
    if (!Check(workshop != nullptr, "workshop handoff smoke expected workshop anchor")) {
        return false;
    }

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.selectedWorld = "service_handoff_smoke.bwld";
    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;
    profile.firstPlayableRoute.firstTankCombatResolved = true;
    profile.partnerTank.worldPositionKnown = true;
    profile.partnerTank.worldX = workshop->x;
    profile.partnerTank.worldY = workshop->y;
    profile.partnerTank.damage.hull = 82.0f;
    profile.partnerTank.energyReserve = 100.0f;
    profile.partnerTank.ammoReserve = 100.0f;
    profile.character.inventory.push_back({"repair_patch", 1, 0.2f});

    bunker::PlayerState player;
    player.insideTank = true;

    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    bunker::HandleInteraction(workshop, world, player, profile, staticEraser, gameState);

    return Check(profile.firstPlayableRoute.firstServicePerformed, "workshop handoff smoke expected first service flag") &&
        Check(gameState.lastEvent.find("First service halt logged") != std::string::npos,
            "workshop handoff smoke expected first service log copy") &&
        Check(gameState.lastEvent.find("recovery node") != std::string::npos,
            "workshop handoff smoke expected relay handoff cue") &&
        Check(gameState.lastEvent.find("Route beat: Recovery Sync") != std::string::npos,
            "workshop handoff smoke expected recovery-sync beat cue");
}

bool RunScalableContainerLootRuntimeSmoke() {
    auto inventoryCount = [](const bunker::SessionProfile& profile, std::string_view itemId) {
        const auto it = std::find_if(
            profile.character.inventory.begin(),
            profile.character.inventory.end(),
            [&](const bunker::InventoryEntry& entry) { return entry.itemId == itemId; });
        return it == profile.character.inventory.end() ? 0 : it->count;
    };

    bunker::World manualWorld;
    bunker::MapObject manualChest;
    manualChest.registryId = "[%manual_loot_chest_smoke]";
    manualChest.displayName = "Manual Loot Chest Smoke";
    manualChest.interaction = bunker::InteractionType::Container;
    manualChest.category = bunker::ObjectCategory::Container;
    manualChest.lootMode = bunker::LootMode::ManualList;
    manualChest.manualLoot = true;
    manualChest.lootEntries = {
        {"manual_smoke_a", 2, 2, 1.0f},
        {"manual_smoke_b", 3, 3, 1.0f},
    };
    manualWorld.AddObject(manualChest);

    bunker::SessionProfile manualProfile = bunker::MakeDefaultSessionProfile();
    manualProfile.selectedWorld = "manual_loot_runtime_smoke.bwld";
    bunker::PlayerState player;
    bunker::StaticEraser manualEraser;
    bunker::GameState manualState;
    const auto* manualNearest = manualWorld.FindObjectByRegistryId(manualChest.registryId);
    if (!Check(manualNearest != nullptr, "scalable container smoke expected manual chest before interaction")) {
        return false;
    }
    bunker::HandleInteraction(manualNearest, manualWorld, player, manualProfile, manualEraser, manualState);
    const int manualACountAfterFirstOpen = inventoryCount(manualProfile, "manual_smoke_a");
    const int manualBCountAfterFirstOpen = inventoryCount(manualProfile, "manual_smoke_b");
    const auto* manualSecondOpen = manualWorld.FindObjectByRegistryId(manualChest.registryId);
    if (manualSecondOpen != nullptr) {
        bunker::HandleInteraction(manualSecondOpen, manualWorld, player, manualProfile, manualEraser, manualState);
    }

    bunker::World randomWorld;
    bunker::MapObject randomChest;
    randomChest.registryId = "[%random_loot_chest_smoke]";
    randomChest.displayName = "Random Loot Chest Smoke";
    randomChest.interaction = bunker::InteractionType::Container;
    randomChest.category = bunker::ObjectCategory::Container;
    randomChest.lootMode = bunker::LootMode::RandomTable;
    randomChest.manualLoot = true;
    randomChest.lootEntries = {
        {"random_smoke_a", 4, 4, 1.0f},
        {"random_smoke_b", 8, 8, 0.0f},
    };
    randomWorld.AddObject(randomChest);

    bunker::SessionProfile randomProfile = bunker::MakeDefaultSessionProfile();
    randomProfile.selectedWorld = "random_loot_runtime_smoke.bwld";
    bunker::StaticEraser randomEraser;
    bunker::GameState randomState;
    const auto* randomNearest = randomWorld.FindObjectByRegistryId(randomChest.registryId);
    if (!Check(randomNearest != nullptr, "scalable container smoke expected random chest before interaction")) {
        return false;
    }
    bunker::HandleInteraction(randomNearest, randomWorld, player, randomProfile, randomEraser, randomState);

    return Check(manualACountAfterFirstOpen == 2, "manual filled chest should grant first scalable loot entry count") &&
        Check(manualBCountAfterFirstOpen == 3, "manual filled chest should grant second scalable loot entry count") &&
        Check(manualWorld.FindObjectByRegistryId(manualChest.registryId) == nullptr, "manual filled chest should be removed after looting") &&
        Check(manualEraser.IsErased(manualChest.registryId), "manual filled chest should be persisted in static eraser after looting") &&
        Check(inventoryCount(manualProfile, "manual_smoke_a") == manualACountAfterFirstOpen,
            "manual filled chest should not refill after erased world object is gone") &&
        Check(inventoryCount(manualProfile, "manual_smoke_b") == manualBCountAfterFirstOpen,
            "manual filled chest should not duplicate second entry after erased world object is gone") &&
        Check(inventoryCount(randomProfile, "random_smoke_a") == 4,
            "random loot chest should grant the single positive weighted entry") &&
        Check(inventoryCount(randomProfile, "random_smoke_b") == 0,
            "random loot chest should skip zero-weight random entries") &&
        Check(randomWorld.FindObjectByRegistryId(randomChest.registryId) == nullptr, "random loot chest should be removed after looting") &&
        Check(randomEraser.IsErased(randomChest.registryId), "random loot chest should be persisted in static eraser after looting");
}

bool RunRuntimeProfileSaveDoesNotReplaceAuthoringWorldSmoke() {
    bunker::World authoredWorld;
    authoredWorld.metadata.name = "Runtime Profile Save Boundary Smoke";
    authoredWorld.metadata.objective = "Runtime saves must not replace authoring .bwld records.";

    bunker::MapObject chest;
    chest.registryId = "[%runtime_save_boundary_chest]";
    chest.displayName = "Runtime Save Boundary Chest";
    chest.interaction = bunker::InteractionType::Container;
    chest.category = bunker::ObjectCategory::Container;
    chest.manualLoot = true;
    chest.lootMode = bunker::LootMode::ManualList;
    chest.lootEntries = {
        {"runtime_boundary_loot", 1, 1, 1.0f},
    };
    authoredWorld.AddObject(chest);

    const auto saveStatus = bunker::SaveWorldAtomically(authoredWorld, bunker::DefaultWorldPath());
    if (!Check(saveStatus.ok, "runtime profile save boundary smoke failed to save authoring world: " + saveStatus.message)) {
        return false;
    }

    bunker::World runtimeWorld;
    if (!Check(runtimeWorld.Load(bunker::DefaultWorldPath().string()), "runtime profile save boundary smoke failed to load runtime world copy")) {
        return false;
    }

    bunker::SessionProfile runtimeProfile = bunker::MakeDefaultSessionProfile();
    runtimeProfile.selectedWorld = bunker::DefaultWorldPath().filename().generic_string();
    bunker::PlayerState player;
    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    const auto* runtimeChest = runtimeWorld.FindObjectByRegistryId(chest.registryId);
    if (!Check(runtimeChest != nullptr, "runtime profile save boundary smoke expected runtime chest before interaction")) {
        return false;
    }

    bunker::HandleInteraction(runtimeChest, runtimeWorld, player, runtimeProfile, staticEraser, gameState);
    const auto profileSave = bunker::SaveProfileAtomically(runtimeProfile, bunker::DefaultSessionProfilePath());
    if (!Check(profileSave.ok, "runtime profile save boundary smoke failed to save runtime profile: " + profileSave.message)) {
        return false;
    }
    staticEraser.Save(runtimeProfile.selectedWorld);

    bunker::World reloadedAuthoringWorld;
    if (!Check(reloadedAuthoringWorld.Load(bunker::DefaultWorldPath().string()), "runtime profile save boundary smoke failed to reload authoring .bwld")) {
        return false;
    }

    const auto* reloadedChest = reloadedAuthoringWorld.FindObjectByRegistryId(chest.registryId);
    return Check(runtimeWorld.FindObjectByRegistryId(chest.registryId) == nullptr,
            "runtime profile save boundary smoke expected runtime copy to remove looted chest") &&
        Check(staticEraser.IsErased(chest.registryId),
            "runtime profile save boundary smoke expected static eraser to hold runtime removal state") &&
        Check(reloadedChest != nullptr,
            "runtime profile save boundary smoke expected authoring .bwld to keep placed chest record") &&
        Check(reloadedChest->lootEntries.size() == 1 && reloadedChest->lootEntries[0].itemId == "runtime_boundary_loot",
            "runtime profile save boundary smoke expected authoring .bwld to preserve scalable loot");
}

bool RunDebriefIndustrialHandoffSmoke() {
    bunker::World world;
    bunker::MapObject debrief;
    debrief.registryId = "[%debrief_0001]";
    debrief.displayName = "Shelter 17 Debrief Console";

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.selectedWorld = "debrief_handoff_smoke.bwld";
    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;
    profile.firstPlayableRoute.firstTankCombatResolved = true;
    profile.firstPlayableRoute.firstServicePerformed = true;
    profile.story.relayRecovered = true;
    profile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    if (!Check(bunker::FindWorldFieldState(profile, profile.selectedWorld, true) != nullptr,
            "debrief handoff smoke expected world field state")) {
        return false;
    }

    bunker::PlayerState player;
    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    bunker::HandleInteraction(&debrief, world, player, profile, staticEraser, gameState);

    return Check(profile.story.returnedToBase, "debrief handoff smoke expected returned-to-base flag") &&
        Check(profile.firstPlayableRoute.debriefSummaryViewed, "debrief handoff smoke expected debrief flag") &&
        Check(gameState.lastEvent.find("Next:") != std::string::npos,
            "debrief handoff smoke expected explicit next-objective cue") &&
        Check(gameState.lastEvent.find("rail depot") != std::string::npos,
            "debrief handoff smoke expected industrial rail-depot handoff") &&
        Check(gameState.lastEvent.find("Backbone stage: Starter Backbone") != std::string::npos,
            "debrief handoff smoke expected starter-backbone readability cue") &&
        Check(gameState.lastEvent.find("Route beat: Industrial Handoff") != std::string::npos,
            "debrief handoff smoke expected industrial-handoff beat cue");
}

bool RunRecoveryHandoffSummarySmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.selectedWorld = "recovery_handoff_summary_smoke.bwld";
    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;
    profile.firstPlayableRoute.firstTankCombatResolved = true;
    profile.firstPlayableRoute.firstServicePerformed = true;
    profile.story.relayRecovered = true;
    profile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    profile.story.returnedToBase = true;
    profile.firstPlayableRoute.debriefSummaryViewed = true;
    profile.fieldCheckpointKnown = true;
    profile.fieldCheckpointWorld = profile.selectedWorld;
    auto* worldState = bunker::FindWorldFieldState(profile, profile.selectedWorld, true);
    if (!Check(worldState != nullptr, "recovery handoff smoke expected world field state")) {
        return false;
    }

    if (!Check(bunker::CurrentRecoveryHandoffSummary(profile).find("rail freight") != std::string::npos,
            "recovery handoff smoke expected rail-freight first step")) {
        return false;
    }

    worldState->towerSyncRecovered = true;
    worldState->localRelayAvailable = true;
    worldState->regionalGridOnline = true;
    worldState->caravanRouteActive = true;
    worldState->railFreightActive = true;
    profile.character.collectedTapes.push_back({"grid_pylon_01", "Pylon 01", false, false, false});
    if (!Check(bunker::CurrentRecoveryHandoffSummary(profile).find("orbital uplink") != std::string::npos,
            "recovery handoff smoke expected orbital uplink after freight")) {
        return false;
    }

    worldState->tradeNetworkActive = true;
    worldState->orbitalUplinkActive = true;
    profile.character.collectedTapes.push_back({"grid_pylon_02", "Pylon 02", false, false, false});
    if (!Check(bunker::CurrentRecoveryHandoffSummary(profile).find("Rail Fortress") != std::string::npos,
            "recovery handoff smoke expected Rail Fortress after orbital uplink")) {
        return false;
    }

    worldState->railFortressActive = true;
    return Check(bunker::CurrentRecoveryHandoffSummary(profile).find("Recovery Fabricator") != std::string::npos,
        "recovery handoff smoke expected fabricator after Rail Fortress");
}

bool RunRecoveryBackboneStatusSmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.selectedWorld = "recovery_backbone_status_smoke.bwld";

    const auto lockedStatus = bunker::CurrentRecoveryBackboneStatus(profile);
    if (!Check(lockedStatus.stage == "Route Locked",
            "recovery backbone smoke expected locked stage before relay sync")) {
        return false;
    }

    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;
    profile.firstPlayableRoute.firstTankCombatResolved = true;
    profile.firstPlayableRoute.firstServicePerformed = true;
    profile.story.relayRecovered = true;
    profile.firstPlayableRoute.firstRecoveryNodeActivated = true;

    const auto pendingStatus = bunker::CurrentRecoveryBackboneStatus(profile);
    if (!Check(pendingStatus.stage == "Debrief Pending",
            "recovery backbone smoke expected debrief-pending stage before debrief")) {
        return false;
    }

    profile.story.returnedToBase = true;
    profile.firstPlayableRoute.debriefSummaryViewed = true;
    profile.fieldCheckpointKnown = true;
    profile.fieldCheckpointWorld = profile.selectedWorld;
    auto* worldState = bunker::FindWorldFieldState(profile, profile.selectedWorld, true);
    if (!Check(worldState != nullptr, "recovery backbone smoke expected world field state")) {
        return false;
    }

    const auto starterStatus = bunker::CurrentRecoveryBackboneStatus(profile);
    if (!Check(starterStatus.stage == "Starter Backbone",
            "recovery backbone smoke expected starter stage after debrief")) {
        return false;
    }
    if (!Check(starterStatus.status.find("0/4") != std::string::npos &&
               starterStatus.status.find("rail freight") != std::string::npos,
            "recovery backbone smoke expected starter status to point at rail freight: " + starterStatus.status)) {
        return false;
    }

    worldState->towerSyncRecovered = true;
    worldState->localRelayAvailable = true;
    worldState->regionalGridOnline = true;
    worldState->caravanRouteActive = true;
    worldState->railFreightActive = true;
    profile.character.collectedTapes.push_back({"grid_pylon_01", "Pylon 01", false, false, false});
    worldState->tradeNetworkActive = true;
    worldState->orbitalUplinkActive = true;
    profile.character.collectedTapes.push_back({"grid_pylon_02", "Pylon 02", false, false, false});

    const auto freightStatus = bunker::CurrentRecoveryBackboneStatus(profile);
    if (!Check(freightStatus.stage == "Starter Backbone",
            "recovery backbone smoke expected starter stage during starter logistics")) {
        return false;
    }
    if (!Check(freightStatus.status.find("2/4") != std::string::npos &&
               freightStatus.status.find("Rail Fortress") != std::string::npos,
            "recovery backbone smoke expected starter status to point at Rail Fortress: " + freightStatus.status)) {
        return false;
    }
    if (!Check(freightStatus.payoff.find("restored spur still lacks hardened control") != std::string::npos,
            "recovery backbone smoke expected starter payoff to explain missing hardened control: " + freightStatus.payoff)) {
        return false;
    }

    worldState->railFortressActive = true;
    worldState->recoveryFabricatorActive = true;
    worldState->industrialGateUnlocked = true;
    worldState->industrialSurveyActive = true;
    worldState->industrialOutpostActive = true;
    worldState->assemblyCellActive = true;

    const auto innerSpurStatus = bunker::CurrentRecoveryBackboneStatus(profile);
    if (!Check(innerSpurStatus.stage == "Inner Spur Expansion",
            "recovery backbone smoke expected inner-spur stage after gate unlock")) {
        return false;
    }
    if (!Check(innerSpurStatus.status.find("4/10") != std::string::npos &&
               innerSpurStatus.status.find("foundry line") != std::string::npos,
            "recovery backbone smoke expected inner-spur status to point at foundry line: " + innerSpurStatus.status)) {
        return false;
    }
    if (!Check(innerSpurStatus.payoff.find("heavy plate output is still dark") != std::string::npos,
            "recovery backbone smoke expected inner-spur payoff to explain foundry gap: " + innerSpurStatus.payoff)) {
        return false;
    }

    worldState->foundryLineActive = true;
    worldState->reactorYardActive = true;
    worldState->capacitorBankActive = true;
    worldState->relaySubstationActive = true;
    worldState->serviceBayActive = true;
    worldState->waterReclaimerActive = true;

    const auto stableStatus = bunker::CurrentRecoveryBackboneStatus(profile);
    return Check(stableStatus.stage == "Backbone Stable",
            "recovery backbone smoke expected stable stage after full backbone") &&
        Check(stableStatus.status.find("14/14") != std::string::npos,
            "recovery backbone smoke expected stable status count after full backbone: " + stableStatus.status) &&
        Check(stableStatus.payoff.find("stable recovery backbone") != std::string::npos,
            "recovery backbone smoke expected stable payoff after full backbone: " + stableStatus.payoff);
}

bool RunRouteEventLifecycleSmoke() {
    bunker::SessionProfile lockedProfile = bunker::MakeDefaultSessionProfile();
    lockedProfile.selectedWorld = "route_event_locked_smoke.bwld";
    auto* lockedWorldState = bunker::FindWorldFieldState(lockedProfile, lockedProfile.selectedWorld, true);
    if (!Check(lockedWorldState != nullptr, "route event smoke expected locked world field state")) {
        return false;
    }
    bunker::GameState lockedState;
    bunker::UpdateRouteRandomEvents(lockedProfile, lockedState, 1.0f);
    if (!Check(!bunker::HasActiveRouteEvent(*lockedWorldState),
            "route event smoke expected layer to stay locked before onboarding and debrief")) {
        return false;
    }
    if (!Check(bunker::ActiveRouteEventSummary(lockedProfile).find("locked before onboarding") != std::string::npos,
            "route event smoke expected locked summary before onboarding")) {
        return false;
    }

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.selectedWorld = "route_event_lifecycle_smoke.bwld";
    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;
    profile.firstPlayableRoute.firstTankCombatResolved = true;
    profile.firstPlayableRoute.firstServicePerformed = true;
    profile.story.relayRecovered = true;
    profile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    profile.story.returnedToBase = true;
    profile.firstPlayableRoute.debriefSummaryViewed = true;
    profile.partnerTank.damage.hull = 58.0f;
    profile.partnerTank.energyReserve = 78.0f;
    profile.partnerTank.ammoReserve = 72.0f;
    auto* worldState = bunker::FindWorldFieldState(profile, profile.selectedWorld, true);
    if (!Check(worldState != nullptr, "route event smoke expected world field state")) {
        return false;
    }

    bunker::GameState gameState;
    bunker::UpdateRouteRandomEvents(profile, gameState, 1.0f);
    if (!Check(worldState->activeRouteEventType == "service_call",
            "route event smoke expected deterministic service-call spawn")) {
        return false;
    }
    if (!Check(worldState->routeEventOfferTimeRemaining > 0.0f,
            "route event smoke expected offered state before activation")) {
        return false;
    }
    if (!Check(bunker::ActiveRouteEventSummary(profile).find("offered") != std::string::npos,
            "route event smoke expected offered route-event summary")) {
        return false;
    }

    bunker::UpdateRouteRandomEvents(profile, gameState, 20.0f);
    if (!Check(worldState->routeEventOfferTimeRemaining <= 0.0f,
            "route event smoke expected offer timer to collapse into active state")) {
        return false;
    }
    if (!Check(bunker::ActiveRouteEventSummary(profile).find("active") != std::string::npos,
            "route event smoke expected active route-event summary after offer")) {
        return false;
    }

    profile.character.awakening.fieldServiceUses = 1;
    profile.partnerTank.damage.hull = 84.0f;
    profile.partnerTank.energyReserve = 81.0f;
    profile.partnerTank.ammoReserve = 77.0f;
    bunker::UpdateRouteRandomEvents(profile, gameState, 1.0f);

    return Check(!bunker::HasActiveRouteEvent(*worldState), "route event smoke expected service-call resolution") &&
        Check(worldState->routeEventsResolved == 1, "route event smoke expected resolved counter increment") &&
        Check(worldState->routeEventCooldown > 0.0f, "route event smoke expected post-resolution cooldown") &&
        Check(gameState.lastEvent.find("ROUTE EVENT RESOLVED") != std::string::npos,
            "route event smoke expected resolution event copy") &&
        Check(bunker::ActiveRouteEventSummary(profile).find("success") != std::string::npos &&
                bunker::ActiveRouteEventSummary(profile).find("cooldown") != std::string::npos,
            "route event smoke expected success-plus-cooldown summary after resolution");
}

bool RunMerchantRouteEventSmoke() {
    auto countInventory = [](const bunker::SessionProfile& profile, const std::string& itemId) {
        int total = 0;
        for (const auto& entry : profile.character.inventory) {
            if (entry.itemId == itemId) {
                total += entry.count;
            }
        }
        return total;
    };

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.selectedWorld = "merchant_route_event_smoke.bwld";
    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;
    profile.firstPlayableRoute.firstTankCombatResolved = true;
    profile.firstPlayableRoute.firstServicePerformed = true;
    profile.story.relayRecovered = true;
    profile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    profile.story.returnedToBase = true;
    profile.firstPlayableRoute.debriefSummaryViewed = true;
    profile.lanlineServices.relayCredits = 150;
    profile.character.inventory.push_back({"trade_voucher", 1, 0.0f});
    profile.character.inventory.push_back({"power_cell", 1, 0.3f});
    profile.partnerTank.damage.hull = 100.0f;
    profile.partnerTank.energyReserve = 94.0f;
    profile.partnerTank.ammoReserve = 92.0f;

    auto* worldState = bunker::FindWorldFieldState(profile, profile.selectedWorld, true);
    if (!Check(worldState != nullptr, "merchant route event smoke expected world field state")) {
        return false;
    }
    worldState->tradeNetworkActive = true;
    worldState->serviceBayActive = true;
    worldState->serviceCyclesCompleted = 1;
    worldState->localRelayAvailable = true;
    worldState->regionalGridOnline = true;
    worldState->routeContamination = 4.0f;
    worldState->infrastructureDecay = 6.0f;
    worldState->routeOverrun = false;

    bunker::GameState gameState;
    bunker::UpdateRouteRandomEvents(profile, gameState, 1.0f);

    const int vouchersBefore = countInventory(profile, "trade_voucher");
    if (!Check(worldState->activeRouteEventType == "merchant_window",
            "merchant route event smoke expected merchant-window spawn")) {
        return false;
    }
    if (!Check(bunker::ActiveRouteEventSummary(profile).find("Merchant window offered") != std::string::npos,
            "merchant route event smoke expected offered merchant summary")) {
        return false;
    }
    if (!Check(bunker::TryResolveMerchantRouteEvent(profile, gameState),
            "merchant route event smoke expected merchant exchange resolution")) {
        return false;
    }

    return Check(!bunker::HasActiveRouteEvent(*worldState), "merchant route event smoke expected exchange to close the window") &&
        Check(worldState->routeEventsResolved == 1, "merchant route event smoke expected resolved counter increment") &&
        Check(countInventory(profile, "trade_voucher") == vouchersBefore - 1,
            "merchant route event smoke expected voucher to be spent first") &&
        Check(gameState.lastEvent.find("merchant window") != std::string::npos ||
                gameState.lastEvent.find("broker trace") != std::string::npos,
            "merchant route event smoke expected merchant resolution copy") &&
        Check(bunker::ActiveRouteEventSummary(profile).find("success") != std::string::npos,
            "merchant route event smoke expected merchant success summary with cooldown");
}

bool RunWeatherWeightedRouteEventSmoke() {
    bunker::SessionProfile acidProfile = bunker::MakeDefaultSessionProfile();
    acidProfile.selectedWorld = "weather_weighted_route_event_acid_smoke.bwld";
    acidProfile.story.awakenedFromCryo = true;
    acidProfile.story.pipPadRecovered = true;
    acidProfile.story.archiveRecovered = true;
    acidProfile.firstPlayableRoute.bt72Restored = true;
    acidProfile.story.tankLinked = true;
    acidProfile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    acidProfile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    acidProfile.firstPlayableRoute.clearanceModuleInstalled = true;
    acidProfile.story.exitedBunker = true;
    acidProfile.firstPlayableRoute.surfaceArrivalReached = true;
    acidProfile.story.outerRoadCleared = true;
    acidProfile.firstPlayableRoute.firstTankCombatResolved = true;
    acidProfile.firstPlayableRoute.firstServicePerformed = true;
    acidProfile.story.relayRecovered = true;
    acidProfile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    acidProfile.story.returnedToBase = true;
    acidProfile.firstPlayableRoute.debriefSummaryViewed = true;
    acidProfile.lanlineServices.relayCredits = 150;
    acidProfile.character.inventory.push_back({"trade_voucher", 1, 0.0f});
    acidProfile.character.inventory.push_back({"power_cell", 1, 0.3f});
    acidProfile.partnerTank.damage.hull = 54.0f;
    acidProfile.partnerTank.energyReserve = 94.0f;
    acidProfile.partnerTank.ammoReserve = 91.0f;

    auto* acidWorld = bunker::FindWorldFieldState(acidProfile, acidProfile.selectedWorld, true);
    if (!Check(acidWorld != nullptr, "weather-weighted route event smoke expected acid world field state")) {
        return false;
    }
    acidWorld->tradeNetworkActive = true;
    acidWorld->serviceCyclesCompleted = 0;
    acidWorld->routeContamination = 4.0f;
    acidWorld->infrastructureDecay = 8.0f;
    acidWorld->routeEventSerial = 5;

    bunker::GameState acidState;
    acidState.weather = bunker::WeatherAnomaly::AcidRain;
    acidState.weatherIntensity = 0.85f;
    bunker::UpdateRouteRandomEvents(acidProfile, acidState, 1.0f);

    if (!Check(acidWorld->activeRouteEventType == "service_call",
            "weather-weighted route event smoke expected acid rain to bias into service_call instead of merchant")) {
        return false;
    }
    if (!Check(acidState.lastEvent.find("Acid rain") != std::string::npos,
            "weather-weighted route event smoke expected acid-rain offer copy")) {
        return false;
    }

    bunker::SessionProfile fogProfile = bunker::MakeDefaultSessionProfile();
    fogProfile.selectedWorld = "weather_weighted_route_event_fog_smoke.bwld";
    fogProfile.story.awakenedFromCryo = true;
    fogProfile.story.pipPadRecovered = true;
    fogProfile.story.archiveRecovered = true;
    fogProfile.firstPlayableRoute.bt72Restored = true;
    fogProfile.story.tankLinked = true;
    fogProfile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    fogProfile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    fogProfile.firstPlayableRoute.clearanceModuleInstalled = true;
    fogProfile.story.exitedBunker = true;
    fogProfile.firstPlayableRoute.surfaceArrivalReached = true;
    fogProfile.story.outerRoadCleared = true;
    fogProfile.firstPlayableRoute.firstTankCombatResolved = true;
    fogProfile.firstPlayableRoute.firstServicePerformed = true;
    fogProfile.story.relayRecovered = true;
    fogProfile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    fogProfile.story.returnedToBase = true;
    fogProfile.firstPlayableRoute.debriefSummaryViewed = true;
    fogProfile.lanlineServices.relayCredits = 150;
    fogProfile.character.inventory.push_back({"trade_voucher", 1, 0.0f});
    fogProfile.character.inventory.push_back({"power_cell", 1, 0.3f});
    fogProfile.partnerTank.damage.hull = 100.0f;
    fogProfile.partnerTank.energyReserve = 94.0f;
    fogProfile.partnerTank.ammoReserve = 92.0f;

    auto* fogWorld = bunker::FindWorldFieldState(fogProfile, fogProfile.selectedWorld, true);
    if (!Check(fogWorld != nullptr, "weather-weighted route event smoke expected fog world field state")) {
        return false;
    }
    fogWorld->tradeNetworkActive = true;
    fogWorld->serviceBayActive = true;
    fogWorld->serviceCyclesCompleted = 1;
    fogWorld->routeContamination = 4.0f;
    fogWorld->infrastructureDecay = 7.0f;
    fogWorld->localRelayAvailable = false;
    fogWorld->regionalGridOnline = false;
    fogWorld->routeEventSerial = 3;

    bunker::GameState fogState;
    fogState.weather = bunker::WeatherAnomaly::EtherFog;
    fogState.weatherIntensity = 0.9f;
    bunker::UpdateRouteRandomEvents(fogProfile, fogState, 1.0f);

    return Check(fogWorld->activeRouteEventType == "relay_instability",
            "weather-weighted route event smoke expected ether fog to bias into relay_instability instead of merchant; got " +
                fogWorld->activeRouteEventType) &&
        Check(fogState.lastEvent.find("Ether fog") != std::string::npos,
            "weather-weighted route event smoke expected ether-fog offer copy");
}

bool RunWorldScopedRouteSummarySmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.selectedWorld = "route_summary_alpha.bwld";
    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;
    profile.firstPlayableRoute.firstTankCombatResolved = true;
    profile.firstPlayableRoute.firstServicePerformed = true;
    profile.story.relayRecovered = true;
    profile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    profile.story.returnedToBase = true;
    profile.firstPlayableRoute.debriefSummaryViewed = true;
    profile.fieldCheckpointKnown = true;
    profile.fieldCheckpointWorld = profile.selectedWorld;
    profile.character.collectedTapes.push_back({"grid_pylon_alpha", "Grid Pylon Alpha", false, false, false});
    profile.character.collectedTapes.push_back({"grid_pylon_beta", "Grid Pylon Beta", false, false, false});

    auto* alphaWorld = bunker::FindWorldFieldState(profile, profile.selectedWorld, true);
    auto* betaWorld = bunker::FindWorldFieldState(profile, "route_summary_beta.bwld", true);
    alphaWorld = bunker::FindWorldFieldState(profile, profile.selectedWorld, true);
    betaWorld = bunker::FindWorldFieldState(profile, "route_summary_beta.bwld", true);
    if (!Check(alphaWorld != nullptr && betaWorld != nullptr,
            "world-scoped route summary smoke expected both world field states")) {
        return false;
    }

    alphaWorld->towerSyncRecovered = true;
    alphaWorld->localRelayAvailable = true;
    alphaWorld->regionalGridOnline = true;
    alphaWorld->caravanRouteActive = true;
    alphaWorld->tradeNetworkActive = true;
    alphaWorld->railFreightActive = true;
    alphaWorld->orbitalUplinkActive = true;
    alphaWorld->railFortressActive = true;
    alphaWorld->recoveryFabricatorActive = true;
    alphaWorld->industrialGateUnlocked = true;
    alphaWorld->industrialSurveyActive = true;
    alphaWorld->industrialOutpostActive = true;
    alphaWorld->assemblyCellActive = true;
    alphaWorld->foundryLineActive = true;
    alphaWorld->reactorYardActive = true;
    alphaWorld->capacitorBankActive = true;
    alphaWorld->relaySubstationActive = true;
    alphaWorld->serviceBayActive = true;
    alphaWorld->waterReclaimerActive = true;

    betaWorld->activeRouteEventType = "blocked_route";
    betaWorld->routeEventTimeRemaining = 33.0f;
    betaWorld->routeEventProgress = 1;
    betaWorld->routeEventStage = 1;

    const std::string alphaObjective = bunker::CurrentStoryObjectivePreview(profile);
    const std::string betaObjective = bunker::CurrentStoryObjectivePreview(profile, "route_summary_beta.bwld");
    const std::string alphaHandoff = bunker::CurrentRecoveryHandoffSummary(profile);
    const std::string betaHandoff = bunker::CurrentRecoveryHandoffSummary(profile, "route_summary_beta.bwld");
    const auto alphaBackbone = bunker::CurrentRecoveryBackboneStatus(profile);
    const auto betaBackbone = bunker::CurrentRecoveryBackboneStatus(profile, "route_summary_beta.bwld");
    const std::string alphaEvent = bunker::ActiveRouteEventSummary(profile);
    const std::string betaEvent = bunker::ActiveRouteEventSummary(profile, "route_summary_beta.bwld");

    return Check(alphaObjective.find("Water reclaimer online") != std::string::npos,
            "world-scoped route summary smoke expected selected-world objective to reflect deep industrial progress: " + alphaObjective) &&
        Check(betaObjective.find("rail depot") != std::string::npos,
            "world-scoped route summary smoke expected beta preview objective to stay at rail depot: " + betaObjective) &&
        Check(alphaHandoff.find("Handoff complete") != std::string::npos,
            "world-scoped route summary smoke expected selected-world handoff to reflect completed backbone: " + alphaHandoff) &&
        Check(betaHandoff.find("rail freight") != std::string::npos,
            "world-scoped route summary smoke expected beta handoff to point at rail freight: " + betaHandoff) &&
        Check(alphaBackbone.stage == "Backbone Stable" &&
                alphaBackbone.payoff.find("stable recovery backbone") != std::string::npos,
            "world-scoped route summary smoke expected selected-world backbone status to reflect stable backbone") &&
        Check(betaBackbone.stage == "Starter Backbone" &&
                betaBackbone.status.find("rail freight") != std::string::npos,
            "world-scoped route summary smoke expected beta backbone status to stay on rail freight: " + betaBackbone.status) &&
        Check(alphaEvent.find("unlocked") != std::string::npos &&
                alphaEvent.find("rare field prompt") != std::string::npos,
            "world-scoped route summary smoke expected selected-world event layer to be idle: " + alphaEvent) &&
        Check(betaEvent.find("Blocked route escalating") != std::string::npos,
            "world-scoped route summary smoke expected beta preview to show its blocked-route event");
}

bool RunHostileAwarenessSmoke() {
    auto almostEqual = [](float lhs, float rhs) {
        return std::fabs(lhs - rhs) < 0.001f;
    };

    bunker::World world;

    bunker::MapObject wall;
    wall.registryId = "[%wall_aware_0001]";
    wall.displayName = "Service Wall";
    wall.interaction = bunker::InteractionType::Static;
    wall.category = bunker::ObjectCategory::Structure;
    wall.x = 2.6f;
    wall.y = 0.0f;
    wall.width = 1.4f;
    wall.depth = 3.0f;
    world.AddObject(wall);

    bunker::MapObject ghoul;
    ghoul.registryId = "[%enemy_ghoul_awareness_0001]";
    ghoul.displayName = "Outer Ghoul";
    ghoul.interaction = bunker::InteractionType::Hostile;
    ghoul.category = bunker::ObjectCategory::Hostile;
    ghoul.x = 5.0f;
    ghoul.y = 0.0f;
    ghoul.width = 1.0f;
    ghoul.depth = 1.0f;
    ghoul.health = 55.0f;
    ghoul.scriptTag = "ghoul_rush";
    world.AddObject(ghoul);

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    bunker::PlayerState player;
    player.x = 0.0f;
    player.y = 0.0f;

    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    bunker::UpdateHostiles(world, player, profile, staticEraser, gameState, 1.0f);

    const auto* quietGhoul = world.FindObjectByRegistryId("[%enemy_ghoul_awareness_0001]");
    if (!Check(quietGhoul != nullptr, "hostile awareness smoke expected ghoul after quiet pass")) {
        return false;
    }
    if (!Check(almostEqual(quietGhoul->x, 5.0f) && almostEqual(quietGhoul->y, 0.0f),
            "hostile awareness smoke expected quiet player behind wall to avoid instant ghoul alert")) {
        return false;
    }

    player.insideTank = true;
    player.muzzleFlashTimer = 0.4f;
    player.muzzleFlashStrength = 1.0f;
    bunker::UpdateHostiles(world, player, profile, staticEraser, gameState, 1.0f);

    const auto* alertedGhoul = world.FindObjectByRegistryId("[%enemy_ghoul_awareness_0001]");
    if (!Check(alertedGhoul != nullptr, "hostile awareness smoke expected ghoul after noisy pass")) {
        return false;
    }

    const auto awarenessIt = std::find_if(
        gameState.hostileAwareness.begin(),
        gameState.hostileAwareness.end(),
        [](const bunker::HostileAwarenessState& state) { return state.registryId == "[%enemy_ghoul_awareness_0001]"; });
    return Check(awarenessIt != gameState.hostileAwareness.end() && awarenessIt->awareness >= 18.0f,
            "hostile awareness smoke expected noisy BT-72 cue to raise awareness") &&
        Check(!almostEqual(alertedGhoul->y, 0.0f),
            "hostile awareness smoke expected blocker-aware sidestep instead of magic wallhack rush");
}

bool RunHumanTriggerDisciplineSmoke() {
    auto almostEqual = [](float lhs, float rhs) {
        return std::fabs(lhs - rhs) < 0.001f;
    };

    bunker::World world;

    bunker::MapObject wall;
    wall.registryId = "[%wall_discipline_0001]";
    wall.displayName = "Concrete Divider";
    wall.interaction = bunker::InteractionType::Static;
    wall.category = bunker::ObjectCategory::Structure;
    wall.x = 2.5f;
    wall.y = 0.0f;
    wall.width = 1.3f;
    wall.depth = 2.8f;
    world.AddObject(wall);

    bunker::MapObject raider;
    raider.registryId = "[%enemy_raider_0001]";
    raider.displayName = "Raider Rifleman";
    raider.interaction = bunker::InteractionType::Hostile;
    raider.category = bunker::ObjectCategory::Hostile;
    raider.x = 5.0f;
    raider.y = 0.0f;
    raider.width = 1.0f;
    raider.depth = 1.0f;
    raider.health = 48.0f;
    raider.scriptTag = "human_tactical";
    world.AddObject(raider);

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    const float hpBefore = profile.character.hp;

    bunker::PlayerState player;
    player.x = 0.0f;
    player.y = 0.0f;

    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    gameState.hostileAwareness.push_back({"[%enemy_raider_0001]", 70.0f, 0.0f});
    bunker::UpdateHostiles(world, player, profile, staticEraser, gameState, 1.0f);

    const auto* updatedRaider = world.FindObjectByRegistryId("[%enemy_raider_0001]");
    return Check(updatedRaider != nullptr, "trigger discipline smoke expected raider after update") &&
        Check(almostEqual(profile.character.hp, hpBefore),
            "trigger discipline smoke expected raider to hold fire through blocked line") &&
        Check(!almostEqual(updatedRaider->y, 0.0f),
            "trigger discipline smoke expected raider to reposition instead of shooting through the wall");
}

bool RunHumanCoverSeekingSmoke() {
    bunker::World world;

    bunker::MapObject cover;
    cover.registryId = "[%cover_cargo_0001]";
    cover.displayName = "Cargo Bastion";
    cover.interaction = bunker::InteractionType::Static;
    cover.category = bunker::ObjectCategory::Structure;
    cover.x = 3.0f;
    cover.y = 0.8f;
    cover.width = 1.6f;
    cover.depth = 1.4f;
    world.AddObject(cover);

    bunker::MapObject raider;
    raider.registryId = "[%enemy_raider_cover_0001]";
    raider.displayName = "Raider Breacher";
    raider.interaction = bunker::InteractionType::Hostile;
    raider.category = bunker::ObjectCategory::Hostile;
    raider.x = 3.0f;
    raider.y = 2.6f;
    raider.width = 1.0f;
    raider.depth = 1.0f;
    raider.health = 16.0f;
    raider.scriptTag = "human_tactical";
    world.AddObject(raider);

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();

    bunker::PlayerState player;
    player.insideTank = true;
    player.x = 0.0f;
    player.y = 0.0f;

    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    gameState.hostileAwareness.push_back({"[%enemy_raider_cover_0001]", 72.0f, 0.0f});
    bunker::UpdateHostiles(world, player, profile, staticEraser, gameState, 1.0f);

    const auto* updatedRaider = world.FindObjectByRegistryId("[%enemy_raider_cover_0001]");
    const std::string positionText = updatedRaider == nullptr
        ? "missing"
        : ("x=" + std::to_string(updatedRaider->x) + ", y=" + std::to_string(updatedRaider->y));
    return Check(updatedRaider != nullptr, "human cover smoke expected raider after update") &&
        Check(updatedRaider->y < 2.3f,
            "human cover smoke expected wounded raider to drop toward cover instead of backing straight off the lane; got " + positionText) &&
        Check(updatedRaider->x > 3.3f,
            "human cover smoke expected wounded raider to tuck behind the cargo bastion; got " + positionText);
}

bool RunBt72CombatFeedbackSmoke() {
    bunker::World world;

    bunker::MapObject robot;
    robot.registryId = "[%enemy_robot_0001]";
    robot.displayName = "Sentinel Drone";
    robot.interaction = bunker::InteractionType::Hostile;
    robot.category = bunker::ObjectCategory::Hostile;
    robot.x = 4.2f;
    robot.y = 0.0f;
    robot.width = 1.1f;
    robot.depth = 1.1f;
    robot.health = 220.0f;
    robot.scriptTag = "robot_control";
    world.AddObject(robot);

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.bt72Restored = true;

    bunker::PlayerState player;
    player.insideTank = true;
    player.x = 0.0f;
    player.y = 0.0f;
    player.facingRadians = 0.0f;

    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    const float ammoBefore = profile.partnerTank.ammoReserve;
    const float energyBefore = profile.partnerTank.energyReserve;
    bunker::HandleSpecialAttack(world, player, profile, staticEraser, gameState);

    return Check(player.muzzleFlashTimer > 0.0f && player.muzzleFlashStrength > 0.0f,
            "bt72 combat feedback smoke expected muzzle flash feedback") &&
        Check(player.shockWaveTimer > 0.0f && player.shockWaveStrength > 0.0f,
            "bt72 combat feedback smoke expected shock wave feedback") &&
        Check(profile.partnerTank.ammoReserve < ammoBefore && profile.partnerTank.energyReserve < energyBefore,
            "bt72 combat feedback smoke expected heavy shot resource cost") &&
        Check(gameState.lastEvent.find("shock") != std::string::npos,
            "bt72 combat feedback smoke expected heavy-shot feedback copy");
}

bool RunReactiveBreakableGlassSmoke() {
    bunker::World world;

    bunker::MapObject glass;
    glass.registryId = "[%glass_break_smoke_0001]";
    glass.displayName = "Observation Glass";
    glass.interaction = bunker::InteractionType::Static;
    glass.category = bunker::ObjectCategory::Structure;
    glass.x = 2.4f;
    glass.y = 0.0f;
    glass.width = 1.4f;
    glass.depth = 0.5f;
    glass.health = 18.0f;
    glass.blocksMovement = true;
    world.AddObject(glass);

    bunker::MapObject robot;
    robot.registryId = "[%enemy_robot_glass_0001]";
    robot.displayName = "Sentinel Drone";
    robot.interaction = bunker::InteractionType::Hostile;
    robot.category = bunker::ObjectCategory::Hostile;
    robot.x = 5.0f;
    robot.y = 0.0f;
    robot.width = 1.1f;
    robot.depth = 1.1f;
    robot.health = 220.0f;
    robot.scriptTag = "robot_control";
    world.AddObject(robot);

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.bt72Restored = true;

    bunker::PlayerState player;
    player.insideTank = true;
    player.x = 0.0f;
    player.y = 0.0f;
    player.facingRadians = 0.0f;

    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    bunker::HandleSpecialAttack(world, player, profile, staticEraser, gameState);

    const auto* survivingRobot = world.FindObjectByRegistryId("[%enemy_robot_glass_0001]");
    return Check(player.muzzleFlashTimer > 0.0f && player.shockWaveTimer > 0.0f,
            "reactive glass smoke expected combat feedback to fire on glass impact") &&
        Check(world.FindObjectByRegistryId("[%glass_break_smoke_0001]") == nullptr,
            "reactive glass smoke expected shatterable glass to be removed") &&
        Check(staticEraser.IsErased("[%glass_break_smoke_0001]"),
            "reactive glass smoke expected shattered glass to persist in static eraser") &&
        Check(survivingRobot != nullptr && std::fabs(survivingRobot->health - 220.0f) < 0.001f,
            "reactive glass smoke expected the intercepting pane to absorb the first shot") &&
        Check(gameState.lastEvent.find("shattered") != std::string::npos &&
                gameState.lastEvent.find("lane") != std::string::npos,
            "reactive glass smoke expected readable shatter feedback copy");
}

bool RunReactiveBreakableFoliageSmoke() {
    bunker::World world;

    bunker::MapObject foliage;
    foliage.registryId = "[%brush_break_smoke_0001]";
    foliage.displayName = "Route Brush Cluster";
    foliage.interaction = bunker::InteractionType::Static;
    foliage.category = bunker::ObjectCategory::ResourceNode;
    foliage.x = 1.7f;
    foliage.y = 0.0f;
    foliage.width = 1.3f;
    foliage.depth = 1.0f;
    foliage.health = 10.0f;
    foliage.blocksMovement = false;
    world.AddObject(foliage);

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.bt72Restored = true;

    bunker::PlayerState player;
    player.insideTank = true;
    player.x = 0.0f;
    player.y = 0.0f;
    player.facingRadians = 0.0f;
    player.velocityX = 2.0f;

    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    bunker::HandleAttack(world, player, profile, staticEraser, gameState);

    return Check(world.FindObjectByRegistryId("[%brush_break_smoke_0001]") == nullptr,
            "reactive foliage smoke expected light vegetation to break under tank pressure") &&
        Check(staticEraser.IsErased("[%brush_break_smoke_0001]"),
            "reactive foliage smoke expected broken vegetation to persist in static eraser") &&
        Check(player.shockWaveTimer > 0.0f && player.shockWaveStrength > 0.0f,
            "reactive foliage smoke expected ram-style shock feedback on foliage break") &&
        Check(gameState.lastEvent.find("BT-72") != std::string::npos &&
                (gameState.lastEvent.find("brush") != std::string::npos ||
                 gameState.lastEvent.find("route edge") != std::string::npos),
            "reactive foliage smoke expected readable foliage-break feedback");
}

bool RunMechanicalHostileDamageSmoke() {
    auto makeWorld = []() {
        bunker::World world;
        bunker::MapObject robot;
        robot.registryId = "[%enemy_robot_modular_0001]";
        robot.displayName = "Sentinel Drone";
        robot.interaction = bunker::InteractionType::Hostile;
        robot.category = bunker::ObjectCategory::Hostile;
        robot.x = 4.0f;
        robot.y = 0.0f;
        robot.width = 1.1f;
        robot.depth = 1.1f;
        robot.health = 260.0f;
        robot.scriptTag = "robot_control";
        world.AddObject(robot);
        return world;
    };

    auto makeProfile = []() {
        bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
        profile.story.tankLinked = true;
        profile.firstPlayableRoute.bt72Restored = true;
        profile.partnerTank.secondSeatUnlocked = true;
        return profile;
    };

    bunker::StaticEraser staticEraser;

    bunker::SessionProfile baselineProfile = makeProfile();
    bunker::World baselineWorld = makeWorld();
    bunker::PlayerState baselinePlayer;
    baselinePlayer.insideTank = true;
    baselinePlayer.bt72GunnerSeat = true;
    baselinePlayer.x = 0.0f;
    baselinePlayer.y = 0.0f;
    baselinePlayer.facingRadians = 0.0f;
    bunker::GameState baselineState;
    baselineState.hostileAwareness.push_back({"[%enemy_robot_modular_0001]", 80.0f, 0.0f, 1.0f});
    const float baselineHullBefore = baselineProfile.partnerTank.damage.hull;
    bunker::UpdateHostiles(baselineWorld, baselinePlayer, baselineProfile, staticEraser, baselineState, 0.1f);
    const float baselineHullLoss = baselineHullBefore - baselineProfile.partnerTank.damage.hull;

    bunker::SessionProfile damagedProfile = makeProfile();
    bunker::World damagedWorld = makeWorld();
    bunker::PlayerState damagedPlayer = baselinePlayer;
    bunker::GameState damagedState;
    bunker::HandleSpecialAttack(damagedWorld, damagedPlayer, damagedProfile, staticEraser, damagedState);
    const auto* damagedRobot = damagedWorld.FindObjectByRegistryId("[%enemy_robot_modular_0001]");
    if (!Check(damagedRobot != nullptr, "mechanical damage smoke expected robot after special attack")) {
        return false;
    }

    const auto damagedIt = std::find_if(
        damagedState.mechanicalHostileDamage.begin(),
        damagedState.mechanicalHostileDamage.end(),
        [](const bunker::MechanicalHostileDamageState& state) {
            return state.registryId == "[%enemy_robot_modular_0001]";
        });
    if (!Check(damagedIt != damagedState.mechanicalHostileDamage.end(),
            "mechanical damage smoke expected robot modular state after special attack")) {
        return false;
    }

    const std::string impactEvent = damagedState.lastEvent;
    const std::string readability = bunker::DescribeHostileReadability(*damagedRobot, damagedState);
    damagedState.hostileAwareness.push_back({"[%enemy_robot_modular_0001]", 80.0f, 0.0f, 1.0f});
    const float damagedHullBefore = damagedProfile.partnerTank.damage.hull;
    bunker::UpdateHostiles(damagedWorld, damagedPlayer, damagedProfile, staticEraser, damagedState, 0.1f);
    const float damagedHullLoss = damagedHullBefore - damagedProfile.partnerTank.damage.hull;

    return Check(baselineHullLoss > 0.1f,
            "mechanical damage smoke expected intact robot to pressure BT-72 at baseline range") &&
        Check(damagedIt->sensorDamage >= 40.0f && damagedIt->weaponDamage >= 40.0f,
            "mechanical damage smoke expected heavy BT-72 hit to damage robot sensors and weapons") &&
        Check(impactEvent.find("Sensor mast") != std::string::npos || impactEvent.find("Weapon arm") != std::string::npos,
            "mechanical damage smoke expected readable subsystem-hit feedback") &&
        Check(readability.find("Control robot") != std::string::npos &&
                readability.find("weapon") != std::string::npos &&
                readability.find("sensor") != std::string::npos,
            "mechanical damage smoke expected hostile readability to expose robot subsystem damage") &&
        Check(damagedHullLoss < baselineHullLoss,
            "mechanical damage smoke expected damaged robot modules to reduce immediate BT-72 pressure");
}

bool RunFieldReflexRpgWeightSmoke() {
    auto makeWorld = []() {
        bunker::World world;
        bunker::MapObject ghoul;
        ghoul.registryId = "[%enemy_ghoul_rpg_0001]";
        ghoul.displayName = "Route Ghoul";
        ghoul.interaction = bunker::InteractionType::Hostile;
        ghoul.category = bunker::ObjectCategory::Hostile;
        ghoul.x = 1.0f;
        ghoul.y = 0.0f;
        ghoul.width = 1.0f;
        ghoul.depth = 1.0f;
        ghoul.health = 120.0f;
        ghoul.scriptTag = "ghoul_rush";
        world.AddObject(ghoul);
        return world;
    };

    bunker::SessionProfile baselineProfile = bunker::MakeDefaultSessionProfile();
    baselineProfile.character.special.strength = 6;
    baselineProfile.character.special.agility = 5;
    baselineProfile.character.inventory.push_back({"#%it_emergency_baton", 1, 0.8f});

    bunker::SessionProfile boostedProfile = baselineProfile;
    boostedProfile.character.special.agility = 8;
    UnlockAndEquipSkill(boostedProfile, "skill_field_reflex");

    bunker::PlayerState player;
    player.x = 0.0f;
    player.y = 0.0f;
    bunker::StaticEraser staticEraser;

    bunker::World baselineWorld = makeWorld();
    bunker::GameState baselineState;
    bunker::HandleAttack(baselineWorld, player, baselineProfile, staticEraser, baselineState);
    const auto* baselineGhoul = baselineWorld.FindObjectByRegistryId("[%enemy_ghoul_rpg_0001]");
    if (!Check(baselineGhoul != nullptr, "field reflex smoke expected baseline ghoul after attack")) {
        return false;
    }
    const float baselineDamage = 120.0f - baselineGhoul->health;

    bunker::World boostedWorld = makeWorld();
    bunker::GameState boostedState;
    bunker::HandleAttack(boostedWorld, player, boostedProfile, staticEraser, boostedState);
    const auto* boostedGhoul = boostedWorld.FindObjectByRegistryId("[%enemy_ghoul_rpg_0001]");
    if (!Check(boostedGhoul != nullptr, "field reflex smoke expected boosted ghoul after attack")) {
        return false;
    }
    const float boostedDamage = 120.0f - boostedGhoul->health;

    return Check(boostedDamage > baselineDamage + 1.0f,
            "field reflex smoke expected agility + Field Reflex to increase first-route foot damage") &&
        Check(boostedState.lastEvent.find("Field Reflex") != std::string::npos,
            "field reflex smoke expected readable RPG feedback copy");
}

bool RunBt72CrewCoordinationWeightSmoke() {
    auto makeWorld = []() {
        bunker::World world;
        bunker::MapObject raider;
        raider.registryId = "[%enemy_raider_rpg_0001]";
        raider.displayName = "Route Raider";
        raider.interaction = bunker::InteractionType::Hostile;
        raider.category = bunker::ObjectCategory::Hostile;
        raider.x = 3.4f;
        raider.y = 0.0f;
        raider.width = 1.0f;
        raider.depth = 1.0f;
        raider.health = 140.0f;
        raider.scriptTag = "human_tactical";
        world.AddObject(raider);
        return world;
    };

    bunker::SessionProfile baselineProfile = bunker::MakeDefaultSessionProfile();
    baselineProfile.story.tankLinked = true;
    baselineProfile.firstPlayableRoute.bt72Restored = true;
    baselineProfile.partnerTank.secondSeatUnlocked = true;

    bunker::SessionProfile boostedProfile = baselineProfile;
    boostedProfile.character.special.charisma = 8;
    boostedProfile.character.special.intelligence = 8;
    boostedProfile.partnerTank.gunnerDrillSeen = true;
    boostedProfile.partnerTank.secondSeatPolicy = "trusted_only";
    boostedProfile.partnerTank.trustedGunnerHandle = "Smoke Client";
    boostedProfile.partnerTank.assignedGunnerHandle = "Smoke Client";
    UnlockAndEquipSkill(boostedProfile, "skill_pilot_sync");

    bunker::PlayerState player;
    player.insideTank = true;
    player.bt72GunnerSeat = true;
    player.x = 0.0f;
    player.y = 0.0f;

    bunker::StaticEraser staticEraser;
    bunker::World baselineWorld = makeWorld();
    bunker::GameState baselineState;
    const float baselineAmmoBefore = baselineProfile.partnerTank.ammoReserve;
    const float baselineEnergyBefore = baselineProfile.partnerTank.energyReserve;
    bunker::HandleAttack(baselineWorld, player, baselineProfile, staticEraser, baselineState);
    const auto* baselineRaider = baselineWorld.FindObjectByRegistryId("[%enemy_raider_rpg_0001]");
    if (!Check(baselineRaider != nullptr, "crew coordination smoke expected baseline raider after attack")) {
        return false;
    }
    const float baselineDamage = 140.0f - baselineRaider->health;
    const float baselineAmmoSpent = baselineAmmoBefore - baselineProfile.partnerTank.ammoReserve;
    const float baselineEnergySpent = baselineEnergyBefore - baselineProfile.partnerTank.energyReserve;

    bunker::World boostedWorld = makeWorld();
    bunker::GameState boostedState;
    const float boostedAmmoBefore = boostedProfile.partnerTank.ammoReserve;
    const float boostedEnergyBefore = boostedProfile.partnerTank.energyReserve;
    bunker::HandleAttack(boostedWorld, player, boostedProfile, staticEraser, boostedState);
    const auto* boostedRaider = boostedWorld.FindObjectByRegistryId("[%enemy_raider_rpg_0001]");
    if (!Check(boostedRaider != nullptr, "crew coordination smoke expected boosted raider after attack")) {
        return false;
    }
    const float boostedDamage = 140.0f - boostedRaider->health;
    const float boostedAmmoSpent = boostedAmmoBefore - boostedProfile.partnerTank.ammoReserve;
    const float boostedEnergySpent = boostedEnergyBefore - boostedProfile.partnerTank.energyReserve;

    return Check(boostedDamage > baselineDamage + 1.0f,
            "crew coordination smoke expected pilot-sync crew support to increase gunner damage") &&
        Check(boostedAmmoSpent < baselineAmmoSpent,
            "crew coordination smoke expected crew support to trim gunner ammo cost") &&
        Check(boostedEnergySpent < baselineEnergySpent,
            "crew coordination smoke expected crew support to trim gunner energy cost") &&
        Check(boostedState.lastEvent.find("Pilot Sync and crew discipline") != std::string::npos,
            "crew coordination smoke expected readable crew-support feedback copy");
}

bool RunBt72WeakPointComboSmoke() {
    auto makeWorld = []() {
        bunker::World world;
        bunker::MapObject robot;
        robot.registryId = "[%enemy_robot_combo_0001]";
        robot.displayName = "Sentinel Drone";
        robot.interaction = bunker::InteractionType::Hostile;
        robot.category = bunker::ObjectCategory::Hostile;
        robot.x = 4.2f;
        robot.y = 0.0f;
        robot.width = 1.1f;
        robot.depth = 1.1f;
        robot.health = 320.0f;
        robot.scriptTag = "robot_control";
        world.AddObject(robot);
        return world;
    };

    auto makeProfile = []() {
        bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
        profile.story.tankLinked = true;
        profile.firstPlayableRoute.bt72Restored = true;
        profile.partnerTank.secondSeatUnlocked = true;
        return profile;
    };

    bunker::StaticEraser staticEraser;

    bunker::World baselineWorld = makeWorld();
    bunker::SessionProfile baselineProfile = makeProfile();
    bunker::GameState baselineState;
    bunker::PlayerState baselinePlayer;
    baselinePlayer.insideTank = true;
    baselinePlayer.bt72GunnerSeat = true;
    baselinePlayer.x = 0.0f;
    baselinePlayer.y = 0.0f;
    baselinePlayer.facingRadians = 0.0f;
    bunker::HandleAttack(baselineWorld, baselinePlayer, baselineProfile, staticEraser, baselineState);
    const auto* baselineRobot = baselineWorld.FindObjectByRegistryId("[%enemy_robot_combo_0001]");
    if (!Check(baselineRobot != nullptr, "weak-point combo smoke expected baseline robot after opening burst")) {
        return false;
    }
    const float baselineBurstDamage = 320.0f - baselineRobot->health;

    bunker::World comboWorld = makeWorld();
    bunker::SessionProfile comboProfile = makeProfile();
    bunker::GameState comboState;
    bunker::PlayerState comboPlayer;
    comboPlayer.insideTank = true;
    comboPlayer.bt72GunnerSeat = true;
    comboPlayer.x = 0.0f;
    comboPlayer.y = 0.0f;
    comboPlayer.facingRadians = 0.0f;
    bunker::HandleSpecialAttack(comboWorld, comboPlayer, comboProfile, staticEraser, comboState);
    const auto* comboRobotAfterSpecial = comboWorld.FindObjectByRegistryId("[%enemy_robot_combo_0001]");
    if (!Check(comboRobotAfterSpecial != nullptr, "weak-point combo smoke expected robot after opening special")) {
        return false;
    }
    const float healthBeforeFollowUp = comboRobotAfterSpecial->health;

    bunker::HandleAttack(comboWorld, comboPlayer, comboProfile, staticEraser, comboState);
    const auto* comboRobotAfterFollowUp = comboWorld.FindObjectByRegistryId("[%enemy_robot_combo_0001]");
    if (!Check(comboRobotAfterFollowUp != nullptr, "weak-point combo smoke expected robot after follow-up burst")) {
        return false;
    }
    const float comboBurstDamage = healthBeforeFollowUp - comboRobotAfterFollowUp->health;

    return Check(comboBurstDamage > baselineBurstDamage + 4.0f,
            "weak-point combo smoke expected damaged robot subsystems to amplify BT-72 follow-up damage") &&
        Check(comboState.lastEvent.find("Weak-point") != std::string::npos,
            "weak-point combo smoke expected readable weak-point follow-through copy");
}

bool RunBt72SeatPolicySmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.partnerTank.secondSeatUnlocked = true;
    profile.partnerTank.secondSeatPolicy = "trusted_only";
    profile.partnerTank.trustedGunnerHandle = "Lan Buddy";

    bunker::PlayerState player;
    player.insideTank = true;
    bunker::GameState gameState;

    bunker::TryToggleBt72CrewSeat(player, profile, gameState);
    if (!Check(!player.bt72GunnerSeat, "bt72 seat policy smoke expected trusted-only policy to block local gunner seat")) {
        return false;
    }
    if (!Check(gameState.lastEvent.find("trusted gunner Lan Buddy") != std::string::npos,
            "bt72 seat policy smoke expected trusted-only block copy")) {
        return false;
    }

    profile.partnerTank.trustedGunnerHandle = profile.character.displayName;
    bunker::TryToggleBt72CrewSeat(player, profile, gameState);
    if (!Check(player.bt72GunnerSeat, "bt72 seat policy smoke expected trusted self to unlock gunner seat")) {
        return false;
    }
    if (!Check(profile.partnerTank.gunnerDrillSeen, "bt72 seat policy smoke expected gunner drill flag after seat shift")) {
        return false;
    }
    if (!Check(profile.partnerTank.assignedGunnerHandle == profile.character.displayName,
            "bt72 seat policy smoke expected local operator to become assigned gunner")) {
        return false;
    }
    if (!Check(gameState.lastEvent.find("Trusted Gunner") != std::string::npos,
            "bt72 seat policy smoke expected readable seat policy on successful seat shift")) {
        return false;
    }

    bunker::TryToggleBt72CrewSeat(player, profile, gameState);
    if (!Check(!player.bt72GunnerSeat, "bt72 seat policy smoke expected second toggle to return to pilot seat")) {
        return false;
    }
    if (!Check(profile.partnerTank.assignedGunnerHandle.empty(),
            "bt72 seat policy smoke expected pilot return to clear local assigned gunner")) {
        return false;
    }

    profile.partnerTank.secondSeatPolicy = "open_crew";
    profile.partnerTank.trustedGunnerHandle = "Remote Scout";
    bunker::TryToggleBt72CrewSeat(player, profile, gameState);
    return Check(player.bt72GunnerSeat, "bt72 seat policy smoke expected open-crew policy to allow local gunner seat") &&
        Check(gameState.lastEvent.find("Open Crew") != std::string::npos,
            "bt72 seat policy smoke expected open-crew readability copy");
}

bool RunServiceChoiceWeightSmoke() {
    bunker::World world;
    world.GeneratePrototypeZone();
    const bunker::MapObject* workshop = world.FindObjectByRegistryId("[%workshop_0001]");
    if (!Check(workshop != nullptr, "service choice smoke expected workshop anchor")) {
        return false;
    }

    auto makeProfile = [&](bunker::ShelterDoctrine doctrine) {
        bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
        profile.selectedWorld = "service_choice_smoke.bwld";
        profile.story.tankLinked = true;
        profile.firstPlayableRoute.bt72Restored = true;
        profile.firstPlayableRoute.firstTankCombatResolved = true;
        profile.partnerTank.worldPositionKnown = true;
        profile.partnerTank.worldX = workshop->x;
        profile.partnerTank.worldY = workshop->y;
        profile.partnerTank.damage.hull = 60.0f;
        profile.partnerTank.damage.turret = 58.0f;
        profile.partnerTank.damage.bucket = 62.0f;
        profile.partnerTank.damage.sensors = 57.0f;
        profile.partnerTank.damage.cockpit = 55.0f;
        profile.partnerTank.damage.powerCore = 59.0f;
        profile.partnerTank.energyReserve = 54.0f;
        profile.partnerTank.ammoReserve = 51.0f;
        profile.character.hp = 45.0f;
        profile.character.mp = 28.0f;
        profile.doctrine = doctrine;
        profile.character.inventory.push_back({"repair_patch", 1, 0.2f});
        return profile;
    };

    bunker::SessionProfile baselineProfile = makeProfile(bunker::ShelterDoctrine::Balanced);
    bunker::SessionProfile boostedProfile = makeProfile(bunker::ShelterDoctrine::Medical);
    boostedProfile.partnerTank.secondSeatUnlocked = true;
    boostedProfile.partnerTank.gunnerDrillSeen = true;
    boostedProfile.partnerTank.trustedGunnerHandle = "Smoke Client";
    boostedProfile.partnerTank.assignedGunnerHandle = "Smoke Client";
    UnlockAndEquipSkill(boostedProfile, "skill_pilot_sync");

    bunker::PlayerState player;
    player.insideTank = false;
    bunker::StaticEraser staticEraser;

    bunker::GameState baselineState;
    baselineState.tankThermalLoad = 50.0f;
    const float baselineHpBefore = baselineProfile.character.hp;
    const float baselineMpBefore = baselineProfile.character.mp;
    const float baselineEnergyBefore = baselineProfile.partnerTank.energyReserve;
    bunker::HandleInteraction(workshop, world, player, baselineProfile, staticEraser, baselineState);

    bunker::GameState boostedState;
    boostedState.tankThermalLoad = 50.0f;
    const float boostedHpBefore = boostedProfile.character.hp;
    const float boostedMpBefore = boostedProfile.character.mp;
    const float boostedEnergyBefore = boostedProfile.partnerTank.energyReserve;
    bunker::HandleInteraction(workshop, world, player, boostedProfile, staticEraser, boostedState);

    return Check((boostedProfile.character.hp - boostedHpBefore) > (baselineProfile.character.hp - baselineHpBefore),
            "service choice smoke expected doctrine-weighted service to recover more operator HP") &&
        Check((boostedProfile.character.mp - boostedMpBefore) > (baselineProfile.character.mp - baselineMpBefore),
            "service choice smoke expected doctrine-weighted service to recover more operator MP") &&
        Check((boostedProfile.partnerTank.energyReserve - boostedEnergyBefore) > (baselineProfile.partnerTank.energyReserve - baselineEnergyBefore),
            "service choice smoke expected Pilot Sync service pass to recover more BT-72 energy") &&
        Check(boostedState.lastEvent.find("Medical doctrine stabilized the operator") != std::string::npos,
            "service choice smoke expected readable doctrine-weight feedback copy");
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
    profile.story.awakenedFromCryo = true;
    profile.story.pipPadRecovered = true;
    profile.story.archiveRecovered = true;
    profile.firstPlayableRoute.bt72Restored = true;
    profile.story.tankLinked = true;
    profile.firstPlayableRoute.clearanceBlueprintRecovered = true;
    profile.firstPlayableRoute.clearanceMaterialsRecovered = true;
    profile.firstPlayableRoute.clearanceModuleInstalled = true;
    profile.story.exitedBunker = true;
    profile.firstPlayableRoute.surfaceArrivalReached = true;
    profile.story.outerRoadCleared = true;
    profile.firstPlayableRoute.firstTankCombatResolved = true;
    profile.firstPlayableRoute.firstServicePerformed = true;
    profile.story.relayRecovered = true;
    profile.firstPlayableRoute.firstRecoveryNodeActivated = true;
    profile.story.returnedToBase = true;
    profile.firstPlayableRoute.debriefSummaryViewed = true;
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
    if (!Check(unlockState.backboneStage == "Backbone Stable",
            "lanline services smoke expected mirrored stable backbone stage")) {
        return false;
    }
    if (!Check(unlockState.backboneStatus.find("14/14") != std::string::npos,
            "lanline services smoke expected mirrored full backbone status")) {
        return false;
    }
    if (!Check(unlockState.backbonePayoff.find("stable recovery backbone") != std::string::npos,
            "lanline services smoke expected mirrored backbone payoff")) {
        return false;
    }
    if (!Check(unlockState.routeEventSummary.find("rare field prompt") != std::string::npos,
            "lanline services smoke expected mirrored idle route-event summary")) {
        return false;
    }
    if (!Check(!unlockState.merchantWindowActive, "lanline services smoke expected merchant window to start inactive")) {
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

    worldState->activeRouteEventType = "merchant_window";
    worldState->routeEventTimeRemaining = 20.0f;
    worldState->routeEventOfferTimeRemaining = 6.0f;
    const auto merchantUnlockState = bunker::BuildServicesUnlockState(profile, worldState);
    if (!Check(merchantUnlockState.merchantWindowActive, "lanline services smoke expected merchant window mirror when active")) {
        return false;
    }
    if (!Check(merchantUnlockState.routeEventSummary.find("Merchant window offered") != std::string::npos,
            "lanline services smoke expected merchant route-event summary mirror")) {
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

bool RunLanlineSeatRoundtripSmoke() {
    bunker::LanlineSessionState savedState;
    savedState.sessionId = "lanline_smoke";
    savedState.mode = "LAN Host";
    savedState.lifecycleStage = "HostRuntimeActive";
    savedState.worldName = "smoke_zone.bwld";
    savedState.hostEndpoint = "127.0.0.1:4400";
    savedState.activeActor = "Smoke Host";
    savedState.bt72SecondSeatUnlocked = true;
    savedState.bt72SecondSeatPolicy = "trusted_only";
    savedState.bt72TrustedGunnerHandle = "Smoke Client";
    savedState.bt72AssignedGunnerHandle = "Smoke Client";
    savedState.players.push_back({"Smoke Host", "Host", true, true, "pilot"});
    savedState.players.push_back({"Smoke Client", "Client", true, true, "gunner"});

    if (!Check(bunker::SaveLanlineSessionState(savedState), "lanline seat smoke failed to save session state")) {
        return false;
    }

    bunker::LanlineSessionState loadedState;
    if (!Check(bunker::LoadLanlineSessionState(loadedState), "lanline seat smoke failed to load session state")) {
        return false;
    }

    return Check(loadedState.bt72SecondSeatUnlocked, "lanline seat smoke expected second seat unlock") &&
        Check(loadedState.bt72SecondSeatPolicy == "trusted_only", "lanline seat smoke expected second seat policy") &&
        Check(loadedState.bt72TrustedGunnerHandle == "Smoke Client", "lanline seat smoke expected trusted gunner handle") &&
        Check(loadedState.bt72AssignedGunnerHandle == "Smoke Client", "lanline seat smoke expected assigned gunner handle") &&
        Check(loadedState.players.size() == 2, "lanline seat smoke expected two roster entries") &&
        Check(loadedState.players[0].seatAssignment == "pilot", "lanline seat smoke expected pilot seat assignment") &&
        Check(loadedState.players[1].seatAssignment == "gunner", "lanline seat smoke expected gunner seat assignment");
}

bool RunLaunchTicketFlow() {
    bunker::LaunchTicketInfo issuedTicket;
    issuedTicket.accountId = "#10077";
    issuedTicket.sessionMode = "lanline";
    issuedTicket.characterName = "Smoke Scout";
    issuedTicket.selectedWorld = "smoke_zone.bwld";
    issuedTicket.lanlineSessionId = "relay-test-01";
    issuedTicket.hostEndpoint = "127.0.0.1:4100";
    issuedTicket.bt72SeatRole = "gunner";
    issuedTicket.bt72SecondSeatPolicy = "trusted_only";
    issuedTicket.bt72TrustedGunnerHandle = "Smoke Scout";
    issuedTicket.launcherRole = "admin";

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
        Check(consumedTicket.bt72SeatRole == "gunner", "launch ticket seat role mismatch") &&
        Check(consumedTicket.bt72SecondSeatPolicy == "trusted_only", "launch ticket seat policy mismatch") &&
        Check(consumedTicket.bt72TrustedGunnerHandle == "Smoke Scout", "launch ticket trusted gunner mismatch") &&
        Check(consumedTicket.launcherRole == "admin", "launch ticket launcher role mismatch") &&
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
    authoredPrefab.object.manualLoot = true;
    authoredPrefab.object.lootMode = bunker::LootMode::RandomTable;
    authoredPrefab.object.lootEntries = {
        {"prefab_scalable_loot_0", 1, 2, 3.0f},
        {"", 1, 1, 0.0f},
        {"prefab_scalable_loot_2", 4, 6, 0.0f},
        {"prefab_scalable_loot_3", 2, 2, 5.0f},
        {"prefab_scalable_loot_4", 1, 3, 1.5f},
        {"", 1, 1, 1.0f},
        {"", 1, 1, 1.0f},
    };
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
        Check(loadedPrefabs[0].object.lootMode == bunker::LootMode::RandomTable, "prefab library should preserve scalable loot mode") &&
        Check(loadedPrefabs[0].object.lootEntries.size() == 5, "prefab library should preserve scalable loot entries without trailing empty UI rows") &&
        Check(loadedPrefabs[0].object.lootEntries[4].itemId == "prefab_scalable_loot_4", "prefab library should preserve high-index scalable loot entry") &&
        Check(loadedPrefabs[0].object.lootEntries[2].weight == 0.0f, "prefab library should preserve zero random loot weight") &&
        Check(loadedPrefabs[0].object.manualLootIds[0] == "prefab_scalable_loot_0", "prefab library should mirror first scalable loot entry into legacy slot 0") &&
        Check(loadedPrefabs[1].label == autoPrefab.label, "prefab library should preserve auto prefab label") &&
        Check(loadedPrefabs[1].id == autoPrefab.id, "prefab library should preserve auto prefab id") &&
        Check(loadedPrefabs[1].targetType == autoPrefab.targetType, "prefab library should preserve auto prefab target type") &&
        Check(loadedPrefabs[1].sourceLabel == autoPrefab.sourceLabel, "prefab library should preserve auto prefab source label") &&
        Check(loadedPrefabs[1].completionMode == autoPrefab.completionMode, "prefab library should preserve auto prefab completion mode") &&
        Check(loadedPrefabs[1].object.editorLayer == autoPrefab.object.editorLayer, "prefab library should preserve custom prefab layer") &&
        Check(loadedPrefabs[1].object.semanticAutoCreated, "prefab library should preserve auto semantic origin") &&
        Check(!loadedPrefabs[1].object.semanticLayoutPinned, "prefab library should preserve unpinned auto semantic state");
}

bool RunLegacyPrefabManualLootMigrationSmoke() {
    std::ofstream file(bunker::EditorPrefabLibraryPath());
    if (!Check(file.is_open(), "legacy prefab loot migration smoke failed to open prefab library for write")) {
        return false;
    }

    file << "# BUNKER_PREFABS_V5\n"
         << std::quoted("Legacy Loot Prefab") << ' '
         << std::quoted("[%legacy_prefab_loot_0001]") << ' '
         << std::quoted("Legacy Prefab Loot Chest") << ' '
         << std::quoted("") << ' '
         << std::quoted("") << ' '
         << std::quoted("Loot") << ' '
         << static_cast<int>(bunker::InteractionType::Container) << ' '
         << static_cast<int>(bunker::ObjectCategory::Container) << ' '
         << 1.0f << ' ' << 2.0f << ' ' << 0.0f << ' '
         << 0.0f << ' ' << 0.0f << ' ' << 0.0f << ' '
         << 1.4f << ' ' << 1.2f << ' ' << 1.2f << ' '
         << 70.0f << ' '
         << false << ' ' << true << ' ' << true << ' '
         << false << ' ' << false << ' '
         << std::quoted("legacy_prefab_loot_a") << ' '
         << std::quoted("") << ' '
         << std::quoted("legacy_prefab_loot_c") << ' '
         << std::quoted("") << ' '
         << std::quoted("prefab_legacy_loot") << ' '
         << std::quoted("Item") << ' '
         << std::quoted("Legacy fixture") << ' '
         << std::quoted("Captured") << '\n';
    file.close();

    std::vector<bunker::PrefabRecord> loadedPrefabs;
    if (!Check(bunker::LoadPrefabLibrary(loadedPrefabs), "legacy prefab loot migration smoke failed to load prefab library")) {
        return false;
    }
    if (!Check(loadedPrefabs.size() == 1, "legacy prefab loot migration smoke expected one prefab")) {
        return false;
    }

    return Check(loadedPrefabs[0].object.lootMode == bunker::LootMode::ManualList,
            "legacy prefab loot migration should default to manual loot mode") &&
        Check(loadedPrefabs[0].object.lootEntries.size() == 2,
            "legacy prefab loot migration should create scalable entries from non-empty manual slots") &&
        Check(loadedPrefabs[0].object.lootEntries[0].itemId == "legacy_prefab_loot_a",
            "legacy prefab loot migration should preserve first legacy item") &&
        Check(loadedPrefabs[0].object.lootEntries[1].itemId == "legacy_prefab_loot_c",
            "legacy prefab loot migration should preserve later non-empty legacy item") &&
        Check(loadedPrefabs[0].object.manualLootIds[0] == "legacy_prefab_loot_a" &&
                loadedPrefabs[0].object.manualLootIds[2] == "legacy_prefab_loot_c",
            "legacy prefab loot migration should keep loaded legacy mirror slots");
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
    relayObject.manualLoot = true;
    relayObject.lootMode = bunker::LootMode::RandomTable;
    relayObject.lootEntries = {
        {"export_loot_a", 1, 1, 1.0f},
        {"", 1, 1, 0.0f},
        {"export_loot_c", 2, 2, 0.0f},
    };
    world.AddObject(relayObject);

    bunker::MapObject brokenObject = relayObject;
    brokenObject.registryId = "[%relay_usage_0002]";
    brokenObject.displayName = "Broken Prefab Usage";
    brokenObject.prefabSourceId = "prefab_missing_service";
    brokenObject.manualLoot = false;
    brokenObject.manualLootIds = {};
    brokenObject.lootEntries.clear();
    brokenObject.lootMode = bunker::LootMode::ManualList;
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

    const std::string report = bunker::LoadTextArtifactPreview(exportResult.validationReportPath, 20000);
    return Check(report.find("Format: BWL7") != std::string::npos, "prefab usage smoke expected export report to mention BWL7 format") &&
        Check(report.find("Target extension label: Bunker Protocol world") != std::string::npos,
            "prefab usage smoke expected export report to identify .bwld as Bunker Protocol world format") &&
        Check(report.find(".bwld :: Bunker Protocol world/record") != std::string::npos,
            "prefab usage smoke expected export report to include .bwld in supported format registry") &&
        Check(report.find("Scalable loot entries: 3") != std::string::npos, "prefab usage smoke expected export report to mention scalable loot entry count") &&
        Check(report.find("Non-empty scalable loot entries: 2") != std::string::npos, "prefab usage smoke expected export report to count non-empty scalable loot entries") &&
        Check(report.find("Prefab-derived objects: 2") != std::string::npos, "prefab usage smoke expected prefab-derived object count in report") &&
        Check(report.find("Broken prefab references: 1") != std::string::npos, "prefab usage smoke expected broken prefab reference count in report");
}

bool RunSupportedFileFormatRegistrySmoke() {
    const auto* worldFormat = bunker::FindSupportedFileFormat(".bwld");
    const auto* upperWorldFormat = bunker::FindSupportedFileFormat("BWLD");
    const auto* packageFormat = bunker::FindSupportedFileFormat(".dba");
    const auto* bsaFormat = bunker::FindSupportedFileFormat(".bsa");
    const auto* pluginFormat = bunker::FindSupportedFileFormat(".esp");
    const auto* archiveFormat = bunker::FindSupportedFileFormat(".ba2");
    const auto* saveFormat = bunker::FindSupportedFileFormat(".fos");
    const auto* sidecarFormat = bunker::FindSupportedFileFormat(".f4se");
    const auto* meshFormat = bunker::FindSupportedFileFormat(".nif");
    const auto* animationFormat = bunker::FindSupportedFileFormat(".kf");
    const auto* behaviorFormat = bunker::FindSupportedFileFormat(".hkx");
    const auto* morphFormat = bunker::FindSupportedFileFormat(".tri");
    const auto* textureFormat = bunker::FindSupportedFileFormat(".dds");
    const auto* sourceTextureFormat = bunker::FindSupportedFileFormat(".png");
    const auto* materialFormat = bunker::FindSupportedFileFormat(".bgsm");
    const auto* shaderFormat = bunker::FindSupportedFileFormat(".fx");
    const auto* scriptFormat = bunker::FindSupportedFileFormat(".pex");
    const auto* scriptSourceFormat = bunker::FindSupportedFileFormat(".psc");
    const auto* audioFormat = bunker::FindSupportedFileFormat(".fuz");
    const auto* audioSourceFormat = bunker::FindSupportedFileFormat(".ogg");
    const auto* localizationFormat = bunker::FindSupportedFileFormat(".dlstrings");
    const auto* interfaceFormat = bunker::FindSupportedFileFormat(".swf");
    const auto* generatedFormat = bunker::FindSupportedFileFormat(".seq");
    const auto* lodFormat = bunker::FindSupportedFileFormat(".bto");
    const auto* textFormat = bunker::FindSupportedFileFormat(".json");
    const auto* configFormat = bunker::FindSupportedFileFormat(".ini");
    const auto* packageZipFormat = bunker::FindSupportedFileFormat(".zip");
    const auto* executableFormat = bunker::FindSupportedFileFormat(".dll");
    const auto registryReport = bunker::BuildSupportedFileFormatRegistryReport();

    return Check(worldFormat != nullptr, "format registry should include .bwld") &&
        Check(upperWorldFormat == worldFormat, "format registry lookup should normalize extension case and missing dot") &&
        Check(worldFormat->canonicalAuthoringWorld && worldFormat->bunkerNative,
            "format registry should mark .bwld as bunker-native canonical authoring world") &&
        Check(packageFormat != nullptr && packageFormat->bunkerNative && packageFormat->packageFormat &&
                !packageFormat->canonicalAuthoringWorld,
            "format registry should mark .dba as bunker-native package but not canonical authored world") &&
        Check(pluginFormat != nullptr && pluginFormat->layer == bunker::SupportedFileFormatLayer::RecordPlugin,
            "format registry should classify .esp as record/plugin reference") &&
        Check(bsaFormat != nullptr && bsaFormat->layer == bunker::SupportedFileFormatLayer::AssetArchive,
            "format registry should classify .bsa as asset archive reference") &&
        Check(archiveFormat != nullptr && archiveFormat->layer == bunker::SupportedFileFormatLayer::AssetArchive,
            "format registry should classify .ba2 as asset archive reference") &&
        Check(saveFormat != nullptr && saveFormat->layer == bunker::SupportedFileFormatLayer::RuntimeSave,
            "format registry should classify .fos as runtime save reference") &&
        Check(sidecarFormat != nullptr && sidecarFormat->layer == bunker::SupportedFileFormatLayer::RuntimeSave,
            "format registry should classify .f4se as runtime sidecar reference") &&
        Check(meshFormat != nullptr && meshFormat->layer == bunker::SupportedFileFormatLayer::MeshModelGeometry,
            "format registry should classify .nif as mesh/model reference") &&
        Check(animationFormat != nullptr && animationFormat->layer == bunker::SupportedFileFormatLayer::AnimationPhysics,
            "format registry should classify .kf as animation reference") &&
        Check(behaviorFormat != nullptr && behaviorFormat->layer == bunker::SupportedFileFormatLayer::AnimationPhysics,
            "format registry should classify .hkx as animation/physics reference") &&
        Check(morphFormat != nullptr && morphFormat->layer == bunker::SupportedFileFormatLayer::MeshModelGeometry,
            "format registry should classify .tri as face/morph geometry reference") &&
        Check(textureFormat != nullptr && textureFormat->layer == bunker::SupportedFileFormatLayer::Texture,
            "format registry should classify .dds as texture reference") &&
        Check(sourceTextureFormat != nullptr && sourceTextureFormat->layer == bunker::SupportedFileFormatLayer::Texture,
            "format registry should classify .png as tooling/source texture reference") &&
        Check(materialFormat != nullptr && materialFormat->layer == bunker::SupportedFileFormatLayer::MaterialShader,
            "format registry should classify .bgsm as material reference") &&
        Check(shaderFormat != nullptr && shaderFormat->layer == bunker::SupportedFileFormatLayer::MaterialShader,
            "format registry should classify .fx as material/shader reference") &&
        Check(scriptFormat != nullptr && scriptFormat->layer == bunker::SupportedFileFormatLayer::Script,
            "format registry should classify .pex as script reference") &&
        Check(scriptSourceFormat != nullptr && scriptSourceFormat->canBeTextPreviewed,
            "format registry should mark .psc as text-previewable script source") &&
        Check(audioFormat != nullptr && audioFormat->layer == bunker::SupportedFileFormatLayer::AudioVoiceLip,
            "format registry should classify .fuz as audio/lip reference") &&
        Check(audioSourceFormat != nullptr && audioSourceFormat->layer == bunker::SupportedFileFormatLayer::AudioVoiceLip,
            "format registry should classify .ogg as audio reference") &&
        Check(localizationFormat != nullptr && localizationFormat->layer == bunker::SupportedFileFormatLayer::Localization,
            "format registry should classify .dlstrings as localization reference") &&
        Check(interfaceFormat != nullptr && interfaceFormat->layer == bunker::SupportedFileFormatLayer::InterfaceUI,
            "format registry should classify .swf as interface reference") &&
        Check(generatedFormat != nullptr && generatedFormat->layer == bunker::SupportedFileFormatLayer::GeneratedWorldData,
            "format registry should classify .seq as generated/dialogue sidecar reference") &&
        Check(lodFormat != nullptr && lodFormat->layer == bunker::SupportedFileFormatLayer::GeneratedWorldData,
            "format registry should classify .bto as generated world data") &&
        Check(textFormat != nullptr && textFormat->layer == bunker::SupportedFileFormatLayer::ConfigTextLog,
            "format registry should classify .json as config/text/log reference") &&
        Check(configFormat != nullptr && configFormat->layer == bunker::SupportedFileFormatLayer::ConfigTextLog,
            "format registry should classify .ini as config/text/log reference") &&
        Check(packageZipFormat != nullptr && packageZipFormat->layer == bunker::SupportedFileFormatLayer::ModPackage,
            "format registry should classify .zip as mod/package tooling reference") &&
        Check(executableFormat != nullptr && executableFormat->layer == bunker::SupportedFileFormatLayer::ExecutableNativePlugin &&
                executableFormat->executableDanger,
            "format registry should classify .dll as dangerous executable/native plugin reference") &&
        Check(bunker::FindSupportedFileFormat(".unknown") == nullptr,
            "format registry should gracefully report unknown extensions") &&
        Check(registryReport.find(".bwld :: Bunker Protocol world/record") != std::string::npos,
            "format registry report should list .bwld explicitly") &&
        Check(registryReport.find(".dba :: Bunker package/archive") != std::string::npos,
            "format registry report should list .dba explicitly") &&
        Check(registryReport.find("dangerous executable/native plugin") != std::string::npos,
            "format registry report should list dangerous executable references");
}

bool RunExportDataScanSmoke() {
    const fs::path tempRoot = fs::temp_directory_path() / "bunker_export_data_scan_smoke";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot / "Export_data", ec);
    if (!Check(!ec, "export_data scan smoke failed to create temporary Export_data folder")) {
        return false;
    }

    const std::vector<std::string> fileNames = {
        "bunker_world.bwld",
        "bunker_dlc.dba",
        "Fallout4.esm",
        "Fallout4.esp",
        "Fallout4.esl",
        "Fallout4 - Main.ba2",
        "Fallout - Meshes.bsa",
        "weapon.nif",
        "anim.kf",
        "controller.kfm",
        "behavior.hkx",
        "face.tri",
        "morph.egm",
        "tint.egt",
        "face.ctl",
        "legacy.rdt",
        "tree.spt",
        "object.bto",
        "terrain.btr",
        "treelod.btt",
        "world.lod",
        "distant.dlod",
        "nav.navmesh",
        "texture.dds",
        "source.tga",
        "source.png",
        "source.jpg",
        "source.bmp",
        "material.bgsm",
        "effect.bgem",
        "generic.mat",
        "shader.fx",
        "env.cub",
        "script.psc",
        "script.pex",
        "voice.fuz",
        "voice.lip",
        "sound.xwm",
        "sound.wav",
        "legacy.ogg",
        "music.mp3",
        "lipsync.dat",
        "Fallout4_en.strings",
        "Fallout4_en.dlstrings",
        "Fallout4_en.ilstrings",
        "quest.seq",
        "menu.swf",
        "ui.gfx",
        "font.fnt",
        "save.fos",
        "co_save.f4se",
        "nvse_sidecar.nvse",
        "native.dll",
        "tool.exe",
        "installer.fomod",
        "old.omod",
        "archive.zip",
        "archive.7z",
        "archive.rar",
        "config.ini",
        "metadata.json",
        "menu.xml",
        "readme.txt",
        "scan.log",
        "unknown.xyz",
    };
    for (const auto& fileName : fileNames) {
        std::ofstream file(tempRoot / "Export_data" / fileName, std::ios::binary);
        file << "smoke";
    }

    const auto summary = bunker::ScanExportDataDirectory(tempRoot);
    const auto bwld = std::find_if(summary.files.begin(), summary.files.end(), [](const bunker::ExternalDataFileRecord& file) {
        return file.extension == ".bwld";
    });
    const auto dba = std::find_if(summary.files.begin(), summary.files.end(), [](const bunker::ExternalDataFileRecord& file) {
        return file.extension == ".dba";
    });
    const auto esp = std::find_if(summary.files.begin(), summary.files.end(), [](const bunker::ExternalDataFileRecord& file) {
        return file.extension == ".esp";
    });
    const auto psc = std::find_if(summary.files.begin(), summary.files.end(), [](const bunker::ExternalDataFileRecord& file) {
        return file.extension == ".psc";
    });
    const auto dll = std::find_if(summary.files.begin(), summary.files.end(), [](const bunker::ExternalDataFileRecord& file) {
        return file.extension == ".dll";
    });
    const auto unknown = std::find_if(summary.files.begin(), summary.files.end(), [](const bunker::ExternalDataFileRecord& file) {
        return file.extension == ".xyz";
    });

    fs::remove_all(tempRoot, ec);

    if (!Check(summary.exists, "export_data scan should detect existing Export_data folder")) {
        return false;
    }
    if (!Check(summary.foundFileCount == fileNames.size(), "export_data scan should count all fake files")) {
        return false;
    }
    if (!Check(summary.recognizedFileCount + summary.unknownFileCount == fileNames.size(),
            "export_data scan should classify every file as recognized or unknown")) {
        return false;
    }
    if (!Check(summary.unknownFileCount == 1, "export_data scan should treat unknown.xyz as the only unknown file")) {
        return false;
    }
    return Check(bwld != summary.files.end() && bwld->bunkerNative && bwld->canonicalAuthoringWorld &&
                bwld->importMode == bunker::ExternalDataImportMode::NativeWorldSource,
            "export_data scan should mark .bwld as native canonical world source") &&
        Check(dba != summary.files.end() && dba->bunkerNative && dba->packageFormat &&
                dba->importMode == bunker::ExternalDataImportMode::BunkerPackageReference,
            "export_data scan should mark .dba as bunker-native package reference") &&
        Check(esp != summary.files.end() && esp->referenceOnly &&
                esp->importMode == bunker::ExternalDataImportMode::ReferenceOnly,
            "export_data scan should mark Fallout-like plugin files as reference-only") &&
        Check(psc != summary.files.end() && psc->textReadable &&
                psc->importMode == bunker::ExternalDataImportMode::TextScriptSource,
            "export_data scan should mark .psc as text script source") &&
        Check(dll != summary.files.end() && dll->recognized && dll->executableDanger &&
                dll->referenceOnly,
            "export_data scan should mark .dll as dangerous reference-only executable data") &&
        Check(unknown != summary.files.end() && !unknown->recognized &&
                unknown->importMode == bunker::ExternalDataImportMode::UnknownReference,
            "export_data scan should treat unknown extensions as warning-only references");
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

bool RunLegacyManualLootMigrationSmoke() {
    std::ofstream file(bunker::DefaultWorldPath(), std::ios::binary);
    if (!Check(file.is_open(), "legacy manual loot migration smoke failed to open world file for write")) {
        return false;
    }

    file.write("BWL2", 4);
    WriteRawString(file, "Legacy Manual Loot Migration");
    WriteRawString(file, "Bunker Interior");
    WriteRawString(file, "Load legacy manualLootIds into scalable lootEntries.");
    const float spawnX = 0.0f;
    const float spawnY = 0.0f;
    file.write(reinterpret_cast<const char*>(&spawnX), sizeof(spawnX));
    file.write(reinterpret_cast<const char*>(&spawnY), sizeof(spawnY));

    const std::uint32_t count = 1;
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    WriteRawString(file, "[%legacy_loot_0001]");
    WriteRawString(file, "Legacy Loot Chest");
    WriteRawString(file, "");
    WriteRawString(file, "");

    const std::uint32_t interaction = static_cast<std::uint32_t>(bunker::InteractionType::Container);
    const std::uint32_t category = static_cast<std::uint32_t>(bunker::ObjectCategory::Container);
    const float x = 2.0f;
    const float y = 3.0f;
    const float z = 0.0f;
    const float width = 1.4f;
    const float depth = 1.2f;
    const float height = 1.2f;
    const float health = 70.0f;
    const bool blocksMovement = false;
    const bool discovered = true;
    const bool manualLoot = true;

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
    WriteRawString(file, "legacy_loot_a");
    WriteRawString(file, "");
    WriteRawString(file, "legacy_loot_c");
    WriteRawString(file, "");
    file.close();

    bunker::World loadedWorld;
    if (!Check(loadedWorld.Load(bunker::DefaultWorldPath().string()), "legacy manual loot migration world load failed")) {
        return false;
    }

    const auto* legacyChest = loadedWorld.FindObjectByRegistryId("[%legacy_loot_0001]");
    return Check(legacyChest != nullptr, "legacy manual loot migration should load chest") &&
        Check(legacyChest->lootMode == bunker::LootMode::ManualList, "legacy manual loot migration should default to manual loot mode") &&
        Check(legacyChest->lootEntries.size() == 2, "legacy manual loot migration should create scalable entries from non-empty legacy slots") &&
        Check(legacyChest->lootEntries[0].itemId == "legacy_loot_a", "legacy manual loot migration should preserve first legacy item") &&
        Check(legacyChest->lootEntries[1].itemId == "legacy_loot_c", "legacy manual loot migration should preserve later non-empty legacy item") &&
        Check(legacyChest->manualLootIds[0] == "legacy_loot_a" && legacyChest->manualLootIds[2] == "legacy_loot_c",
            "legacy manual loot migration should keep legacy mirror values loaded");
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
    world.objects[1].manualLoot = true;
    world.objects[1].manualLootIds = {"undo_loot_a", "undo_loot_b", "", ""};
    world.objects[1].lootMode = bunker::LootMode::RandomTable;
    world.objects[1].lootEntries = {
        {"undo_loot_a", 1, 2, 2.0f},
        {"undo_loot_b", 3, 3, 0.0f},
    };
    undoStack.PushObjectUpdated("Edit service relay", beforeSecondUpdate, world.objects[1], 1);

    if (!Check(undoStack.UndoCount() == 2, "undo smoke expected update coalescing to keep a single object-edit record")) {
        return false;
    }
    const auto updateUndo = undoStack.Undo(world);
    if (!Check(updateUndo.changed, "undo smoke expected object-edit undo to apply")) {
        return false;
    }
    if (!Check(world.objects[1].displayName == beforeFirstUpdate.displayName &&
            world.objects[1].x == beforeFirstUpdate.x &&
            world.objects[1].lootEntries == beforeFirstUpdate.lootEntries &&
            world.objects[1].lootMode == beforeFirstUpdate.lootMode &&
            world.objects[1].manualLootIds == beforeFirstUpdate.manualLootIds,
            "undo smoke expected coalesced object-edit undo to restore original object")) {
        return false;
    }
    const auto updateRedo = undoStack.Redo(world);
    if (!Check(updateRedo.changed, "undo smoke expected object-edit redo to apply")) {
        return false;
    }
    if (!Check(world.objects[1].displayName == "Service Relay Updated" &&
            world.objects[1].x == 14.0f &&
            world.objects[1].lootMode == bunker::LootMode::RandomTable &&
            world.objects[1].lootEntries.size() == 2 &&
            world.objects[1].lootEntries[1].itemId == "undo_loot_b" &&
            world.objects[1].lootEntries[1].weight == 0.0f &&
            world.objects[1].manualLootIds[0] == "undo_loot_a",
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

const bunker::GameComponent* FindComponent(
    const bunker::GameObjectInstance& instance,
    bunker::GameComponentKind kind) {
    const auto it = std::find_if(instance.components.begin(), instance.components.end(), [&](const bunker::GameComponent& component) {
        return component.kind == kind;
    });
    return it == instance.components.end() ? nullptr : &(*it);
}

bool RunGameExecutionResourceLookupSmoke() {
    const fs::path tempRoot = fs::current_path() / "game_execution_resource_lookup";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot / "Export_data", ec);
    if (!Check(!ec, "game execution resource smoke failed to create temp Export_data")) {
        return false;
    }

    const std::vector<std::string> files = {
        "generator_0001.NIF",
        "generator_0001.DDS",
        "terminal_sync.PEX",
        "terminal_sync.PSC",
        "Fallout4.ESM",
        "random_unknown.xyz",
    };
    for (const auto& fileName : files) {
        std::ofstream file(tempRoot / "Export_data" / fileName, std::ios::binary);
        file << "fake";
    }

    bunker::World world;
    bunker::MapObject generator;
    generator.registryId = "[%generator_0001]";
    generator.displayName = "Backup Generator";
    generator.prefabSourceId = "generator_0001";
    generator.category = bunker::ObjectCategory::Structure;
    world.AddObject(generator);

    bunker::MapObject terminal;
    terminal.registryId = "[%terminal_0001]";
    terminal.displayName = "Sync Terminal";
    terminal.interaction = bunker::InteractionType::Terminal;
    terminal.category = bunker::ObjectCategory::Terminal;
    terminal.scriptTag = "terminal_sync";
    world.AddObject(terminal);

    bunker::MapObject pluginProxy;
    pluginProxy.registryId = "[%Fallout4]";
    pluginProxy.displayName = "Plugin Proxy";
    world.AddObject(pluginProxy);

    const auto context = bunker::BuildWorldExecutionContext(world, tempRoot);
    const auto missingContext = bunker::BuildWorldExecutionContext(world, tempRoot / "missing");
    const auto* generatorInstance = bunker::FindGameObjectInstance(context, generator.registryId);
    const auto* terminalInstance = bunker::FindGameObjectInstance(context, terminal.registryId);
    const auto* pluginInstance = bunker::FindGameObjectInstance(context, pluginProxy.registryId);

    fs::remove_all(tempRoot, ec);

    return Check(context.externalData.exists, "game execution resource smoke expected Export_data scan to exist") &&
        Check(context.externalData.foundFileCount == files.size(), "game execution resource smoke expected every fake file to be scanned") &&
        Check(context.externalData.unknownFileCount == 1, "game execution resource smoke expected unknown files to stay warning-only") &&
        Check(missingContext.objects.size() == world.objects.size(),
            "game execution resource smoke expected missing scan root to build context without crashing") &&
        Check(generatorInstance != nullptr && !generatorInstance->renderResourcePath.empty() &&
                (generatorInstance->renderResourcePath.extension().string() == ".NIF" ||
                 generatorInstance->renderResourcePath.extension().string() == ".DDS"),
            "game execution resource smoke expected uppercase render asset lookup") &&
        Check(terminalInstance != nullptr && !terminalInstance->compiledScriptPath.empty() &&
                !terminalInstance->sourceScriptPath.empty(),
            "game execution resource smoke expected script paths for terminal_sync") &&
        Check(pluginInstance != nullptr && !pluginInstance->pluginProxyPath.empty(),
            "game execution resource smoke expected plugin proxy match by normalized key") &&
        Check(bunker::NormalizeResourceLookupKey("[%generator_0001]") == "generator0001",
            "game execution resource smoke expected registry id normalization");
}

bool RunGameExecutionLootTemplateSmoke() {
    bunker::World world;
    auto makeContainer = [](std::string registryId, std::vector<bunker::LootEntry> loot) {
        bunker::MapObject container;
        container.registryId = std::move(registryId);
        container.displayName = "Loot Container";
        container.interaction = bunker::InteractionType::Container;
        container.category = bunker::ObjectCategory::Container;
        container.lootEntries = std::move(loot);
        return container;
    };

    world.AddObject(makeContainer("[%empty_loot]", std::vector<bunker::LootEntry>(20)));

    std::vector<bunker::LootEntry> row10(20);
    row10[9].itemId = "row_10_real";
    row10[9].minCount = 2;
    world.AddObject(makeContainer("[%row10_loot]", row10));

    std::vector<bunker::LootEntry> row20(20);
    row20[19].itemId = "row_20_real";
    row20[19].maxCount = 3;
    world.AddObject(makeContainer("[%row20_loot]", row20));

    std::vector<bunker::LootEntry> mixed(4);
    mixed[0].itemId = "first_real";
    mixed[2].itemId = "second_real";
    world.AddObject(makeContainer("[%mixed_loot]", mixed));

    const auto context = bunker::BuildWorldExecutionContext(world);
    const auto* emptyInstance = bunker::FindGameObjectInstance(context, "[%empty_loot]");
    const auto* row10Instance = bunker::FindGameObjectInstance(context, "[%row10_loot]");
    const auto* row20Instance = bunker::FindGameObjectInstance(context, "[%row20_loot]");
    const auto* mixedInstance = bunker::FindGameObjectInstance(context, "[%mixed_loot]");

    const auto* emptyInventory = emptyInstance == nullptr ? nullptr : FindComponent(*emptyInstance, bunker::GameComponentKind::Inventory);
    const auto* row10Inventory = row10Instance == nullptr ? nullptr : FindComponent(*row10Instance, bunker::GameComponentKind::Inventory);
    const auto* row20Inventory = row20Instance == nullptr ? nullptr : FindComponent(*row20Instance, bunker::GameComponentKind::Inventory);
    const auto* mixedInventory = mixedInstance == nullptr ? nullptr : FindComponent(*mixedInstance, bunker::GameComponentKind::Inventory);

    return Check(emptyInventory == nullptr, "game execution loot smoke expected all-empty UI rows to produce no inventory") &&
        Check(row10Inventory != nullptr && row10Inventory->inventoryTemplate.size() == 1 &&
                row10Inventory->inventoryTemplate[0].itemId == "row_10_real",
            "game execution loot smoke expected high row 10 loot to be preserved") &&
        Check(row20Inventory != nullptr && row20Inventory->inventoryTemplate.size() == 1 &&
                row20Inventory->inventoryTemplate[0].itemId == "row_20_real",
            "game execution loot smoke expected high row 20 loot to be preserved") &&
        Check(mixedInventory != nullptr && mixedInventory->inventoryTemplate.size() == 2 &&
                mixedInventory->inventoryTemplate[0].itemId == "first_real" &&
                mixedInventory->inventoryTemplate[1].itemId == "second_real",
            "game execution loot smoke expected empty rows to be ignored");
}

bool RunGameExecutionScriptBridgeSmoke() {
    const fs::path tempRoot = fs::current_path() / "game_execution_script_bridge";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot / "Export_data", ec);
    if (!Check(!ec, "game execution script bridge smoke failed to create temp Export_data")) {
        return false;
    }
    {
        std::ofstream terminalPex(tempRoot / "Export_data" / "terminal_sync.pex", std::ios::binary);
        terminalPex << "fake";
        std::ofstream sourceOnly(tempRoot / "Export_data" / "source_only.psc", std::ios::binary);
        sourceOnly << "fake";
    }

    bunker::World world;
    auto makeTerminal = [](std::string registryId, std::string scriptTag) {
        bunker::MapObject terminal;
        terminal.registryId = std::move(registryId);
        terminal.displayName = "Bridge Terminal";
        terminal.interaction = bunker::InteractionType::Terminal;
        terminal.category = bunker::ObjectCategory::Terminal;
        terminal.scriptTag = std::move(scriptTag);
        return terminal;
    };
    world.AddObject(makeTerminal("[%terminal_sync]", "terminal_sync"));
    world.AddObject(makeTerminal("[%source_only]", "source_only"));
    world.AddObject(makeTerminal("[%missing_script]", "missing_script"));
    world.AddObject(makeTerminal("[%empty_script]", ""));

    const auto context = bunker::BuildWorldExecutionContext(world, tempRoot);
    std::string status;
    const bool compiled = bunker::TryExecuteCompiledScript(world.objects[0], context, status);
    const bool compiledStatus = status.find("compiled script bridge") != std::string::npos;
    status.clear();
    const bool source = bunker::TryExecuteCompiledScript(world.objects[1], context, status);
    const bool sourceStatus = status.find("source script bridge") != std::string::npos;
    status.clear();
    const bool missing = bunker::TryExecuteCompiledScript(world.objects[2], context, status);
    const bool empty = bunker::TryExecuteCompiledScript(world.objects[3], context, status);

    fs::remove_all(tempRoot, ec);

    return Check(compiled && compiledStatus, "game execution script smoke expected compiled bridge status") &&
        Check(source && sourceStatus, "game execution script smoke expected source fallback bridge status") &&
        Check(!missing, "game execution script smoke expected missing script to return false") &&
        Check(!empty, "game execution script smoke expected empty script tag to return false");
}

bool RunPlayerSweepCollisionSmoke() {
    bunker::World blockingWorld;
    bunker::MapObject blocker;
    blocker.registryId = "[%blocker]";
    blocker.displayName = "Blocking Crate";
    blocker.category = bunker::ObjectCategory::Structure;
    blocker.x = 1.2f;
    blocker.y = 0.0f;
    blocker.width = 1.0f;
    blocker.depth = 1.0f;
    blocker.blocksMovement = true;
    blockingWorld.AddObject(blocker);

    bunker::PlayerState player;
    player.x = 0.0f;
    player.y = 0.0f;
    const bool blockedMove = bunker::SweepMovePlayerAgainstWorld(blockingWorld, player, 1.0f, 0.0f);
    const float afterBlockedX = player.x;
    const bool movedAway = bunker::SweepMovePlayerAgainstWorld(blockingWorld, player, -1.0f, 0.0f);

    bunker::World nonBlockingWorld;
    blocker.blocksMovement = false;
    nonBlockingWorld.AddObject(blocker);
    bunker::PlayerState nonBlockedPlayer;
    const bool nonBlockedMove = bunker::SweepMovePlayerAgainstWorld(nonBlockingWorld, nonBlockedPlayer, 1.0f, 0.0f);

    return Check(!blockedMove && std::abs(afterBlockedX) < 0.0001f,
            "player sweep collision smoke expected blocking object to stop movement") &&
        Check(movedAway && player.x < -0.9f,
            "player sweep collision smoke expected movement away from blocker to work") &&
        Check(nonBlockedMove && nonBlockedPlayer.x > 0.9f,
            "player sweep collision smoke expected non-blocking object not to stop movement");
}

bool RunRuntimeCameraSmoke() {
    bunker::PlayerState player;
    player.x = 10.0f;
    player.y = 20.0f;
    player.facingRadians = 0.0f;
    player.viewMode = bunker::ViewMode::ThirdPerson;
    const auto thirdPerson = bunker::BuildRuntimeCamera(player);

    player.viewMode = bunker::ViewMode::FirstPerson;
    const auto firstPerson = bunker::BuildRuntimeCamera(player);

    player.insideTank = true;
    player.viewMode = bunker::ViewMode::Cockpit;
    const auto cockpit = bunker::BuildRuntimeCamera(player);

    return Check(std::string(bunker::ToString(bunker::ViewMode::FirstPerson)) == "First Person",
            "runtime camera smoke expected FirstPerson label") &&
        Check(thirdPerson.positionX < player.x && thirdPerson.positionY > firstPerson.positionY,
            "runtime camera smoke expected third-person camera behind and above player") &&
        Check(firstPerson.targetX > firstPerson.positionX && std::abs(firstPerson.targetZ - firstPerson.positionZ) < 0.001f,
            "runtime camera smoke expected first-person camera to look forward") &&
        Check(cockpit.fovDegrees < firstPerson.fovDegrees && cockpit.positionY > firstPerson.positionY,
            "runtime camera smoke expected cockpit camera to use tighter raised view");
}

bool RunPipDeviceCapabilityContractSmoke() {
    return Check(bunker::DeviceDisplayName(bunker::PipDeviceModel::PipBoy3000) ==
                std::string("Pip-Boy 3000 / Mark IV"),
            "pipboy_3000_equals_mark_iv_capability expected canonical display name") &&
        Check(bunker::HasDigitalMap(bunker::PipDeviceModel::PipBoy3000) &&
                bunker::HasDigitalMap(bunker::PipDeviceModel::PipBoy3000MarkIV),
            "pipboy_3000_equals_mark_iv_capability expected digital map support") &&
        Check(!bunker::IsSelectablePipDevice(bunker::PipDeviceModel::PipBoy3000MarkV) &&
                bunker::IsPropOnlyPipDevice(bunker::PipDeviceModel::PipBoy3000MarkV) &&
                !bunker::HasDigitalMap(bunker::PipDeviceModel::PipBoy3000MarkV),
            "pipboy_3000_mark_v_not_selectable expected rejected prop-only model") &&
        Check(bunker::IsSelectablePipDevice(bunker::PipDeviceModel::PipBoy2000MarkVI) &&
                bunker::UsesPhysicalNavigation(bunker::PipDeviceModel::PipBoy2000MarkVI) &&
                bunker::SupportsMediaIndex(bunker::PipDeviceModel::PipBoy2000MarkVI),
            "fo76_pipboy_shell_is_viable_user_preferred_option expected viable physical-navigation shell") &&
        Check(bunker::IsSelectablePipDevice(bunker::PipDeviceModel::PipPad3500) &&
                bunker::SupportsFullPipPadWorkspace(bunker::PipDeviceModel::PipPad3500) &&
                bunker::GetPipDeviceCapabilities(bunker::PipDeviceModel::PipPad3500).ruggedMilitaryPipBoyDerived,
            "pippad_is_not_consumer_tablet expected rugged Pip-Boy-derived field tablet") &&
        Check(bunker::UsesPhysicalNavigation(bunker::PipDeviceModel::PipBoy01) &&
                bunker::UsesPhysicalNavigation(bunker::PipDeviceModel::PipBoy10) &&
                bunker::UsesPhysicalNavigation(bunker::PipDeviceModel::PipBoy2000MarkVI),
            "no_map_devices_use_physical_navigation expected physical navigation for no-map shells");
}

bool RunBlueLinkExpansionModuleContractSmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();

    if (!Check(!bunker::InstallBlueLinkModule(profile) &&
            !bunker::CanUsePipPadMediaIndex(profile),
            "bluelink_requires_pippad_before_install expected install to fail without Pip-Pad")) {
        return false;
    }
    bunker::RecoverPipPad(profile);
    if (!Check(!bunker::InstallBlueLinkModule(profile) &&
            !bunker::CanUsePipPadMediaIndex(profile) &&
            profile.pipPadExpansionCoverPresent,
            "bluelink_requires_module_before_install expected install to fail without recovered module")) {
        return false;
    }

    if (!Check(profile.pipPadExpansionCoverPresent &&
            !bunker::HasBlueLinkModule(profile) &&
            !bunker::IsBlueLinkInstalled(profile),
            "pippad_has_expansion_cover_before_bluelink expected dummy cover without module")) {
        return false;
    }
    if (!Check(!bunker::CanUsePipPadMediaIndex(profile),
            "pippad_media_index_locked_without_bluelink expected media index lock")) {
        return false;
    }

    profile.blueLinkModuleRecovered = true;
    if (!Check(bunker::InstallBlueLinkModule(profile) &&
            bunker::IsBlueLinkInstalled(profile) &&
            bunker::CanUsePipPadMediaIndex(profile) &&
            !profile.pipPadExpansionCoverPresent,
            "bluelink_install_unlocks_media_index expected installed module to unlock media index")) {
        return false;
    }
    if (!Check(!profile.story.archiveRecovered &&
            !profile.story.relayRecovered &&
            !profile.story.returnedToBase,
            "bluelink_does_not_unlock_map_without_map_data expected no synthetic map/story data")) {
        return false;
    }

    const fs::path tempRoot = fs::current_path() / "bluelink_expansion_contract_smoke";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);
    if (!Check(!ec, "bluelink expansion smoke failed to create temp directory")) {
        return false;
    }
    const fs::path profilePath = tempRoot / "profile.txt";
    const auto saveStatus = bunker::SaveProfileAtomically(profile, profilePath);
    if (!Check(saveStatus.ok, "bluelink expansion smoke failed to save profile: " + saveStatus.message)) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    bunker::SessionProfile loadedProfile;
    if (!Check(bunker::LoadSessionProfile(profilePath, loadedProfile),
            "bluelink expansion smoke failed to load profile")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    const bool ok = Check(bunker::HasBlueLinkModule(loadedProfile) &&
            bunker::IsBlueLinkInstalled(loadedProfile) &&
            bunker::CanUsePipPadMediaIndex(loadedProfile) &&
            !loadedProfile.pipPadExpansionCoverPresent,
            "bluelink expansion smoke expected module state to persist after save/load");
    fs::remove_all(tempRoot, ec);
    return ok;
}

bool RunBlueLinkRuntimeInteractionSmoke() {
    bunker::World world;
    bunker::MapObject module;
    module.registryId = "[%bluelink_module_0001]";
    module.displayName = "BlueLink Media Module";
    module.interaction = bunker::InteractionType::Container;
    module.category = bunker::ObjectCategory::Container;
    world.AddObject(module);

    bunker::MapObject expansionBay;
    expansionBay.registryId = "[%pippad_expansion_bay_0001]";
    expansionBay.displayName = "Pip-Pad Expansion Bay";
    expansionBay.interaction = bunker::InteractionType::Terminal;
    expansionBay.category = bunker::ObjectCategory::Terminal;
    world.AddObject(expansionBay);

    bunker::PlayerState player;
    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();

    const auto* moduleObject = world.FindObjectByRegistryId("[%bluelink_module_0001]");
    const auto* bayObject = world.FindObjectByRegistryId("[%pippad_expansion_bay_0001]");
    if (!Check(moduleObject != nullptr && bayObject != nullptr,
            "bluelink runtime smoke expected synthetic module and expansion bay")) {
        return false;
    }

    bunker::HandleInteraction(moduleObject, world, player, profile, staticEraser, gameState);
    if (!Check(!bunker::HasBlueLinkModule(profile) &&
            !bunker::CanUsePipPadMediaIndex(profile) &&
            gameState.lastEvent.find("Recover the Pip-Pad") != std::string::npos,
            "bluelink_pickup_requires_pippad expected pickup to fail before Pip-Pad")) {
        return false;
    }

    bunker::RecoverPipPad(profile);
    bunker::HandleInteraction(moduleObject, world, player, profile, staticEraser, gameState);
    if (!Check(profile.blueLinkModuleRecovered &&
            !profile.blueLinkModuleInstalled &&
            profile.pipPadExpansionCoverPresent &&
            !bunker::CanUsePipPadMediaIndex(profile),
            "bluelink_pickup_recovers_module_only expected recovered module without install")) {
        return false;
    }

    bunker::SessionProfile installLockedProfile = bunker::MakeDefaultSessionProfile();
    bunker::RecoverPipPad(installLockedProfile);
    bunker::HandleInteraction(bayObject, world, player, installLockedProfile, staticEraser, gameState);
    if (!Check(!bunker::IsBlueLinkInstalled(installLockedProfile) &&
            !bunker::CanUsePipPadMediaIndex(installLockedProfile) &&
            installLockedProfile.pipPadExpansionCoverPresent,
            "bluelink_install_requires_recovered_module expected install lock before module pickup")) {
        return false;
    }

    bunker::HandleInteraction(bayObject, world, player, profile, staticEraser, gameState);
    if (!Check(profile.blueLinkModuleRecovered &&
            profile.blueLinkModuleInstalled &&
            !profile.pipPadExpansionCoverPresent &&
            bunker::CanUsePipPadMediaIndex(profile),
            "bluelink_install_unlocks_media_index_runtime expected runtime install to unlock media index")) {
        return false;
    }

    return Check(!profile.story.archiveRecovered &&
            !profile.story.relayRecovered &&
            !profile.story.returnedToBase,
            "bluelink_install_does_not_unlock_map_or_story_data expected no synthetic story/map unlocks");
}

bool RunBt72CraneRestorationContractSmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    profile.firstPlayableRoute.bt72HullInspected = true;

    if (!Check(!bunker::CanAttachBt72HullToCrane(profile) &&
            !bunker::AttachBt72HullToCrane(profile),
            "bt72_crane_requires_hangar_power expected crane attach to require hangar power")) {
        return false;
    }

    profile.hangarPowerRestored = true;
    profile.bt72CraneControlOnline = true;
    if (!Check(!bunker::CanAttachBt72HullToCrane(profile) &&
            !bunker::AttachBt72HullToCrane(profile),
            "bt72_hull_requires_crane_path_clear expected blocked crane path to prevent attach")) {
        return false;
    }

    profile.bt72CranePathClear = true;
    if (!Check(bunker::CanAttachBt72HullToCrane(profile) &&
            bunker::AttachBt72HullToCrane(profile) &&
            bunker::CanMoveBt72HullToServiceLift(profile) &&
            bunker::MoveBt72HullToServiceLift(profile) &&
            profile.bt72HullMovedToServiceLift &&
            profile.bt72HullLockedInRestorationCradle,
            "bt72_hull_moved_to_service_lift_before_core_install expected crane transfer to lock cradle")) {
        return false;
    }

    bunker::SessionProfile blockedCoreProfile = bunker::MakeDefaultSessionProfile();
    blockedCoreProfile.firstPlayableRoute.bt72HullInspected = true;
    blockedCoreProfile.firstPlayableRoute.bt72CoreRecovered = true;
    blockedCoreProfile.hangarPowerRestored = true;
    blockedCoreProfile.bt72CraneControlOnline = true;
    blockedCoreProfile.bt72CranePathClear = true;
    if (!Check(!bunker::CanInstallBt72Core(blockedCoreProfile),
            "bt72_core_install_locked_until_restoration_cradle expected core install lock before cradle")) {
        return false;
    }

    profile.firstPlayableRoute.bt72CoreRecovered = true;
    if (!Check(bunker::CanInstallBt72Core(profile) &&
            !bunker::CanCompleteBt72StagedRestoration(profile),
            "bt72_staged_restoration_requires_service_notes expected service notes to gate final restoration")) {
        return false;
    }
    profile.firstPlayableRoute.bt72ServiceNotesRecovered = true;
    if (!Check(bunker::CanCompleteBt72StagedRestoration(profile),
            "bt72_staged_restoration_requires_service_notes expected staged restoration after notes")) {
        return false;
    }

    const fs::path tempRoot = fs::current_path() / "bt72_crane_contract_smoke";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);
    if (!Check(!ec, "bt72 crane smoke failed to create temp directory")) {
        return false;
    }
    const fs::path profilePath = tempRoot / "profile.txt";
    const auto saveStatus = bunker::SaveProfileAtomically(profile, profilePath);
    if (!Check(saveStatus.ok, "bt72 crane smoke failed to save profile: " + saveStatus.message)) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    bunker::SessionProfile loadedProfile;
    if (!Check(bunker::LoadSessionProfile(profilePath, loadedProfile),
            "bt72 crane smoke failed to load profile")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    const bool ok = Check(loadedProfile.hangarPowerRestored &&
            loadedProfile.bt72CraneControlOnline &&
            loadedProfile.bt72CranePathClear &&
            loadedProfile.bt72HullMovedToServiceLift &&
            loadedProfile.bt72HullLockedInRestorationCradle &&
            bunker::CanCompleteBt72StagedRestoration(loadedProfile),
            "bt72 crane smoke expected crane restoration flags to persist");
    fs::remove_all(tempRoot, ec);
    return ok;
}

bool RunBt72CraneRuntimeInteractionSmoke() {
    bunker::World world;
    auto addTerminal = [&](const std::string& registryId, const std::string& displayName) {
        bunker::MapObject object;
        object.registryId = registryId;
        object.displayName = displayName;
        object.interaction = bunker::InteractionType::Terminal;
        object.category = bunker::ObjectCategory::Terminal;
        world.AddObject(object);
    };
    addTerminal("[%hangar_power_0001]", "Hangar Power Bus");
    addTerminal("[%bt72_crane_control_0001]", "BT-72 Crane Control");
    addTerminal("[%bt72_crane_path_0001]", "BT-72 Crane Path");
    addTerminal("[%bt72_crane_hook_0001]", "BT-72 Crane Hook");
    addTerminal("[%bt72_service_lift_0001]", "BT-72 Service Lift");

    bunker::MapObject hull;
    hull.registryId = "[#tr_hull_0001]";
    hull.displayName = "BT-72 Hull";
    hull.interaction = bunker::InteractionType::VehicleAnchor;
    hull.category = bunker::ObjectCategory::Vehicle;
    world.AddObject(hull);

    const auto* power = world.FindObjectByRegistryId("[%hangar_power_0001]");
    const auto* control = world.FindObjectByRegistryId("[%bt72_crane_control_0001]");
    const auto* path = world.FindObjectByRegistryId("[%bt72_crane_path_0001]");
    const auto* hook = world.FindObjectByRegistryId("[%bt72_crane_hook_0001]");
    const auto* lift = world.FindObjectByRegistryId("[%bt72_service_lift_0001]");
    const auto* hullObject = world.FindObjectByRegistryId("[#tr_hull_0001]");
    if (!Check(power != nullptr && control != nullptr && path != nullptr && hook != nullptr && lift != nullptr && hullObject != nullptr,
            "bt72 crane runtime smoke expected all synthetic crane objects")) {
        return false;
    }

    bunker::PlayerState player;
    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    bunker::HandleInteraction(control, world, player, profile, staticEraser, gameState);
    if (!Check(!profile.bt72CraneControlOnline,
            "bt72_crane_control_requires_power expected crane control to stay offline without power")) {
        return false;
    }

    bunker::HandleInteraction(power, world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(control, world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(path, world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(hook, world, player, profile, staticEraser, gameState);
    if (!Check(!profile.bt72HullAttachedToCrane,
            "bt72_crane_hook_requires_hull_survey expected hook to reject unsurveyed hull")) {
        return false;
    }

    bunker::SessionProfile blockedPathProfile = bunker::MakeDefaultSessionProfile();
    blockedPathProfile.firstPlayableRoute.bt72HullInspected = true;
    bunker::HandleInteraction(power, world, player, blockedPathProfile, staticEraser, gameState);
    bunker::HandleInteraction(control, world, player, blockedPathProfile, staticEraser, gameState);
    bunker::HandleInteraction(hook, world, player, blockedPathProfile, staticEraser, gameState);
    if (!Check(!blockedPathProfile.bt72HullAttachedToCrane,
            "bt72_crane_hook_requires_clear_path expected hook to reject blocked path")) {
        return false;
    }

    bunker::SessionProfile liftBlockedProfile = blockedPathProfile;
    liftBlockedProfile.bt72CranePathClear = true;
    bunker::HandleInteraction(lift, world, player, liftBlockedProfile, staticEraser, gameState);
    if (!Check(!liftBlockedProfile.bt72HullMovedToServiceLift &&
            !liftBlockedProfile.bt72HullLockedInRestorationCradle,
            "bt72_service_lift_requires_attached_hull expected lift to reject unattached hull")) {
        return false;
    }

    bunker::SessionProfile restoreBlockedProfile = bunker::MakeDefaultSessionProfile();
    bunker::RecoverPipPad(restoreBlockedProfile);
    restoreBlockedProfile.firstPlayableRoute.bt72HullInspected = true;
    restoreBlockedProfile.firstPlayableRoute.bt72CoreRecovered = true;
    restoreBlockedProfile.firstPlayableRoute.bt72ServiceNotesRecovered = true;
    restoreBlockedProfile.character.inventory.push_back({"power_cell", 1, 0.3f});
    restoreBlockedProfile.character.inventory.push_back({"repair_patch", 1, 0.2f});
    restoreBlockedProfile.character.inventory.push_back({"old_plate", 1, 0.5f});
    bunker::HandleInteraction(hullObject, world, player, restoreBlockedProfile, staticEraser, gameState);
    if (!Check(!restoreBlockedProfile.firstPlayableRoute.bt72Restored,
            "bt72_restore_rejected_before_cradle expected restore to reject pre-cradle hull")) {
        return false;
    }

    bunker::SessionProfile restoreProfile = bunker::MakeDefaultSessionProfile();
    bunker::RecoverPipPad(restoreProfile);
    restoreProfile.firstPlayableRoute.bt72HullInspected = true;
    restoreProfile.firstPlayableRoute.bt72CoreRecovered = true;
    restoreProfile.firstPlayableRoute.bt72ServiceNotesRecovered = true;
    restoreProfile.character.inventory.push_back({"power_cell", 1, 0.3f});
    restoreProfile.character.inventory.push_back({"repair_patch", 1, 0.2f});
    restoreProfile.character.inventory.push_back({"old_plate", 1, 0.5f});
    bunker::HandleInteraction(power, world, player, restoreProfile, staticEraser, gameState);
    bunker::HandleInteraction(control, world, player, restoreProfile, staticEraser, gameState);
    bunker::HandleInteraction(path, world, player, restoreProfile, staticEraser, gameState);
    bunker::HandleInteraction(hook, world, player, restoreProfile, staticEraser, gameState);
    bunker::HandleInteraction(lift, world, player, restoreProfile, staticEraser, gameState);
    bunker::HandleInteraction(hullObject, world, player, restoreProfile, staticEraser, gameState);
    return Check(restoreProfile.bt72HullLockedInRestorationCradle &&
            restoreProfile.firstPlayableRoute.bt72Restored &&
            restoreProfile.partnerTank.secondSeatUnlocked,
            "bt72_restore_allowed_after_cradle_core_notes_materials expected runtime restore after crane chain");
}

bool RunBt72ServiceNotesAndPatchRuntimeSmoke() {
    bunker::World world;
    auto addObject = [&](const std::string& registryId,
                         const std::string& displayName,
                         bunker::InteractionType interaction,
                         bunker::ObjectCategory category) {
        bunker::MapObject object;
        object.registryId = registryId;
        object.displayName = displayName;
        object.interaction = interaction;
        object.category = category;
        world.AddObject(object);
    };

    addObject("[%bt72_service_notes_0001]", "BT-72 Service Notes", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal);
    addObject("[%bt72_repair_patch_0001]", "BT-72 Repair Patch Locker", bunker::InteractionType::Container, bunker::ObjectCategory::Container);

    auto objectById = [&](const std::string& registryId) {
        return world.FindObjectByRegistryId(registryId);
    };

    bunker::PlayerState player;
    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;

    bunker::SessionProfile lockedProfile = bunker::MakeDefaultSessionProfile();
    bunker::HandleInteraction(objectById("[%bt72_service_notes_0001]"), world, player, lockedProfile, staticEraser, gameState);
    if (!Check(!lockedProfile.firstPlayableRoute.bt72ServiceNotesRecovered &&
            !bunker::HasCollectedTapeId(lockedProfile, "bt72_service_reel_001"),
            "bt72_service_notes_require_pippad expected service notes to stay locked before Pip-Pad")) {
        return false;
    }

    bunker::SessionProfile notesProfile = bunker::MakeDefaultSessionProfile();
    bunker::RecoverPipPad(notesProfile);
    notesProfile.story.archiveRecovered = true;
    bunker::HandleInteraction(objectById("[%bt72_service_notes_0001]"), world, player, notesProfile, staticEraser, gameState);
    if (!Check(notesProfile.firstPlayableRoute.bt72ServiceNotesRecovered &&
            bunker::HasCollectedTapeId(notesProfile, "bt72_service_reel_001") &&
            !notesProfile.story.tankLinked &&
            !notesProfile.firstPlayableRoute.bt72Restored,
            "bt72_service_notes_runtime_recovered expected runtime service notes without tank unlock")) {
        return false;
    }

    bunker::SessionProfile patchProfile = bunker::MakeDefaultSessionProfile();
    bunker::RecoverPipPad(patchProfile);
    bunker::HandleInteraction(objectById("[%bt72_repair_patch_0001]"), world, player, patchProfile, staticEraser, gameState);
    if (!Check(bunker::HasInventoryItem(patchProfile, "repair_patch") &&
            !patchProfile.firstPlayableRoute.bt72Restored &&
            !patchProfile.story.tankLinked,
            "bt72_repair_patch_runtime_recovered expected runtime repair patch without tank unlock")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[%bt72_service_notes_0001]"), world, player, notesProfile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_repair_patch_0001]"), world, player, patchProfile, staticEraser, gameState);
    return Check(notesProfile.firstPlayableRoute.bt72ServiceNotesRecovered &&
            bunker::HasCollectedTapeId(notesProfile, "bt72_service_reel_001") &&
            bunker::HasInventoryItem(patchProfile, "repair_patch") &&
            !notesProfile.firstPlayableRoute.bt72Restored &&
            !patchProfile.firstPlayableRoute.bt72Restored,
            "bt72_service_material_pickups_are_idempotent expected stable duplicate interactions");
}

const std::vector<std::string>& RequiredFirstRouteRuntimeObjectIds() {
    static const std::vector<std::string> ids = {
        "[%cryo_0001]",
        "[%core_0001]",
        "[%pip_0001]",
        "[%archive_0001]",
        "[%garage_0001]",
        "[#tr_hull_0001]",
        "[%bluelink_module_0001]",
        "[%pippad_expansion_bay_0001]",
        "[%hangar_power_0001]",
        "[%bt72_crane_control_0001]",
        "[%bt72_crane_path_0001]",
        "[%bt72_crane_hook_0001]",
        "[%bt72_service_lift_0001]",
        "[%bt72_service_notes_0001]",
        "[%bt72_repair_patch_0001]",
    };
    return ids;
}

const std::vector<std::string>& V940StarterRouteRuntimeObjectIds() {
    static const std::vector<std::string> ids = {
        "[%bluelink_module_0001]",
        "[%pippad_expansion_bay_0001]",
        "[%hangar_power_0001]",
        "[%bt72_crane_control_0001]",
        "[%bt72_crane_path_0001]",
        "[%bt72_crane_hook_0001]",
        "[%bt72_service_lift_0001]",
        "[%bt72_service_notes_0001]",
        "[%bt72_repair_patch_0001]",
    };
    return ids;
}

bool HasRequiredFirstRouteRuntimeObjects(const bunker::World& world) {
    return std::all_of(RequiredFirstRouteRuntimeObjectIds().begin(),
        RequiredFirstRouteRuntimeObjectIds().end(),
        [&](const std::string& registryId) {
            return world.FindObjectByRegistryId(registryId) != nullptr;
        });
}

bool HasV940StarterRouteRuntimeObjects(const bunker::World& world) {
    return std::all_of(V940StarterRouteRuntimeObjectIds().begin(),
        V940StarterRouteRuntimeObjectIds().end(),
        [&](const std::string& registryId) {
            return world.FindObjectByRegistryId(registryId) != nullptr;
        });
}

bool StarterRouteRuntimeObjectTypesMatch(const bunker::World& world) {
    const auto hasTypedObject = [&](const std::string& registryId,
                                    bunker::InteractionType interaction,
                                    bunker::ObjectCategory category) {
        const auto* object = world.FindObjectByRegistryId(registryId);
        return object != nullptr &&
            object->interaction == interaction &&
            object->category == category;
    };

    return hasTypedObject("[%bluelink_module_0001]", bunker::InteractionType::Container, bunker::ObjectCategory::Container) &&
        hasTypedObject("[%pippad_expansion_bay_0001]", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal) &&
        hasTypedObject("[%hangar_power_0001]", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal) &&
        hasTypedObject("[%bt72_crane_control_0001]", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal) &&
        hasTypedObject("[%bt72_crane_path_0001]", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal) &&
        hasTypedObject("[%bt72_crane_hook_0001]", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal) &&
        hasTypedObject("[%bt72_service_lift_0001]", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal) &&
        hasTypedObject("[%bt72_service_notes_0001]", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal) &&
        hasTypedObject("[%bt72_repair_patch_0001]", bunker::InteractionType::Container, bunker::ObjectCategory::Container) &&
        hasTypedObject("[#tr_hull_0001]", bunker::InteractionType::VehicleAnchor, bunker::ObjectCategory::Vehicle);
}

bool RunStarterWorldFirstRouteObjectAvailabilitySmoke() {
    bunker::World world;
    world.GeneratePrototypeZone();
    world.EnsureStarterInfrastructure();

    const auto hasObject = [&](const std::string& registryId) {
        return world.FindObjectByRegistryId(registryId) != nullptr;
    };

    if (!Check(world.IsStarterScenarioWorld(),
            "starter_world_contains_pippad_and_bt72_route_objects expected starter scenario world")) {
        return false;
    }
    if (!Check(hasObject("[%cryo_0001]") &&
            hasObject("[%core_0001]") &&
            hasObject("[%pip_0001]") &&
            hasObject("[%archive_0001]") &&
            hasObject("[%garage_0001]") &&
            hasObject("[#tr_hull_0001]"),
            "starter_world_contains_pippad_and_bt72_route_objects expected core route objects")) {
        return false;
    }
    if (!Check(hasObject("[%bluelink_module_0001]") &&
            hasObject("[%pippad_expansion_bay_0001]"),
            "starter_world_contains_bluelink_runtime_objects expected BlueLink runtime objects")) {
        return false;
    }
    if (!Check(hasObject("[%hangar_power_0001]") &&
            hasObject("[%bt72_crane_control_0001]") &&
            hasObject("[%bt72_crane_path_0001]") &&
            hasObject("[%bt72_crane_hook_0001]") &&
            hasObject("[%bt72_service_lift_0001]"),
            "starter_world_contains_bt72_crane_runtime_objects expected crane runtime objects")) {
        return false;
    }
    return Check(hasObject("[%bt72_service_notes_0001]") &&
            hasObject("[%bt72_repair_patch_0001]"),
            "starter_world_contains_bt72_service_material_objects expected service/material runtime objects");
}

bool RunStarterWorldFirstPlayableRouteSmoke() {
    bunker::World world;
    world.GeneratePrototypeZone();
    world.EnsureStarterInfrastructure();

    auto objectById = [&](const std::string& registryId) {
        return world.FindObjectByRegistryId(registryId);
    };

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    bunker::PlayerState player;
    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;

    bunker::HandleInteraction(objectById("[%cryo_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%core_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%pip_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.story.awakenedFromCryo &&
            profile.continuityAnchorSeeded &&
            profile.firstPlayableRoute.accessCardRecovered &&
            bunker::HasInventoryItem(profile, "bunker_access_card") &&
            bunker::HasPipPad(profile) &&
            bunker::HasInventoryItem(profile, "#%it_pippad"),
            "starter_world_e2e_cryo_to_pippad expected cryo, access card, and Pip-Pad recovery")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[%bluelink_module_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.blueLinkModuleRecovered &&
            !bunker::CanUsePipPadMediaIndex(profile) &&
            !profile.story.archiveRecovered &&
            !profile.story.relayRecovered &&
            !profile.story.returnedToBase,
            "starter_world_e2e_bluelink_installed expected BlueLink pickup without story unlock")) {
        return false;
    }
    bunker::HandleInteraction(objectById("[%pippad_expansion_bay_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(bunker::IsBlueLinkInstalled(profile) &&
            bunker::CanUsePipPadMediaIndex(profile) &&
            !profile.pipPadExpansionCoverPresent &&
            !profile.story.archiveRecovered &&
            !profile.story.relayRecovered &&
            !profile.story.returnedToBase,
            "starter_world_e2e_bluelink_installed expected installed media module only")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[%archive_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%garage_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%core_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_service_notes_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_repair_patch_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.story.archiveRecovered &&
            bunker::HasCollectedTapeId(profile, "archive_missing_personnel") &&
            profile.firstPlayableRoute.bt72HullInspected &&
            profile.firstPlayableRoute.bt72CoreRecovered &&
            profile.firstPlayableRoute.bt72ServiceNotesRecovered &&
            bunker::HasCollectedTapeId(profile, "bt72_service_reel_001") &&
            bunker::HasInventoryItem(profile, "old_plate") &&
            bunker::HasInventoryItem(profile, "power_cell") &&
            bunker::HasInventoryItem(profile, "repair_patch"),
            "starter_world_e2e_archive_and_bt72_knowledge expected archive and BT-72 route knowledge/materials")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[%hangar_power_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_crane_control_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_crane_path_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_crane_hook_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_service_lift_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.hangarPowerRestored &&
            profile.bt72CraneControlOnline &&
            profile.bt72CranePathClear &&
            !profile.bt72HullAttachedToCrane &&
            profile.bt72HullMovedToServiceLift &&
            profile.bt72HullLockedInRestorationCradle,
            "starter_world_e2e_crane_cradle_locked expected authored crane chain to lock cradle")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[#tr_hull_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.firstPlayableRoute.bt72Restored &&
            profile.partnerTank.secondSeatUnlocked &&
            !profile.story.tankLinked,
            "starter_world_e2e_bt72_restored_and_linked expected restore before cockpit link")) {
        return false;
    }
    bunker::HandleInteraction(objectById("[#tr_hull_0001]"), world, player, profile, staticEraser, gameState);
    return Check(player.insideTank &&
            profile.story.tankLinked &&
            profile.partnerTank.deployed &&
            player.viewMode == bunker::ViewMode::Cockpit &&
            bunker::CurrentStoryObjectivePreview(profile).find("Restore BT-72") == std::string::npos,
            "starter_world_e2e_bt72_restored_and_linked expected cockpit link and advanced objective");
}

bool RunStarterWorldRouteObjectsPersistAfterSaveLoadSmoke() {
    const fs::path tempRoot = fs::current_path() / "starter_world_route_persistence_smoke";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);
    if (!Check(!ec, "starter_world_route_objects_persist_after_save_load failed to create temp directory")) {
        return false;
    }

    bunker::World world;
    world.GeneratePrototypeZone();
    world.EnsureStarterInfrastructure();
    if (!Check(world.IsStarterScenarioWorld(),
            "starter_world_route_objects_persist_after_save_load expected generated starter world")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    const fs::path worldPath = tempRoot / "starter_route_persistence.bwld";
    if (!Check(world.Save(worldPath.string()),
            "starter_world_route_objects_persist_after_save_load failed to save starter world")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    bunker::World loadedWorld;
    if (!Check(loadedWorld.Load(worldPath.string()),
            "starter_world_route_objects_persist_after_save_load failed to load starter world")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    loadedWorld.EnsureStarterInfrastructure();

    const bool ok = Check(loadedWorld.IsStarterScenarioWorld() &&
            HasRequiredFirstRouteRuntimeObjects(loadedWorld),
            "starter_world_route_objects_persist_after_save_load expected all first route objects") &&
        Check(StarterRouteRuntimeObjectTypesMatch(loadedWorld),
            "starter_world_route_object_types_persist_after_save_load expected route object interaction types");

    fs::remove_all(tempRoot, ec);
    return ok;
}

bool RunLegacyStarterWorldGetsV940RouteInfrastructureSmoke() {
    bunker::World legacyWorld;
    legacyWorld.GeneratePrototypeZone();
    for (const auto& registryId : V940StarterRouteRuntimeObjectIds()) {
        legacyWorld.RemoveObject(registryId);
    }

    if (!Check(legacyWorld.IsStarterScenarioWorld() &&
            !HasV940StarterRouteRuntimeObjects(legacyWorld),
            "legacy_starter_world_missing_v940_objects_before_upgrade expected removed v9.40 route objects")) {
        return false;
    }

    legacyWorld.EnsureStarterInfrastructure();
    if (!Check(HasV940StarterRouteRuntimeObjects(legacyWorld),
            "legacy_starter_world_gets_v940_route_infrastructure expected EnsureStarterInfrastructure to restore v9.40 objects")) {
        return false;
    }

    const auto objectCountAfterUpgrade = legacyWorld.objects.size();
    legacyWorld.EnsureStarterInfrastructure();
    return Check(legacyWorld.objects.size() == objectCountAfterUpgrade,
            "starter_infrastructure_upgrade_is_idempotent expected second infrastructure pass not to duplicate objects");
}

bool RunLoadedStarterWorldFirstPlayableRouteSmoke() {
    const fs::path tempRoot = fs::current_path() / "loaded_starter_world_route_smoke";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);
    if (!Check(!ec, "loaded_starter_world_route_remains_playable_after_save_load failed to create temp directory")) {
        return false;
    }

    bunker::World world;
    world.GeneratePrototypeZone();
    world.EnsureStarterInfrastructure();
    const fs::path worldPath = tempRoot / "starter_route_playable.bwld";
    if (!Check(world.Save(worldPath.string()),
            "loaded_starter_world_route_remains_playable_after_save_load failed to save starter world")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    bunker::World loadedWorld;
    if (!Check(loadedWorld.Load(worldPath.string()),
            "loaded_starter_world_route_remains_playable_after_save_load failed to load starter world")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    loadedWorld.EnsureStarterInfrastructure();
    if (!Check(loadedWorld.IsStarterScenarioWorld() &&
            HasRequiredFirstRouteRuntimeObjects(loadedWorld),
            "loaded_starter_world_route_remains_playable_after_save_load expected loaded starter route objects")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }

    auto objectById = [&](const std::string& registryId) {
        return loadedWorld.FindObjectByRegistryId(registryId);
    };

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    bunker::PlayerState player;
    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;

    bunker::HandleInteraction(objectById("[%cryo_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%core_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%pip_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bluelink_module_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%pippad_expansion_bay_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%archive_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%garage_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%core_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_service_notes_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_repair_patch_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    if (!Check(profile.story.archiveRecovered &&
            profile.firstPlayableRoute.bt72HullInspected &&
            profile.firstPlayableRoute.bt72CoreRecovered &&
            profile.firstPlayableRoute.bt72ServiceNotesRecovered &&
            bunker::HasInventoryItem(profile, "power_cell") &&
            bunker::HasInventoryItem(profile, "old_plate") &&
            bunker::HasInventoryItem(profile, "repair_patch"),
            "loaded_starter_world_route_remains_playable_after_save_load expected archive, BT-72 knowledge, and materials before restore")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    bunker::HandleInteraction(objectById("[%hangar_power_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_crane_control_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_crane_path_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_crane_hook_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_service_lift_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[#tr_hull_0001]"), loadedWorld, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[#tr_hull_0001]"), loadedWorld, player, profile, staticEraser, gameState);

    const bool ok = Check(bunker::HasPipPad(profile) &&
            bunker::IsBlueLinkInstalled(profile) &&
            bunker::CanUsePipPadMediaIndex(profile),
            "loaded_starter_world_route_remains_playable_after_save_load expected Pip-Pad and BlueLink after load") &&
        Check(profile.story.archiveRecovered,
            "loaded_starter_world_route_remains_playable_after_save_load expected archive after load") &&
        Check(profile.firstPlayableRoute.bt72HullInspected &&
                profile.firstPlayableRoute.bt72CoreRecovered &&
                profile.firstPlayableRoute.bt72ServiceNotesRecovered,
            "loaded_starter_world_route_remains_playable_after_save_load expected BT-72 knowledge after load") &&
        Check(profile.bt72HullLockedInRestorationCradle,
            "loaded_starter_world_route_remains_playable_after_save_load expected cradle after load") &&
        Check(profile.firstPlayableRoute.bt72Restored,
            "loaded_starter_world_route_remains_playable_after_save_load expected BT-72 restore after load") &&
        Check(player.insideTank &&
                profile.story.tankLinked &&
                bunker::CurrentStoryObjectivePreview(profile).find("Restore BT-72") == std::string::npos,
            "loaded_starter_world_route_remains_playable_after_save_load expected cockpit link and advanced objective after load");

    fs::remove_all(tempRoot, ec);
    return ok;
}

bool RunFirstPlayableRouteEndToEndSmoke() {
    bunker::World world;
    auto addObject = [&](const std::string& registryId,
                         const std::string& displayName,
                         bunker::InteractionType interaction,
                         bunker::ObjectCategory category) {
        bunker::MapObject object;
        object.registryId = registryId;
        object.displayName = displayName;
        object.interaction = interaction;
        object.category = category;
        world.AddObject(object);
    };

    addObject("[%cryo_0001]", "Cryo Bay", bunker::InteractionType::Static, bunker::ObjectCategory::Structure);
    addObject("[%core_0001]", "Core Rack", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal);
    addObject("[%pip_0001]", "Pip-Pad Locker", bunker::InteractionType::Container, bunker::ObjectCategory::Container);
    addObject("[%bluelink_module_0001]", "BlueLink Media Module", bunker::InteractionType::Container, bunker::ObjectCategory::Container);
    addObject("[%pippad_expansion_bay_0001]", "Pip-Pad Expansion Bay", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal);
    addObject("[%archive_0001]", "Archive Terminal", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal);
    addObject("[%garage_0001]", "BT-72 Garage Lift", bunker::InteractionType::Terminal, bunker::ObjectCategory::Hangar);
    addObject("[%hangar_power_0001]", "Hangar Power Bus", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal);
    addObject("[%bt72_crane_control_0001]", "BT-72 Crane Control", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal);
    addObject("[%bt72_crane_path_0001]", "BT-72 Crane Path", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal);
    addObject("[%bt72_crane_hook_0001]", "BT-72 Crane Hook", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal);
    addObject("[%bt72_service_lift_0001]", "BT-72 Service Lift", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal);
    addObject("[%bt72_service_notes_0001]", "BT-72 Service Notes", bunker::InteractionType::Terminal, bunker::ObjectCategory::Terminal);
    addObject("[%bt72_repair_patch_0001]", "BT-72 Repair Patch Locker", bunker::InteractionType::Container, bunker::ObjectCategory::Container);
    addObject("[#tr_hull_0001]", "BT-72 Hull", bunker::InteractionType::VehicleAnchor, bunker::ObjectCategory::Vehicle);

    auto objectById = [&](const std::string& registryId) {
        return world.FindObjectByRegistryId(registryId);
    };

    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    bunker::PlayerState player;
    bunker::StaticEraser staticEraser;
    bunker::GameState gameState;

    bunker::HandleInteraction(objectById("[%cryo_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.story.awakenedFromCryo &&
            profile.continuityAnchorSeeded &&
            !bunker::HasPipPad(profile) &&
            !player.uiVisible,
            "first_route_e2e_cryo_seeds_continuity expected cryo wake without Pip-Pad UI")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[%core_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.firstPlayableRoute.accessCardRecovered &&
            bunker::HasInventoryItem(profile, "bunker_access_card") &&
            profile.firstPlayableRoute.prePipPadClueCount >= 2,
            "first_route_e2e_pippad_recovered_before_ui expected access card and pre-Pip-Pad trail")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[%pip_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(bunker::HasPipPad(profile) &&
            profile.story.pipPadRecovered &&
            bunker::HasInventoryItem(profile, "#%it_pippad") &&
            bunker::TryTogglePipPadUi(player, profile, gameState) &&
            player.uiVisible &&
            bunker::TryTogglePipPadUi(player, profile, gameState) &&
            !player.uiVisible,
            "first_route_e2e_pippad_recovered expected Pip-Pad pickup before UI access")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[%bluelink_module_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.blueLinkModuleRecovered &&
            !bunker::CanUsePipPadMediaIndex(profile),
            "first_route_e2e_bluelink_media_ready expected BlueLink pickup without media unlock")) {
        return false;
    }
    bunker::HandleInteraction(objectById("[%pippad_expansion_bay_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(bunker::IsBlueLinkInstalled(profile) &&
            bunker::CanUsePipPadMediaIndex(profile) &&
            !profile.pipPadExpansionCoverPresent &&
            !profile.story.archiveRecovered &&
            !profile.story.relayRecovered &&
            !profile.story.returnedToBase,
            "first_route_e2e_bluelink_media_ready expected BlueLink media only, no story/map unlock")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[%archive_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.story.archiveRecovered &&
            bunker::HasCollectedTapeId(profile, "archive_missing_personnel") &&
            (bunker::CurrentStoryObjectivePreview(profile).find("BT-72") != std::string::npos ||
                bunker::CurrentFirstPlayableRouteBeat(profile).label.find("BT-72") != std::string::npos),
            "first_route_e2e_archive_synced expected archive sync to point toward BT-72")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[%garage_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%core_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_service_notes_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_repair_patch_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.firstPlayableRoute.bt72HullInspected &&
            bunker::HasInventoryItem(profile, "old_plate") &&
            profile.firstPlayableRoute.bt72CoreRecovered &&
            bunker::HasInventoryItem(profile, "power_cell") &&
            profile.firstPlayableRoute.bt72ServiceNotesRecovered &&
            bunker::HasInventoryItem(profile, "repair_patch"),
            "first_route_e2e_bt72_crane_cradle_locked expected hull, core, notes, and materials staged")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[%hangar_power_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_crane_control_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_crane_path_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_crane_hook_0001]"), world, player, profile, staticEraser, gameState);
    bunker::HandleInteraction(objectById("[%bt72_service_lift_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.hangarPowerRestored &&
            profile.bt72CraneControlOnline &&
            profile.bt72CranePathClear &&
            !profile.bt72HullAttachedToCrane &&
            profile.bt72HullMovedToServiceLift &&
            profile.bt72HullLockedInRestorationCradle,
            "first_route_e2e_bt72_crane_cradle_locked expected crane chain to lock cradle")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[#tr_hull_0001]"), world, player, profile, staticEraser, gameState);
    if (!Check(profile.firstPlayableRoute.bt72Restored &&
            profile.partnerTank.secondSeatUnlocked &&
            !profile.story.tankLinked &&
            !player.insideTank,
            "first_route_e2e_bt72_restored expected restore before cockpit link")) {
        return false;
    }

    bunker::HandleInteraction(objectById("[#tr_hull_0001]"), world, player, profile, staticEraser, gameState);
    const std::string previewAfterLink = bunker::CurrentStoryObjectivePreview(profile);
    const std::string objectiveAfterLink = bunker::CurrentStoryObjective(profile, staticEraser);
    if (!Check(player.insideTank &&
            profile.story.tankLinked &&
            profile.partnerTank.deployed &&
            player.viewMode == bunker::ViewMode::Cockpit &&
            previewAfterLink.find("Restore BT-72") == std::string::npos &&
            objectiveAfterLink.find("Restore BT-72") == std::string::npos,
            "first_route_e2e_bt72_link_established expected cockpit link and next objective")) {
        return false;
    }

    const fs::path tempRoot = fs::current_path() / "first_playable_route_e2e_smoke";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);
    if (!Check(!ec, "first_route_e2e_profile_persists_final_state failed to create temp directory")) {
        return false;
    }
    const fs::path profilePath = tempRoot / "profile.txt";
    const auto saveStatus = bunker::SaveProfileAtomically(profile, profilePath);
    if (!Check(saveStatus.ok, "first_route_e2e_profile_persists_final_state failed to save profile: " + saveStatus.message)) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    bunker::SessionProfile loadedProfile;
    if (!Check(bunker::LoadSessionProfile(profilePath, loadedProfile),
            "first_route_e2e_profile_persists_final_state failed to load profile")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    const bool ok = Check(loadedProfile.story.pipPadRecovered &&
            loadedProfile.blueLinkModuleInstalled &&
            loadedProfile.bt72HullLockedInRestorationCradle &&
            loadedProfile.firstPlayableRoute.bt72Restored &&
            loadedProfile.story.tankLinked &&
            loadedProfile.continuityAnchorSeeded,
            "first_route_e2e_profile_persists_final_state expected final profile state to persist");
    fs::remove_all(tempRoot, ec);
    return ok;
}

bool RunPipPadAccessGatingSmoke() {
    bunker::SessionProfile profile = bunker::MakeDefaultSessionProfile();
    bunker::PlayerState player;
    bunker::GameState gameState;

    if (!Check(!player.uiVisible, "pip-pad gating smoke expected default UI to start hidden")) {
        return false;
    }
    if (!Check(!profile.story.pipPadRecovered &&
            !bunker::HasPipPad(profile) &&
            !bunker::PlayerHasPipPadAccess(profile),
            "pip_pad_not_available_at_cryo_wake expected no Pip-Pad access before pickup")) {
        return false;
    }
    player.uiVisible = true;
    if (!Check(!bunker::TryTogglePipPadUi(player, profile, gameState),
            "pip_pad_not_available_at_cryo_wake expected TAB helper to reject missing Pip-Pad")) {
        return false;
    }
    if (!Check(!player.uiVisible &&
            gameState.lastEvent.find("No Pip-Pad linked yet") != std::string::npos,
            "pip_pad_not_available_at_cryo_wake expected missing Pip-Pad feedback and hidden UI")) {
        return false;
    }

    bunker::World world;
    bunker::MapObject pipPad;
    pipPad.registryId = "[%pip_0001]";
    pipPad.displayName = "Pip-Pad Locker";
    pipPad.interaction = bunker::InteractionType::Container;
    pipPad.category = bunker::ObjectCategory::Container;
    world.AddObject(pipPad);

    profile.firstPlayableRoute.accessCardRecovered = true;
    profile.firstPlayableRoute.prePipPadClueCount = 2;
    profile.selectedWorld = "pip_pad_access_gating_smoke.bwld";
    bunker::StaticEraser staticEraser;
    const auto* locker = world.FindObjectByRegistryId("[%pip_0001]");
    bunker::HandleInteraction(locker, world, player, profile, staticEraser, gameState);

    if (!Check(profile.story.pipPadRecovered &&
            bunker::HasPipPad(profile) &&
            bunker::PlayerHasPipPadAccess(profile) &&
            bunker::HasInventoryItem(profile, "#%it_pippad"),
            "pip_pad_recovered_after_route_pickup expected pickup to set story flag and inventory access")) {
        return false;
    }
    if (!Check(!player.uiVisible,
            "pip-pad gating smoke expected pickup to leave UI hidden until TAB")) {
        return false;
    }
    if (!Check(gameState.lastEvent.find("Press TAB") != std::string::npos,
            "pip-pad gating smoke expected pickup feedback to mention TAB")) {
        return false;
    }

    const fs::path tempRoot = fs::current_path() / "pip_pad_access_gating_smoke";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);
    if (!Check(!ec, "pip_pad_recovery_persists_after_save_load failed to create temp directory")) {
        return false;
    }
    const fs::path profilePath = tempRoot / "profile.txt";
    const auto saveStatus = bunker::SaveProfileAtomically(profile, profilePath);
    if (!Check(saveStatus.ok, "pip_pad_recovery_persists_after_save_load failed to save profile: " + saveStatus.message)) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    bunker::SessionProfile loadedProfile;
    if (!Check(bunker::LoadSessionProfile(profilePath, loadedProfile),
            "pip_pad_recovery_persists_after_save_load failed to load profile")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    if (!Check(loadedProfile.story.pipPadRecovered &&
            bunker::HasPipPad(loadedProfile) &&
            bunker::PlayerHasPipPadAccess(loadedProfile),
            "pip_pad_recovery_persists_after_save_load expected recovered Pip-Pad access")) {
        fs::remove_all(tempRoot, ec);
        return false;
    }
    fs::remove_all(tempRoot, ec);

    return Check(bunker::TryTogglePipPadUi(player, profile, gameState) && player.uiVisible,
            "pip-pad gating smoke expected TAB helper to open after pickup") &&
        Check(bunker::TryTogglePipPadUi(player, profile, gameState) && !player.uiVisible,
            "pip-pad gating smoke expected TAB helper to close after pickup");
}

}  // namespace

int main() {
    const fs::path sandboxRoot = fs::current_path() / ".bunker_smoke_checks";
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
        RunEditorWorldFileHelpersSmoke() &&
        RunProfileRoundtrip() &&
        RunProfileMigrationContractSmoke() &&
        RunContinuityAnchorContractSmoke() &&
        RunFirstPlayableRouteStorySmoke() &&
        RunRouteBeatPresentationSmoke() &&
        RunFirstPlayableRouteReadoutSmoke() &&
        RunBt72RestorationObjectiveReadoutSmoke() &&
        RunSurfaceArrivalWorldEventSmoke() &&
        RunFirstCombatWorldEventSmoke() &&
        RunFirstCombatResolutionHandoffSmoke() &&
        RunWorkshopServiceRouteHandoffSmoke() &&
        RunScalableContainerLootRuntimeSmoke() &&
        RunRuntimeProfileSaveDoesNotReplaceAuthoringWorldSmoke() &&
        RunDebriefIndustrialHandoffSmoke() &&
        RunRecoveryHandoffSummarySmoke() &&
        RunRecoveryBackboneStatusSmoke() &&
        RunRouteEventLifecycleSmoke() &&
        RunMerchantRouteEventSmoke() &&
        RunWeatherWeightedRouteEventSmoke() &&
        RunWorldScopedRouteSummarySmoke() &&
        RunHostileAwarenessSmoke() &&
        RunHumanTriggerDisciplineSmoke() &&
        RunHumanCoverSeekingSmoke() &&
        RunBt72CombatFeedbackSmoke() &&
        RunReactiveBreakableGlassSmoke() &&
        RunReactiveBreakableFoliageSmoke() &&
        RunMechanicalHostileDamageSmoke() &&
        RunFieldReflexRpgWeightSmoke() &&
        RunBt72CrewCoordinationWeightSmoke() &&
        RunBt72WeakPointComboSmoke() &&
        RunBt72SeatPolicySmoke() &&
        RunServiceChoiceWeightSmoke() &&
        RunLauncherAnnouncementSmoke() &&
        RunLanlineServicesRoundtripSmoke() &&
        RunTankServiceKitSmoke() &&
        RunLanlineSeatRoundtripSmoke() &&
        RunLaunchTicketFlow() &&
        RunGameplayDescriptorValidationSmoke() &&
        RunSemanticDependencyValidationSmoke() &&
        RunSemanticDependencyGraphSmoke() &&
        RunSemanticLayoutSmoke() &&
        RunSemanticLayoutPreserveManualSmoke() &&
        RunSemanticAuthoringStateRoundtripSmoke() &&
        RunSemanticAutoAnchorValidationSmoke() &&
        RunPrefabLibrarySemanticStateSmoke() &&
        RunLegacyPrefabManualLootMigrationSmoke() &&
        RunPrefabUsageAndExportReportSmoke() &&
        RunSupportedFileFormatRegistrySmoke() &&
        RunExportDataScanSmoke() &&
        RunStrictSemanticExportPolicySmoke() &&
        RunValidatedWorldExportArtifactSmoke() &&
        RunWorldExportAuditTrailSmoke() &&
        RunShippingBaselineDiffSmoke() &&
        RunShippingBaselineObjectAwareDriftSmoke() &&
        RunExportHistoryCheckpointSelectionSmoke() &&
        RunLegacySemanticAutoInferenceSmoke() &&
        RunLegacyManualLootMigrationSmoke() &&
        RunLegacyWorldEditorLayerInferenceSmoke() &&
        RunWorldEditorUndoSmoke() &&
        RunWorldReferenceGraphSmoke() &&
        RunSemanticAuthoringCascadeSmoke() &&
        RunStarterSemanticLinkTargetSmoke() &&
        RunLegacyWorldAliasMigrationSmoke() &&
        RunGameExecutionResourceLookupSmoke() &&
        RunGameExecutionLootTemplateSmoke() &&
        RunGameExecutionScriptBridgeSmoke() &&
        RunPlayerSweepCollisionSmoke() &&
        RunRuntimeCameraSmoke() &&
        RunPipDeviceCapabilityContractSmoke() &&
        RunBlueLinkExpansionModuleContractSmoke() &&
        RunBlueLinkRuntimeInteractionSmoke() &&
        RunBt72CraneRestorationContractSmoke() &&
        RunBt72CraneRuntimeInteractionSmoke() &&
        RunBt72ServiceNotesAndPatchRuntimeSmoke() &&
        RunStarterWorldFirstRouteObjectAvailabilitySmoke() &&
        RunStarterWorldFirstPlayableRouteSmoke() &&
        RunStarterWorldRouteObjectsPersistAfterSaveLoadSmoke() &&
        RunLegacyStarterWorldGetsV940RouteInfrastructureSmoke() &&
        RunLoadedStarterWorldFirstPlayableRouteSmoke() &&
        RunFirstPlayableRouteEndToEndSmoke() &&
        RunPipPadAccessGatingSmoke();

    fs::remove_all(sandboxRoot, ec);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
