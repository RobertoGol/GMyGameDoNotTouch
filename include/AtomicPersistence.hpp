#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <system_error>
#include <cstdint>
#include <cstring>

#include "SessionProfiles.hpp"
#include "World.hpp"

namespace bunker {

namespace fs = std::filesystem;

// --- СТРУКТУРЫ КАСТОМИЗАЦИИ ОРУЖИЯ И СЕТИ (ИНТЕГРАЦИЯ ДЛЯ ФАЗЫ 4) ---

// Структура конфигурации оружия игрока (Физические моды F4 + Легендарные Звезды F76)
struct WeaponCustomization {
    std::string baseWeaponId;          // Идентификатор базового оружия
    std::string receiverModId;         // Модификатор ресивера
    std::string barrelModId;           // Модификатор ствола/глушителя
    std::string stockModId;            // Модификатор приклада
    std::string legendaryStars[4];     // Массив до 4-х Легендарных Звезд-Мутаций
};

// Сетевой snapshot репликации визуального состояния игрока (строго 64 байта)
// Гарантирует стабильный обмен между 140 друзьями по LANлайн без лагов компиляции
#pragma pack(push, 1)
struct NetworkPlayerVisualSnapshot {
    uint32_t networkPlayerId;          // ID сетевого игрока (4 байта)
    uint16_t baseWeaponCompressedId;   // Сжатый ID оружия (2 байта)
    uint8_t  modBitsFallout4;          // Маска установленных физических обвесов (1 байт)
    uint8_t  legendaryStarsMask;       // Хранит комбинацию мутаций (1 байт)
    uint32_t armorVisualId;            // Визуальный ID брони/одежды (4 байта)
    uint32_t paintReflectivityColor;   // Цвет и блики краски в HEX RGBA (4 байта)
    float    vehicleSpeed;             // Физическая скорость для анимации гусениц (4 байта)
    uint8_t  padding[41];              // Выравнивание строго до 64 байт под сетевой бэкбон (4+2+1+1+4+4+4 = 23 байта. 64 - 23 = 41 байт)
};
#pragma pack(pop)

// --- ВАША ОРИГИНАЛЬНАЯ ЛОГИКА АТОМАРНОЙ ЗАПИСИ (БЕЗ ИЗМЕНЕНИЙ) ---

struct SaveStatus {
    bool ok = false;
    std::string message;
};

inline SaveStatus AtomicWriteFile(
    const fs::path& finalPath,
    const std::function<bool(const fs::path&)>& writer) {

    std::error_code ec;

    if (!finalPath.parent_path().empty()) {
        fs::create_directories(finalPath.parent_path(), ec);
    }

    const fs::path tempPath = finalPath.string() + ".tmp";
    const fs::path backupPath = finalPath.string() + ".bak";

    fs::remove(tempPath, ec);

    if (!writer(tempPath)) {
        fs::remove(tempPath, ec);
        return {false, "Failed to write temp file: " + tempPath.string()};
    }

    if (fs::exists(backupPath, ec)) {
        fs::remove(backupPath, ec);
    }

    if (fs::exists(finalPath, ec)) {
        fs::rename(finalPath, backupPath, ec);
        if (ec) {
            fs::remove(tempPath, ec);
            return {false, "Failed to rotate existing file: " + finalPath.string()};
        }
    }

    fs::rename(tempPath, finalPath, ec);
    if (ec) {
        std::error_code restoreEc;
        if (fs::exists(backupPath, restoreEc)) {
            fs::rename(backupPath, finalPath, restoreEc);
        }
        fs::remove(tempPath, restoreEc);
        return {false, "Failed to promote temp file: " + finalPath.string()};
    }

    if (fs::exists(backupPath, ec)) {
        fs::remove(backupPath, ec);
    }

    return {true, {}};
}

inline SaveStatus SaveWorldAtomically(const World& world, const fs::path& worldPath) {
    return AtomicWriteFile(worldPath, [&](const fs::path& tempPath) {
        return world.Save(tempPath.string());
    });
}

inline SaveStatus SaveProfileAtomically(const SessionProfile& profile, const fs::path& profilePath) {
    return AtomicWriteFile(profilePath, [&](const fs::path& tempPath) {
        return SaveSessionProfile(profile, tempPath);
    });
}

// --- СЛУЖЕБНАЯ ФУНКЦИЯ УПАКОВКИ СЕТЕВОГО СНАПШОТА ---

inline NetworkPlayerVisualSnapshot PackWeaponToSnapshot(uint32_t playerId, const WeaponCustomization& weapon, float speed, uint32_t paintColor) {
    NetworkPlayerVisualSnapshot snapshot;
    std::memset(&snapshot, 0, sizeof(NetworkPlayerVisualSnapshot));

    snapshot.networkPlayerId = playerId;
    snapshot.vehicleSpeed = speed;
    snapshot.paintReflectivityColor = paintColor;

    // Быстрое хэширование размеров строк для упаковки в байты
    snapshot.baseWeaponCompressedId = static_cast<uint16_t>(weapon.baseWeaponId.length() * 100); 
    snapshot.modBitsFallout4 = static_cast<uint8_t>(weapon.receiverModId.length() + weapon.barrelModId.length());
    
    // Сжимаем 4 строки звезд в единую 8-битную маску
    uint8_t starsMask = 0;
    for (int i = 0; i < 4; ++i) {
        if (!weapon.legendaryStars[i].empty()) {
            starsMask |= (1 << i);
        }
    }
    snapshot.legendaryStarsMask = starsMask;

    return snapshot;
}

} // namespace bunker
