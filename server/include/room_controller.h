#pragma once

#include "const_api.h"
#include "room_manager.h"
#include "token_storage.h"

#include <drogon/HttpController.h>
#include <json/json.h>


// REST API для работы с комнатами: создание, переход в комнату, выход в общую комнату, список комнат, текушая комната, список клиентов в комнате

class RoomController : public drogon::HttpController<RoomController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(RoomController::CreateRoom, std::string(api::ROOM_CREATE), drogon::Post); // TODO вынести в константы
        ADD_METHOD_TO(RoomController::JoinRoom, std::string(api::ROOM_JOIN), drogon::Post);
        ADD_METHOD_TO(RoomController::LeaveRoom, std::string(api::ROOM_LEAVE), drogon::Post);
        ADD_METHOD_TO(RoomController::ListRooms, std::string(api::ROOM_LIST), drogon::Get);
        ADD_METHOD_TO(RoomController::CurrentRoom,std::string(api::ROOM_CURRENT), drogon::Get);
        ADD_METHOD_TO(RoomController::ListUsersInRoom, std::string(api::ROOM_USERS), drogon::Get);
    METHOD_LIST_END

    void CreateRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void JoinRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void LeaveRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void ListRooms(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void CurrentRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void ListUsersInRoom(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
