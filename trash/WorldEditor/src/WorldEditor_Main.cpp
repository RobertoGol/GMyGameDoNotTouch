#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

// Подключаем заголовки из основного проекта
#include "../../include/World_Resources.hpp"
#include "../../include/Registry_ID.hpp"

/**
 * PROJECT: Bunker Protocol - World Editor (Creation Kit Analog)
 * MODULE: WorldEditor_Main
 * DESCRIPTION: Главный файл редактора. Управляет сценой, ассетами и экспортом.
 */

using namespace BunkerProtocol;

// Перечисления для режимов манипуляции (как в Creation Kit)
enum class GizmoMode { Translate, Rotate, Scale };

class WorldEditorApp {
private:
    // 1. Библиотека объектов (Object Window)
    struct AssetTemplate {
        char id;
        std::string name;
        InteractionType type;
    };
    std::vector<AssetTemplate> library;

    // 2. Текущая сцена (Cell View / Render Window)
    std::vector<MapObject> activeScene;
    int selectedIndex = -1; // Индекс выделенного объекта

    // 3. Состояние редактора
    GizmoMode currentGizmo = GizmoMode::Translate;
    float snapValue = 1.0f; // Привязка к сетке (Snap)
    bool isRunning = true;

public:
    WorldEditorApp() {
        InitLibrary();
        std::cout << "[Editor] Creation Kit Mode Active. Welcome, Architect." << std::endl;
    }

    /**
     * Инициализация списка доступных ассетов (левая панель)
     */
    void InitLibrary() {
        library.push_back({1, "Wall_Bunker_High", InteractionType::Static});
        library.push_back({2, "Floor_Steel_Grid", InteractionType::Static});
        library.push_back({3, "Emergency_Crate",  InteractionType::Container});
        library.push_back({4, "Sentry_Bot_Spawn", InteractionType::Static});
    }

    /**
     * Логика горячих клавиш (Hotkeys)
     */
    void HandleHotkeys(char key) {
        switch (key) {
            case 'w': currentGizmo = GizmoMode::Translate; break;
            case 'e': currentGizmo = GizmoMode::Rotate; break;
            case 'r': currentGizmo = GizmoMode::Scale; break;
            case 'd': // Дублирование (Ctrl+D аналог)
                if (selectedIndex != -1) DuplicateSelected();
                break;
            case 's': SaveWorld("test_zone"); break; // Сохранение
            case 'q': isRunning = false; break;
        }
    }

    /**
     * Размещение нового объекта (Drag & Drop аналог)
     */
    void PlaceFromLibrary(int libraryIndex, float x, float y, float z) {
        if (libraryIndex >= library.size()) return;

        MapObject newObj;
        newObj.objectID = library[libraryIndex].id;
        newObj.interactType = library[libraryIndex].type;
        newObj.x = std::round(x / snapValue) * snapValue;
        newObj.y = std::round(y / snapValue) * snapValue;
        newObj.z = std::round(z / snapValue) * snapValue;
        newObj.health = 100.0f;

        activeScene.push_back(newObj);
        selectedIndex = activeScene.size() - 1;
        std::cout << "[Editor] Placed: " << library[libraryIndex].name << " at [" << newObj.x << "]" << std::endl;
    }

    void DuplicateSelected() {
        if (selectedIndex == -1) return;
        MapObject copy = activeScene[selectedIndex];
        copy.x += snapValue; // Смещаем дубликат на шаг сетки
        activeScene.push_back(copy);
        selectedIndex = activeScene.size() - 1;
        std::cout << "[Editor] Object Duplicated." << std::endl;
    }

    /**
     * Экспорт в .wld (в папку /world/)
     */
    void SaveWorld(const std::string& name) {
        std::string path = "../../world/" + name + ".wld";
        std::ofstream file(path, std::ios::binary);

        if (!file.is_open()) {
            std::cerr << "[Error] Cannot write to /world/ folder!" << std::endl;
            return;
        }

        // Пишем Header и Данные (как в World_Resources)
        file.write("BWLD", 4);
        
        char areaName = "Bunker Alpha - Sector 1";
        file.write(areaName, 64);

        uint32_t count = (uint32_t)activeScene.size();
        file.write(reinterpret_cast<char*>(&count), sizeof(count));
        file.write(reinterpret_cast<char*>(activeScene.data()), sizeof(MapObject) * count);

        file.close();
        std::cout << "[Editor] World saved: " << path << " (" << count << " objects)" << std::endl;
    }

    bool IsActive() const { return isRunning; }
};

int main() {
    WorldEditorApp editor;

    // Имитация рабочего процесса (в будущем здесь будет ImGui цикл)
    editor.PlaceFromLibrary(0, 0, 0, 0); // Ставим стену
    editor.PlaceFromLibrary(1, 1, 0, 0); // Ставим пол рядом
    
    editor.HandleHotkeys('d'); // Дублируем пол
    editor.HandleHotkeys('s'); // Сохраняем результат в .wld

    std::cout << "\n[Editor] Work session finished. All layers exported to /world/." << std::endl;
    return 0;
}
