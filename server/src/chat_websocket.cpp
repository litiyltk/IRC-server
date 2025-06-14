#include "chat_websocket.h"


// описание ?
std::mutex ChatWebSocket::conn_mutex_;
std::unordered_map<std::string, drogon::WebSocketConnectionPtr> ChatWebSocket::connections_;

void ChatWebSocket::handleNewConnection(const drogon::HttpRequestPtr &req, const drogon::WebSocketConnectionPtr &conn) {
    const auto &query = req->getParameters();
    auto it = query.find("token");
    if (it == query.end()) {
        conn->shutdown();
        std::cout << "[ChatWS] missing token\n";
        return;
    }

    const std::string &token = it->second;
    std::string user;

    if (!TokenStorage::instance().HasUserByToken(token, user)) {
        conn->shutdown();
        std::cout << "[ChatWS] invalid token\n";
        return;
    }

    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        connections_[user] = conn;
    }

    RoomManager::instance().JoinRoom(user, GENERAL_ROOM); // при логине пользователь появляется в общей комнате

    std::cout << "[ChatWS] connected: " << user << "\n";
    std::cout << "[ChatWS] client in room: " << RoomManager::instance().GetRoomOfUser(user) << "\n";
}

void ChatWebSocket::handleNewMessage(const drogon::WebSocketConnectionPtr &ws_conn, std::string &&message, const drogon::WebSocketMessageType &) {
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(message, root)) {
        return;
    }

    const std::string text = root.get("text", "").asString();
    const std::string to = root.get("to", "").asString();

    std::string from;
    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        for (const auto &[user, conn] : connections_) {
            if (conn == ws_conn) {
                from = user;
                break;
            }
        }
    }
    if (from.empty()) {
        return;
    }

    Json::Value response;
    response["from"] = from;
    response["text"] = text;

    std::string serialized = Json::FastWriter().write(response);

    std::cout << "[ChatWS] message received: " << text << " to " << (to.empty() ? "all" : to) << "\n";

    Broadcast(from, to, serialized);
}

void ChatWebSocket::handleConnectionClosed(const drogon::WebSocketConnectionPtr &conn) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    for (auto it = connections_.begin(); it != connections_.end(); ++it) {
        if (it->second == conn) {
            std::cout << "[ChatWS] client disconnected: " << it->first << "\n";
            RoomManager::instance().LeaveRoom(it->first);
            connections_.erase(it);
            break;
        }
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

void ChatWebSocket::Broadcast(const std::string &from, const std::string &to, const std::string &message) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    const std::string fromRoom = RoomManager::instance().GetRoomOfUser(from);

    // по умолчанию - сообщение рассылается всем клиентам в комнате
    if (to.empty()) {
        std::cout << "[ChatWS] broadcast to room: " << fromRoom << "\n";
        for (const auto &[user, conn] : connections_) {
            if (RoomManager::instance().GetRoomOfUser(user) == fromRoom) {
                conn->send(message);
            }
        }
    // опция - ищем конкретного клиента в комнате, для персонального сообщения
    } else {
        auto it = connections_.find(to);
        if (it != connections_.end()) {
            const std::string toRoom = RoomManager::instance().GetRoomOfUser(to);
            if (toRoom == fromRoom) {
                std::cout << "[ChatWS] send message to: " << to << "\n";
                it->second->send(message);
            } else {
                std::cout << "[ChatWS] unnable send to " << to << ": different room\n";
                auto senderIt = connections_.find(from);
                if (senderIt != connections_.end()) {
                    Json::Value warn;
                    warn["from"] = "system";
                    warn["text"] = "unnable send message to " + to + " different room\n";
                    senderIt->second->send(Json::FastWriter().write(warn));
                }
            }
        } else {
            std::cout << "[ChatWS] client not connected: " << to << "\n";
        }
    }
}
