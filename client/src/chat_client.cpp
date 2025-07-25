#include "chat_client.h"


ChatClient::ChatClient(const std::string& ip, uint16_t port) {
    base_url_ = "http://" + ip + ":" + std::to_string(port);
    ws_url_ = "ws://" + ip + ":" + std::to_string(port) + "/ws/chat";
}

bool ChatClient::RegisterUser(const std::string& login, const std::string& password) {
    Json::Value body;
    body["login"] = login;
    body["password"] = PasswordHasher().HashPassword(password);
    auto res = SendPostRequest(std::string(api::AUTH_REGISTER), body, false);
    std::cout << res.text << "\n";
    return res.status_code == 201;
}

bool ChatClient::LoginUser(const std::string& login, const std::string& password) {
    if (IsLoggedIn()) {
        std::cout << "Already logged in " << login_ << "\n";
        return false;
    }

    Json::Value body;
    body["login"] = login;
    body["password"] = PasswordHasher().HashPassword(password);
    auto res = SendPostRequest(std::string(api::AUTH_LOGIN), body, false);

    if (res.status_code == 200 && ParseTokenFromJson(res.text)) {
        login_ = login;
        RunWebSocket();
        return true;
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
    auto res = SendPostRequest(std::string(api::AUTH_LOGOUT), Json::objectValue);
    token_.clear();
    StopWebSocket();
    std::cout << "Logged out successfully\n";
    return res.status_code == 200;
}

bool ChatClient::GetOnlineUsers() {
    auto res = SendGetRequest(std::string(api::USERS_ONLINE));

    if (res.status_code != 200) {
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(res.text, root) || !root.isArray()) {
        return false;
    }

    std::cout << "\n--- Пользователи онлайн ---\n";

    for (const auto& user : root) {
        std::cout << "- " << user.asString() << "\n";
    }

    std::cout << "----------------------------\n";

    return true;
}

bool ChatClient::SendMessage(const std::string& text) {
    Json::Value body;
    body["text"] = text;
    auto res = SendPostRequest(std::string(api::MESSAGE_SEND), body);
    std::cout << res.text << "\n";
    return res.status_code == 200;
}

bool ChatClient::UploadMessage(const std::string& text) {
    Json::Value body;
    body["text"] = text;
    auto res = SendPostRequest(std::string(api::MESSAGE_UPLOAD), body);
    std::cout << res.text << "\n";
    return res.status_code == 200;
}

bool ChatClient::GetRecentMessages(const std::string& room, int max_items) {
    auto res = SendGetRequest(std::string(api::MESSAGE_RECENT) + "?room=" + room + "&max_items=" + std::to_string(max_items));
    
    if (res.status_code != 200) {
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(res.text, root) || !root.isArray()) {
        return false;
    }

    std::cout << "\n--- Сообщения в комнате " << room << " ---\n";

    for (const auto& msg : root) {
        const std::string from = msg.get("from", "").asString();
        //const std::string sent_at = msg.get("sent_at", "").asString();
        const std::string text = msg.get("text", "").asString();

        std::cout << /*"[" << sent_at << "] " <<*/ from << ": " << text << "\n";
    }

    std::cout << "--------------------------------------------\n";

    return true;
}

bool ChatClient::CreateRoom(const std::string& name) {
    Json::Value body;
    body["name"] = name;
    auto res = SendPostRequest(std::string(api::ROOM_CREATE), body);
    std::cout << res.text << "\n";
    return res.status_code == 201;
}

bool ChatClient::JoinRoom(const std::string& name) {
    Json::Value body;
    body["name"] = name;
    auto res = SendPostRequest(std::string(api::ROOM_JOIN), body);
    std::cout << res.text << "\n";
    return res.status_code == 200;
}

bool ChatClient::LeaveRoom() {
    auto res = SendPostRequest(std::string(api::ROOM_LEAVE), Json::objectValue);
    std::cout << res.text << "\n";
    return res.status_code == 200;
}

bool ChatClient::ListRooms() {
    auto res = SendGetRequest(std::string(api::ROOM_LIST));

    if (res.status_code != 200) {
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(res.text, root) || !root.isArray()) {
        return false;
    }

    std::cout << "\n---- Комнаты ----\n";

    for (const auto& room : root) {
        std::cout << "- " << room.asString() << "\n";
    }

    std::cout << "----------------------\n";

    return true;
}

bool ChatClient::GetCurrentRoom() {
    auto res = SendGetRequest(std::string(api::ROOM_CURRENT));
    std::cout << res.text << "\n";
    return res.status_code == 200;
}

std::string ChatClient::GetCurrentRoomName() {
    auto res = SendGetRequest(std::string(api::ROOM_CURRENT));
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(res.text);
    if (Json::parseFromStream(reader, s, &root, &errs)) {
        if (root.isMember("room")) {
            return root["room"].asString();
        }
    }
    return "";
}

bool ChatClient::GetUsersInRoom(const std::string& name) {
    auto res = SendGetRequest(std::string(api::ROOM_USERS) + "?name=" + name);

    if (res.status_code != 200) {
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(res.text, root) || !root.isArray()) {
        return false;
    }

    std::cout << "\n--- Пользователи в комнате " << name << " ---\n";

    for (const auto& user : root) {
        std::cout << "- " << user.asString() << "\n";
    }

    std::cout << "--------------------------------------------\n";

    return true;
}

bool ChatClient::IsLoggedIn() const {
    return !token_.empty();
}

bool ChatClient::ParseTokenFromJson(const std::string& json_str) {
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(json_str);
    if (Json::parseFromStream(reader, s, &root, &errs)) {
        if (root.isMember("token")) {
            token_ = root["token"].asString();
            std::cout << "Token: " << token_ << "\n"; // для отладки
            return true;
        }
    }
    return false;
}

cpr::Response ChatClient::SendPostRequest(const std::string& endpoint, const Json::Value& body, bool auth) {
    Json::StreamWriterBuilder writer;
    cpr::Header headers = {{"Content-Type", "application/json"}};
    if (auth) headers["Authorization"] = "Bearer " + token_;
    return cpr::Post(cpr::Url{base_url_ + endpoint}, headers, cpr::Body{Json::writeString(writer, body)});
}

cpr::Response ChatClient::SendGetRequest(const std::string& endpoint, bool auth) {
    cpr::Header headers;
    if (auth) headers["Authorization"] = "Bearer " + token_;
    return cpr::Get(cpr::Url{base_url_ + endpoint}, headers);
}

void ChatClient::RunWebSocket() {
    ws_client_ = std::make_unique<ix::WebSocket>();
    std::string url = ws_url_ + "?token=" + token_;
    ws_client_->setUrl(url);

    std::cout << "WebSocket connected: " << url << "\n";

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
}

void ChatClient::StopWebSocket() {
    if (ws_client_) {
        ws_client_->stop();
        ws_client_.reset();
    }
}
