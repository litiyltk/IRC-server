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
    if (!UserManager::instance().RegisterUser(login, password)) {
        http_utils::RespondWithError("User already registred", drogon::k409Conflict, std::move(callback));
        return;
    }

    // успех регистрации // TODO исправить проблему, когда валидный запрос шлёт залогиненный клиент
    http_utils::RespondWithSuccess("Registration successful", drogon::k201Created, std::move(callback));
}

void AuthController::LoginUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

    // пользователь ранее не был зарегистрирован
    if (!UserManager::instance().HasUser(login, password)) {
        http_utils::RespondWithError("Invalid login or password", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    std::string empty_password = ""; // достаточно проверить по имени

    // пользователь не может войти, если не вышел из чата (токен не был анулирован)
    if (TokenStorage::instance().HasTokenByUser(login, empty_password)) {
        http_utils::RespondWithError("User already logged in", drogon::k403Forbidden, std::move(callback));
        return;
    }

    // успешный вход // TODO исправить проблему, когда валидный запрос шлёт залогиненный клиент
    const std::string token = Token::GENERATOR.GenerateHEXToken(); // 1. генерим новый токен

    std::cout << "User: " << login << " has token:" << token << "\n"; // Для отладки в консоли
    std::cout << "User: " << login << " has password hash:" << password << "\n"; // Для отладки в консоли

    TokenStorage::instance().SaveToken(login, token); // 2. сохраняем токен, старый токен удаляется

    Json::Value result;
    result["token"] = token;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
    // http_utils::RespondWithSuccess(token, drogon::k200OK, std::move(callback)); // TODO создать удобную обертку c логгированием
}

void AuthController::LogoutUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto authHeader = req->getHeader("Authorization");
    std::string token;
    if (authHeader.find("Bearer ") == 0) {
        token = authHeader.substr(7);
    // Некорректный заголовок: токен нельзя извлечь, либо нет токена
    } else {
        http_utils::RespondWithError("Failed to extract token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    // Токен авторизации не найден
    std::string user = "";
    if (!TokenStorage::instance().HasUserByToken(token, user)) {
        http_utils::RespondWithError("Invalid token", drogon::k401Unauthorized, std::move(callback));
        return;
    }

    // Успешный выход из чата (удаляем токен и пользователя из комнаты)
    TokenStorage::instance().RemoveToken(token); // удаляем токен из user_to_token_, token_to_user_

    http_utils::RespondWithSuccess("Logged out", drogon::k200OK, std::move(callback));
}
