#pragma once

#include <cpr/cpr.h>
#include <json/json.h>
#include <ixwebsocket/IXWebSocket.h>
#include <openssl/sha.h>

#include <atomic>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>


class ChatClient {
public:
    ChatClient(const std::string& ip = "127.0.0.1", uint16_t port = 8080);

    bool RegisterUser(const std::string& login, const std::string& password);
    bool LoginUser(const std::string& login, const std::string& password);
    bool LogoutUser();
    bool GetOnlineUsers();
    bool SendMessage(const std::string& text, const std::string& to = "");
    bool IsLoggedIn() const;
    std::string GetToken() const;
    std::string GetBaseUrl() const;

    void RunWebSocket();
    void StopWebSocket();

private:
    std::string token_;
    std::string login_;
    std::string base_url_;
    std::string ws_url_;

    std::unique_ptr<ix::WebSocket> ws_client_;
    std::thread ws_thread_;
    std::atomic<bool> stop_ws_{false};
};
