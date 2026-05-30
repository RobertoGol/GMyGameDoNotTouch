#pragma once
#include <string>
#include <vector>
#include "Game_Chat.hpp"

/**
 * PROJECT: Bunker Protocol
 * MODULE: UI_Chat
 * DESCRIPTION: Отрисовка чата и всплывающих уведомлений (Location Titles).
 */

namespace BunkerProtocol {

    struct UITheme {
        float opacity = 0.7f;
        uint32_t accentColor = 0x00AEEF; // Фирменный голубой Log Horizon
    };

    class UIChat {
    private:
        ChatChannel activeTab = ChatChannel::Global;
        bool isChatOpen = false;
        
        // Для уведомлений о локации
        std::string currentLocTitle;
        float titleTimer = 0.0f; // Время отображения заголовка
        bool showTitle = false;

    public:
        UIChat() = default;

        /**
         * Метод для переключения вкладок (вызывается мышью или кнопками)
         */
        void SwitchTab(ChatChannel newTab) {
            activeTab = newTab;
        }

        /**
         * Твое пожелание: Всплывающее объявление при переходе на локацию.
         * Вызывается из Scene_Manager при загрузке нового .map файла.
         */
        void ShowLocationTitle(const std::string& areaName, const std::string& subRegion = "") {
            currentLocTitle = areaName + (subRegion.empty() ? "" : " - " + subRegion);
            titleTimer = 5.0f; // Показываем 5 секунд
            showTitle = true;
        }

        /**
         * Обновление таймеров UI (вызывается каждый кадр)
         */
        void Update(float deltaTime) {
            if (titleTimer > 0.0f) {
                titleTimer -= deltaTime;
            } else {
                showTitle = false;
            }
        }

        /**
         * Заглушка для рендера (здесь будет вызов Vulkan/DX11 библиотек)
         */
        void Draw(const GameChat& chatLogic) {
            // 1. Отрисовка основного окна чата
            auto messages = chatLogic.GetMessagesByChannel(activeTab);
            // RenderWindow(activeTab, messages);

            // 2. Отрисовка заголовка локации (вверху экрана)
            if (showTitle) {
                // RenderTextCenteredTop(currentLocTitle, titleTimer);
            }
        }

        // Геттеры для управления вводом
        bool IsInputActive() const { return isChatOpen; }
        void ToggleChat() { isChatOpen = !isChatOpen; }
    };
}
