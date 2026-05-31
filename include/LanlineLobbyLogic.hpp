    #pragma once

    #include <algorithm>

    #include "LanlineSession.hpp"

    namespace bunker {

    inline bool IsLanlineAwaitingSlot(const LanlinePlayerEntry& entry) {
        return entry.role == "Awaiting";
    }

    inline bool IsLanlinePendingSlot(const LanlinePlayerEntry& entry) {
        return entry.role == "Pending Client";
    }

    inline bool IsLanlineReservedSlot(const LanlinePlayerEntry& entry) {
        return entry.role == "Reserved Client";
    }

    inline bool IsLanlineAcceptedSlot(const LanlinePlayerEntry& entry) {
        return entry.role == "Client";
    }

    inline bool IsLanlineReadyEligibleSlot(const LanlinePlayerEntry& entry) {
        return entry.role == "Host" || entry.role == "Client" || entry.role == "Local Operator";
    }

    inline const char* LanlineSlotStateLabel(const LanlinePlayerEntry& entry) {
        if (IsLanlineAwaitingSlot(entry)) {
            return "Open";
        }
        if (IsLanlinePendingSlot(entry)) {
            return "Pending";
        }
        if (IsLanlineReservedSlot(entry)) {
            return "Reserved";
        }
        if (IsLanlineAcceptedSlot(entry)) {
            return "Accepted";
        }
        return entry.online ? "Active" : "Reserved";
    }

    inline const char* LanlineReadyLabel(const LanlinePlayerEntry& entry) {
        if (!IsLanlineReadyEligibleSlot(entry)) {
            return "-";
        }
        return entry.ready ? "Ready" : "Not Ready";
    }

    inline int FindFirstAwaitingSlotIndex(const LanlineSessionState& state) {
        for (int index = 0; index < static_cast<int>(state.players.size()); ++index) {
            if (IsLanlineAwaitingSlot(state.players[static_cast<std::size_t>(index)])) {
                return index;
            }
        }
        return -1;
    }

    inline int MaxLanlineSessionSlots(const LanlineSessionState& session) {
        if (session.mode != "LAN Host") {
            return 0;
        }
        return std::max(1, static_cast<int>(session.players.size()));
    }

    inline int OccupiedLanlineSessionSlots(const LanlineSessionState& session) {
        int occupied = 0;
        for (const auto& player : session.players) {
            if (session.mode == "LAN Host" && IsLanlineAwaitingSlot(player)) {
                continue;
            }
            if (player.role != "Awaiting") {
                ++occupied;
            }
        }
        return occupied;
    }

    inline int AvailableLanlineSessionSlots(const LanlineSessionState& session) {
        return std::max(0, MaxLanlineSessionSlots(session) - OccupiedLanlineSessionSlots(session));
    }

    inline int PendingLanlineSessionSlots(const LanlineSessionState& session) {
        int count = 0;
        for (const auto& player : session.players) {
            if (IsLanlinePendingSlot(player)) {
                ++count;
            }
        }
        return count;
    }

    inline int ReservedLanlineSessionSlots(const LanlineSessionState& session) {
        int count = 0;
        for (const auto& player : session.players) {
            if (IsLanlineReservedSlot(player)) {
                ++count;
            }
        }
        return count;
    }

    inline int AcceptedLanlineSessionSlots(const LanlineSessionState& session) {
        int count = 0;
        for (const auto& player : session.players) {
            if (IsLanlineAcceptedSlot(player)) {
                ++count;
            }
        }
        return count;
    }

    inline int ReadyLanlineSessionSlots(const LanlineSessionState& session) {
        int count = 0;
        for (const auto& player : session.players) {
            if (IsLanlineReadyEligibleSlot(player) && player.ready) {
                ++count;
            }
        }
        return count;
    }

    inline bool IsJoinableLanlineSession(const LanlineSessionState& session) {
        if (session.mode != "LAN Host") {
            return false;
        }
        if (!session.connectedPeer.empty()) {
            return false;
        }
        return session.lifecycleStage == "HostLobbyOpen" ||
            session.lifecycleStage == "HostJoinPending" ||
            session.lifecycleStage == "HostRuntimeActive";
    }

    inline bool IsLanlineMatchStartReady(const LanlineSessionState& session) {
        if (session.mode != "LAN Host") {
            return false;
        }
        if (ReservedLanlineSessionSlots(session) > 0 || PendingLanlineSessionSlots(session) > 0) {
            return false;
        }

        int readyEligible = 0;
        for (const auto& player : session.players) {
            if (!IsLanlineReadyEligibleSlot(player)) {
                continue;
            }
            ++readyEligible;
            if (!player.ready) {
                return false;
            }
        }
        return readyEligible >= 2;
    }

    }  // namespace bunker
