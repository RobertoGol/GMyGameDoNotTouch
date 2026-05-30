#pragma once
#include <string>
#include <vector>
#include <deque>
#include "Registry_ID.hpp"

/**
 * PROJECT: Bunker Protocol
 * MODULE: Game_Chat
 * DESCRIPTION: Логика чата с разделением на каналы (стиль Crossout/Log Horizon).
 */

namespace BunkerProtocol {

    enum class ChatChannel {
        Global,     // Общий мир
        System,     // Логи системы и безопасности
        Private,    // Личка
        Group,      // Отряд / Группа
        Trade       // Торговля
    };

    struct ChatMessage {
        std::string senderID;   // Формат #XXXXXXX или @XXXXXXX
        std::string senderName;
        std::string text;
        ChatChannel channel;
        std::string timestamp;
    };

    class GameChat {
    private:
        // Используем deque для эффективного удаления старых сообщений (FIFO)
        std::deque<ChatMessage> messageHistory;
        const size_t maxHistory = 100; // Лимит для оптимизации под слабые ПК

    public:
        GameChat() = default;

        /**
         * Добавление нового сообщения в чат.
         * Вызывается как от игрока, так и от ядра системы (System Log).
         */
        void PushMessage(const std::string& senderID, const std::string& name, 
                         const std::string& msg, ChatChannel channel) 
        {
            // Валидация через наш RegistryID
            if (!RegistryID::IsValid(senderID) && channel != ChatChannel::System) {
                return; // Игнорируем подозрительные сообщения
            }

            ChatMessage newMsg;
            newMsg.senderID = senderID;
            newMsg.senderName = name;
            newMsg.text = msg;
            newMsg.channel = channel;
            newMsg.timestamp = "12:00"; // Здесь будет вызов из OS_Isolation::GetSystemTimeSync()

            messageHistory.push_back(newMsg);

            // Очистка старых сообщений
            if (messageHistory.size() > maxHistory) {
                messageHistory.pop_front();
            }
        }

        /**
         * Получение сообщений для конкретной вкладки UI (например, только System)
         */
        std::vector<ChatMessage> GetMessagesByChannel(ChatChannel filter) const {
            std::vector<ChatMessage> filtered;
            for (const auto& msg : messageHistory) {
                if (msg.channel == filter) {
                    filtered.push_back(msg);
                }
            }
            return filtered;
        }

        /**
         * Быстрый системный лог (упрощенный вызов для ядра)
         */
        void LogSystem(const std::string& text) {
            PushMessage("#SYSTEM", "SYS", text, ChatChannel::System);
        }

        size_t GetTotalMessages() const { return messageHistory.size(); }
    };
}
