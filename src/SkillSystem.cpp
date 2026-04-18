#include "../include/SkillSystem.hpp"

namespace bunker {

namespace {

PassiveSkill* FindSkill(SessionProfile& profile, const std::string& skillId) {
    for (auto& skill : profile.character.passiveSkills) {
        if (skill.skillId == skillId) {
            return &skill;
        }
    }
    return nullptr;
}

bool HasRecipeItem(const SessionProfile& profile, const std::string& itemId) {
    for (const auto& item : profile.character.inventory) {
        if (item.itemId == itemId && item.count > 0) {
            return true;
        }
    }
    return false;
}

void UnlockSkill(SessionProfile& profile, const std::string& skillId, const std::string& fallbackName, std::string* eventText) {
    auto* skill = FindSkill(profile, skillId);
    if (skill == nullptr) {
        profile.character.passiveSkills.push_back({skillId, fallbackName, true, true});
        if (eventText != nullptr) {
            *eventText = fallbackName + " awakened and equipped.";
        }
        return;
    }
    if (!skill->unlocked) {
        skill->unlocked = true;
        skill->equipped = true;
        if (eventText != nullptr) {
            *eventText = skill->displayName + " awakened and equipped.";
        }
    }
}

}  // namespace

bool HasEquippedPassiveSkill(const SessionProfile& profile, const std::string& skillId) {
    for (const auto& skill : profile.character.passiveSkills) {
        if (skill.skillId == skillId && skill.unlocked && skill.equipped) {
            return true;
        }
    }
    return false;
}

void RegisterArchiveSync(SessionProfile& profile, std::string* eventText) {
    profile.character.awakening.archiveSyncs += 1;
    if (profile.character.awakening.archiveSyncs >= 2) {
        UnlockSkill(profile, "skill_data_miner", "Data Miner", eventText);
    }
}

void RegisterFootKill(SessionProfile& profile, std::string* eventText) {
    profile.character.awakening.footKills += 1;
    if (profile.character.awakening.footKills >= 2) {
        UnlockSkill(profile, "skill_field_reflex", "Field Reflex", eventText);
    }
}

void RegisterTankAction(SessionProfile& profile, std::string* eventText) {
    profile.character.awakening.tankActions += 1;
    if (profile.character.awakening.tankActions >= 2) {
        UnlockSkill(profile, "skill_pilot_sync", "Pilot Sync", eventText);
    }
}

void RegisterStressSurvival(SessionProfile& profile, std::string* eventText) {
    profile.character.awakening.stressSurvivals += 1;
    if (profile.character.awakening.stressSurvivals >= 1) {
        UnlockSkill(profile, "skill_second_wind", "Second Wind", eventText);
    }
}

void RegisterHeavyCarryDrill(SessionProfile& profile, std::string* eventText) {
    profile.character.awakening.heavyCarryDrills += 1;
    if (profile.character.awakening.heavyCarryDrills >= 4) {
        UnlockSkill(profile, "skill_muscle_memory", "Muscle Memory", eventText);
    }
}

void RegisterFieldServiceUse(SessionProfile& profile, std::string* eventText) {
    profile.character.awakening.fieldServiceUses += 1;
    if (profile.character.awakening.fieldServiceUses >= 3 && !HasRecipeItem(profile, "recipe_repair_patch")) {
        profile.character.inventory.push_back({"recipe_repair_patch", 1, 0.0f});
        if (eventText != nullptr) {
            *eventText = "Awakened Recipe discovered: Repair Patch.";
        }
    }
}

}  // namespace bunker
