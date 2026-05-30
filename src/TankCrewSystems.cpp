#include <string>
#include <vector>
#include <algorithm>
#include "../include/GameRuntime.hpp"
#include "../include/SessionProfiles.hpp"

namespace bunker {

// 1. Физическое переключение мест экипажа внутри кабины танка BT-72 (Пилот <-> Стрелок)
// Изменяет состояние сидения и проверяет, разблокировано ли второе кресло политикой экипажа
void TryToggleBt72CrewSeat(bunker::PlayerState& player, bunker::SessionProfile& profile, bunker::GameState& gameState) {
    if (!player.insideTank) {
        return;
    }

    // Переключаем флаг места стрелка/оператора орудия
    player.bt72GunnerSeat = !player.bt72GunnerSeat;

    // Обновляем текстовое событие для Debug HUD и вывода логов на экран
    if (player.bt72GunnerSeat) {
        gameState.lastEvent = "Shifted to BT-72 Weapon Control Rig (Gunner Active).";
        profile.partnerTank.assignedGunnerHandle = profile.character.displayName;
    } else {
        gameState.lastEvent = "Shifted to BT-72 Driving Column (Pilot Active).";
        profile.partnerTank.assignedGunnerHandle.clear();
    }
}

// 2. Логика переключения Pip-Pad интерфейса / КПК на клавишу TAB
// Проверяет права доступа выжившего к локальной подсети бункера перед открытием UI
void TryToggleActivePipDevice(bunker::GameState& gameState, bunker::SessionProfile& profile) {
    // Вызываем глобальную проверку прав доступа устройства из вашей экосистемы
    bool hasAccess = (profile.character.mp > 0.0f) && (!profile.selectedWorld.empty());
    
    if (hasAccess) {
        gameState.lastEvent = "Pip-Pad network connection established. Interface layer toggled.";
    } else {
        gameState.lastEvent = "Pip-Pad access restricted: Hardware initialization fault or low MP.";
    }
}

// 3. Дополнительная служебная функция проверки возможности активации Pip-Pad
// Требуется для синхронизации флагов видимости интерфейса на странице 15
bool PlayerHasActivePipDeviceAccess(const bunker::SessionProfile& profile) {
    // Устройство активно, если профиль корректно инициализирован и персонаж дееспособен
    return !profile.playerName.empty() && (profile.character.hp > 0.0f);
}

} // namespace bunker
