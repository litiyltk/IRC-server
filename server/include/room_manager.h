#pragma once

#include <algorithm>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>


// Логика работы с комнатами для room_controller.h (REST API) + потокобезопасное хранилище комнат и клиентов

namespace {
    constexpr const char* GENERAL_ROOM = "general"; // TODO вынести в константы
}

class RoomManager {
public:
    static RoomManager& instance();

    // Создание новой комнаты
    bool CreateRoom(const std::string& room);

    // Вход в комнату: удаляет из старой и добавляет в новую
    bool JoinRoom(const std::string& user, const std::string& room);

    // Выход из комнаты: перемещает в общую комнату
    bool LeaveRoom(const std::string& user);

    // Текущая комната пользователя
    std::string GetRoomOfUser(const std::string& user);

    // Наличие комнаты
    bool HasRoom(const std::string& room);

    // Список комнат
    std::vector<std::string> ListRooms();

    // Список пользователей в комнате
    std::vector<std::string> GetUsersInRoom(const std::string& room);

    // Удаление пользователя из комнаты
    void RemoveUser(const std::string& user);

private:
    RoomManager();

    std::unordered_map<std::string, std::unordered_set<std::string>> room_to_users_;
    std::unordered_map<std::string, std::string> user_to_room_;
    std::mutex mutex_;
    const std::string default_room = GENERAL_ROOM;
};
