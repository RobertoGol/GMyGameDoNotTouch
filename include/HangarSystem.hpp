#pragma once

#include <algorithm>
#include <string>
#include <string_view>

#include "MapObject.hpp"
#include "SessionProfiles.hpp"

namespace bunker {

struct HangarState {
    bool open = false;
    bool repairRigOnline = true;
    int selectedTankModule = 0;
    int selectedVehicle = 0;
    std::string lastAction = "Hangar idle.";
};

inline bool IsHangarObject(const MapObject* object) {
    if (!object) return false;
    return object->displayName == "Field Workshop" || object->displayName == "Player Hangar";
}

inline void QuickRepairPartnerTank(SessionProfile& profile, HangarState& hangar) {
    profile.partnerTank.damage.hull = 100.0f;
    profile.partnerTank.damage.turret = 100.0f;
    profile.partnerTank.damage.bucket = 100.0f;
    profile.partnerTank.damage.sensors = 100.0f;
    profile.partnerTank.damage.cockpit = 100.0f;
    profile.partnerTank.damage.powerCore = 100.0f;
    profile.partnerTank.energyReserve = 100.0f;
    profile.partnerTank.ammoReserve = 100.0f;
    profile.partnerTank.inRepair = false;
    hangar.lastAction = "Partner tank serviced, rearmed, and fully repaired.";
}

inline void CycleTankModule(SessionProfile& profile, HangarState& hangar) {
    if (profile.partnerTank.loadout.modules.empty()) {
        hangar.lastAction = "No tank modules available.";
        return;
    }

    hangar.selectedTankModule = (hangar.selectedTankModule + 1) % static_cast<int>(profile.partnerTank.loadout.modules.size());
    auto& module = profile.partnerTank.loadout.modules[static_cast<std::size_t>(hangar.selectedTankModule)];

    if (module.type == TankModuleSlotType::Bucket) {
        module.moduleId = (module.moduleId == "bucket_shield_a") ? "bucket_shield_b" : "bucket_shield_a";
        module.displayName = (module.moduleId == "bucket_shield_a") ? "Bucket Shield A" : "Bucket Shield B";
    } else if (module.type == TankModuleSlotType::Sensor) {
        module.moduleId = (module.moduleId == "nerv_sensor_mk1") ? "nerv_sensor_mk2" : "nerv_sensor_mk1";
        module.displayName = (module.moduleId == "nerv_sensor_mk1") ? "Nerv Sensor Mk.I" : "Nerv Sensor Mk.II";
    } else if (module.type == TankModuleSlotType::Turret) {
        module.moduleId = (module.moduleId == "turret_field_mk1") ? "turret_siege_mk1" : "turret_field_mk1";
        module.displayName = (module.moduleId == "turret_field_mk1") ? "Field Turret Mk.I" : "Siege Turret Mk.I";
        profile.partnerTank.tankClass = (module.moduleId == "turret_field_mk1") ? TankClass::Vanguard : TankClass::Destroyer;
        profile.partnerTank.loadout.turretId = module.moduleId;
        profile.partnerTank.loadout.turretName = module.displayName;
    }

    hangar.lastAction = "Adjusted tank module: " + module.displayName;
}

inline void ServiceSelectedVehicle(SessionProfile& profile, HangarState& hangar) {
    if (profile.ownedVehicles.empty()) {
        hangar.lastAction = "No auxiliary vehicles available.";
        return;
    }

    if (hangar.selectedVehicle >= static_cast<int>(profile.ownedVehicles.size())) {
        hangar.selectedVehicle = 0;
    }

    auto& vehicle = profile.ownedVehicles[static_cast<std::size_t>(hangar.selectedVehicle)];
    vehicle.durability = 100.0f;
    vehicle.fuelOrCharge = 100.0f;
    hangar.lastAction = "Serviced vehicle: " + vehicle.displayName;
}

inline TankModuleSlot* FindTankModuleSlot(SessionProfile& profile, TankModuleSlotType type) {
    for (auto& module : profile.partnerTank.loadout.modules) {
        if (module.type == type) {
            return &module;
        }
    }
    return nullptr;
}

inline void RestoreTankModuleDurability(SessionProfile& profile, TankModuleSlotType type, float amount) {
    if (amount <= 0.0f) {
        return;
    }
    if (auto* module = FindTankModuleSlot(profile, type); module != nullptr) {
        module->durability = std::min(100.0f, module->durability + amount);
    }
}

inline bool ApplyKnownTankServiceKit(SessionProfile& profile,
                                     std::string_view itemId,
                                     std::string* eventText = nullptr) {
    auto& damage = profile.partnerTank.damage;
    if (itemId == "track_patch") {
        damage.hull = std::min(100.0f, damage.hull + 22.0f);
        damage.bucket = std::min(100.0f, damage.bucket + 28.0f);
        RestoreTankModuleDurability(profile, TankModuleSlotType::Chassis, 18.0f);
        RestoreTankModuleDurability(profile, TankModuleSlotType::Bucket, 22.0f);
        profile.partnerTank.inRepair = false;
        if (eventText != nullptr) {
            *eventText = "Suspension repair kit applied. Chassis carriage and bucket rig stabilized.";
        }
        return true;
    }
    if (itemId == "servo_patch") {
        damage.turret = std::min(100.0f, damage.turret + 32.0f);
        RestoreTankModuleDurability(profile, TankModuleSlotType::Turret, 24.0f);
        profile.partnerTank.inRepair = false;
        if (eventText != nullptr) {
            *eventText = "Turret service kit applied. Servo ring and stabilizer alignment restored.";
        }
        return true;
    }
    if (itemId == "engine_seal") {
        damage.powerCore = std::min(100.0f, damage.powerCore + 32.0f);
        profile.partnerTank.energyReserve = std::min(100.0f, profile.partnerTank.energyReserve + 18.0f);
        RestoreTankModuleDurability(profile, TankModuleSlotType::PowerCore, 24.0f);
        profile.partnerTank.inRepair = false;
        if (eventText != nullptr) {
            *eventText = "Engine service kit applied. Core seals replaced and reserve charge restored.";
        }
        return true;
    }
    if (itemId == "lens_pack") {
        damage.sensors = std::min(100.0f, damage.sensors + 34.0f);
        damage.cockpit = std::min(100.0f, damage.cockpit + 10.0f);
        RestoreTankModuleDurability(profile, TankModuleSlotType::Sensor, 26.0f);
        profile.partnerTank.inRepair = false;
        if (eventText != nullptr) {
            *eventText = "Sensor recovery kit applied. Optics recalibrated and cockpit feed stabilized.";
        }
        return true;
    }
    return false;
}

inline float TankServiceKitNeedScore(const SessionProfile& profile, std::string_view itemId) {
    const auto& damage = profile.partnerTank.damage;
    if (itemId == "track_patch") {
        return std::max(0.0f, 100.0f - damage.hull) * 0.65f +
            std::max(0.0f, 100.0f - damage.bucket) * 0.85f;
    }
    if (itemId == "servo_patch") {
        return std::max(0.0f, 100.0f - damage.turret);
    }
    if (itemId == "engine_seal") {
        return std::max(0.0f, 100.0f - damage.powerCore) +
            std::max(0.0f, 100.0f - profile.partnerTank.energyReserve) * 0.65f;
    }
    if (itemId == "lens_pack") {
        return std::max(0.0f, 100.0f - damage.sensors) +
            std::max(0.0f, 100.0f - damage.cockpit) * 0.25f;
    }
    return 0.0f;
}

inline bool ConsumeTankServiceKit(SessionProfile& profile,
                                  const std::string& itemId,
                                  TankModuleSlotType subsystem,
                                  std::string* eventText = nullptr) {
    for (auto& entry : profile.character.inventory) {
        if (entry.itemId != itemId || entry.count <= 0) {
            continue;
        }

        --entry.count;
        if (ApplyKnownTankServiceKit(profile, itemId, eventText)) {
            return true;
        }

        auto& damage = profile.partnerTank.damage;
        switch (subsystem) {
            case TankModuleSlotType::Turret:
                damage.turret = std::min(100.0f, damage.turret + 30.0f);
                RestoreTankModuleDurability(profile, TankModuleSlotType::Turret, 20.0f);
                break;
            case TankModuleSlotType::PowerCore:
                damage.powerCore = std::min(100.0f, damage.powerCore + 30.0f);
                RestoreTankModuleDurability(profile, TankModuleSlotType::PowerCore, 20.0f);
                break;
            case TankModuleSlotType::Sensor:
                damage.sensors = std::min(100.0f, damage.sensors + 30.0f);
                RestoreTankModuleDurability(profile, TankModuleSlotType::Sensor, 20.0f);
                break;
            case TankModuleSlotType::Bucket:
                damage.bucket = std::min(100.0f, damage.bucket + 30.0f);
                RestoreTankModuleDurability(profile, TankModuleSlotType::Bucket, 20.0f);
                break;
            case TankModuleSlotType::Chassis:
                damage.hull = std::min(100.0f, damage.hull + 24.0f);
                RestoreTankModuleDurability(profile, TankModuleSlotType::Chassis, 20.0f);
                break;
            default:
                damage.hull = std::min(100.0f, damage.hull + 20.0f);
                break;
        }
        profile.partnerTank.inRepair = false;
        if (eventText != nullptr) {
            *eventText = "Tank service kit applied to the selected subsystem.";
        }
        return true;
    }

    if (eventText != nullptr) {
        *eventText = "Required tank service kit not found.";
    }
    return false;
}

inline bool TryConsumeBestTankServiceKit(SessionProfile& profile, std::string* eventText = nullptr) {
    struct TankServiceCandidate {
        const char* itemId;
        TankModuleSlotType subsystem;
    };

    static constexpr TankServiceCandidate kCandidates[] = {
        {"track_patch", TankModuleSlotType::Chassis},
        {"servo_patch", TankModuleSlotType::Turret},
        {"engine_seal", TankModuleSlotType::PowerCore},
        {"lens_pack", TankModuleSlotType::Sensor},
    };

    const TankServiceCandidate* bestCandidate = nullptr;
    float bestScore = 0.0f;
    bool hasAnyServiceKit = false;

    for (const auto& candidate : kCandidates) {
        const auto inventoryIt = std::find_if(
            profile.character.inventory.begin(),
            profile.character.inventory.end(),
            [&](const InventoryEntry& entry) {
                return entry.itemId == candidate.itemId && entry.count > 0;
            });
        if (inventoryIt == profile.character.inventory.end()) {
            continue;
        }

        hasAnyServiceKit = true;
        const float candidateScore = TankServiceKitNeedScore(profile, candidate.itemId);
        if (bestCandidate == nullptr || candidateScore > bestScore) {
            bestScore = candidateScore;
            bestCandidate = &candidate;
        }
    }

    if (bestCandidate == nullptr) {
        if (eventText != nullptr) {
            *eventText = "Compatible tank service kit not found.";
        }
        return false;
    }

    if (bestScore <= 0.0f) {
        if (eventText != nullptr) {
            *eventText = hasAnyServiceKit
                ? "BT-72 service anchor reports no damaged subsystem that matches the available kits."
                : "Compatible tank service kit not found.";
        }
        return false;
    }

    return ConsumeTankServiceKit(profile, bestCandidate->itemId, bestCandidate->subsystem, eventText);
}

}  // namespace bunker
