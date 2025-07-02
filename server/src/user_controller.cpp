#include "user_controller.h"


void UserController::getOnlineUsers(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto chat_service = ChatServicePlugin::GetService();

    // Токен авторизации не извлечен
    auto token_opt = http_utils::TryExtractToken(req);
    if (!token_opt) {
        http_utils::RespondWithError("Failed to extract token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    const std::string token = token_opt.value();

    // Токен авторизации не найден
    if (!chat_service->GetUserByToken(token)) {
        http_utils::RespondWithError("Invalid token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    // Успех - отдаем список всех клиентов WebSocket // TODO заменить или дополнить методом списка зарегистрированных клиентов
    // можно использовать ChatService::GetOnlineUserNames - список активных токенов
    http_utils::RespondWithStringArray("Online WebSocket users", ChatWebSocket::GetConnectedUsers(), callback);
}
