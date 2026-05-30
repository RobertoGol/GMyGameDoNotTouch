#include "include/Input_Direct.hpp"
#include <cmath>
#include <iostream>

/**
 * PROJECT: Bunker Protocol
 * MODULE: Input_Direct
 * DESCRIPTION: Обработка трекбола с инерцией и трекинг действий для SSR-скиллов.
 */

namespace BunkerProtocol {

    void InputHandler::Update(float deltaTime, RawInput& raw, SkillBase& skillSystem, CameraAvatar& camera) {
        
        // 1. ЛОГИКА ТРЕКБОЛА (С инерцией)
        // Трекбол выдает дельту. Даже если рука убрана, шарик может крутиться.
        float sensitivity = 0.15f;
        float friction = 0.95f; // Замедление шарика

        // Если есть физический ввод - обновляем скорость вращения
        if (std::abs(raw.mouseX) > 0.01f || std::abs(raw.mouseY) > 0.01f) {
            currentBallVelocityX = raw.mouseX * sensitivity;
            currentBallVelocityY = raw.mouseY * sensitivity;
        } else {
            // Если ввода нет - шарик плавно останавливается (инерция)
            currentBallVelocityX *= friction;
            currentBallVelocityY *= friction;
        }

        // Передаем вращение в камеру (Fallout Style 1st/3rd person)
        camera.ApplyRotation(currentBallVelocityX, currentBallVelocityY);

        // 2. ТРЕКИНГ ДЕЙСТВИЙ (RecordAction)
        // Проверяем нажатия и записываем их для генерации навыков
        if (raw.isKeyPressed(Keys::Space)) {
            skillSystem.RecordAction("Jump");
            
            // Проверка комбо: Прыжок + Удар (Jump_Attack)
            if (raw.isMouseButtonPressed(0)) {
                skillSystem.RecordAction("Aerial_Strike"); 
                std::cout << "[System] Action Tracked: Aerial Strike (SSR Progress)" << std::endl;
            }
        }

        if (raw.isMouseButtonPressed(0)) {
            skillSystem.RecordAction("Basic_Attack");
        }

        // 3. ОБРАБОТКА ЗУМА (Колесико или кнопки трекбола)
        if (std::abs(raw.wheelDelta) > 0.1f) {
            camera.UpdateZoom(raw.wheelDelta);
            skillSystem.RecordAction("View_Adjust"); // Даже это можно превратить в перк на обзор!
        }
    }

    /**
     * Помощник для определения сложных комбо (Log Horizon Style)
     */
    void InputHandler::AnalyzePattern(const std::vector<std::string>& sequence, SkillBase& skillSystem) {
        // Здесь можно добавить проверку последовательностей типа W+W (Рывок)
        // Если последовательность совпадает с уникальной - генерируем UR навык.
    }
}
