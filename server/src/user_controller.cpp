#include "user_controller.h"


void UserController::getOnlineUsers(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

    // Успех - отдаем список всех клиентов WebSocket
    Json::Value result(Json::arrayValue);

    for (const auto& login : ChatWebSocket::GetConnectedUsers()) {
        result.append(login);
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
    // http_utils::RespondWithSuccess(?); // TODO создать удобную обертку c логгированием
}
