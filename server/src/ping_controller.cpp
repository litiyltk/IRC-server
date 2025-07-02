#include "ping_controller.h"


void PingController::Ping(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    http_utils::RespondWithSuccess("Ping success", drogon::k200OK, std::move(callback));
}