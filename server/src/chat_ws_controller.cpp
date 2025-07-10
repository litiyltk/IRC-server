#include "chat_ws_controller.h"


std::mutex ChatWebSocket::conn_mutex_;
std::unordered_map<std::string, drogon::WebSocketConnectionPtr> ChatWebSocket::connections_;
std::unordered_map<drogon::WebSocketConnectionPtr, std::string> ChatWebSocket::conn_to_user_;

void ChatWebSocket::handleNewConnection(const drogon::HttpRequestPtr &req, const drogon::WebSocketConnectionPtr &conn) {
    const auto &query = req->getParameters();
    auto it = query.find("token");
    if (it == query.end()) {
        conn->send("Missing token");
        conn->shutdown();
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Missing token");
        return;
    }

    const std::string &token = it->second;
    auto chat_service = ChatServicePlugin::GetService();
    auto user_opt = chat_service->GetUserByToken(token);
    if (!user_opt) {
        conn->send("Invalid token");
        conn->shutdown();
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Invalid token");
        return;
    }

    const std::string &user = user_opt->username;

    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        connections_[user] = conn;
        conn_to_user_[conn] = user;
    }

    drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("User " + user + " connected");

    auto room_opt = chat_service->GetCurrentRoomName(token);
    if (room_opt) {
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("User " + user + " in room " + *room_opt);
    }
}

void ChatWebSocket::handleNewMessage(const drogon::WebSocketConnectionPtr &ws_conn, std::string &&message, const drogon::WebSocketMessageType &type) {
    if (type != drogon::WebSocketMessageType::Text) {
        return;
    }

    std::string username;
    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        if (!conn_to_user_.contains(ws_conn)) {
            ws_conn->shutdown();
            return;
        }
        username = conn_to_user_[ws_conn];
    }

    auto chat_service = ChatServicePlugin::GetService();
    auto token_opt = chat_service->GetTokenByUserName(username);
    if (!token_opt || !chat_service->GetUserByToken(token_opt.value())) {
        ws_conn->shutdown();
        return;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(message, root) || !root.isObject()) {
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Failed to parse JSON message");
        return;
    }

    const std::string text = root.get("text", "").asString();
    if (text.empty()) {
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Empty message");
        return;
    }

    Json::Value response;
    response["from"] = username;
    response["text"] = text;

    std::string serialized = Json::FastWriter().write(response);

    drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("From " + username + ": " + text);
    Broadcast(token_opt.value(), serialized);
}

void ChatWebSocket::handleConnectionClosed(const drogon::WebSocketConnectionPtr &conn) {
    std::string username;
    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        auto it = conn_to_user_.find(conn);
        if (it != conn_to_user_.end()) {
            username = it->second;
            conn_to_user_.erase(it);
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
    users.reserve(connections_.size());
    for (const auto& [login, _] : connections_) {
        users.push_back(login);
    }
    return users;
}

void ChatWebSocket::Broadcast(const std::string& token, const std::string& message) {
    auto chat_service = ChatServicePlugin::GetService();
    auto room_opt = chat_service->GetCurrentRoomName(token);
    if (!room_opt) {
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Broadcast failed, user not in any room");
        return;
    }

    drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("Broadcast in room " + *room_opt);

    auto room_user_names_vec = chat_service->GetUserNamesInRoom(room_opt.value());
    std::unordered_set<std::string> room_user_names(room_user_names_vec.begin(), room_user_names_vec.end());

    std::vector<drogon::WebSocketConnectionPtr> conns_to_send;
    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        for (const auto& [user, conn] : connections_) {
            if (room_user_names.contains(user)) {
                conns_to_send.push_back(conn);
            }
        }
    }

    for (const auto& conn : conns_to_send) {
        conn->send(message);
    }
}

void ChatWebSocket::ForceDisconnectByUsername(const std::string& username) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    auto it = connections_.find(username);
    if (it != connections_.end() && it->second) {
        it->second->shutdown();
        conn_to_user_.erase(it->second);
        connections_.erase(it);
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("User " + username + " force disconnected due to token expiration");
    }
}

void ChatWebSocket::DisconnectInvalidUsers() {
    std::vector<std::string> to_disconnect;
    auto chat_service = ChatServicePlugin::GetService();

    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        for (const auto& [username, _] : connections_) {
            auto token_opt = chat_service->GetTokenByUserName(username);
            if (!token_opt || !chat_service->GetUserByToken(token_opt.value())) {
                to_disconnect.push_back(username);
            }
        }
    }

    for (const auto& user : to_disconnect) {
        ForceDisconnectByUsername(user);
        drogon::app().getPlugin<LoggerPlugin>()->LogWebSocketEvent("ForceDisconnect by timeout: " + user);
    }
}