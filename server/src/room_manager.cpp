#include "room_manager.h"


RoomManager& RoomManager::instance() {
    static RoomManager inst;
    return inst;
}

RoomManager::RoomManager() {
    room_to_users_[default_room]; // Общая комната по умолчанию
}

bool RoomManager::CreateRoom(const std::string& room) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (room_to_users_.count(room)) {
        return false;
    }
    room_to_users_[room];
    return true;
}

bool RoomManager::JoinRoom(const std::string& user, const std::string& room) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Нельзя перейти в несуществующую комнату
    if (!room_to_users_.count(room)) {
        return false;
    }

    std::string current_room = user_to_room_[user];

    // Нет смысла переходить в ту же самую комнату
    if (current_room == room) {
        return false;
    }

    // Удаляем из текущей комнаты
    if (!current_room.empty()) {
        room_to_users_[current_room].erase(user);
    }

    // Добавляем в новую комнату
    room_to_users_[room].insert(user);
    user_to_room_[user] = room;

    return true;
}

bool RoomManager::LeaveRoom(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string current_room = user_to_room_[user];

    // Нельзя выйти из общей комнаты
    if (current_room == default_room) {
        return false;
    }

    // Удаляем из текущей комнаты
    if (!current_room.empty()) {
        room_to_users_[current_room].erase(user);
    }

    // Перемещаем в общую комнату
    room_to_users_[default_room].insert(user);
    user_to_room_[user] = default_room;
    return true;
}

std::string RoomManager::GetRoomOfUser(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = user_to_room_.find(user);
    return (it != user_to_room_.end()) ? it->second : default_room;
}

bool RoomManager::HasRoom(const std::string& room) {
    std::lock_guard<std::mutex> lock(mutex_);
    return room_to_users_.find(room) != room_to_users_.end();
}

std::vector<std::string> RoomManager::ListRooms() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    for (const auto& [room, _] : room_to_users_) {
        names.push_back(room);
    }
    return names;
}

std::vector<std::string> RoomManager::GetUsersInRoom(const std::string& room) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> users;
    if (room_to_users_.count(room)) {
        for (const auto& user : room_to_users_[room]) {
            users.push_back(user);
        }
    }
    return users;
}

void RoomManager::RemoveUser(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = user_to_room_.find(user);
    if (it != user_to_room_.end()) {
        const std::string& room = it->second;
        auto room_it = room_to_users_.find(room);
        if (room_it != room_to_users_.end()) {
            room_it->second.erase(user);
        }
        user_to_room_.erase(it);
    }
}
