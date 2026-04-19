#pragma once

#include <array>
#include <string_view>

#include "MapObject.hpp"

namespace bunker {

struct GameplayDescriptorSpec {
    std::string_view scriptTag;
    std::string_view label;
    InteractionType preferredInteraction;
    ObjectCategory preferredCategory;
    bool requiresLinkTarget = false;
};

inline const GameplayDescriptorSpec* FindGameplayDescriptor(std::string_view scriptTag) {
    static constexpr std::array<GameplayDescriptorSpec, 15> kSpecs{{
        {"tower_sync", "Tower Sync", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"power_pylon", "Power Pylon", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"drone_station", "Drone Station", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"rail_freight", "Rail Freight", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"orbital_uplink", "Orbital Uplink", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"rail_fortress", "Rail Fortress", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"recovery_fabricator", "Recovery Fabricator", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"industrial_gate", "Industrial Gate", InteractionType::Transition, ObjectCategory::Landmark, false},
        {"industrial_survey", "Industrial Survey", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"industrial_outpost", "Industrial Outpost", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"assembly_cell", "Assembly Cell", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"reactor_yard", "Reactor Yard", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"lanline_service_hub", "Lanline Service Hub", InteractionType::Terminal, ObjectCategory::Terminal, false},
        {"tank_service", "Tank Service", InteractionType::Workshop, ObjectCategory::Hangar, false},
        {"fey_ring", "Fey Ring", InteractionType::Transition, ObjectCategory::Landmark, true},
    }};

    for (const auto& spec : kSpecs) {
        if (spec.scriptTag == scriptTag) {
            return &spec;
        }
    }
    return nullptr;
}

inline bool ScriptTagRequiresLinkTarget(std::string_view scriptTag) {
    if (const auto* spec = FindGameplayDescriptor(scriptTag)) {
        return spec->requiresLinkTarget;
    }
    return false;
}

} // namespace bunker