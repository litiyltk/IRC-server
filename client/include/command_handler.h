#pragma once

#include "chat_client.h"

#include <iostream>
#include <cpr/cpr.h>
#include <json/json.h>
#include <string>
#include <unordered_map>


enum class Command {
    Register,
    Login,
    SendMessage,
    UploadMessage,
    GetRecentMessages,
    List,
    Logout,
    Exit,
    CreateRoom,
    JoinRoom,
    LeaveRoom,
    ListRooms,
    CurrentRoom,
    UsersInRoom,
    Unknown
};

Command ParseCommand(const std::string& input);
bool HandleCommand(Command cmd, ChatClient& client);
