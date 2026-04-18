#pragma once

#include "World.hpp"

namespace bunker {

enum class ViewMode {
    FirstPerson,
    ThirdPerson,
    Cockpit
};

enum class WeatherAnomaly {
    Clear,
    AcidRain,
    EtherFog
};

struct PlayerState {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float facingRadians = 0.0f;
    float speed = 8.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float recoilOffset = 0.0f;
    bool insideTank = false;
    bool bucketRaised = false;
    bool uiVisible = true;
    ViewMode viewMode = ViewMode::ThirdPerson;
};

const char* ToString(ViewMode mode);
const char* ToString(WeatherAnomaly weather);
void RenderWorld(const World& world, const PlayerState& player, WeatherAnomaly weather, float weatherIntensity, int width, int height);

}  // namespace bunker
