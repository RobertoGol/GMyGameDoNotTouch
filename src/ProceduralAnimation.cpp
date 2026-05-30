#include <cmath>
#include <algorithm>

namespace bunker {

// Структура трансформации суставов механической ноги робота
struct LegTransform {
    float offsetZ = 0.0f;
    float offsetY = 0.0f;
    float rotationJoint = 0.0f;
};

// 1. Процедурный шаг роботов фракции RobotControl
// Рассчитывает волновые сдвиги ног на основе времени сессии и скорости движения
LegTransform CalculateRobotLegAnim(float speed, float sessionTime, int legIndex) {
    LegTransform transform{ 0.0f, 0.0f, 0.0f };
    
    // Если робот стоит или скорость слишком мала — возвращаем нулевое смещение
    if (speed < 0.1f) {
        return transform;
    }

    // Сдвиг фазы (PI радиан), чтобы левая и правая ноги двигались поочередно
    float phaseShift = (legIndex % 2 == 0) ? 0.0f : 3.14159f;
    float wave = std::sin(sessionTime * speed * 2.5f + phaseShift);

    transform.offsetZ = wave * 0.35f;                 // Амплитуда шага вперед/назад
    transform.offsetY = std::max(0.0f, wave) * 0.25f; // Подъем ноги вверх (не проваливается под террейн)
    transform.rotationJoint = wave * 0.15f;           // Изгиб сустава колена

    return transform;
}

// 2. Вращение катков и прокрутка UV-координат гусениц танка BT-72
// Предотвращает проскальзывание текстуры, привязывая сдвиг строго к дельте времени и скорости
void UpdateTankTrackUV(float currentVehicleSpeed, float dt, float& currentScrollU, float& outWheelRotation) {
    // Если танк не движется — анимация полностью замораживается
    if (std::abs(currentVehicleSpeed) < 0.05f) {
        return;
    }

    // Вращение колес по кругу (в радианах от 0 до 2*PI)
    outWheelRotation = std::fmod(outWheelRotation + (currentVehicleSpeed / 0.4f) * dt, 6.28318f);

    // Сдвигаем UV-координаты текстуры трака из .ba2 в шейдере DX11, чтобы гусеница визуально "ехала"
    currentScrollU = std::fmod(currentScrollU + (currentVehicleSpeed * 0.15f * dt), 1.0f);

    // Коррекция отрицательного сдвига текстуры при движении танка задним ходом
    if (currentScrollU < 0.0f) {
        currentScrollU += 1.0f;
    }
}

} // namespace bunker
