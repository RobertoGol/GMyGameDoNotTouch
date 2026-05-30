#include <cmath>
#include <algorithm>

namespace bunker {

struct LegTransform {
    float offsetZ;
    float offsetY;
    float rotationJoint;
};

// 1. Процедурный шаг роботов (RobotControl)
LegTransform CalculateRobotLegAnim(float speed, float sessionTime, int legIndex) {
    LegTransform transform{0.0f, 0.0f, 0.0f};
    if (speed < 0.1f) return transform;

    // Сдвиг фазы, чтобы левая и правая ноги двигались поочередно
    float phaseShift = (legIndex % 2 == 0) ? 0.0f : 3.14159f;
    float wave = std::sin(sessionTime * speed * 2.5f + phaseShift);

    transform.offsetZ = wave * 0.35f; // Амплитуда шага вперед
    transform.offsetY = std::max(0.0f, wave) * 0.25f; // Подъем ноги вверх
    transform.rotationJoint = wave * 0.15f; // Изгиб сустава колена

    return transform;
}

// 2. Вращение катков и прокрутка UV-координат гусениц танка BT-72
void UpdateTankTrackUV(float currentVehicleSpeed, float dt, float& currentScrollU, float& outWheelRotation) {
    if (std::abs(currentVehicleSpeed) < 0.05f) return;

    // Вращение колес по кругу (в радианах от 0 до 2*PI)
    outWheelRotation = std::fmod(outWheelRotation + (currentVehicleSpeed / 0.4f) * dt, 6.28318f);

    // Сдвигаем UV-координаты текстуры трака из .ba2 в шейдере DX11, чтобы гусеница "ехала"
    currentScrollU = std::fmod(currentScrollU + (currentVehicleSpeed * 0.15f * dt), 1.0f);
}

} // namespace bunker
