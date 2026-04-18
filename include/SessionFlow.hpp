#pragma once

#include <string>

namespace bunker {

enum class SessionMode {
    Solo,
    LanHost,
    LanClient
};

enum class AppFlowState {
    Boot,
    Launcher,
    WorldLoading,
    ActivePlay,
    Paused
};

inline const char* ToString(SessionMode mode) {
    switch (mode) {
        case SessionMode::Solo:
            return "Solo";
        case SessionMode::LanHost:
            return "LAN Host";
        case SessionMode::LanClient:
            return "LAN Client";
    }
    return "Unknown";
}

inline const char* ToString(AppFlowState state) {
    switch (state) {
        case AppFlowState::Boot:
            return "Boot";
        case AppFlowState::Launcher:
            return "Launcher";
        case AppFlowState::WorldLoading:
            return "WorldLoading";
        case AppFlowState::ActivePlay:
            return "ActivePlay";
        case AppFlowState::Paused:
            return "Paused";
    }
    return "Unknown";
}

}  // namespace bunker
