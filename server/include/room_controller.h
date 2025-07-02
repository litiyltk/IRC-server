/**
 * @file room_controller.h
 * @brief Заголовочный файл для класса RoomController, реализующего REST API для работы с комнатами.
 * @details Определяет класс RoomController, который предоставляет методы для создания,
 *  присоединения, выхода из комнат, получения списка комнат, текущей комнаты и списка
 *  пользователей в комнате, используя фреймворк Drogon.
 */
#pragma once

#include "const_api.h"
#include "http_utils.h"

#include "chat_service_plugin.h"

#include <drogon/HttpController.h>
#include <json/json.h>


    // REST API для работы с комнатами: создание, переход в комнату, выход в общую комнату, список комнат, текушая комната, список клиентов в комнате
    /**
     * @class RoomController
     * @brief Контроллер для обработки запросов, связанных с комнатами.
     * @details Реализует REST API для создания, присоединения, выхода из комнат,
     *  получения списка комнат, текущей комнаты и списка пользователей в комнате,
     *  используя фреймворк Drogon.
     */
    class RoomController : public drogon::HttpController<RoomController>
{
public:
    /**
     *  @brief Макрос для определения списка HTTP-методов, обрабатываемых контроллером.
     */
    METHOD_LIST_BEGIN
    /**
     * @brief Регистрирует метод CreateRoom для обработки POST-запросов на маршруте ROOM_CREATE.
     */
    ADD_METHOD_TO(RoomController::CreateRoom, std::string(api::ROOM_CREATE), drogon::Post);
    /**
     * @brief Регистрирует метод JoinRoom для обработки POST-запросов на маршруте ROOM_JOIN.
     */
    ADD_METHOD_TO(RoomController::JoinRoom, std::string(api::ROOM_JOIN), drogon::Post);
    /**
     * @brief Регистрирует метод LeaveRoom для обработки POST-запросов на маршруте ROOM_LEAVE.
     */
    ADD_METHOD_TO(RoomController::LeaveRoom, std::string(api::ROOM_LEAVE), drogon::Post);
    /**
     * @brief Регистрирует метод ListRooms для обработки GET-запросов на маршруте ROOM_LIST.
     */
    ADD_METHOD_TO(RoomController::ListRooms, std::string(api::ROOM_LIST), drogon::Get);
    /**
     * @brief Регистрирует метод CurrentRoom для обработки GET-запросов на маршруте ROOM_CURRENT.
     */
    ADD_METHOD_TO(RoomController::CurrentRoom, std::string(api::ROOM_CURRENT), drogon::Get);
    /**
     * @brief Регистрирует метод ListUsersInRoom для обработки GET-запросов на маршруте ROOM_USERS.
     */
    ADD_METHOD_TO(RoomController::ListUsersInRoom, std::string(api::ROOM_USERS), drogon::Get);
    METHOD_LIST_END

    /**
     * @brief Обрабатывает запрос на создание комнаты.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void CreateRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    /**
     * @brief Обрабатывает запрос на присоединение к комнате.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void JoinRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    /**
     * @brief Обрабатывает запрос на выход из комнаты.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void LeaveRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    /**
     * @brief Обрабатывает запрос на получение списка комнат.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void ListRooms(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    /**
     * @brief Обрабатывает запрос на получение текущей комнаты пользователя.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void CurrentRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    /**
     * @brief Обрабатывает запрос на получение списка пользователей в комнате.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param callback Функция обратного вызова для отправки HTTP-ответа.
     */
    void ListUsersInRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
