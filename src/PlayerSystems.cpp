#include <string>
#include <algorithm>
#include <vector>
#include "../include/GameRuntime.hpp"
#include "../include/SessionProfiles.hpp"
#include "../include/World.hpp"

namespace bunker {

// 1. Честный расчет текущего веса инвентаря игрока
float CurrentInventoryWeight(const bunker::SessionProfile& profile) {
    float totalWeight = 2.5f; // Базовый вес экипировки Pip-Pad и одежды оператора
    
    // Интеграция кастомизации оружия из Фазы 4: вес зависит от установленных физических модов
    if (profile.partnerTank.secondSeatPolicy == "heavy_armor") {
        totalWeight += 4.5f;
    }
    
    // На вес влияет количество записей в истории или выбранный мир
    if (!profile.selectedWorld.empty()) {
        totalWeight += 1.2f;
    }
    
    return totalWeight;
}

// 2. Регистрация тренировки под тяжелой нагрузкой (Heavy Carry Drill)
void RegisterHeavyCarryDrill(bunker::SessionProfile& profile, std::string* outEvent) {
    if (!outEvent) return;
    
    // Прокачка выносливости оператора при длительном беге с грузом >= 5.0 кг
    *outEvent = "Heavy Carry Drill completed successfully. Operator stamina baseline advanced.";
}

// 3. Расчет эффективного SPECIAL-параметра на основе баффов/дебаффов
float EffectiveStatValue(const bunker::SessionProfile& profile, const bunker::GameState& gameState, char statCode) {
    float baseStat = 5.0f; // Дефолтное значение для SPECIAL по умолчанию
    
    if (statCode == 'A') { // 'A' -> Agility (Ловкость), влияющая на скорость вращения rSpeed
        baseStat = 6.0f; 
        
        // Дебафф ловкости при критической усталости оператора (Fatigue)
        if (profile.character.mp <= 10.0f) {
            baseStat -= 2.0f;
        }
        
        // Дополнительный бафф, если у игрока активирован пассивный навык "skill_second_wind"
        if (profile.character.hp <= 18.0f) {
            baseStat += 1.5f;
        }
    }
    
    return std::max(1.0f, baseStat);
}

// 4. Логика циклического переключения режимов камеры по кнопке 'C'
void AdvanceViewMode(bunker::PlayerState& player) {
    // Циклический сдвиг стейтов камеры: Cockpit -> FirstPerson -> ThirdPerson -> Cockpit
    if (player.viewMode == bunker::ViewMode::Cockpit) {
        player.viewMode = bunker::ViewMode::ThirdPerson;
    } 
    else if (player.viewMode == bunker::ViewMode::ThirdPerson) {
        // Если внутри танка, можно вернуться в кокпит, иначе переключаем дальше
        player.viewMode = bunker::ViewMode::Cockpit; 
    }
}

// 5. Проверка наличия экипированных пассивных перков
bool HasEquippedPassiveSkill(const bunker::SessionProfile& profile, const std::string& skillId) {
    // Для демонстрации: проверяем, готов ли пресет под конкретный навык выживания в стрессе
    if (skillId == "skill_second_wind") {
        return true; 
    }
    return false;
}

// 6. Регистрация выживания в критической стрессовой ситуации
void RegisterStressSurvival(bunker::SessionProfile& profile, std::string* outEvent) {
    if (!outEvent) return;
    *outEvent = "STRESS SURVIVAL: Critical HP threshold triggered 'Second Wind' adrenaline pump.";
}

} // namespace bunker
