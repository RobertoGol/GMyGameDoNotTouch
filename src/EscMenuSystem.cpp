#include "../include/GameRuntime.hpp"
#include "imgui.h"
#include <GLFW/glfw3.h>

namespace bunker {

// Изолированная системная переменная текущей открытой страницы меню
EscMenuState g_CurrentMenuState = EscMenuState::Closed;

// Функция-переключатель состояний при нажатии на клавишу ESCAPE
void HandleEscKeyPress(GameState& gameState) {
    if (g_CurrentMenuState == EscMenuState::Closed) {
        // Каноничное условие: если карта активирована голозаписью — заходим через бумажный пергамент Fallout 76
        g_CurrentMenuState = gameState.isPaperMapAvailable ? EscMenuState::PaperMap : EscMenuState::Fallout4Menu;
    } else {
        g_CurrentMenuState = EscMenuState::Closed; // Закрыть меню по нажатию на Esc повторно
    }
}

// Рендеринг интерфейса ImGui поверх трехмерного ландшафта игры
void DrawImGuiEscMenuSystem(GameState& gameState, SessionProfile& profile) {
    if (g_CurrentMenuState == EscMenuState::Closed) {
        return;
    }

    // --- СЛОЙ 1: Топографическая бумажная карта местности (Стиль Fallout 76) ---
    if (g_CurrentMenuState == EscMenuState::PaperMap) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("##PaperMapF76", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
        
        ImGui::TextColored(ImVec4(0.88f, 0.78f, 0.58f, 1.0f), "[ ТОПОГРАФИЧЕСКАЯ БУМАЖНАЯ КАРТА РЕГИОНА - СТИЛЬ FALLOUT 76 ]");
        ImGui::TextDisabled("Синхронизация данных голозаписи: Онлайн (Игровая условность)");
        ImGui::Separator();
        
        if (ImGui::Button("ОТКРЫТЬ НАСТРОЙКИ И ИНСТРУМЕНТЫ СИСТЕМЫ (Esc)")) {
            g_CurrentMenuState = EscMenuState::Fallout4Menu; // Переход во второй слой
        }
        ImGui::End();
    }

    // --- СЛОЙ 2: Вертикальный зеленый список кнопок (Монохромный стиль Fallout 4) ---
    if (g_CurrentMenuState == EscMenuState::Fallout4Menu) {
        ImGui::SetNextWindowPos(ImVec2(50, 100));
        ImGui::SetNextWindowSize(ImVec2(290, 340));
        ImGui::Begin("##Fallout4Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f)); // Фирменный зеленый цвет Fallout 4
        ImGui::Text("ОПЕРАЦИОННАЯ СИСТЕМА БУНКЕРА");
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("ПРОДОЛЖИТЬ ВЫЖИВАНИЕ", ImVec2(270, 40))) {
            g_CurrentMenuState = EscMenuState::Closed;
        }
        if (ImGui::Button("НАСТРОЙКИ ГРАФИКИ И УПРАВЛЕНИЯ", ImVec2(270, 40))) {
            g_CurrentMenuState = EscMenuState::TitanfallSettings; // Переход к ползункам FOV
        }
        if (ImGui::Button("ВЫЙТИ В ЛАУНЧЕР ELDER TALES", ImVec2(270, 40))) {
            g_CurrentMenuState = EscMenuState::Closed;
            // Код вызова закрытия текущей сессии и возврата к окну авторизации
        }
        
        ImGui::PopStyleColor();
        ImGui::End();
    }

    // --- СЛОЙ 3: Продвинутые настройки FOV и звука (Стиль Titanfall 2) ---
    if (g_CurrentMenuState == EscMenuState::TitanfallSettings) {
        ImGui::SetNextWindowPos(ImVec2(120, 120));
        ImGui::SetNextWindowSize(ImVec2(500, 260));
        ImGui::Begin("ПАРАМЕТРЫ СИМУЛЯЦИИ", nullptr, ImGuiWindowFlags_NoResize);
        
        // Ползунок угла обзора FOV из Titanfall 2 - критически важен для разгрузки слабых ноутбуков
        ImGui::SliderFloat("Угол обзора (FOV)", &gameState.cameraFOV, 70.0f, 110.0f);
        ImGui::SliderFloat("Чувствительность мыши", &gameState.mouseSensitivity, 0.1f, 4.0f);
        ImGui::Separator();
        
        // Настройка ползунка аудио из Fallout 4
        ImGui::SliderFloat("Громкость радиостанций Pip-Pad", &gameState.audioRadioVolume, 0.0f, 1.0f);
        ImGui::Spacing();
        
        if (ImGui::Button("НАЗАД В МЕНЮ")) {
            g_CurrentMenuState = EscMenuState::Fallout4Menu; // Возврат на шаг назад
        }
        ImGui::End();
    }
}

} // namespace bunker
