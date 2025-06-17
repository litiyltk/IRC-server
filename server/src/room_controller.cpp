#include "room_controller.h"


static std::string ExtractUserFromRequest(const drogon::HttpRequestPtr &req, drogon::HttpResponsePtr &resp) { // TODO вынести в отдельный файл
    /*
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
    std::string from = "";
    if (!TokenStorage::instance().HasUserByToken(token, from)) { // если отправитель будет найден, то from сохранит имя
        http_utils::RespondWithError("Invalid token", drogon::k401Unauthorized, std::move(callback));
        return;
    }
    */
    auto authHeader = req->getHeader("Authorization");
    if (authHeader.find("Bearer ") != 0) {
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Missing token");
        return "";
    }

    std::string token = authHeader.substr(7);
    std::string user;
    if (!TokenStorage::instance().HasUserByToken(token, user)) {
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Invalid token");
        return "";
    }
    return user;
}

void RoomController::CreateRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    auto resp = drogon::HttpResponse::newHttpResponse();
    std::string user = ExtractUserFromRequest(req, resp);
    if (user.empty()) return callback(resp);

    if (!json || !json->isMember("name")) {
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Missing room name");
        callback(resp);
        return;
    }

    std::string name = (*json)["name"].asString();
    if (!RoomManager::instance().CreateRoom(name)) {
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Invalid or duplicate room name");
        callback(resp);
        return;
    }

    resp->setStatusCode(drogon::k201Created);
    resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    resp->setBody("Room created");
    callback(resp);
}

void RoomController::JoinRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    auto resp = drogon::HttpResponse::newHttpResponse();
    std::string user = ExtractUserFromRequest(req, resp);
    if (user.empty()) return callback(resp);

    if (!json || !json->isMember("name")) {
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Missing room name");
        callback(resp);
        return;
    }

    std::string name = (*json)["name"].asString();
    if (!RoomManager::instance().JoinRoom(user, name)) {
        resp->setStatusCode(drogon::k404NotFound);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Room does not exist");
        callback(resp);
        return;
    }

    resp->setStatusCode(drogon::k200OK);
    resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    resp->setBody("Joined room");
    callback(resp);
}

void RoomController::LeaveRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    std::string user = ExtractUserFromRequest(req, resp);
    if (user.empty()) {
        return callback(resp);
    }

    RoomManager::instance().LeaveRoom(user); // TODO: можно вернуть false, если пользователь не в комнате

    resp->setStatusCode(drogon::k200OK); // Успешно, без тела
    callback(resp);
}

void RoomController::ListRooms(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    Json::Value result(Json::arrayValue);
    for (const auto &name : RoomManager::instance().ListRooms()) {
        result.append(name);
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}

void RoomController::CurrentRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    std::string user = ExtractUserFromRequest(req, resp);
    if (user.empty()) {
        return callback(resp);
    }

    Json::Value json;
    json["room"] = RoomManager::instance().GetRoomOfUser(user);

    auto jsonResp = drogon::HttpResponse::newHttpJsonResponse(json);
    jsonResp->setStatusCode(drogon::k200OK);
    callback(jsonResp);
}

void RoomController::ListUsersInRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto room = req->getParameter("name");

    if (!RoomManager::instance().HasRoom(room)) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k404NotFound);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Room not found");
        callback(resp);
        return;
    }

    Json::Value result(Json::arrayValue);
    for (const auto &user : RoomManager::instance().GetUsersInRoom(room)) {
        result.append(user);
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}
