#include "../include/AtomicPersistence.hpp"
#include "../include/World.hpp"
#include "../include/SessionProfiles.hpp"
#include <fstream>
#include <cstring>

namespace fs = std::filesystem;

namespace bunker {

    // --- ВАША ОРИГИНАЛЬНАЯ ЛОГИКА АТОМАРНОЙ ЗАПИСИ (БЕЗ ИЗМЕНЕНИЙ) ---
    SaveStatus AtomicWriteFile(const fs::path& finalPath, const std::function<bool(const fs::path&)>& writer) {
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

    SaveStatus SaveWorldAtomically(const World& world, const fs::path& worldPath) {
        return AtomicWriteFile(worldPath, [&](const fs::path& tempPath) {
            return world.Save(tempPath.string());
        });
    }

    SaveStatus SaveProfileAtomically(const SessionProfile& profile, const fs::path& profilePath) {
        return AtomicWriteFile(profilePath, [&](const fs::path& tempPath) {
            return SaveSessionProfile(profile, tempPath);
        });
    }
    // --- СЛУЖЕБНАЯ ФУНКЦИЯ УПАКОВКИ СЕТЕВОГО СНАПШОТА (ПОД ПОЛНУЮ СТРУКТУРУ) ---
    NetworkPlayerVisualSnapshot PackWeaponToSnapshot(uint32_t playerId, const WeaponCustomization& weapon, float speed, uint32_t paintColor) {
        NetworkPlayerVisualSnapshot snapshot;
        std::memset(&snapshot, 0, sizeof(NetworkPlayerVisualSnapshot));
        
        // 1. Прямая привязка базовых полей (Стр. 3-4 PDF)
        snapshot.networkPlayerId = playerId;
        snapshot.vehicleSpeed = speed;
        
        // Перенаправляем цвет в каноничное поле полной структуры
        snapshot.apparel.customColorHEX = paintColor;

        // 2. Хэшируем размеры строк модификаций Fallout 4 в байтовые поля подструктуры
        snapshot.equippedWeapon.weaponRegistryIdHash = static_cast<uint32_t>(weapon.baseWeaponId.length() * 100);
        snapshot.equippedWeapon.receiverId = static_cast<uint8_t>(weapon.receiverModId.length());
        snapshot.equippedWeapon.barrelId = static_cast<uint8_t>(weapon.barrelModId.length());

        // 3. Архивация массива строк мутаций Fallout 76 в единую 8-битную маску
        uint8_t starsMask = 0;
        for (int i = 0; i < 4; ++i) {
            if (!weapon.legendaryStars[i].empty()) {
                starsMask |= (1 << i);
            }
        }
        
        // Сохраняем готовую маску звезд в свободный байт paintJobId полной структуры
        snapshot.equippedWeapon.paintJobId = starsMask;
        return snapshot;
    }

