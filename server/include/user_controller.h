#pragma once

#include "token_storage.h"
#include "chat_websocket.h"

#include <drogon/HttpController.h>
#include <json/json.h>


// REST API для работы с пользователями: список всех клиентов

class UserController : public drogon::HttpController<UserController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(UserController::getOnlineUsers, "/api/users/online", drogon::Get); // TODO вынести в константы
    METHOD_LIST_END

    void getOnlineUsers(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
