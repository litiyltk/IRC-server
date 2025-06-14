#pragma once

#include "token_storage.h"
#include "room_manager.h"

#include <drogon/WebSocketController.h>
#include <json/json.h>
#include <iostream>
#include <mutex>
#include <unordered_map>


// WebSocket'ы для рассылки сообщений

class ChatWebSocket : public drogon::WebSocketController<ChatWebSocket> {
public:
    // Обрабатывает входящее сообщение от клиента через WebSocket // drogon/WebSocketController.h
    void handleNewMessage(const drogon::WebSocketConnectionPtr &ws_conn, std::string &&message, const drogon::WebSocketMessageType &type) override;

    // Обрабатывает новое WebSocket-соединения по HTTP запросу // drogon/WebSocketController.h
    void handleNewConnection(const drogon::HttpRequestPtr &req, const drogon::WebSocketConnectionPtr &conn) override;

    // Обрабатывает закрытие WebSocket-соединения // drogon/WebSocketController.h
    void handleConnectionClosed(const drogon::WebSocketConnectionPtr &conn) override;


    // Возвращает всех клиентов, подключенных по WebSocket-соединениям
    static std::vector<std::string> GetConnectedUsers();

    // Рассылает сообщения: если to = "" - рассылка сообщения всем клиентам в комнате
    static void Broadcast(const std::string& from, const std::string& to, const std::string& message);

    WS_PATH_LIST_BEGIN
        WS_PATH_ADD("/ws/chat", drogon::Get);
    WS_PATH_LIST_END

private:
    static std::mutex conn_mutex_;
    static std::unordered_map<std::string, drogon::WebSocketConnectionPtr> connections_;
};