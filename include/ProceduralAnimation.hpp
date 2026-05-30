#pragma once

namespace bunker {

struct LegTransform {
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float offsetZ = 0.0f;
    float rotationJoint = 0.0f;
};

struct TrackAnimationState {
    float textureScrollU = 0.0f;
    float wheelRotation = 0.0f;
};

LegTransform CalculateRobotLegAnim(float speed, float sessionTime, int legIndex);
TrackAnimationState UpdateTankTracks(float currentVehicleSpeed, float dt, float currentScrollU);

}  // namespace bunker
