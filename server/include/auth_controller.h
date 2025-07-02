/**
 * @file auth_controller.h
 * @brief Заголовочный файл для класса AuthController, реализующего REST API для аутентификации.
 * @details Определяет класс AuthController, который предоставляет методы для регистрации,
 *  входа и выхода пользователей, используя фреймворк Drogon.
 */

#pragma once

#include "const_api.h"
#include "http_utils.h"

#include "chat_service_plugin.h"

#include <drogon/HttpController.h>
#include <json/json.h>


// REST API для аутентификации: регистрация, вход, выход
/**
 * @class AuthController
 * @brief Контроллер для обработки запросов аутентификации.
 * @details Реализует REST API для регистрации, входа и выхода пользователей,
 *  используя фреймворк Drogon.
 */
class AuthController : public drogon::HttpController<AuthController>
{
public:
    /**
     * @brief Макрос для определения списка HTTP-методов, обрабатываемых контроллером.
     */
    METHOD_LIST_BEGIN
    /**
         * @brief Регистрирует метод RegisterUser для обработки POST-запросов на маршруте AUTH_REGISTER.
         */
    ADD_METHOD_TO(AuthController::RegisterUser, std::string(api::AUTH_REGISTER), drogon::Post);
    /**
         * @brief Регистрирует метод LoginUser для обработки POST-запросов на маршруте AUTH_LOGIN.
         */
    ADD_METHOD_TO(AuthController::LoginUser, std::string(api::AUTH_LOGIN), drogon::Post);
     /**
         * @brief Регистрирует метод LogoutUser для обработки POST-запросов на маршруте AUTH_LOGOUT.
         */
    ADD_METHOD_TO(AuthController::LogoutUser, std::string(api::AUTH_LOGOUT), drogon::Post);
    METHOD_LIST_END
    
      /**
     * @brief Обрабатывает запрос на регистрацию пользователя.
     * @details Извлекает логин и хэшированный пароль из тела запроса, регистрирует пользователя
     *  в системе и отправляет ответ об успехе или ошибке.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void RegisterUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    /**
     * @brief Обрабатывает запрос на вход пользователя в систему.
     * @details Извлекает логин и пароль из тела запроса, аутентифицирует пользователя
     *  и, в случае успеха, отправляет токен авторизации.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void LoginUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    /**
     * @brief Обрабатывает запрос на выход пользователя из системы.
     * @details Извлекает токен авторизации из заголовка запроса, деактивирует его
     *  и отправляет подтверждение выхода из системы.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void LogoutUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};