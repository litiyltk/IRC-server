#pragma once

#include "token_storage.h"
#include "chat_websocket.h"

#include <drogon/HttpController.h>
#include <json/json.h>


// REST API для отправки сообщения

class MessageController : public drogon::HttpController<MessageController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(MessageController::SendMessage, "/api/messages", drogon::Post); // TODO вынести в константы
    METHOD_LIST_END

    void SendMessage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
