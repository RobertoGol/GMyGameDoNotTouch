#pragma once
#include <string>

/**
 * PROJECT: Bunker Protocol
 * MODULE: UI_Ping
 * DESCRIPTION: Виджет сетевой задержки (Ping) в стиле Log Horizon.
 */

namespace BunkerProtocol {

    enum class LinkStatus { Stable, Unstable, Critical, Offline };

    class UIPing {
    private:
        int currentPing = 0;
        LinkStatus status = LinkStatus::Offline;

    public:
        UIPing() = default;

        /**
         * Обновление значения пинга (получаем из сетевого модуля)
         */
        void SetPing(int ms) {
            currentPing = ms;
            if (ms < 60) status = LinkStatus::Stable;
            else if (ms < 150) status = LinkStatus::Unstable;
            else status = LinkStatus::Critical;
        }

        /**
         * Получение цвета для текста (для Vulkan/DX11 рендера)
         * 0x00FF00 - Зеленый, 0xFFFF00 - Желтый, 0xFF0000 - Красный
         */
        uint32_t GetStatusColor() const {
            switch (status) {
                case LinkStatus::Stable:   return 0x00FF00; 
                case LinkStatus::Unstable: return 0xFFFF00;
                case LinkStatus::Critical: return 0xFF0000;
                default:                   return 0xAAAAAA;
            }
        }

        /**
         * Форматированная строка для HUD (например: "MS: 42")
         */
        std::string GetPingString() const {
            if (status == LinkStatus::Offline) return "LINK: LOST";
            return "MS: " + std::to_string(currentPing);
        }
    };
}
