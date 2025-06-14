#include "user_controller.h"


void UserController::getOnlineUsers(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

    Json::Value result(Json::arrayValue);
    auto activeUsers = ChatWebSocket::GetConnectedUsers();

    for (const auto& login : activeUsers) {
        result.append(login);
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}
