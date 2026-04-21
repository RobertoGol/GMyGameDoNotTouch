#include "../include/GameplayDescriptorRegistry.hpp"

#include <array>

namespace bunker {

namespace {

constexpr std::array<GameplayDescriptorSpec, 27> kGameplayDescriptorSpecs{{
    {"archive_sync", "Archive Sync", InteractionType::Terminal, ObjectCategory::Terminal, {}, false},
    {"terminal_sync", "Terminal Sync", InteractionType::Terminal, ObjectCategory::Terminal, {}, false},
    {"remote_link", "Remote Link", InteractionType::Terminal, ObjectCategory::Terminal, "remote_world_anchor", true},
    {"workshop_service", "Workshop Service", InteractionType::Workshop, ObjectCategory::Terminal, {}, false},
    {"tower_sync", "Tower Sync", InteractionType::Terminal, ObjectCategory::Terminal, "regional_grid", false},
    {"power_pylon", "Power Pylon", InteractionType::Terminal, ObjectCategory::Terminal, "regional_grid_north", false},
    {"drone_station", "Drone Station", InteractionType::Terminal, ObjectCategory::Terminal, "salvage_sweep", false},
    {"rail_depot", "Rail Depot", InteractionType::Terminal, ObjectCategory::Terminal, "industrial_spur_alpha", false},
    {"orbital_uplink", "Orbital Uplink", InteractionType::Terminal, ObjectCategory::Terminal, "low_orbit_scan", false},
    {"rail_fortress_hub", "Rail Fortress Hub", InteractionType::Terminal, ObjectCategory::Terminal, "magistral_anchor_alpha", false},
    {"recovery_fabricator", "Recovery Fabricator", InteractionType::Terminal, ObjectCategory::Terminal, "shelter17_refinery", false},
    {"industrial_gate", "Industrial Gate", InteractionType::Transition, ObjectCategory::Landmark, "inner_spur_alpha", true},
    {"industrial_survey", "Industrial Survey", InteractionType::Terminal, ObjectCategory::Terminal, "inner_spur_survey", false},
    {"industrial_outpost", "Industrial Outpost", InteractionType::Terminal, ObjectCategory::Terminal, "inner_spur_outpost", false},
    {"assembly_cell", "Assembly Cell", InteractionType::Terminal, ObjectCategory::Terminal, "inner_spur_assembly", false},
    {"foundry_line", "Foundry Line", InteractionType::Terminal, ObjectCategory::Terminal, "inner_spur_foundry", false},
    {"reactor_yard", "Reactor Yard", InteractionType::Terminal, ObjectCategory::Terminal, "inner_spur_reactor", false},
    {"capacitor_bank", "Capacitor Bank", InteractionType::Terminal, ObjectCategory::Terminal, "inner_spur_capacitor", false},
    {"relay_substation", "Relay Substation", InteractionType::Terminal, ObjectCategory::Terminal, "shelter17_backbone", false},
    {"service_bay", "Service Bay", InteractionType::Terminal, ObjectCategory::Terminal, "inner_spur_service", false},
    {"water_reclaimer", "Water Reclaimer", InteractionType::Terminal, ObjectCategory::Terminal, "inner_spur_water", false},
    {"lanline_service_hub", "Lanline Service Hub", InteractionType::Terminal, ObjectCategory::Terminal, "shelter17_services", false},
    {"tank_service", "Tank Service", InteractionType::Workshop, ObjectCategory::Hangar, "bt72_service", false},
    {"medical_support", "Medical Support", InteractionType::Terminal, ObjectCategory::Terminal, "field_medical", false},
    {"fey_ring", "Fey Ring", InteractionType::Transition, ObjectCategory::Landmark, "intercity_ring", true},
    {"echo_trace", "Echo Trace", InteractionType::Terminal, ObjectCategory::Terminal, "[%workshop_cache_0001]", false},
    {"specialist_cryo", "Specialist Cryo", InteractionType::Terminal, ObjectCategory::Terminal, "engineer", false},
}};

constexpr std::array<GameplayDescriptorAlias, 4> kGameplayDescriptorAliases{{
    {"radio_tower", "tower_sync"},
    {"workshop_field_service", "workshop_service"},
    {"rail_freight", "rail_depot"},
    {"rail_fortress", "rail_fortress_hub"},
}};

} // namespace

std::string_view NormalizeGameplayDescriptorTag(std::string_view scriptTag) {
    for (const auto& alias : kGameplayDescriptorAliases) {
        if (alias.alias == scriptTag) {
            return alias.canonicalTag;
        }
    }
    return scriptTag;
}

const GameplayDescriptorSpec* FindGameplayDescriptor(std::string_view scriptTag) {
    const std::string_view normalizedTag = NormalizeGameplayDescriptorTag(scriptTag);
    for (const auto& spec : kGameplayDescriptorSpecs) {
        if (spec.scriptTag == normalizedTag) {
            return &spec;
        }
    }
    return nullptr;
}

bool IsKnownGameplayDescriptor(std::string_view scriptTag) {
    return FindGameplayDescriptor(scriptTag) != nullptr;
}

bool ScriptTagRequiresLinkTarget(std::string_view scriptTag) {
    if (const auto* spec = FindGameplayDescriptor(scriptTag)) {
        return spec->requiresLinkTarget;
    }
    return false;
}

const char* DefaultGameplayDescriptorLinkTarget(std::string_view scriptTag) {
    if (const auto* spec = FindGameplayDescriptor(scriptTag)) {
        if (!spec->defaultLinkTarget.empty()) {
            return spec->defaultLinkTarget.data();
        }
    }
    return nullptr;
}

} // namespace bunker
