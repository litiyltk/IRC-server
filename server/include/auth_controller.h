#pragma once

#include "const_api.h"
#include "http_utils.h"
#include "user_manager.h"
#include "token_generator.h"
#include "token_storage.h"
#include "room_manager.h"

#include <drogon/HttpController.h>
#include <json/json.h>


// REST API для аутентификации: регистрация, вход, выход

class AuthController : public drogon::HttpController<AuthController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(AuthController::RegisterUser, std::string(api::AUTH_REGISTER), drogon::Post);
        ADD_METHOD_TO(AuthController::LoginUser, std::string(api::AUTH_LOGIN), drogon::Post);
        ADD_METHOD_TO(AuthController::LogoutUser, std::string(api::AUTH_LOGOUT), drogon::Post);
    METHOD_LIST_END

    void RegisterUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void LoginUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void LogoutUser(const drogon::HttpRequestPtr &req,std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};