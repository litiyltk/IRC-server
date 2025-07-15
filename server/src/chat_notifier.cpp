#include "chat_notifier.h"


void ChatNotifier::Broadcast(const std::string& token, const std::string& message) {
    ChatWebSocket::Broadcast(token, message);
}

std::vector<std::string> ChatNotifier::GetConnectedUsers() {
    return ChatWebSocket::GetConnectedUsers();
}

void ChatNotifier::DisconnectInvalidUsers() {
    ChatWebSocket::DisconnectInvalidUsers();
}
