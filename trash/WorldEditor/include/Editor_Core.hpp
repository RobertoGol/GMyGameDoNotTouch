#pragma once
#include "../../include/World_Resources.hpp"
#include <vector>

namespace BunkerProtocol {

    enum class EditorMode { Translate, Rotate, Scale };

    class EditorCore {
    private:
        std::vector<MapObject> sceneObjects; // Объекты в текущем .wld
        int selectedIndex = -1;             // Индекс выбранного объекта
        EditorMode currentMode = EditorMode::Translate;
        float snapValue = 1.0f;             // Шаг сетки (Snap)

    public:
        // Выбор объекта кликом (Raycasting заглушка)
        void SelectObject(int index) { selectedIndex = index; }

        // Манипуляция (как в Creation Kit)
        void ApplyTransform(float deltaX, float deltaY, float deltaZ) {
            if (selectedIndex == -1) return;
            
            auto& obj = sceneObjects[selectedIndex];
            if (currentMode == EditorMode::Translate) {
                obj.x += std::round(deltaX / snapValue) * snapValue;
                obj.y += std::round(deltaY / snapValue) * snapValue;
                obj.z += std::round(deltaZ / snapValue) * snapValue;
            }
        }

        // Дублирование (Ctrl+D)
        void DuplicateSelected() {
            if (selectedIndex != -1) {
                sceneObjects.push_back(sceneObjects[selectedIndex]);
                selectedIndex = sceneObjects.size() - 1;
            }
        }

        std::vector<MapObject>& GetScene() { return sceneObjects; }
    };
}
