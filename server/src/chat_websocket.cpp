#include "chat_websocket.h"


std::mutex ChatWebSocket::conn_mutex_;
std::unordered_map<std::string, drogon::WebSocketConnectionPtr> ChatWebSocket::connections_;

void ChatWebSocket::handleNewConnection(const drogon::HttpRequestPtr &req, const drogon::WebSocketConnectionPtr &conn) {
    const auto &query = req->getParameters();
    auto it = query.find("token");
    if (it == query.end()) {
        conn->shutdown();
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Missing token");
        return;
    }

    const std::string &token = it->second;
    std::string user;

    auto chat_service = ChatServicePlugin::GetService();
    auto user_opt = chat_service->GetUserByToken(token);
    if (!user_opt) {
        conn->shutdown();
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Invalid token");
        return;
    }

    user = user_opt->username;

    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        connections_[user] = conn;
    }

    drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("User " + user + " connected");

    auto room_opt = chat_service->GetCurrentRoomName(token);
    if (room_opt) {
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("User " + user + " in room " + *room_opt);
    }
}

void ChatWebSocket::handleNewMessage(const drogon::WebSocketConnectionPtr &ws_conn, std::string &&message, const drogon::WebSocketMessageType &type) {

    // Json обрабатывается, если сообщение текстовое, иначе игнорируем
    if (type != drogon::WebSocketMessageType::Text) {
        return;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(message, root)) {
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Failed to parse message");
        return;
    }

    const std::string text = root.get("text", "").asString();
    if (text.empty()) {
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Empty message");
        return;
    }

    std::string token;
    
    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        for (const auto &[user, conn] : connections_) {
            if (conn == ws_conn) {
                // найти токен по имени пользователя
                auto chat_service = ChatServicePlugin::GetService();
                auto token_opt = chat_service->GetTokenByUserName(user);
                if (token_opt) {
                    token = token_opt.value();
                }
                break;
            }
        }
    }

    if (token.empty()) {
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Empty token");
        return;
    }

    // сериализуем ответ
    auto chat_service = ChatServicePlugin::GetService();
    auto user_opt = chat_service->GetUserByToken(token);
    if (!user_opt) return;

    Json::Value response;
    response["from"] = user_opt->username;
    response["text"] = text;

    std::string serialized = Json::FastWriter().write(response);

    drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("From " + user_opt->username + ": " + text);
    Broadcast(token, serialized);
}

void ChatWebSocket::handleConnectionClosed(const drogon::WebSocketConnectionPtr &conn) {
    std::string username;

    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        for (const auto& [user, connection] : connections_) {
            if (connection == conn) {
                username = user;
                break;
            }
        }

        if (!username.empty()) {
            connections_.erase(username);
        }
    }

    if (username.empty()) {
        return;
    }

    drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("User " + username + " disconnected");

    auto chat_service = ChatServicePlugin::GetService();
    auto token_opt = chat_service->GetTokenByUserName(username);

    if (!token_opt) {
        return;
    }

    if (chat_service->Logout(token_opt.value())) {
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("User " + username + " logged out after disconnect");
    } 
}

std::vector<std::string> ChatWebSocket::GetConnectedUsers() {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    std::vector<std::string> users;
    for (const auto& [login, _] : connections_) {
        users.push_back(login);
    }
    return users;
}

void ChatWebSocket::Broadcast(const std::string& token, const std::string& message) {
    std::lock_guard<std::mutex> lock(conn_mutex_);

    auto chat_service = ChatServicePlugin::GetService();
    auto room_opt = chat_service->GetCurrentRoomName(token);
    if (!room_opt) {
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Broadcast failed, user not in any room");
        return;
    }

    drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Broadcast in room " + *room_opt);

    auto room_user_names = chat_service->GetUserNamesInRoom(room_opt.value());
    for (const auto &[user, conn] : connections_) {
        if (std::find(room_user_names.begin(), room_user_names.end(), user) != room_user_names.end()) {
            conn->send(message);
        }
    }
}