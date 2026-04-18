#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace bunker {

namespace fs = std::filesystem;

inline fs::path ProjectRoot() {
    return fs::current_path();
}

inline fs::path WorldDirectory() {
    return ProjectRoot() / "world";
}

inline fs::path ProfilesDirectory() {
    return ProjectRoot() / "profiles";
}

inline fs::path ExportsDirectory() {
    return ProjectRoot() / "exports";
}

inline fs::path LanlineSessionsDirectory() {
    return ProfilesDirectory() / "lanline_sessions";
}

inline void EnsureProjectDirectories() {
    fs::create_directories(WorldDirectory());
    fs::create_directories(ProfilesDirectory());
    fs::create_directories(ExportsDirectory());
    fs::create_directories(LanlineSessionsDirectory());
}

inline fs::path DefaultWorldPath() {
    return WorldDirectory() / "start_zone.bwld";
}

inline bool IsLexicallyWithin(const fs::path& candidate, const fs::path& root) {
    const fs::path normalizedCandidate = candidate.lexically_normal();
    const fs::path normalizedRoot = root.lexically_normal();
    const fs::path relative = normalizedCandidate.lexically_relative(normalizedRoot);
    if (relative.empty()) {
        return normalizedCandidate == normalizedRoot;
    }
    const auto begin = relative.begin();
    return begin != relative.end() && begin->string() != "..";
}

inline fs::path TrimWorldDirectoryPrefix(const fs::path& candidate) {
    if (candidate.empty()) {
        return {};
    }

    auto part = candidate.begin();
    if (part != candidate.end() && *part == "world") {
        ++part;
        fs::path trimmed;
        for (; part != candidate.end(); ++part) {
            trimmed /= *part;
        }
        return trimmed;
    }
    return candidate;
}

inline std::string NormalizeWorldReference(std::string_view worldName) {
    if (worldName.empty()) {
        return DefaultWorldPath().filename().generic_string();
    }

    fs::path candidate(worldName);
    if (candidate.empty()) {
        return DefaultWorldPath().filename().generic_string();
    }

    candidate = candidate.lexically_normal();
    if (candidate.is_absolute()) {
        if (IsLexicallyWithin(candidate, WorldDirectory())) {
            const fs::path relativeToWorld = candidate.lexically_relative(WorldDirectory().lexically_normal());
            return relativeToWorld.empty()
                ? DefaultWorldPath().filename().generic_string()
                : relativeToWorld.generic_string();
        }
        if (IsLexicallyWithin(candidate, ProjectRoot())) {
            candidate = candidate.lexically_relative(ProjectRoot().lexically_normal());
        } else {
            return candidate.has_filename()
                ? candidate.filename().generic_string()
                : DefaultWorldPath().filename().generic_string();
        }
    }

    candidate = TrimWorldDirectoryPrefix(candidate.lexically_normal());
    if (candidate.empty() || candidate == ".") {
        return DefaultWorldPath().filename().generic_string();
    }
    return candidate.generic_string();
}

inline fs::path ResolveWorldPath(std::string_view worldName) {
    const std::string normalizedWorldName = NormalizeWorldReference(worldName);
    fs::path candidate(normalizedWorldName);
    if (candidate.is_absolute()) {
        return candidate;
    }
    if (candidate.has_parent_path()) {
        return WorldDirectory() / candidate;
    }
    return WorldDirectory() / candidate;
}

inline fs::path EditorConceptManifestPath() {
    return ExportsDirectory() / "concept_manifest.txt";
}

inline fs::path EditorPrefabLibraryPath() {
    return ExportsDirectory() / "prefab_library.txt";
}

inline std::string SanitizeWorldKey(std::string_view worldName) {
    std::string key;
    key.reserve(worldName.size());
    for (const char ch : worldName) {
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')) {
            key.push_back(ch);
        } else {
            key.push_back('_');
        }
    }
    if (key.empty()) {
        key = "start_zone_bwld";
    }
    return key;
}

inline fs::path StaticEraserPath(std::string_view worldName) {
    return WorldDirectory() / (SanitizeWorldKey(worldName) + ".erased_objects.dat");
}

inline fs::path DefaultSessionProfilePath() {
    return ProfilesDirectory() / "current_session.profile";
}

inline fs::path LaunchTicketPath() {
    return ProfilesDirectory() / "launch.ticket";
}

inline fs::path LanlineSessionPath() {
    return ProfilesDirectory() / "lanline_session.state";
}

inline fs::path LanlineSessionSnapshotPath(std::string_view sessionId) {
    return LanlineSessionsDirectory() / (SanitizeWorldKey(sessionId) + ".state");
}

inline fs::path CharacterProfilePath(std::string_view characterId) {
    return ProfilesDirectory() / (std::string(characterId) + ".profile");
}

}  // namespace bunker
