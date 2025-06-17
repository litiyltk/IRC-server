#pragma once

#include "logger_plugin.h"

#include <drogon/HttpResponse.h>
#include <json/json.h>
#include <functional>
#include <string>
#include <vector>


namespace http_utils {

inline drogon::HttpResponsePtr MakeJsonResponse(const Json::Value& body, drogon::HttpStatusCode code) {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(code);
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    return resp;
}

inline void RespondWithSuccess(const std::string& message,
                               drogon::HttpStatusCode code,
                               const std::function<void(const drogon::HttpResponsePtr&)>& callback) {
    Json::Value body;
    body["info"] = message;
    drogon::app().getPlugin<LoggerPlugin>()->LogResponse(message); // TODO подумать, как вынести логирование
    callback(MakeJsonResponse(body, code));
}

inline void RespondWithError(const std::string& message,
                             drogon::HttpStatusCode code,
                             const std::function<void(const drogon::HttpResponsePtr&)>& callback) {
    Json::Value body;
    body["error"] = message;
    drogon::app().getPlugin<LoggerPlugin>()->LogResponse(message); // TODO подумать, как вынести логирование
    callback(MakeJsonResponse(body, code));
}

}  // namespace http_utils