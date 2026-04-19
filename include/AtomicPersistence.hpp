#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <system_error>

#include "SessionProfiles.hpp"
#include "World.hpp"

namespace bunker {

namespace fs = std::filesystem;

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

} // namespace bunker