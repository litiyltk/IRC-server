#include "msg_controller.h"


void MessageController::SendMessage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto chat_service = ChatServicePlugin::GetService();

    // Токен авторизации не извлечен
    auto token_opt = http_utils::TryExtractToken(req);
    if (!token_opt) {
        http_utils::RespondWithError("Failed to extract token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    const std::string token = token_opt.value();

    // Токен авторизации не найден
    auto user_opt = chat_service->GetUserByToken(token);
    if (!user_opt) {
        http_utils::RespondWithError("Invalid token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    // Обновляем активность пользователя
    chat_service->UpdateActivityByToken(token);

    // нет нужных полей, невалидный json
    auto json = req->getJsonObject();
    if (!json || !json->isMember("text")) {
        http_utils::RespondWithError("Invalid JSON format", drogon::k400BadRequest, std::move(callback));
        return;
    }

    std::string from = user_opt->username;
    std::string text = (*json)["text"].asString(); // парсим текст

    // пустое поле json
    if (text.empty()) {
        http_utils::RespondWithError("Empty message from", drogon::k400BadRequest, std::move(callback));
        return;
    }

    // успех - выполняем рассылку
    Json::Value msg;
    msg["from"] = from;
    msg["text"] = text;
    std::string serialized = Json::FastWriter().write(msg); 

    ChatNotifier::Broadcast(token, serialized); // рассылка через WebSocket в пределах комнаты

    http_utils::RespondWithSuccess("Message from " + from + " sent", drogon::k200OK, std::move(callback));
}

void MessageController::UploadMessage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto chat_service = ChatServicePlugin::GetService();

    // Токен авторизации не извлечен
    auto token_opt = http_utils::TryExtractToken(req);
    if (!token_opt) {
        http_utils::RespondWithError("Failed to extract token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    const std::string token = token_opt.value();

    // Токен авторизации не найден
    auto user_opt = chat_service->GetUserByToken(token);
    if (!user_opt) {
        http_utils::RespondWithError("Invalid token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    // Обновляем активность пользователя
    chat_service->UpdateActivityByToken(token);

    // нет нужных полей, невалидный json
    auto json = req->getJsonObject();
    if (!json || !json->isMember("text")) {
        http_utils::RespondWithError("Invalid JSON format", drogon::k400BadRequest, std::move(callback));
        return;
    }

    // пустое поле json
    std::string text = (*json)["text"].asString();
    if (text.empty()) {
        http_utils::RespondWithError("Empty message", drogon::k400BadRequest, std::move(callback));
        return;
    }

    // Ошибка при сохранении в БД
    if (!chat_service->SaveMessage(token, text)) { // TODO разнообразить ошибки сохранения
        http_utils::RespondWithError("Failed to save message to DB", drogon::k500InternalServerError, std::move(callback));
        return;
    }

    // Успех
    http_utils::RespondWithSuccess("Message saved to DB", drogon::k200OK, std::move(callback));
}

void MessageController::GetRecentMessages(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

    // Обновляем активность пользователя
    chat_service->UpdateActivityByToken(token);

    const std::string room = req->getParameter("room");
    const std::string max_items_str = req->getParameter("max_items");

    // Проверка параметров
    if (room.empty() || max_items_str.empty()) {
        http_utils::RespondWithError("Invalid parameters", drogon::k400BadRequest, std::move(callback));
        return;
    }

    int max_items = 0;
    auto [ptr, ec] = std::from_chars(max_items_str.data(), max_items_str.data() + max_items_str.size(), max_items);
    if (ec != std::errc()) {
        http_utils::RespondWithError("Invalid parameter: max_items", drogon::k400BadRequest, std::move(callback));
        return;
    }

    if (max_items <= 0) {
        http_utils::RespondWithError("Parameter must be positive: max_items", drogon::k400BadRequest, std::move(callback));
        return;
    }

    if (!chat_service->HasRoom(room)) {
        http_utils::RespondWithError("Room not found", drogon::k404NotFound, std::move(callback));
        return;
    }

    const auto messages = chat_service->GetRecentMessages(room, max_items);

    Json::Value result(Json::arrayValue);
    for (const auto& msg : messages) {
        Json::Value item;

        item["sent_at"] = msg.sent_at;
        item["text"] = msg.message;

        auto name_opt = chat_service->GetUserNameById(msg.user_id);
        item["from"] = name_opt.value_or(""); // если пользователь не найден - пустая строка // TODO добавить ошибку 400 "User not found"

        result.append(item);
    }

    http_utils::RespondWithJson(result, "Messages extracted from DB", drogon::k200OK, callback);
}

void MessageController::GetRoomMessagesPage(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
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

    // Обновляем активность пользователя
    chat_service->UpdateActivityByToken(token);

    const std::string room = req->getParameter("room");
    const std::string offset_str = req->getParameter("offset");
    const std::string limit_str = req->getParameter("limit");

    // Проверка параметров
    if (room.empty() || offset_str.empty() || limit_str.empty()) {
        http_utils::RespondWithError("Invalid parameters", drogon::k400BadRequest, std::move(callback));
        return;
    }

    int offset = 0;
    int limit = 0;
    auto [ptr1, ec1] = std::from_chars(offset_str.data(), offset_str.data() + offset_str.size(), offset);
    auto [ptr2, ec2] = std::from_chars(limit_str.data(), limit_str.data() + limit_str.size(), limit);

    if (ec1 != std::errc() || ec2 != std::errc()) {
        http_utils::RespondWithError("Invalid parameter: offset or limit", drogon::k400BadRequest, std::move(callback));
        return;
    }

    if (offset < 0 || limit <= 0) {
        http_utils::RespondWithError("Parameters must be: offset >= 0, limit > 0", drogon::k400BadRequest, std::move(callback));
        return;
    }

    if (!chat_service->HasRoom(room)) {
        http_utils::RespondWithError("Room not found", drogon::k404NotFound, std::move(callback));
        return;
    }

    const auto messages = chat_service->GetRoomMessagesPage(room, offset, limit);

    Json::Value result(Json::arrayValue);
    for (const auto& msg : messages) {
        Json::Value item;

        item["sent_at"] = msg.sent_at;
        item["text"] = msg.message;

        auto name_opt = chat_service->GetUserNameById(msg.user_id);
        item["from"] = name_opt.value_or(""); // если пользователь не найден - пустая строка // TODO добавить ошибку 400 "User not found"

        result.append(item);
    }

    http_utils::RespondWithJson(result, "Paginated messages extracted from DB", drogon::k200OK, callback);
}

