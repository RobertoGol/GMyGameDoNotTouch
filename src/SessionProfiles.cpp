#include "../include/SessionProfiles.hpp"
#include <fstream>

namespace bunker {

namespace {

constexpr char kSessionProfileFormat[] = "BPF1";
constexpr int kCurrentSessionProfileVersion = 14;

bool HasInventoryEntry(const SessionProfile& profile, const std::string& itemId) {
    return std::any_of(
        profile.character.inventory.begin(),
        profile.character.inventory.end(),
        [&](const InventoryEntry& entry) { return entry.itemId == itemId && entry.count > 0; });
}

std::string FirstInventoryPipDeviceId(const SessionProfile& profile) {
    for (const auto& item : profile.character.inventory) {
        if (item.count > 0 && IsCanonicalPipDeviceItemId(item.itemId)) {
            return item.itemId;
        }
    }
    return {};
}

bool IsKnownPipDeviceThemeId(std::string_view themeId) {
    return themeId == "classic_green" ||
           themeId == "amber" ||
           themeId == "monochrome" ||
           themeId == "high_contrast";
}

bool IsKnownPipDeviceDisplayMode(std::string_view displayMode) {
    return displayMode == "standard" ||
           displayMode == "readable" ||
           displayMode == "compact";
}

bool IsKnownPipDeviceCarryMode(std::string_view carryMode) {
    return carryMode == "auto" ||
           carryMode == "wrist" ||
           carryMode == "handheld";
}

void NormalizeFirstPlayableRouteProgress(SessionProfile& profile) {
    auto& route = profile.firstPlayableRoute;
    route.prePipPadClueCount = std::clamp(route.prePipPadClueCount, 0, 2);
    route.introSeen = route.introSeen || profile.story.awakenedFromCryo;
    route.emergencyMeleeRecovered = route.emergencyMeleeRecovered || 
        HasInventoryEntry(profile, "#%it_emergency_baton");
    route.accessCardRecovered = route.accessCardRecovered ||
        HasInventoryEntry(profile, "bunker_access_card") ||
        profile.story.pipPadRecovered;
    route.earlyVerminEncounterResolved = route.earlyVerminEncounterResolved ||
        profile.character.awakening.footKills > 0 ||
        profile.story.tankLinked;
    route.bt72HullInspected = route.bt72HullInspected || profile.story.tankLinked || 
        profile.story.bucketRecovered;
    route.bt72CoreRecovered = route.bt72CoreRecovered || profile.story.tankLinked || 
        profile.story.bucketRecovered;
    route.bt72ServiceNotesRecovered = route.bt72ServiceNotesRecovered ||
        profile.story.tankLinked ||
        profile.story.bucketRecovered ||
        HasCollectedTapeId(profile, "bt72_service_reel_001");
    route.bt72Restored = route.bt72Restored || profile.story.tankLinked || 
        profile.story.bucketRecovered;
    route.clearanceBlueprintRecovered = route.clearanceBlueprintRecovered ||
        profile.story.bucketRecovered ||
        HasCollectedTapeId(profile, "bt72_clearance_blueprint");
    route.clearanceMaterialsRecovered = route.clearanceMaterialsRecovered || 
        profile.story.bucketRecovered;
    route.clearanceModuleInstalled = route.clearanceModuleInstalled || 
        profile.story.bucketRecovered;
    route.surfaceArrivalReached = route.surfaceArrivalReached ||
        profile.story.outerRoadCleared ||
        profile.story.relayRecovered ||
        profile.story.returnedToBase;
    route.firstTankCombatResolved = route.firstTankCombatResolved || 
        profile.story.relayRecovered || profile.story.returnedToBase;
    route.firstServicePerformed = route.firstServicePerformed ||
        profile.story.relayRecovered ||
        profile.story.returnedToBase ||
        profile.character.awakening.fieldServiceUses > 0;
    route.firstRecoveryNodeActivated = route.firstRecoveryNodeActivated || 
        profile.story.relayRecovered;
    route.debriefSummaryViewed = route.debriefSummaryViewed ||
        profile.story.returnedToBase ||
        HasCollectedTapeId(profile, "debrief_shelter17");
    profile.partnerTank.secondSeatUnlocked = profile.partnerTank.secondSeatUnlocked || 
        route.bt72Restored || profile.story.tankLinked;
    profile.partnerTank.secondSeatPolicy = 
        NormalizeBt72SecondSeatPolicy(profile.partnerTank.secondSeatPolicy);
    if (!profile.partnerTank.secondSeatUnlocked) {
        profile.partnerTank.secondSeatPolicy = "pilot_only";
        profile.partnerTank.assignedGunnerHandle.clear();
    } else if (profile.partnerTank.secondSeatPolicy == "pilot_only" &&
        profile.partnerTank.assignedGunnerHandle == profile.character.displayName) {
        profile.partnerTank.assignedGunnerHandle.clear();
    }
    profile.story.bucketRecovered = profile.story.bucketRecovered || 
        route.clearanceModuleInstalled;
    profile.story.relayRecovered = profile.story.relayRecovered || 
        route.firstRecoveryNodeActivated;
    profile.story.returnedToBase = profile.story.returnedToBase || route.debriefSummaryViewed;
}

void MigrateSessionProfile(SessionProfile& profile, int loadedVersion) {
    if (loadedVersion < 2) {
        if (profile.fieldCheckpointKnown && profile.fieldCheckpointWorld.empty()) {
            profile.fieldCheckpointWorld = profile.selectedWorld;
        }
    }
    if (loadedVersion < 4) {
        auto& route = profile.firstPlayableRoute;
        route.introSeen = profile.story.awakenedFromCryo;
        route.emergencyMeleeRecovered = profile.story.awakenedFromCryo;
        route.accessCardRecovered = profile.story.pipPadRecovered;
        route.earlyVerminEncounterResolved = profile.story.archiveRecovered || 
            profile.story.tankLinked;
        route.prePipPadClueCount = profile.story.pipPadRecovered ? 2 : 
            (profile.story.awakenedFromCryo ? 1 : 0);
        route.bt72HullInspected = profile.story.archiveRecovered || profile.story.tankLinked;
        route.bt72CoreRecovered = profile.story.tankLinked || profile.story.bucketRecovered;
        route.bt72ServiceNotesRecovered = profile.story.tankLinked || 
            profile.story.bucketRecovered;
        route.bt72Restored = profile.story.tankLinked || profile.story.bucketRecovered;
        route.clearanceBlueprintRecovered = profile.story.bucketRecovered;
        route.clearanceMaterialsRecovered = profile.story.bucketRecovered;
        route.clearanceModuleInstalled = profile.story.bucketRecovered;
        route.surfaceArrivalReached = profile.story.outerRoadCleared || 
            profile.story.relayRecovered || profile.story.returnedToBase;
        route.firstTankCombatResolved = profile.story.relayRecovered || 
            profile.story.returnedToBase;
        route.firstServicePerformed = profile.story.relayRecovered ||
            profile.story.returnedToBase ||
            profile.character.awakening.fieldServiceUses > 0;
        route.firstRecoveryNodeActivated = profile.story.relayRecovered;
        route.debriefSummaryViewed = profile.story.returnedToBase;
    }
    if (loadedVersion < 5) {
        profile.partnerTank.secondSeatUnlocked = profile.story.tankLinked || 
            profile.firstPlayableRoute.bt72Restored;
        profile.partnerTank.secondSeatPolicy = "pilot_only";
        profile.partnerTank.gunnerDrillSeen = false;
        profile.partnerTank.trustedGunnerHandle.clear();
        profile.partnerTank.assignedGunnerHandle.clear();
        if (profile.story.pipPadRecovered) {
            profile.firstPlayableRoute.accessCardRecovered = true;
        }
    }
    if (loadedVersion < 6) {
        profile.firstPlayableRoute.surfaceArrivalReached =
            profile.story.outerRoadCleared ||
            profile.story.relayRecovered ||
            profile.story.returnedToBase;
    }
    if (loadedVersion < 7) {
        for (auto& worldState : profile.worldFieldStates) {
            worldState.activeRouteEventType.clear();
            worldState.routeEventTimeRemaining = 0.0f;
            worldState.routeEventCooldown = 0.0f;
            worldState.routeEventProgress = 0;
            worldState.routeEventStage = 0;
            worldState.routeEventSerial = 0;
            worldState.routeEventsResolved = 0;
            worldState.routeEventsFailed = 0;
        }
    }
    if (loadedVersion < 8) {
        for (auto& worldState : profile.worldFieldStates) {
            worldState.routeEventOfferTimeRemaining = 0.0f;
            worldState.routeEventsExpired = 0;
            worldState.lastRouteEventType.clear();

                        worldState.lastRouteEventOutcome.clear();
        }
    }
}

} // namespace

SessionProfile MakeDefaultSessionProfile() {
    SessionProfile profile;
    profile.account.registerDate = std::time(nullptr);
    profile.character.inventory.push_back({"#%it_field_ration", 2, 0.3f});
    profile.character.inventory.push_back({"#%it_ptrs_ammo", 8, 0.7f});
    profile.character.collectedTapes.push_back({"tape_intro_001", "Cryo Wing Log", false, false, 
    false});
    profile.character.collectedTapes.push_back({"music_recovery_001", "Recovery Station \
    Mixtape", false, false, false});
    profile.character.passiveSkills.push_back({"skill_field_reflex", "Field Reflex", false, false});
    profile.character.passiveSkills.push_back({"skill_pilot_sync", "Pilot Sync", false, false});
    profile.character.passiveSkills.push_back({"skill_data_miner", "Data Miner", false, false});
    profile.character.passiveSkills.push_back({"skill_second_wind", "Second Wind", false, false});
    profile.character.passiveSkills.push_back({"skill_muscle_memory", "Muscle Memory", false, 
    false});
    profile.ownedVehicles.push_back({"[#tr2001]", "Dust Runner Bike", VehicleType::Motorcycle, 
    true, false, 92.0f, 74.0f, {"cargo_rack_light"}});
    profile.ownedVehicles.push_back({"[#tr2002]", "Wallbreaker Utility", VehicleType::Truck, 
    true, false, 88.0f, 61.0f, {"salvage_bed", "field_crane"}});
    return profile;
}

void NormalizeWorldFieldState(WorldFieldState& state) {
    if (!state.towerSyncRecovered) {
        state.localRelayAvailable = false;
        state.feyRingIntercityUnlocked = false;
        state.feyRingInterserverUnlocked = false;
    }
    if (!state.regionalGridOnline && !state.towerSyncRecovered) {
        state.feyRingIntercityUnlocked = false;
        state.feyRingInterserverUnlocked = false;
    }
    state.routeEventTimeRemaining = std::max(0.0f, state.routeEventTimeRemaining);
    state.routeEventCooldown = std::max(0.0f, state.routeEventCooldown);
    state.routeEventOfferTimeRemaining = std::max(0.0f, state.routeEventOfferTimeRemaining);
    state.routeEventProgress = std::max(0, state.routeEventProgress);
    state.routeEventStage = std::max(0, state.routeEventStage);
    state.routeEventSerial = std::max(0, state.routeEventSerial);
    state.routeEventsResolved = std::max(0, state.routeEventsResolved);
    state.routeEventsFailed = std::max(0, state.routeEventsFailed);
    state.routeEventsExpired = std::max(0, state.routeEventsExpired);
    if (state.activeRouteEventType.empty() || state.routeEventTimeRemaining <= 0.0f) {
        state.activeRouteEventType.clear();
        state.routeEventTimeRemaining = 0.0f;
        state.routeEventOfferTimeRemaining = 0.0f;
        state.routeEventProgress = 0;
        state.routeEventStage = 0;
    }
    if (state.routeEventCooldown <= 0.0f && state.activeRouteEventType.empty()) {
        state.lastRouteEventType.clear();
        state.lastRouteEventOutcome.clear();
    }
}

void NormalizePipDeviceCustomization(SessionProfile& profile) {
    if (!IsKnownPipDeviceThemeId(profile.pipDeviceThemeId)) {
        profile.pipDeviceThemeId = "classic_green";
    }
    if (!IsKnownPipDeviceDisplayMode(profile.pipDeviceDisplayMode)) {
        profile.pipDeviceDisplayMode = "standard";
    }
    if (!IsKnownPipDeviceCarryMode(profile.pipDeviceCarryMode)) {
        profile.pipDeviceCarryMode = "auto";
    }
}

void NormalizeSessionProfile(SessionProfile& profile) {
    profile.selectedWorld = NormalizeWorldReference(profile.selectedWorld);
    profile.fieldCheckpointWorld = profile.fieldCheckpointWorld.empty()
        ? std::string()
        : NormalizeWorldReference(profile.fieldCheckpointWorld);
    std::vector<WorldFieldState> normalizedWorldStates;
    normalizedWorldStates.reserve(profile.worldFieldStates.size());
    for (const auto& worldState : profile.worldFieldStates) {
        WorldFieldState normalizedState = worldState;
        normalizedState.worldName = NormalizeWorldReference(worldState.worldName);
        NormalizeWorldFieldState(normalizedState);
        auto existing = std::find_if(normalizedWorldStates.begin(), normalizedWorldStates.end(),
            [&](const WorldFieldState& state) { return state.worldName == 
            normalizedState.worldName; });
        if (existing != normalizedWorldStates.end()) {
            MergeWorldFieldState(*existing, normalizedState);
        } else {
            normalizedWorldStates.push_back(normalizedState);
        }
    }
    profile.worldFieldStates = std::move(normalizedWorldStates);
    profile.lanlineServices.relayCredits = std::max(0, profile.lanlineServices.relayCredits);
    profile.launcherAnnouncements.lastSeenBuildNumber = std::max(0, 
        profile.launcherAnnouncements.lastSeenBuildNumber);
    profile.continuityAnchorVariance = std::clamp(profile.continuityAnchorVariance, 0.0f, 1.0f);
    NormalizePipDeviceCustomization(profile);
    if (!profile.activePipDeviceId.empty() && 
        !IsCanonicalPipDeviceItemId(profile.activePipDeviceId)) {
        profile.activePipDeviceId.clear();
    }
    if (profile.activePipDeviceId.empty()) {
        profile.activePipDeviceId = FirstInventoryPipDeviceId(profile);
    }
    if (profile.story.pipPadRecovered && profile.activePipDeviceId.empty()) {
        profile.activePipDeviceId = "#%it_pippad";
    }
    profile.story.pipPadRecovered = profile.story.pipPadRecovered || 
        !profile.activePipDeviceId.empty();
    if (!profile.story.pipPadRecovered) {
        profile.pipDeviceReselectPending = false;
    }
    if (profile.blueLinkModuleInstalled) {
        profile.blueLinkModuleRecovered = true;
        profile.pipPadExpansionCoverPresent = false;
    } else {
        profile.pipPadExpansionCoverPresent = true;
    }
    if (profile.firstPlayableRoute.bt72Restored || profile.story.tankLinked || 
        profile.story.bucketRecovered) {
        profile.hangarPowerRestored = true;
        profile.bt72CraneControlOnline = true;
        profile.bt72CranePathClear = true;
        profile.bt72HullAttachedToCrane = false;
        profile.bt72HullMovedToServiceLift = true;
        profile.bt72HullLockedInRestorationCradle = true;
    }
    if (const auto* selectedWorldState = FindWorldFieldState(profile, profile.selectedWorld); 
        selectedWorldState != nullptr) {
        if (!selectedWorldState->towerSyncRecovered) {
            profile.lanlineServices.serviceHubKnown = false;
        }
    }
    std::sort(profile.lanlineServices.ownedCosmetics.begin(), 
        profile.lanlineServices.ownedCosmetics.end());
    profile.lanlineServices.ownedCosmetics.erase(
        std::unique(profile.lanlineServices.ownedCosmetics.begin(), 
        profile.lanlineServices.ownedCosmetics.end()),
        profile.lanlineServices.ownedCosmetics.end());
    if (profile.account.accountId.empty() || !RegistryId::IsValid(profile.account.accountId)) 
        profile.account.accountId = "#10001";
    if (profile.character.characterId.empty() || !RegistryId::IsValid(profile.character.characterId)) 
        profile.character.characterId = "@20001";
    if (profile.account.username.empty()) profile.account.username = "wanderer";
    if (profile.character.displayName.empty()) profile.character.displayName = "Scout";
    if (profile.account.registerDate == 0) profile.account.registerDate = std::time(nullptr);
    while (profile.account.linkedCharacters.size() < 3) {
        profile.account.linkedCharacters.push_back(RegistryId::MakeCharacterId(20001 + 
            static_cast<int>(profile.account.linkedCharacters.size())));
    }
    if (profile.character.inventory.empty()) 
        profile.character.inventory.push_back({"#%it_field_ration", 1, 0.3f});
    if (profile.character.collectedTapes.empty()) {
        profile.character.collectedTapes.push_back({"tape_intro_001", "Cryo Wing Log", false, 
            false, false});
        profile.character.collectedTapes.push_back({"music_recovery_001", "Recovery Station \
            Mixtape", false, false, false});
    }
    if (profile.character.passiveSkills.empty()) {
        profile.character.passiveSkills.push_back({"skill_field_reflex", "Field Reflex", false, false});
        profile.character.passiveSkills.push_back({"skill_pilot_sync", "Pilot Sync", false, false});
        profile.character.passiveSkills.push_back({"skill_data_miner", "Data Miner", false, false});
        profile.character.passiveSkills.push_back({"skill_second_wind", "Second Wind", false, 
            false});
        profile.character.passiveSkills.push_back({"skill_muscle_memory", "Muscle Memory", 
            false, false});
    }
    if (profile.partnerTank.callSign.empty()) profile.partnerTank.callSign = "BT-72";
    profile.partnerTank.secondSeatPolicy = 
        NormalizeBt72SecondSeatPolicy(profile.partnerTank.secondSeatPolicy);
    if (profile.ownedVehicles.empty()) profile.ownedVehicles.push_back({"[#tr2001]", "Dust \
        Runner Bike", VehicleType::Motorcycle, true, false, 92.0f, 74.0f, {"cargo_rack_light"}});
    if (profile.fieldCheckpointKnown && profile.fieldCheckpointWorld.empty()) {
        profile.fieldCheckpointWorld = profile.selectedWorld;
    }
    NormalizeFirstPlayableRouteProgress(profile);
    (void)FindWorldFieldState(profile, profile.selectedWorld, true);
}

bool SeedContinuityAnchorAfterBunkerAnomaly(SessionProfile& profile, std::string* diagnosticText) {
    const bool wasSeeded = profile.continuityAnchorSeeded;
    profile.continuityAnchorSeeded = true;
    // Исправляем синтаксическую опечатку: используем тернарный оператор или обычный if
    if (diagnosticText != nullptr) {
        *diagnosticText = (profile.continuityAnchorVariance > 0.0f)
            ? "Continuity Anchor variance detected."
            : "Identity continuity profile recovered.";
    }
    return !wasSeeded;
}


bool SaveSessionProfile(const SessionProfile& profile, const fs::path& filePath) {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;
    out << "profile_format=" << kSessionProfileFormat << '\n';
    out << "profile_version=" << kCurrentSessionProfileVersion << '\n';
    out << "account_id=" << profile.account.accountId << '\n';
    out << "username=" << profile.account.username << '\n';
    out << "email=" << profile.account.email << '\n';
    out << "register_date=" << static_cast<long long>(profile.account.registerDate) << '\n';
    out << "playtime_minutes=" << profile.account.totalPlayTimeMinutes << '\n';
    out << "character_id=" << profile.character.characterId << '\n';
    out << "character_name=" << profile.character.displayName << '\n';
    out << "character_level=" << profile.character.level << '\n';
    out << "character_exp=" << profile.character.experience << '\n';
    out << "unused_points=" << profile.character.unusedPoints << '\n';
    out << "hp=" << profile.character.hp << '\n';
    out << "max_hp=" << profile.character.maxHp << '\n';
    out << "mp=" << profile.character.mp << '\n';
    out << "max_mp=" << profile.character.maxMp << '\n';
    out << "carry_weight=" << profile.character.carryWeight << '\n';
    out << "special=S:" << profile.character.special.strength
        << ",P:" << profile.character.special.perception
        << ",E:" << profile.character.special.endurance
        << ",C:" << profile.character.special.charisma
        << ",I:" << profile.character.special.intelligence
        << ",A:" << profile.character.special.agility
        << ",L:" << profile.character.special.luck << '\n';
    out << "session_mode=" << profile.sessionMode << '\n';
    out << "selected_world=" << profile.selectedWorld << '\n';
    out << "lanline_relay_credits=" << profile.lanlineServices.relayCredits << '\n';
    out << "lanline_service_hub_known=" << (profile.lanlineServices.serviceHubKnown ? 1 : 0) << '\n';
    out << "lanline_cosmetics_seen=" << (profile.lanlineServices.cosmeticsShopSeen ? 1 : 0) << '\n';
    out << "launcher_last_seen_build=" << profile.launcherAnnouncements.lastSeenBuildNumber << '\n';
    out << "launcher_last_seen_announcement=" << profile.launcherAnnouncements.lastSeenAnnouncementId << '\n';
    out << "launcher_last_seen_version=" << profile.launcherAnnouncements.lastSeenVersionLabel << '\n';
    out << "shelter_doctrine=" << static_cast<int>(profile.doctrine) << '\n';
    out << "field_checkpoint_known=" << (profile.fieldCheckpointKnown ? 1 : 0) << '\n';
    out << "field_checkpoint_x=" << profile.fieldCheckpointX << '\n';
    out << "field_checkpoint_y=" << profile.fieldCheckpointY << '\n';
    out << "field_checkpoint_world=" << profile.fieldCheckpointWorld << '\n';
    out << "field_checkpoint_label=" << profile.fieldCheckpointLabel << '\n';
    out << "scavenger_runs_completed=" << profile.scavengerRunsCompleted << '\n';
    out << "continuity_anchor_seeded=" << (profile.continuityAnchorSeeded ? 1 : 0) << '\n';
    out << "continuity_anchor_variance=" << profile.continuityAnchorVariance << '\n';
    out << "active_pip_device_id=" << profile.activePipDeviceId << '\n';
    out << "pip_device_reselect_pending=" << (profile.pipDeviceReselectPending ? 1 : 0) << '\n';
    out << "pip_device_theme_id=" << profile.pipDeviceThemeId << '\n';
    out << "pip_device_display_mode=" << profile.pipDeviceDisplayMode << '\n';
    out << "pip_device_carry_mode=" << profile.pipDeviceCarryMode << '\n';
    out << "pippad_expansion_cover_present=" << (profile.pipPadExpansionCoverPresent ? 1 : 0) << '\n';
    out << "bluelink_module_recovered=" << (profile.blueLinkModuleRecovered ? 1 : 0) << '\n';
    out << "bluelink_module_installed=" << (profile.blueLinkModuleInstalled ? 1 : 0) << '\n';
    out << "hangar_power_restored=" << (profile.hangarPowerRestored ? 1 : 0) << '\n';
    out << "bt72_crane_control_online=" << (profile.bt72CraneControlOnline ? 1 : 0) << '\n';
    out << "bt72_crane_path_clear=" << (profile.bt72CranePathClear ? 1 : 0) << '\n';
    out << "bt72_hull_attached_to_crane=" << (profile.bt72HullAttachedToCrane ? 1 : 0) << '\n';
    out << "bt72_hull_moved_to_service_lift=" << (profile.bt72HullMovedToServiceLift ? 1 : 0) << '\n';
    out << "bt72_hull_locked_in_restoration_cradle=" << (profile.bt72HullLockedInRestorationCradle ? 1 : 0) << '\n';
    out << "partner_tank_id=" << profile.partnerTank.partnerTankId << '\n';
    out << "partner_tank_callsign=" << profile.partnerTank.callSign << '\n';
    out << "partner_tank_class=" << static_cast<int>(profile.partnerTank.tankClass) << '\n';
    out << "partner_tank_link=" << profile.partnerTank.trustLink << '\n';
    out << "partner_tank_energy=" << profile.partnerTank.energyReserve << '\n';
    out << "partner_tank_ammo=" << profile.partnerTank.ammoReserve << '\n';
    out << "partner_tank_deployed=" << (profile.partnerTank.deployed ? 1 : 0) << '\n';
    out << "partner_tank_repair=" << (profile.partnerTank.inRepair ? 1 : 0) << '\n';
    out << "partner_tank_world_known=" << (profile.partnerTank.worldPositionKnown ? 1 : 0) << '\n';
    out << "partner_tank_x=" << profile.partnerTank.worldX << '\n';
    out << "partner_tank_y=" << profile.partnerTank.worldY << '\n';
    out << "partner_tank_sync_ram=" << profile.partnerTank.syncRamActions << '\n';
    out << "partner_tank_sync_shot=" << profile.partnerTank.syncShotActions << '\n';
    out << "partner_tank_second_seat=" << (profile.partnerTank.secondSeatUnlocked ? 1 : 0) << '\n';
    out << "partner_tank_second_seat_policy=" << profile.partnerTank.secondSeatPolicy << '\n';
    out << "partner_tank_trusted_gunner=" << profile.partnerTank.trustedGunnerHandle << '\n';
    out << "partner_tank_assigned_gunner=" << profile.partnerTank.assignedGunnerHandle << '\n';
    out << "partner_tank_gunner_drill=" << (profile.partnerTank.gunnerDrillSeen ? 1 : 0) << '\n';
    out << "tank_hull=" << profile.partnerTank.damage.hull << '\n';
    out << "tank_turret=" << profile.partnerTank.damage.turret << '\n';
    out << "tank_bucket=" << profile.partnerTank.damage.bucket << '\n';
    out << "tank_sensors=" << profile.partnerTank.damage.sensors << '\n';
    out << "tank_cockpit=" << profile.partnerTank.damage.cockpit << '\n';
    out << "tank_powercore=" << profile.partnerTank.damage.powerCore << '\n';
    out << "story_awakened=" << (profile.story.awakenedFromCryo ? 1 : 0) << '\n';
    out << "story_pippad=" << (profile.story.pipPadRecovered ? 1 : 0) << '\n';
    out << "story_archive=" << (profile.story.archiveRecovered ? 1 : 0) << '\n';
    out << "story_tank=" << (profile.story.tankLinked ? 1 : 0) << '\n';
    out << "story_bucket=" << (profile.story.bucketRecovered ? 1 : 0) << '\n';
    out << "story_exit=" << (profile.story.exitedBunker ? 1 : 0) << '\n';
    out << "story_debris=" << (profile.story.outerRoadCleared ? 1 : 0) << '\n';
    out << "story_relay=" << (profile.story.relayRecovered ? 1 : 0) << '\n';
    out << "story_return=" << (profile.story.returnedToBase ? 1 : 0) << '\n';
    out << "route_intro_seen=" << (profile.firstPlayableRoute.introSeen ? 1 : 0) << '\n';
    out << "route_emergency_melee=" << (profile.firstPlayableRoute.emergencyMeleeRecovered ? 1 : 0) << '\n';
    out << "route_access_card=" << (profile.firstPlayableRoute.accessCardRecovered ? 1 : 0) << '\n';
    out << "route_early_vermin=" << (profile.firstPlayableRoute.earlyVerminEncounterResolved ? 1 : 0) << '\n';
    out << "route_pre_pippad_clues=" << profile.firstPlayableRoute.prePipPadClueCount << '\n';
    out << "route_bt72_hull=" << (profile.firstPlayableRoute.bt72HullInspected ? 1 : 0) << '\n';
    out << "route_bt72_core=" << (profile.firstPlayableRoute.bt72CoreRecovered ? 1 : 0) << '\n';
    out << "route_bt72_notes=" << (profile.firstPlayableRoute.bt72ServiceNotesRecovered ? 1 : 0) << '\n';
    out << "route_bt72_restored=" << (profile.firstPlayableRoute.bt72Restored ? 1 : 0) << '\n';
    out << "route_clearance_blueprint=" << (profile.firstPlayableRoute.clearanceBlueprintRecovered ? 1 : 0) << '\n';
    out << "route_clearance_materials=" << (profile.firstPlayableRoute.clearanceMaterialsRecovered ? 1 : 0) << '\n';
    out << "route_clearance_installed=" << (profile.firstPlayableRoute.clearanceModuleInstalled ? 1 : 0) << '\n';
    out << "route_surface_arrival=" << (profile.firstPlayableRoute.surfaceArrivalReached ? 1 : 0) << '\n';
    out << "route_first_tank_combat=" << (profile.firstPlayableRoute.firstTankCombatResolved ? 1 : 0) << '\n';
    out << "route_first_service=" << (profile.firstPlayableRoute.firstServicePerformed ? 1 : 0) << '\n';
    out << "route_first_recovery=" << (profile.firstPlayableRoute.firstRecoveryNodeActivated ? 1 : 0) << '\n';
    out << "route_debrief=" << (profile.firstPlayableRoute.debriefSummaryViewed ? 1 : 0) << '\n';
    out << "awakening_archive=" << profile.character.awakening.archiveSyncs << '\n';
    out << "awakening_foot=" << profile.character.awakening.footKills << '\n';
    out << "awakening_tank=" << profile.character.awakening.tankActions << '\n';
    out << "awakening_stress=" << profile.character.awakening.stressSurvivals << '\n';
    out << "awakening_heavy_carry=" << profile.character.awakening.heavyCarryDrills << '\n';
    out << "awakening_field_service=" << profile.character.awakening.fieldServiceUses << '\n';
    for (const auto& id : profile.account.linkedCharacters) out << "linked_character=" << id << '\n';
    for (const auto& item : profile.character.inventory) out << "inventory=" << item.itemId << ',' << item.count << ',' << item.unitWeight << '\n';
    for (const auto& weapon : profile.character.weaponMods) {
        out << "weapon_mods=" << weapon.weaponItemId << ','
            << weapon.receiverId << ','
            << weapon.barrelId << ','
            << weapon.magazineId << ','
            << weapon.muzzleId << ','
            << weapon.star1 << ','
            << weapon.star2 << ','
            << weapon.star3 << ','
            << weapon.star4 << '\n';
    }
    for (const auto& tape : profile.character.collectedTapes) {
        out << "tape=" << tape.tapeId << ',' << tape.title << ',' << (tape.played ? 1 : 0)
            << ',' << (tape.damaged ? 1 : 0) << ',' << (tape.reconstructed ? 1 : 0) << '\n';
    }
    for (const auto& skill : profile.character.passiveSkills) {
        out << "passive_skill=" << skill.skillId << ',' << skill.displayName << ',' << (skill.unlocked ? 1 : 0) << ',' << (skill.equipped ? 1 : 0) << '\n';
    }
    for (const auto& specialist : profile.rescuedSpecialists) {
        out << "specialist=" << specialist.specialistId << ',' << specialist.displayName << ',' << specialist.role << ',' << specialist.assignment << ',' << (specialist.awakened ? 1 : 0) << '\n';
    }
    for (const auto& cosmeticId : profile.lanlineServices.ownedCosmetics) {
        out << "lanline_cosmetic=" << cosmeticId << '\n';
    }
    for (const auto& orderId : profile.lanlineServices.pendingSupportOrders) {
        out << "lanline_pending_order=" << orderId << '\n';
    }
    for (const auto& worldState : profile.worldFieldStates) {
        out << "world_field=" << worldState.worldName << ','
            << worldState.etherErosion << ','
            << worldState.infrastructureDecay << ','
            << (worldState.towerSyncRecovered ? 1 : 0) << ','
            << (worldState.localRelayAvailable ? 1 : 0) << ','
            << (worldState.regionalGridOnline ? 1 : 0) << ','
            << (worldState.caravanRouteActive ? 1 : 0) << ','
            << (worldState.droneStationsActive ? 1 : 0) << ','
            << (worldState.tradeNetworkActive ? 1 : 0) << ','
            << (worldState.railFreightActive ? 1 : 0) << ','
            << (worldState.orbitalUplinkActive ? 1 : 0) << ','
            << (worldState.railFortressActive ? 1 : 0) << ','
            << (worldState.recoveryFabricatorActive ? 1 : 0) << ','
            << (worldState.industrialGateUnlocked ? 1 : 0) << ','
            << worldState.routeContamination << ','
            << (worldState.routeOverrun ? 1 : 0) << ','
            << worldState.caravanRunsCompleted << ','
            << worldState.droneRunsCompleted << ','
            << worldState.tradeCyclesCompleted << ','
            << worldState.purgeCycles << ','
            << worldState.railRunsCompleted << ','
            << worldState.orbitalScansCompleted << ','
            << worldState.railFortressDeployments << ','
            << worldState.fabricatorCyclesCompleted << ','
            << worldState.recoveryMilestonesClaimed << ','
            << worldState.campFortificationLevel << ','
            << (worldState.industrialSurveyActive ? 1 : 0) << ','
            << worldState.surveyRunsCompleted << ','
            << (worldState.industrialOutpostActive ? 1 : 0) << ','
            << worldState.outpostSupplyRuns << ','
            << (worldState.assemblyCellActive ? 1 : 0) << ','
            << worldState.assemblyCyclesCompleted << ','
            << (worldState.foundryLineActive ? 1 : 0) << ','
            << worldState.foundryCyclesCompleted << ','
            << (worldState.reactorYardActive ? 1 : 0) << ','
            << worldState.reactorCyclesCompleted << ','
            << (worldState.capacitorBankActive ? 1 : 0) << ','
            << worldState.capacitorDischargeCycles << ','
            << (worldState.relaySubstationActive ? 1 : 0) << ','
            << worldState.relaySyncCycles << ','
            << (worldState.serviceBayActive ? 1 : 0) << ','
            << worldState.serviceCyclesCompleted << ','
            << (worldState.waterReclaimerActive ? 1 : 0) << ','
            << worldState.waterCyclesCompleted << ','
            << (worldState.feyRingIntercityUnlocked ? 1 : 0) << ','
            << (worldState.feyRingInterserverUnlocked ? 1 : 0) << ','
            << worldState.relayCreditsEarned << ','
            << worldState.relayCreditsSpent << ','
            << worldState.activeRouteEventType << ','
            << worldState.routeEventTimeRemaining << ','
            << worldState.routeEventCooldown << ','
            << worldState.routeEventOfferTimeRemaining << ','
            << worldState.routeEventProgress << ','
            << worldState.routeEventStage << ','
            << worldState.routeEventSerial << ','
            << worldState.routeEventsResolved << ','
            << worldState.routeEventsFailed << ','
            << worldState.routeEventsExpired << ','
            << worldState.lastRouteEventType << ','
            << worldState.lastRouteEventOutcome << '\n';
    }
    out << "active_tape_index=" << profile.character.activeTapeIndex << '\n';
    for (const auto& vehicle : profile.ownedVehicles) {
        out << "vehicle=" << vehicle.vehicleId << ',' << vehicle.displayName << ',' << static_cast<int>(vehicle.type) << ','
            << (vehicle.available ? 1 : 0) << ',' << (vehicle.deployed ? 1 : 0) << ',' << vehicle.durability << ','
            << vehicle.fuelOrCharge << '\n';
    }
    return true;
}

bool LoadSessionProfile(const fs::path& filePath, SessionProfile& outProfile) {
    std::ifstream in(filePath);
    if (!in.is_open()) return false;
    int loadedVersion = 1;
    bool hasExplicitFormatHeader = false;
    
    outProfile = MakeDefaultSessionProfile();
    outProfile.account.linkedCharacters.clear();
    outProfile.character.inventory.clear();
    outProfile.character.weaponMods.clear();
    outProfile.character.collectedTapes.clear();
    outProfile.rescuedSpecialists.clear();
    outProfile.worldFieldStates.clear();
    outProfile.ownedVehicles.clear();
    
    std::string line;
    while (std::getline(in, line)) {
        const auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        const std::string key = line.substr(0, pos);
        const std::string value = line.substr(pos + 1);
        
        if (key == "active_pip_device_id" || key == "selected_pip_device_id") {
            outProfile.activePipDeviceId = value;
            continue;
        }
        if (key == "pip_device_reselect_pending") {
            outProfile.pipDeviceReselectPending = (std::stoi(value) != 0);
            continue;
        }
        if (key == "pip_device_theme_id" || key == "active_pip_device_theme_id") {
            outProfile.pipDeviceThemeId = value;
            continue;
        }
        if (key == "pip_device_display_mode" || key == "active_pip_device_display_mode") {
            outProfile.pipDeviceDisplayMode = value;
            continue;
        }
        if (key == "pip_device_carry_mode" || key == "pip_device_wrist_mode") {
            outProfile.pipDeviceCarryMode = value;
            continue;
        }
        
        if (key == "profile_format") {
            hasExplicitFormatHeader = (value == kSessionProfileFormat);
        } else if (key == "profile_version") {
            loadedVersion = std::max(1, std::stoi(value));
        } else if (key == "account_id") outProfile.account.accountId = value;
        else if (key == "username") outProfile.account.username = value;
        else if (key == "email") outProfile.account.email = value;
        else if (key == "register_date") outProfile.account.registerDate = static_cast<std::time_t>(std::stoll(value));
        else if (key == "playtime_minutes") outProfile.account.totalPlayTimeMinutes = std::stoi(value);
        else if (key == "character_id") outProfile.character.characterId = value;
        else if (key == "character_name") outProfile.character.displayName = value;
        else if (key == "character_level") outProfile.character.level = std::stoi(value);
        else if (key == "character_exp") outProfile.character.experience = std::stoi(value);
        else if (key == "unused_points") outProfile.character.unusedPoints = std::stoi(value);
        else if (key == "hp") outProfile.character.hp = std::stof(value);
        else if (key == "max_hp") outProfile.character.maxHp = std::stof(value);
        else if (key == "mp") outProfile.character.mp = std::stof(value);
        else if (key == "max_mp") outProfile.character.maxMp = std::stof(value);
        else if (key == "carry_weight") outProfile.character.carryWeight = std::stof(value);
        else if (key == "session_mode") outProfile.sessionMode = value;
        else if (key == "selected_world") outProfile.selectedWorld = value;
        else if (key == "lanline_relay_credits") outProfile.lanlineServices.relayCredits = std::stoi(value);
        else if (key == "lanline_service_hub_known") outProfile.lanlineServices.serviceHubKnown = (std::stoi(value) != 0);
        else if (key == "lanline_cosmetics_seen") outProfile.lanlineServices.cosmeticsShopSeen = (std::stoi(value) != 0);
        else if (key == "launcher_last_seen_build") outProfile.launcherAnnouncements.lastSeenBuildNumber = std::stoi(value);
        else if (key == "launcher_last_seen_announcement") outProfile.launcherAnnouncements.lastSeenAnnouncementId = value;
        else if (key == "launcher_last_seen_version") outProfile.launcherAnnouncements.lastSeenVersionLabel = value;
        else if (key == "shelter_doctrine") outProfile.doctrine = static_cast<ShelterDoctrine>(std::stoi(value));
        else if (key == "field_checkpoint_known") outProfile.fieldCheckpointKnown = (std::stoi(value) != 0);
        else if (key == "field_checkpoint_x") outProfile.fieldCheckpointX = std::stof(value);
        else if (key == "field_checkpoint_y") outProfile.fieldCheckpointY = std::stof(value);
        else if (key == "field_checkpoint_world") outProfile.fieldCheckpointWorld = value;
        else if (key == "field_checkpoint_label") outProfile.fieldCheckpointLabel = value;
        else if (key == "scavenger_runs_completed") outProfile.scavengerRunsCompleted = std::stoi(value);
        else if (key == "continuity_anchor_seeded" || key == "soulline_seeded") outProfile.continuityAnchorSeeded = (std::stoi(value) != 0);
        else if (key == "continuity_anchor_variance" || key == "soulline_variance") outProfile.continuityAnchorVariance = std::stof(value);
        else if (key == "pippad_expansion_cover_present" || key == "pip_pad_expansion_cover_present") outProfile.pipPadExpansionCoverPresent = (std::stoi(value) != 0);
        else if (key == "bluelink_module_recovered") outProfile.blueLinkModuleRecovered = (std::stoi(value) != 0);
        else if (key == "bluelink_module_installed") outProfile.blueLinkModuleInstalled = (std::stoi(value) != 0);
        else if (key == "hangar_power_restored") outProfile.hangarPowerRestored = (std::stoi(value) != 0);
        else if (key == "bt72_crane_control_online") outProfile.bt72CraneControlOnline = (std::stoi(value) != 0);
        else if (key == "bt72_crane_path_clear") outProfile.bt72CranePathClear = (std::stoi(value) != 0);
        else if (key == "bt72_hull_attached_to_crane") outProfile.bt72HullAttachedToCrane = (std::stoi(value) != 0);
        else if (key == "bt72_hull_moved_to_service_lift") outProfile.bt72HullMovedToServiceLift = (std::stoi(value) != 0);
        else if (key == "bt72_hull_locked_in_restoration_cradle") outProfile.bt72HullLockedInRestorationCradle = (std::stoi(value) != 0);
        else if (key == "partner_tank_id") outProfile.partnerTank.partnerTankId = value;
        else if (key == "partner_tank_callsign") outProfile.partnerTank.callSign = value;
        else if (key == "partner_tank_class") outProfile.partnerTank.tankClass = static_cast<TankClass>(std::stoi(value));
        else if (key == "partner_tank_link") outProfile.partnerTank.trustLink = std::stof(value);
        else if (key == "partner_tank_energy") outProfile.partnerTank.energyReserve = std::stof(value);
        else if (key == "partner_tank_ammo") outProfile.partnerTank.ammoReserve = std::stof(value);
        else if (key == "partner_tank_deployed") outProfile.partnerTank.deployed = (std::stoi(value) != 0);
        else if (key == "partner_tank_repair") outProfile.partnerTank.inRepair = (std::stoi(value) != 0);
        else if (key == "partner_tank_world_known") outProfile.partnerTank.worldPositionKnown = (std::stoi(value) != 0);
        else if (key == "partner_tank_x") outProfile.partnerTank.worldX = std::stof(value);
        else if (key == "partner_tank_y") outProfile.partnerTank.worldY = std::stof(value);
        else if (key == "partner_tank_sync_ram") outProfile.partnerTank.syncRamActions = std::stoi(value);
        else if (key == "partner_tank_sync_shot") outProfile.partnerTank.syncShotActions = std::stoi(value);
        else if (key == "partner_tank_second_seat") outProfile.partnerTank.secondSeatUnlocked = (std::stoi(value) != 0);
        else if (key == "partner_tank_second_seat_policy") outProfile.partnerTank.secondSeatPolicy = value;
        else if (key == "partner_tank_trusted_gunner") outProfile.partnerTank.trustedGunnerHandle = value;
        else if (key == "partner_tank_assigned_gunner") outProfile.partnerTank.assignedGunnerHandle = value;
        else if (key == "partner_tank_gunner_drill") outProfile.partnerTank.gunnerDrillSeen = (std::stoi(value) != 0);
        else if (key == "tank_hull") outProfile.partnerTank.damage.hull = std::stof(value);
        else if (key == "tank_turret") outProfile.partnerTank.damage.turret = std::stof(value);
        else if (key == "tank_bucket") outProfile.partnerTank.damage.bucket = std::stof(value);
        else if (key == "tank_sensors") outProfile.partnerTank.damage.sensors = std::stof(value);
        else if (key == "tank_cockpit") outProfile.partnerTank.damage.cockpit = std::stof(value);
        else if (key == "tank_powercore") outProfile.partnerTank.damage.powerCore = std::stof(value);
        else if (key == "story_awakened") outProfile.story.awakenedFromCryo = (std::stoi(value) != 0);
        else if (key == "story_pippad") outProfile.story.pipPadRecovered = (std::stoi(value) != 0);
        else if (key == "story_archive") outProfile.story.archiveRecovered = (std::stoi(value) != 0);
        else if (key == "story_tank") outProfile.story.tankLinked = (std::stoi(value) != 0);
        else if (key == "story_bucket") outProfile.story.bucketRecovered = (std::stoi(value) != 0);
        else if (key == "story_exit") outProfile.story.exitedBunker = (std::stoi(value) != 0);
        else if (key == "story_debris") outProfile.story.outerRoadCleared = (std::stoi(value) != 0);
        else if (key == "story_relay") outProfile.story.relayRecovered = (std::stoi(value) != 0);
        else if (key == "story_return") outProfile.story.returnedToBase = (std::stoi(value) != 0);
        else if (key == "route_intro_seen") outProfile.firstPlayableRoute.introSeen = (std::stoi(value) != 0);
        else if (key == "route_emergency_melee") outProfile.firstPlayableRoute.emergencyMeleeRecovered = (std::stoi(value) != 0);
        else if (key == "route_access_card") outProfile.firstPlayableRoute.accessCardRecovered = (std::stoi(value) != 0);
        else if (key == "route_early_vermin") outProfile.firstPlayableRoute.earlyVerminEncounterResolved = (std::stoi(value) != 0);
        else if (key == "route_pre_pippad_clues") outProfile.firstPlayableRoute.prePipPadClueCount = std::stoi(value);
        else if (key == "route_bt72_hull") outProfile.firstPlayableRoute.bt72HullInspected = (std::stoi(value) != 0);
        else if (key == "route_bt72_core") outProfile.firstPlayableRoute.bt72CoreRecovered = (std::stoi(value) != 0);
        else if (key == "route_bt72_notes") outProfile.firstPlayableRoute.bt72ServiceNotesRecovered = (std::stoi(value) != 0);
        else if (key == "route_bt72_restored") outProfile.firstPlayableRoute.bt72Restored = (std::stoi(value) != 0);
        else if (key == "route_clearance_blueprint") outProfile.firstPlayableRoute.clearanceBlueprintRecovered = (std::stoi(value) != 0);
        else if (key == "route_clearance_materials") outProfile.firstPlayableRoute.clearanceMaterialsRecovered = (std::stoi(value) != 0);
        else if (key == "route_clearance_installed") outProfile.firstPlayableRoute.clearanceModuleInstalled = (std::stoi(value) != 0);
        else if (key == "route_surface_arrival") outProfile.firstPlayableRoute.surfaceArrivalReached = (std::stoi(value) != 0);
        else if (key == "route_first_tank_combat") outProfile.firstPlayableRoute.firstTankCombatResolved = (std::stoi(value) != 0);
        else if (key == "route_first_service") outProfile.firstPlayableRoute.firstServicePerformed = 
        (std::stoi(value) != 0);
        else if (key == "route_first_recovery") 
        outProfile.firstPlayableRoute.firstRecoveryNodeActivated = (std::stoi(value) != 0);
        else if (key == "route_debrief") outProfile.firstPlayableRoute.debriefSummaryViewed = 
        (std::stoi(value) != 0);
        else if (key == "awakening_archive") outProfile.character.awakening.archiveSyncs = 
        std::stoi(value);
        else if (key == "awakening_foot") outProfile.character.awakening.footKills = std::stoi(value);
        else if (key == "awakening_tank") outProfile.character.awakening.tankActions = 
        std::stoi(value);
        else if (key == "awakening_stress") outProfile.character.awakening.stressSurvivals = 
        std::stoi(value);
        else if (key == "awakening_heavy_carry") outProfile.character.awakening.heavyCarryDrills = 
        std::stoi(value);
        else if (key == "awakening_field_service") outProfile.character.awakening.fieldServiceUses = 
        std::stoi(value);
        else if (key == "linked_character") outProfile.account.linkedCharacters.push_back(value);
        else if (key == "inventory") {
            const auto first = value.find(',');
            const auto second = value.find(',', first == std::string::npos ? first : first + 1);
            if (first != std::string::npos && second != std::string::npos) {
                outProfile.character.inventory.push_back({value.substr(0, first), 
                std::stoi(value.substr(first + 1, second - first - 1)), std::stof(value.substr(second + 1))});
            }
        } else if (key == "weapon_mods") {
            std::array<std::string, 9> fields{};
            std::size_t begin = 0;
            bool complete = true;
            for (std::size_t index = 0; index < fields.size(); ++index) {
                const std::size_t end = index + 1 == fields.size() ? std::string::npos : value.find(',', begin);
                if (end == std::string::npos && index + 1 != fields.size()) {
                    complete = false;
                    break;
                }
                fields[index] = value.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
                begin = end == std::string::npos ? value.size() : end + 1;
            }
            if (complete && !fields[0].empty()) {
                outProfile.character.weaponMods.push_back({fields[0], fields[1], fields[2], fields[3], 
                fields[4], fields[5], fields[6], fields[7], fields[8]});
            }
        } else if (key == "tape") {
            const auto first = value.find(',');
            const auto second = value.find(',', first == std::string::npos ? first : first + 1);
            const auto third = value.find(',', second == std::string::npos ? second : second + 1);
            const auto fourth = value.find(',', third == std::string::npos ? third : third + 1);
            if (first != std::string::npos && second != std::string::npos) {
                TapeEntry tape{};
                tape.tapeId = value.substr(0, first);
                tape.title = value.substr(first + 1, second - first - 1);
                if (third == std::string::npos) {
                    tape.played = std::stoi(value.substr(second + 1)) != 0;
                } else {
                    tape.played = std::stoi(value.substr(second + 1, third - second - 1)) != 0;
                    if (fourth == std::string::npos) {
                        tape.damaged = std::stoi(value.substr(third + 1)) != 0;
                    } else {
                        tape.damaged = std::stoi(value.substr(third + 1, fourth - third - 1)) != 0;
                        tape.reconstructed = std::stoi(value.substr(fourth + 1)) != 0;
                    }
                }
                outProfile.character.collectedTapes.push_back(tape);
            }
        } else if (key == "active_tape_index") {
            outProfile.character.activeTapeIndex = std::stoi(value);
        } else if (key == "passive_skill") {
            const auto first = value.find(',');
            const auto second = value.find(',', first == std::string::npos ? first : first + 1);
            const auto third = value.find(',', second == std::string::npos ? second : second + 1);
            if (first != std::string::npos && second != std::string::npos && third != std::string::npos) {
                outProfile.character.passiveSkills.push_back({value.substr(0, first), value.substr(first + 
                1, second - first - 1), std::stoi(value.substr(second + 1, third - second - 1)) != 0, 
                std::stoi(value.substr(third + 1)) != 0});
            }
        } else if (key == "specialist") {
            const auto first = value.find(',');
            const auto second = value.find(',', first == std::string::npos ? first : first + 1);
            const auto third = value.find(',', second == std::string::npos ? second : second + 1);
            const auto fourth = value.find(',', third == std::string::npos ? third : third + 1);
            if (first != std::string::npos && second != std::string::npos && third != std::string::npos) {
                SpecialistEntry specialist{};
                specialist.specialistId = value.substr(0, first);
                specialist.displayName = value.substr(first + 1, second - first - 1);
                specialist.role = value.substr(second + 1, third - second - 1);
                if (fourth == std::string::npos) {
                    specialist.awakened = std::stoi(value.substr(third + 1)) != 0;
                    specialist.assignment = specialist.role == "engineer" ? "workshop" : "field";
                } else {
                    specialist.assignment = value.substr(third + 1, fourth - third - 1);
                    specialist.awakened = std::stoi(value.substr(fourth + 1)) != 0;
                }
                outProfile.rescuedSpecialists.push_back(specialist);
            }
        } else if (key == "lanline_cosmetic") {
            outProfile.lanlineServices.ownedCosmetics.push_back(value);
        } else if (key == "lanline_pending_order") {
            outProfile.lanlineServices.pendingSupportOrders.push_back(value);
        } else if (key == "world_field") {
            WorldFieldState state{};
            state.worldName = value;
            outProfile.worldFieldStates.push_back(state);
        } else if (key == "special") {
            std::size_t start = 0;
            while (start < value.size()) {
                const std::size_t next = value.find(',', start);
                const std::string token = value.substr(start, next == std::string::npos ? std::string::npos : 
                next - start);
                if (token.size() >= 3 && token[1] == ':') {
                    const int statValue = std::stoi(token.substr(2));
                    switch (token[0]) {
                        case 'S': outProfile.character.special.strength = statValue; break;
                        case 'P': outProfile.character.special.perception = statValue; break;
                        case 'E': outProfile.character.special.endurance = statValue; break;
                        case 'C': outProfile.character.special.charisma = statValue; break;
                        case 'I': outProfile.character.special.intelligence = statValue; break;
                        case 'A': outProfile.character.special.agility = statValue; break;
                        case 'L': outProfile.character.special.luck = statValue; break;
                    }
                }
                if (next == std::string::npos) break;
                start = next + 1;
            }
       } else if (key == "vehicle") {
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (start <= value.size()) {
                const std::size_t next = value.find(',', start);
                parts.push_back(value.substr(start, next == std::string::npos ? std::string::npos : next - start));
                if (next == std::string::npos) break;
                start = next + 1;
            }
            if (parts.size() >= 7) {
                // Агрегатный пуш напрямую в вектор: обходим использование имени VehicleEntry
                outProfile.ownedVehicles.push_back({
                    parts[0],                                    // vehicleId
                    parts[1],                                    // displayName
                    static_cast<VehicleType>(std::stoi(parts[2])), // type
                    (std::stoi(parts[3]) != 0),                  // available
                    (std::stoi(parts[4]) != 0),                  // deployed
                    std::stof(parts[5]),                         // durability
                    std::stof(parts[6]),                         // fuelOrCharge
                    {}                                           // пустой список модулей
                });
            }
        }

    }
    
    if (!hasExplicitFormatHeader) {
        loadedVersion = 1;
    }
    MigrateSessionProfile(outProfile, loadedVersion);
    NormalizeSessionProfile(outProfile);
    return true;
}

} // namespace bunker
