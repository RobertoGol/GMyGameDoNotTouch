#pragma once
#include <vector>
#include <string>
#include "Skill_Base.hpp"
#include "Camera_Avatar.hpp"

/**
 * PROJECT: Bunker Protocol
 * MODULE: Input_Direct
 * DESCRIPTION: Заголовочный файл управления (Клавиатура, Мышь, Трекбол).
 */

namespace BunkerProtocol {

    // Коды клавиш (упрощенно, для связи с SDL/GLFW)
    enum class Keys { W, A, S, D, Space, Shift, LClick, RClick };

    struct RawInput {
        float mouseX, mouseY;   // Дельта для трекбола
        float wheelDelta;       // Для зума Fallout-камеры
        std::vector<Keys> pressedKeys;

        bool isDown(Keys k) const {
            for(auto key : pressedKeys) if(key == k) return true;
            return false;
        }
    };

    class InputHandler {
    private:
        // Физика трекбола (инерция)
        float ballVelX = 0.0f;
        float ballVelY = 0.0f;
        const float friction = 0.96f; 
        const float sensitivity = 0.2f;

    public:
        InputHandler() = default;

        /**
         * Основной цикл обновления ввода
         */
        void Update(float dt, const RawInput& raw, SkillBase& skills, CameraAvatar& camera);

        /**
         * Анализ действий для создания SSR-навыков (Log Horizon Style)
         */
        void TrackActions(const RawInput& raw, SkillBase& skills);
    };
}
