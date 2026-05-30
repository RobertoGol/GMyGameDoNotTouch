#pragma once
#include <algorithm>

/**
 * PROJECT: Bunker Protocol
 * MODULE: Camera_Avatar
 * DESCRIPTION: Гибридная камера (1st/3rd Person) с плавным зумом в стиле Fallout.
 */

namespace BunkerProtocol {

    enum class CameraMode {
        FirstPerson,
        ThirdPerson,
        Transition
    };

    struct Vector3 { float x, y, z; };

    class CameraAvatar {
    private:
        CameraMode currentMode = CameraMode::ThirdPerson;
        
        float currentDistance = 3.0f;  // Текущее расстояние от игрока
        float minDistance = 0.0f;      // 0.0 превращает камеру в 1st person
        float maxDistance = 10.0f;     // Максимальный вылет камеры
        
        float zoomSpeed = 0.5f;        // Чувствительность колесика
        float smoothFactor = 0.1f;     // Насколько плавно движется камера (Lerp)

        Vector3 offsetThirdPerson = { 0.5f, 1.7f, 0.0f }; // Смещение за плечо (x - право, y - высота)

    public:
        CameraAvatar() = default;

        /**
         * Обработка зума (прокрутка колесика мыши или кнопки геймпада)
         */
        void UpdateZoom(float delta) {
            currentDistance = std::clamp(currentDistance - (delta * zoomSpeed), minDistance, maxDistance);

            // Автоматическая смена режима
            if (currentDistance <= 0.1f) {
                currentMode = CameraMode::FirstPerson;
            } else {
                currentMode = CameraMode::ThirdPerson;
            }
        }

        /**
         * Расчет позиции камеры (вызывается каждый кадр перед рендером)
         * targetPos - координаты персонажа (@XXXXXXX)
         */
        Vector3 CalculatePosition(const Vector3& targetPos, float yaw, float pitch) {
            if (currentMode == CameraMode::FirstPerson) {
                // Камера строго в голове (условно 1.7 метра от земли)
                return { targetPos.x, targetPos.y + 1.7f, targetPos.z };
            }

            // Логика 3-го лица: расчет точки на сфере с учетом офсета за плечо
            // (Здесь будет простая тригонометрия в .cpp файле)
            Vector3 finalPos = targetPos;
            // ... расчет вектора назад от взгляда ...
            return finalPos; 
        }

        // Геттеры для UI и движка
        CameraMode GetMode() const { return currentMode; }
        float GetZoomPercent() const { return (currentDistance / maxDistance) * 100.0f; }
    };
}
