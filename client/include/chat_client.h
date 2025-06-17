#pragma once

#include "const_api.h"
#include "password_hasher.h"

#include <cpr/cpr.h>
#include <json/json.h>
#include <ixwebsocket/IXWebSocket.h>

#include <iostream>
#include <sstream>
#include <string>


class ChatClient {
public:
    ChatClient(const std::string& ip = "127.0.0.1",
               uint16_t port = 8080);

    // REST API для аутентификации: регистрация, логина, выхода
    bool RegisterUser(const std::string& login, const std::string& password);
    bool LoginUser(const std::string& login, const std::string& password);
    bool LogoutUser();

    // REST API список всех клиентов и рассылка
    bool GetOnlineUsers();
    bool SendMessage(const std::string& text,
                     const std::string& to = ""); // по умолчанию рассылка всем (Broadcast)

    // REST API для работы с комнатами
    bool CreateRoom(const std::string& name);
    bool JoinRoom(const std::string& name);
    bool LeaveRoom();
    bool ListRooms();
    bool GetCurrentRoom();
    bool GetUsersInRoom(const std::string& roomName);

private:
    std::string token_;
    std::string login_;
    std::string base_url_;
    std::string ws_url_;

    std::unique_ptr<ix::WebSocket> ws_client_;

    bool IsLoggedIn() const;
    bool ParseTokenFromJson(const std::string& jsonText);

    // WebSocket'ы
    void RunWebSocket();
    void StopWebSocket();

    // Вспомогательные HTTP методы
    cpr::Response SendPostRequest(const std::string& endpoint, const Json::Value& body, bool auth = true);
    cpr::Response SendGetRequest(const std::string& endpoint, bool auth = true);
};
