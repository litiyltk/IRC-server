#include "auth_controller.h"


void AuthController::RegisterUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    // req.get()->version(); // version, keep_alive
    auto json = req->getJsonObject();
    if (!json || !json->isMember("login") || !json->isMember("password")) {
        Json::Value body;
        body["error"] = "Invalid JSON format";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        callback(resp);
        return;
    }

    const std::string login = (*json)["login"].asString();
    const std::string password = (*json)["password"].asString();

    if (login.empty() || password.empty()) {
        Json::Value body;
        body["error"] = "Fields 'login' and 'password' must be non-empty";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        callback(resp);
        return;
    }

    if (!UserManager::instance().RegisterUser(login, password)) {
        Json::Value body;
        body["error"] = "User already exists";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
        resp->setStatusCode(drogon::k409Conflict);
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        callback(resp);
        return;
    }

    Json::Value body;
    body["info"] = "Registration successful";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(drogon::k201Created); // 200 OK
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
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
    std::cout << "User: " << login << " has token:" << token << "\n"; // Для отладки в консоли
    std::cout << "User: " << login << " has password hash:" << password << "\n"; // Для отладки в консоли
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
