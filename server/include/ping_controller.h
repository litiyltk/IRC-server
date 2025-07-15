/**
 * @file ping_controller.h
 * @brief Заголовочный файл для класса PingController, реализующего REST API для проверки доступности сервера.
 * @details Определяет класс PingController, который предоставляет метод для проверки
 *  доступности сервера.
 */
#pragma once

#include "const_api.h"
#include "http_utils.h"

#include <drogon/HttpController.h>
#include <json/json.h>


/**
 * @class PingController
 * @brief Контроллер для обработки запросов Ping.
 * @details Реализует REST API для проверки доступности сервера.
 */
class PingController : public drogon::HttpController<PingController> { // TODO замена на healthcheck в docker-compose.yml
public:
    /**
     * @brief Макрос для определения списка HTTP-методов, обрабатываемых контроллером.
     */
    METHOD_LIST_BEGIN
        /**
         * @brief Регистрирует метод Ping для обработки GET-запросов на маршруте PING.
         */
        ADD_METHOD_TO(PingController::Ping, std::string(api::PING), drogon::Get);
    METHOD_LIST_END
    
     /**
     * @brief Обрабатывает запрос Ping и отправляет ответ об успехе.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void Ping(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
