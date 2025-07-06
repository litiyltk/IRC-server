/**
 * @file msg_controller.h
 * @brief Заголовочный файл для класса MessageController, реализующего REST API для работы с сообщениями.
 * @details Определяет класс MessageController, который предоставляет методы для отправки,
 *  сохранения в БД и получения сообщений.
 */
#pragma once

#include "const_api.h"
#include "http_utils.h"

#include "chat_service_plugin.h"
#include "chat_notifier.h"

#include <drogon/HttpController.h>
#include <json/json.h>


/**
 * @class MessageController
 * @brief Контроллер для обработки запросов, связанных с сообщениями.
 * @details Реализует REST API для отправки, сохранения в БД и получения сообщений.
 *  Важно! При запросе на отправку SendMessage сохранение не выполняется,
 *  сохранение происходит по запросу UploadMessage.
 */
class MessageController : public drogon::HttpController<MessageController> {
public:
    /**
     * @brief Макрос для определения списка HTTP-методов, обрабатываемых контроллером.
     */
    METHOD_LIST_BEGIN
         /**
         * @brief Регистрирует метод SendMessage для обработки POST-запросов на маршруте MESSAGE_SEND.
         */
        ADD_METHOD_TO(MessageController::SendMessage, std::string(api::MESSAGE_SEND), drogon::Post);
         /**
         * @brief Регистрирует метод UploadMessage для обработки POST-запросов на маршруте MESSAGE_UPLOAD.
         */
        ADD_METHOD_TO(MessageController::UploadMessage, std::string(api::MESSAGE_UPLOAD), drogon::Post);
          /**
         * @brief Регистрирует метод GetRecentMessages для обработки GET-запросов на маршруте MESSAGE_RECENT.
         */
        ADD_METHOD_TO(MessageController::GetRecentMessages, std::string(api::MESSAGE_RECENT), drogon::Get);
         /**
         * @brief Регистрирует метод GetRoomMessagesPage для обработки GET-запросов на маршруте MESSAGE_PAGE.
         */
        ADD_METHOD_TO(MessageController::GetRoomMessagesPage, std::string(api::MESSAGE_PAGE), drogon::Get);
    METHOD_LIST_END

     /**
     * @brief Отправляет сообщение через WebSocket.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void SendMessage(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    /**
     * @brief Сохраняет сообщение в базу данных.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void UploadMessage(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    /**
     * @brief Получает последние сообщения из базы данных.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void GetRecentMessages(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    /**
     * @brief Получает постранично сообщения из базы данных.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void GetRoomMessagesPage(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
