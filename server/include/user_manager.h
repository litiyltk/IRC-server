#pragma once

#include <string>
#include <unordered_map>
#include <mutex>


// Логика работы с клиентами для user_controller.h (REST API) + потокобезопасное хранилище клиентов

class UserManager {
public:
    static UserManager& instance();

    bool RegisterUser(const std::string& login, const std::string& password);
    bool HasUser(const std::string& login, const std::string& password);

private:
    std::unordered_map<std::string, std::string> users_db_; // TODO разнести и не хранить вместе имя и хэш пароля
    std::mutex mutex_;
};
