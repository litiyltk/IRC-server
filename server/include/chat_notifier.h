#pragma once

#include "chat_ws_controller.h"


class ChatNotifier {
public:
    static void Broadcast(const std::string& token, const std::string& message) {
        ChatWebSocket::Broadcast(token, message);
    }

    static std::vector<std::string> GetConnectedUsers() {
        return ChatWebSocket::GetConnectedUsers();
    }
};
