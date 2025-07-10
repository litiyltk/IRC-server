#include "room_controller.h"


void RoomController::CreateRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

    // нет нужных полей, невалидный json
    auto json = req->getJsonObject();
    if (!json || !json->isMember("name")) {
        http_utils::RespondWithError("Invalid JSON format", drogon::k400BadRequest, std::move(callback));
        return;
    }

    const std::string name = (*json)["name"].asString();

    // пустое поле json
    if (name.empty()) {
        http_utils::RespondWithError("Empty room name", drogon::k400BadRequest, std::move(callback));
        return;
    }

    // комната с таким именем уже есть
    if (!chat_service->CreateRoom(name)) {
        http_utils::RespondWithError("Room already created", drogon::k409Conflict, std::move(callback));
        return;
    }

    // успех
    http_utils::RespondWithSuccess("Room created", drogon::k201Created, std::move(callback));
}

void RoomController::JoinRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

    // нет нужных полей, невалидный json
    auto json = req->getJsonObject();
    if (!json || !json->isMember("name")) {
        http_utils::RespondWithError("Invalid JSON format", drogon::k400BadRequest, std::move(callback));
        return;
    }

    const std::string name = (*json)["name"].asString();

    // пустое имя комнаты
    if (name.empty()) {
        http_utils::RespondWithError("Empty room name", drogon::k400BadRequest, std::move(callback));
        return;
    }

    // нельзя перейти в несуществующую комнату
    if (!chat_service->JoinRoom(token, name)) {
        http_utils::RespondWithError("Invalid room join", drogon::k404NotFound, std::move(callback));
        return;
    }

    // успех
    http_utils::RespondWithSuccess(name, drogon::k200OK, std::move(callback)); // {"info" : room_name} или заменить на  {"room" : room_name} ?
}

// возврат в GENERAL_ROOM
void RoomController::LeaveRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

    // нельзя перейти в несуществующую комнату
    if (!chat_service->LeaveRoom(token)) {
        http_utils::RespondWithError("Room general not founded", drogon::k404NotFound, std::move(callback));
        return;
    }

    // успех
    http_utils::RespondWithSuccess("Left to general room", drogon::k200OK, std::move(callback));
}

void RoomController::ListRooms(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

    // успех - возвращаем список всех комнат
    http_utils::RespondWithStringArray("List of rooms", chat_service->GetRoomNames(), callback);
}

void RoomController::CurrentRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

    // успех - возвращаем текущую комнату пользователя
    Json::Value json;
    auto room_opt = chat_service->GetCurrentRoomName(token_opt.value());
    json["room"] = room_opt.value_or(""); // если комната не определена - пустая строка // TODO добавить ошибку 400 "User not in any room"

    http_utils::RespondWithJson(json, "Current room name extracted", drogon::k200OK, callback);
}

void RoomController::ListUsersInRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

    const std::string room = req->getParameter("name");

    // проверка параметров
    if (room.empty()) {
        http_utils::RespondWithError("Invalid parameters", drogon::k400BadRequest, std::move(callback));
        return;
    }

    // комната не существует
    
    if (!chat_service->HasRoom(room)) {
        http_utils::RespondWithError("Room not found", drogon::k404NotFound, std::move(callback));
        return;
    }

    // успех - список пользователей в указанной комнате
    const auto users = chat_service->GetUserNamesInRoom(room);
    http_utils::RespondWithStringArray("Users in room " + room + " ", users, callback);
}
