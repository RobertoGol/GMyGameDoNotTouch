#pragma once

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

struct GameplayDescriptorAlias {
    std::string_view alias;
    std::string_view canonicalTag;
};

std::string_view NormalizeGameplayDescriptorTag(std::string_view scriptTag);
const GameplayDescriptorSpec* FindGameplayDescriptor(std::string_view scriptTag);
bool IsKnownGameplayDescriptor(std::string_view scriptTag);
bool ScriptTagRequiresLinkTarget(std::string_view scriptTag);

} // namespace bunker
