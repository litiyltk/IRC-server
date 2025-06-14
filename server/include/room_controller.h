#pragma once

#include "room_manager.h"
#include "token_storage.h"

#include <drogon/HttpController.h>
#include <json/json.h>


// REST API для работы с комнатами: создание, переход в комнату, выход в общую комнату, список комнат, текушая комната, список клиентов в комнате

class RoomController : public drogon::HttpController<RoomController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(RoomController::CreateRoom, "/api/room/create", drogon::Post); // TODO вынести в константы
        ADD_METHOD_TO(RoomController::JoinRoom, "/api/room/join", drogon::Post);
        ADD_METHOD_TO(RoomController::LeaveRoom, "/api/room/leave", drogon::Post);
        ADD_METHOD_TO(RoomController::ListRooms, "/api/room/list", drogon::Get);
        ADD_METHOD_TO(RoomController::CurrentRoom, "/api/room/current", drogon::Get);
        ADD_METHOD_TO(RoomController::ListUsersInRoom, "/api/room/users", drogon::Get);
    METHOD_LIST_END

    void CreateRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void JoinRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void LeaveRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void ListRooms(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void CurrentRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void ListUsersInRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
