#pragma once

#include <string_view>

namespace bunker {

struct BuildAnnouncementEntry {
    int buildNumber = 0;
    const char* announcementId = "";
    const char* versionLabel = "";
    const char* buildId = "";
    const char* dateLabel = "";
    const char* title = "";
    const char* summary = "";
    const char* details = "";
};

inline constexpr int kCurrentBuildNumber = 20260421;
inline constexpr std::string_view kCurrentVersionLabel = "0.6.21-dev";
inline constexpr std::string_view kCurrentBuildId = "bp-2026-04-21-launcher-notice";
inline constexpr std::string_view kCurrentBuildDate = "2026-04-21";

inline const BuildAnnouncementEntry& CurrentBuildAnnouncement() {
    static const BuildAnnouncementEntry kAnnouncement = {
        kCurrentBuildNumber,
        "launcher_notice_2026_04_21",
        "0.6.21-dev",
        "bp-2026-04-21-launcher-notice",
        "2026-04-21",
        "Node Notice // Build Updated",
        "Launcher now surfaces a local build notice and remembers dismiss state per profile.",
        "This build adds a compact announcement widget on the launcher main screen, keeps read-state locally, "
        "and re-opens the notice when a newer build or newer local announcement is shipped. "
        "No network fetch is involved; this is a local system notice tied to the current build."
    };
    return kAnnouncement;
}

inline bool ShouldShowBuildAnnouncement(
    int lastSeenBuildNumber,
    std::string_view lastSeenAnnouncementId) {
    const auto& announcement = CurrentBuildAnnouncement();
    return announcement.buildNumber > lastSeenBuildNumber ||
        lastSeenAnnouncementId != announcement.announcementId;
}

}  // namespace bunker
