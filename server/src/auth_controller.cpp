#include "auth_controller.h"


void AuthController::RegisterUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    // req.get()->version(); // TODO нужно добавить обработку version, keep_alive (или необязательно?)
    auto json = req->getJsonObject();

    // нет нужных полей, невалидный json
    if (!json || !json->isMember("login") || !json->isMember("password")) {
        http_utils::RespondWithError("Invalid JSON format", drogon::k400BadRequest, std::move(callback));
        return;
    }

    const std::string login = (*json)["login"].asString();
    const std::string password = (*json)["password"].asString();

    // пустые поля json
    if (login.empty() || password.empty()) {
        http_utils::RespondWithError("Empty login or password", drogon::k400BadRequest, std::move(callback));
        return;
    }

    // пользователь уже зарегистрирован
    auto chat_service = ChatServicePlugin::GetService();
    if (!chat_service->Register(login, password)) {
        http_utils::RespondWithError("User " + login + " already registered", drogon::k409Conflict, std::move(callback));
        return;
    }

    // успех регистрации //
    http_utils::RespondWithSuccess("Registration successful: " + login, drogon::k201Created, std::move(callback));
}

void AuthController::LoginUser(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();

    // нет нужных полей, невалидный json
    if (!json || !json->isMember("login") || !json->isMember("password")) {
        http_utils::RespondWithError("Invalid JSON format", drogon::k400BadRequest, std::move(callback));
        return;
    }

    const std::string login = (*json)["login"].asString();
    const std::string password = (*json)["password"].asString();

    // пустые поля json
    if (login.empty() || password.empty()) {
        http_utils::RespondWithError("Empty login or password", drogon::k400BadRequest, std::move(callback));
        return;
    }

    auto chat_service = ChatServicePlugin::GetService();
    auto token_opt = chat_service->Login(login, password); // выполняем вход и получаем токен
    if (!token_opt.has_value()) { // вход невозможен при отсутствии регистрации или если пользователь уже залогинен
        http_utils::RespondWithError("User is not registered or User already logged in", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    // успешный вход
    http_utils::RespondAuthSuccess(login, token_opt.value(), callback);
}

void AuthController::LogoutUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    // Токен авторизации не извлечен
    auto token_opt = http_utils::TryExtractToken(req);
    if (!token_opt) {
        http_utils::RespondWithError("Failed to extract token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    const std::string token = token_opt.value();

    // Токен авторизации не найден
    auto chat_service = ChatServicePlugin::GetService();
    if (!chat_service->Logout(token)) { 
        http_utils::RespondWithError("Invalid token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    http_utils::RespondWithSuccess("Logged out", drogon::k200OK, std::move(callback));
}
