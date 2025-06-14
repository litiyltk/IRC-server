#include "auth_controller.h"


void AuthController::RegisterUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    if (!json || !json->isMember("login") || !json->isMember("password")) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("Missing login or password"));
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    const std::string login = (*json)["login"].asString();
    const std::string password = (*json)["password"].asString();

    if (!UserManager::instance().RegisterUser(login, password)) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("User already exists"));
        resp->setStatusCode(drogon::k409Conflict);
        callback(resp);
        return;
    }

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k200OK);
    resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    resp->setBody("Registration successful");
    callback(resp);
}

void AuthController::LoginUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    if (!json || !json->isMember("login") || !json->isMember("password")) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("Missing login or password"));
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    const std::string login = (*json)["login"].asString();
    const std::string password = (*json)["password"].asString();

    if (!UserManager::instance().HasUser(login, password)) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("Invalid login or password"));
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    std::string empty_password = "";
    if (TokenStorage::instance().HasTokenByUser(login, empty_password)) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("User already logged in"));
        resp->setStatusCode(drogon::k403Forbidden);
        callback(resp);
        return;
    }

    const std::string token = Token::GENERATOR.GenerateHEXToken();
    TokenStorage::instance().SaveToken(login, token);

    Json::Value result;
    result["token"] = token;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}

void AuthController::LogoutUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto authHeader = req->getHeader("Authorization");
    std::string token;
    if (authHeader.find("Bearer ") == 0) {
        token = authHeader.substr(7);
    } else {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Missing token");
        callback(resp);
        return;
    }

    std::string user;
    if (!TokenStorage::instance().HasUserByToken(token, user)) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Invalid token");
        callback(resp);
        return;
    }

    TokenStorage::instance().RemoveToken(token); // удаляем токен из user_to_token_, token_to_user_
    RoomManager::instance().RemoveUser(user); // удаляем пользователя из room_to_users_, user_to_room_

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k200OK);
    resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    resp->setBody("Logged out");
    callback(resp);
}
