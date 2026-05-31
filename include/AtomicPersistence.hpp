#pragma once
#ifndef BUNKER_ATOMIC_PERSISTENCE_HPP
#define BUNKER_ATOMIC_PERSISTENCE_HPP

#include <filesystem>
#include <functional>
#include <string>
#include <system_error>
#include <cstdint>
#include <cstring>

#include "SessionProfiles.hpp"
#include "World.hpp"

// Опережающие объявления (Forward Declarations) для разрыва циклической зависимости
namespace bunker {
    class World;
    struct SessionProfile;
}

namespace bunker {

    
   // Структура конфигурации оружия игрока (Физические моды F4 + Легендарные Звезды F76)
    struct WeaponCustomization {
        std::string baseWeaponId;          // Идентификатор базового оружия
        std::string receiverModId;         // Модификатор ресивера
        std::string barrelModId;           // Модификатор ствола/глушителя
        std::string stockModId;            // Модификатор приклада
        std::string legendaryStars[4];     // Массив до 4-х Легендарных Звезд-Мутаций
    };

#ifndef NETWORK_PLAYER_VISUAL_SNAPSHOT_GUARD
#define NETWORK_PLAYER_VISUAL_SNAPSHOT_GUARD

// Гарантирует стабильный обмен между 140 друзьями по LANлайн без лагов компиляции
#pragma pack(push, 1)
    struct WeaponVisualSnapshot {
        uint32_t weaponRegistryIdHash;
        uint8_t  receiverId;
        uint8_t  barrelId;
        uint8_t  magazineId;
        uint8_t  muzzleId;
        uint8_t  paintJobId;
        uint8_t  wearLevel;
        uint8_t  metallicGloss;
    };

    struct ApparelVisualSnapshot {
        uint8_t  undergarmentId;
        uint8_t  armorPlatesId;
        uint8_t  decalId;
        uint8_t  decalPosition;
        uint32_t customColorHEX;
    };

    struct NetworkPlayerVisualSnapshot {
        uint32_t networkPlayerId;            // ID сетевого игрока (4 байта)
        char     characterName[32];          // Текстовый буфер имени оператора (32 байта)
        WeaponVisualSnapshot equippedWeapon; // Структура оружия (11 байт)
        ApparelVisualSnapshot apparel;       // Структура одежды (8 байт)
        float    positionX;                  // Координата X (4 байта)
        float    positionZ;                  // Координата Z (4 байта)
        float    rotationY;                  // Угол поворота Y (4 байта)
        float    vehicleSpeed;               // Физическая скорость для анимации гусениц (4 байта)
    };
#pragma pack(pop)
#endif // NETWORK_PLAYER_VISUAL_SNAPSHOT_GUARD

    // --- СТРУКТУРЫ И ПРОТОТИПЫ АТОМАРНОЙ ЗАПИСИ (ИНТЕРФЕЙСНЫЙ СЛОЙ) ---
    struct SaveStatus {
        bool ok = false;
        std::string message;
    };
    // --- СТРУКТУРЫ И ПРОТОТИПЫ АТОМАРНОЙ ЗАПИСИ (ИНТЕРФЕЙСНЫЙ СЛОЙ) ---
    struct SaveStatus {
        bool ok = false;
        std::string message;
    };
  // Объявление прототипов функций (Сама тяжелая логика унесена в .cpp)
    SaveStatus AtomicWriteFile(const std::filesystem::path& finalPath, const std::function<bool(const std::filesystem::path&)>& writer);
    SaveStatus SaveWorldAtomically(const World& world, const std::filesystem::path& worldPath);
    SaveStatus SaveProfileAtomically(const SessionProfile& profile, const std::filesystem::path& profilePath);
    NetworkPlayerVisualSnapshot PackWeaponToSnapshot(uint32_t playerId, const WeaponCustomization& weapon, float speed, uint32_t paintColor);

} // namespace bunker

#endif // BUNKER_ATOMIC_PERSISTENCE_HPP
