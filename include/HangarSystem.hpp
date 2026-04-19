#pragma once

#include <algorithm>
#include <string>

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

inline bool ConsumeTankServiceKit(SessionProfile& profile,
                                  const std::string& itemId,
                                  TankModuleSlotType subsystem,
                                  std::string* eventText = nullptr) {
    for (auto& entry : profile.character.inventory) {
        if (entry.itemId != itemId || entry.count <= 0) {
            continue;
        }

        --entry.count;
        auto& damage = profile.partnerTank.damage;
        switch (subsystem) {
            case TankModuleSlotType::Turret:
                damage.turret = std::min(100.0f, damage.turret + 30.0f);
                break;
            case TankModuleSlotType::PowerCore:
                damage.powerCore = std::min(100.0f, damage.powerCore + 30.0f);
                break;
            case TankModuleSlotType::Sensor:
                damage.sensors = std::min(100.0f, damage.sensors + 30.0f);
                break;
            case TankModuleSlotType::Bucket:
                damage.bucket = std::min(100.0f, damage.bucket + 30.0f);
                break;
            default:
                damage.hull = std::min(100.0f, damage.hull + 20.0f);
                break;
        }
        if (eventText != nullptr) {
            *eventText = "Tank service kit applied successfully.";
        }
        return true;
    }

    if (eventText != nullptr) {
        *eventText = "Required tank service kit not found.";
    }
    return false;
}

}  // namespace bunker
