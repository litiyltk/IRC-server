#pragma once

#include "user_manager.h"
#include "token_generator.h"
#include "token_storage.h"
#include "room_manager.h"

#include <drogon/HttpController.h>
#include <json/json.h>


// REST API для аутентификации: регистрация, логина, выхода

class AuthController : public drogon::HttpController<AuthController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(AuthController::RegisterUser, "/api/auth/register", drogon::Post); // TODO вынести в константы
        ADD_METHOD_TO(AuthController::LoginUser, "/api/auth/login", drogon::Post);
        ADD_METHOD_TO(AuthController::LogoutUser, "/api/auth/logout", drogon::Post);
    METHOD_LIST_END

    void RegisterUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void LoginUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void LogoutUser(const drogon::HttpRequestPtr &req,std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};