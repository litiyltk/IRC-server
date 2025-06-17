#include "msg_controller.h"


void MessageController::SendMessage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto authHeader = req->getHeader("Authorization");
    std::string token;
    if (authHeader.find("Bearer ") == 0) {
        token = authHeader.substr(7);
    // Некорректный заголовок: токен нельзя извлечь, либо нет токена
    } else {
        http_utils::RespondWithError("Failed to extract token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    // Токен авторизации не найден
    std::string user = "";
    if (!TokenStorage::instance().HasUserByToken(token, user)) {
        http_utils::RespondWithError("Invalid token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    // нет нужных полей, невалидный json
    auto json = req->getJsonObject();
    if (!json || !json->isMember("text")) {
        http_utils::RespondWithError("Invalid JSON format", drogon::k400BadRequest, std::move(callback));
        return;
    }

    std::string from = user; // имя было найдено HasUserByToken
    std::string text = (*json)["text"].asString(); // парсим текст
    std::string to = json->get("to", "").asString(); // находим получателя (рассылка внутри команты отправителя)

    // пустое поле json
    if (text.empty()) {
        http_utils::RespondWithError("Empty message", drogon::k400BadRequest, std::move(callback));
        return;
    }

    // успех - выполняем рассылку
    Json::Value msg;
    msg["from"] = from;
    msg["text"] = text;
    std::string serialized = Json::FastWriter().write(msg); 

    ChatWebSocket::Broadcast(from, to, serialized); // рассылка через WebSocket

    http_utils::RespondWithSuccess("Message sent", drogon::k200OK, std::move(callback));
}
