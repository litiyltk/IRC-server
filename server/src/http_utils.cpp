#include "http_utils.h"

#include <drogon/drogon.h>


namespace http_utils {

drogon::HttpResponsePtr MakeJsonResponse(const Json::Value& body, drogon::HttpStatusCode code) {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(code);
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    return resp;
}

void RespondWithSuccess(const std::string& message,
                        drogon::HttpStatusCode code,
                        const std::function<void(const drogon::HttpResponsePtr&)>& callback) {
    Json::Value body;
    body["info"] = message;
    drogon::app().getPlugin<LoggerPlugin>()->LogResponse(message);
    callback(MakeJsonResponse(body, code));
}

void RespondWithError(const std::string& message,
                      drogon::HttpStatusCode code,
                      const std::function<void(const drogon::HttpResponsePtr&)>& callback) {
    Json::Value body;
    body["error"] = message;
    drogon::app().getPlugin<LoggerPlugin>()->LogResponse(message);
    callback(MakeJsonResponse(body, code));
}

std::optional<std::string> TryExtractToken(const drogon::HttpRequestPtr& req) {
    const auto& authHeader = req->getHeader("Authorization");
    if (authHeader.find("Bearer ") == 0) {
        return authHeader.substr(7);
    }
    return std::nullopt;
}

void RespondAuthSuccess(const std::string& username,
                        const std::string& token,
                        const std::function<void(const drogon::HttpResponsePtr&)>& callback) {
    Json::Value body;
    body["user"] = username;
    body["token"] = token;
    drogon::app().getPlugin<LoggerPlugin>()->LogResponse("[Login] " + username);
    callback(MakeJsonResponse(body, drogon::k200OK));
}

void RespondWithStringArray(const std::string& message,
                            const std::vector<std::string>& values,
                            const std::function<void(const drogon::HttpResponsePtr&)>& callback) {
    Json::Value body(Json::arrayValue);
    for (const auto& v : values) {
        body.append(v);
    }
    drogon::app().getPlugin<LoggerPlugin>()->LogResponse("[Success] " + message);
    callback(MakeJsonResponse(body, drogon::k200OK));
}

void RespondWithJson(const Json::Value& body,
                     const std::string& message,
                     drogon::HttpStatusCode code,
                     const std::function<void(const drogon::HttpResponsePtr&)>& callback) {
    drogon::app().getPlugin<LoggerPlugin>()->LogResponse(message);
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(code);
    callback(resp);
}

}  // namespace http_utils
