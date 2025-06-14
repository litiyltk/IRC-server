#include "user_manager.h"


UserManager& UserManager::instance() {
    static UserManager instance;
    return instance;
}

bool UserManager::RegisterUser(const std::string& login, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (users_db_.find(login) != users_db_.end()) {
        return false;
    }
    users_db_[login] = password;
    return true;
}

bool UserManager::HasUser(const std::string& login, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = users_db_.find(login);
    return it != users_db_.end() && it->second == password;
}
