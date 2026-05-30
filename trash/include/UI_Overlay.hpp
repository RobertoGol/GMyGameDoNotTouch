#pragma once
#include <vector>
#include <string>
#include "Avatar_State.hpp"
#include "Skill_Base.hpp"

namespace BunkerProtocol {

    // Состояния меню в стиле Ancient Tales (вкладки сверху)
    enum class MenuTab { 
        Attributes, // S.P.E.C.I.A.L.
        Perks,      // Карточки навыков (Fallout 76 style)
        Inventory,  // Предметы и Крафт
        Map,        // Навигация
        Social      // Группа и Друзья
    };

    class UIOverlay {
    private:
        MenuTab activeTab = MenuTab::Attributes;
        float interfaceScale = 1.0f;

    public:
        /**
         * 1. ГЛАВНЫЙ ЭКРАН (HUD) - Стиль Log Horizon
         * Плавающие виджеты, прозрачные окна, Hardware Monitor.
         */
        void DrawMainHUD(const AvatarState& player, const SystemSpecs& specs) {
            // В УГЛУ: Плашка игрока (@ID) + Лог железа (CPU/RAM)
            // ПО ЦЕНТРУ СВЕРХУ: Всплывающие названия локаций (из Scene_Manager)
            // ПО БОКАМ: Список группы (Party Widgets) с HP/MP барами
        }

        /**
         * 2. СИСТЕМА ПЕРКОВ - Стиль Fallout 76 (по твоему скриншоту)
         * Но в визуальной обертке Log Horizon (неоновые рамки, аниме-иконки).
         */
        void DrawPerkMenu(const AvatarState& player, const std::vector<SkillData>& skills) {
            // СВЕРХУ: 7 колонок букв S.P.E.C.I.A.L.
            // ПОД НИМИ: Карточки навыков (R, SR, SSR, UR ранги)
            // ЛОГИКА: Если Интеллект 5, можно вставить карточек SSR на 5 очков.
            
            for (char stat : {'S','P','E','C','I','A','L'}) {
                RenderStatColumn(stat, player.GetStatValue(stat));
                // Отрисовка карт, которые игрок "создал" своими действиями
            }
        }

        /**
         * 3. ИНВЕНТАРЬ И КРАФТ - Смесь Fallout 76 и Ancient Tales
         * Функционал списка с весом, но с подменю выбора.
         */
        void DrawInventoryMenu(const AvatarState& player) {
            // ЛЕВАЯ ПАНЕЛЬ: Список предметов (#%it_XXXXX) и их вес.
            // ПРАВАЯ ПАНЕЛЬ: 3D-модель (или цветной куб-заглушка) + кнопка "РАЗОБРАТЬ".
            // НИЖНЯЯ ПАНЕЛЬ: Кнопки управления (как на твоем скрине: [X] Выбросить, [Y] Осмотреть).
        }

        /**
         * 4. БОЕВОЙ ИНТЕРФЕЙС - Стиль Fallout 76
         * Динамические элементы во время экшена.
         */
        void DrawCombatUI(const AvatarState& player) {
            // НИЖНИЙ ЛЕВЫЙ УГОЛ: Полоска HP и уровень радиации/заражения.
            // НИЖНИЙ ПРАВЫЙ УГОЛ: Мана (MP) и стамина для рывков.
            // ЦЕНТР: Прицел, адаптированный под Трекбол (плавное перекрестие).
        }
    };
}
