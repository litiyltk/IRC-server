#include "msg_controller.h"


void MessageController::SendMessage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto authHeader = req->getHeader("Authorization");
    std::string token;
    if (authHeader.find("Bearer ") == 0) {
        token = authHeader.substr(7);
    } else {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Missing or invalid token");
        callback(resp);
        return;
    }

    std::string from;
    if (!TokenStorage::instance().HasUserByToken(token, from)) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        resp->setBody("Invalid token");
        callback(resp);
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !json->isMember("text")) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("Missing text"));
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string text = (*json)["text"].asString();
    std::string to = json->get("to", "").asString();

    Json::Value msg;
    msg["from"] = from;
    msg["text"] = text;
    std::string serialized = Json::FastWriter().write(msg);

    ChatWebSocket::Broadcast(from, to, serialized);

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k200OK);
    resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    resp->setBody("Message sent");
    callback(resp);
}
