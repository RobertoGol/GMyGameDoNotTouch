#ifndef BUNKER_ESC_MENU_SYSTEM_HPP
#define BUNKER_ESC_MENU_SYSTEM_HPP

#include "GameRuntime.hpp"

namespace bunker {

    // Перечисление слоёв интерфейса графического меню (Fallout 76 -> Fallout 4 -> Titanfall 2)
    enum class EscMenuState {
        Closed,
        PaperMap,
        Fallout4Menu,
        TitanfallSettings
    };

    // Глобальная переменная состояния текущего открытого слоя меню
    extern EscMenuState g_CurrentMenuState;

    // Функция-переключатель состояний при нажатии на клавишу ESCAPE
    void HandleEscKeyPress(GameState& gameState);

    // Функция-переключатель состояний при нажатии на клавишу ESCAPE (Перегрузка для калибровки профиля)
    void HandleEscKeyPress(GameState& gameState, const SessionProfile& profile);

    // Рендеринг интерфейса ImGui поверх трехмерного ландшафта игры
    void DrawImGuiEscMenuSystem(GameState& gameState, SessionProfile& profile);

} // namespace bunker

#endif // BUNKER_ESC_MENU_SYSTEM_HPP
