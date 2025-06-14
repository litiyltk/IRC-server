#include "chat_client.h"


std::string HashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);

    std::ostringstream oss;
    for (unsigned char c : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    return oss.str();
}

ChatClient::ChatClient(const std::string& ip, uint16_t port) {
    base_url_ = "http://" + ip + ":" + std::to_string(port);
    ws_url_ = "ws://" + ip + ":" + std::to_string(port) + "/ws/chat";
}

bool ChatClient::RegisterUser(const std::string& login, const std::string& password) {
    Json::Value body;
    body["login"] = login;
    body["password"] = HashPassword(password); // хэш пароля вместо чистого пароля!!!
    Json::StreamWriterBuilder writer;
    std::string json_str = Json::writeString(writer, body);

    auto res = cpr::Post(cpr::Url{base_url_ + "/api/auth/register"},
                         cpr::Header{{"Content-Type", "application/json"}},
                         cpr::Body{json_str});
    std::cout << res.text << "\n";
    return res.status_code == 200;
}

bool ChatClient::LoginUser(const std::string& login, const std::string& password) {
    if (IsLoggedIn()) {
        std::cout << "Already logged in: '" << login_ << "\n";
        return false;
    }

    Json::Value body;
    body["login"] = login;
    body["password"] = HashPassword(password); // хэш пароля вместо чистого пароля!!!
    Json::StreamWriterBuilder writer;
    std::string json_str = Json::writeString(writer, body);

    auto res = cpr::Post(cpr::Url{base_url_ + "/api/auth/login"},
                         cpr::Header{{"Content-Type", "application/json"}},
                         cpr::Body{json_str});
    if (res.status_code == 200) {
        Json::CharReaderBuilder reader;
        Json::Value root;
        std::string errs;
        std::istringstream s(res.text);
        if (Json::parseFromStream(reader, s, &root, &errs)) {
            if (root.isObject() && root.isMember("token")) {
                token_ = root["token"].asString();
                login_ = login;
                std::cout << "Token: " << token_ << "\n";
                RunWebSocket();
                return true;
            }
        }
    }

    token_.clear();
    std::cout << "Login failed: " << res.text << "\n";
    return false;
}

bool ChatClient::LogoutUser() {
    if (!IsLoggedIn()) {
        std::cout << "You are not logged in\n";
        return false;
    }

    auto res = cpr::Post(cpr::Url{base_url_ + "/api/auth/logout"},
                         cpr::Header{{"Authorization", "Bearer " + token_}});
    std::cout << res.text << "\n";

    token_.clear();
    StopWebSocket();
    std::cout << "Logged out successfully.\n";
    return res.status_code == 200;
}

bool ChatClient::GetOnlineUsers() {
    if (!IsLoggedIn()) {
        std::cout << "You are not logged in\n";
        return false;
    }

    auto res = cpr::Get(cpr::Url{base_url_ + "/api/users/online"},
                        cpr::Header{{"Authorization", "Bearer " + token_}});
    std::cout << res.text << "\n";
    return res.status_code == 200;
}

bool ChatClient::SendMessage(const std::string& text, const std::string& to) {
    if (!IsLoggedIn()) {
        std::cout << "You are not logged in\n";
        return false;
    }

    Json::Value body;
    body["text"] = text;
    body["to"] = to;
    Json::StreamWriterBuilder writer;
    std::string json_str = Json::writeString(writer, body);

    auto res = cpr::Post(cpr::Url{base_url_ + "/api/messages"},
                         cpr::Header{{"Authorization", "Bearer " + token_},
                                     {"Content-Type", "application/json"}},
                         cpr::Body{json_str});
    std::cout << res.text << "\n";
    return res.status_code == 200;
}

bool ChatClient::IsLoggedIn() const {
    return !token_.empty();
}

std::string ChatClient::GetToken() const {
    return token_;
}

std::string ChatClient::GetBaseUrl() const {
    return base_url_;
}

void ChatClient::RunWebSocket() {
    ws_client_ = std::make_unique<ix::WebSocket>();
    std::string url = ws_url_ + "?token=" + token_;
    ws_client_->setUrl(url);

    std::cout << "WebSocket: connecting to " << url << "...\n";

    ws_client_->setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::cout << "Message: " << msg->str << "\n";
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "Connection opened\n";
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            std::cout << "Closed with code " << msg->closeInfo.code
                      << ", reason: " << msg->closeInfo.reason << "\n";
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            std::cout << "Error: " << msg->errorInfo.reason << "\n";
        }
    });

    ws_client_->start();

    ws_thread_ = std::thread([this]() {
        while (!stop_ws_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << "Stop WebSocket\n";
    });
}

void ChatClient::StopWebSocket() {
    stop_ws_ = true;

    if (ws_thread_.joinable()) {
        ws_thread_.join(); // ждём завершения
    }

    if (ws_client_) {
        ws_client_->stop(); // затем останавливаем WebSocket
        ws_client_.reset(); // после удаляем
    }
}
