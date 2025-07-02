/**
 * @file user_controller.h
 * @brief Заголовочный файл для класса UserController, реализующего REST API для работы с пользователями.
 * @details Определяет класс UserController, который предоставляет методы для получения
 *  списка онлайн-пользователей, используя фреймворк Drogon.
 */
#pragma once

#include "const_api.h"
#include "http_utils.h"

#include "chat_service_plugin.h"
#include "chat_websocket.h"

#include <drogon/HttpController.h>
#include <json/json.h>


// REST API для работы с пользователями: список всех клиентов
/**
 * @class UserController
 * @brief Контроллер для обработки запросов, связанных с пользователями.
 * @details Реализует REST API для получения списка онлайн-пользователей,
 *  используя фреймворк Drogon.
 */
class UserController : public drogon::HttpController<UserController> {
public:
      /**
     * @brief Макрос для определения списка HTTP-методов, обрабатываемых контроллером.
     */
    METHOD_LIST_BEGIN
         /**
         * @brief Регистрирует метод getOnlineUsers для обработки GET-запросов на маршруте USERS_ONLINE.
         */
        ADD_METHOD_TO(UserController::getOnlineUsers,  std::string(api::USERS_ONLINE), drogon::Get);
    METHOD_LIST_END

    /**
     * @brief Обрабатывает запрос на получение списка онлайн-пользователей.
     * @details Аутентифицирует пользователя по токену, извлекает список онлайн-пользователей
     *  из ChatWebSocket и отправляет его в ответе.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void getOnlineUsers(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
