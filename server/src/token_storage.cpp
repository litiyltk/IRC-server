#include "token_storage.h"


TokenStorage& TokenStorage::instance() {
    static TokenStorage inst;
    return inst;
}

void TokenStorage::SaveToken(const std::string& user, const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Удаляем старый токен, если был
    auto it = user_to_token_.find(user);
    if (it != user_to_token_.end()) {
        token_to_user_.erase(it->second);
    }

    // Сохраняем новый токен
    user_to_token_[user] = token;
    token_to_user_[token] = user;
}

bool TokenStorage::HasUserByToken(const std::string& token, std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = token_to_user_.find(token);
    if (it != token_to_user_.end()) {
        user = it->second;
        return true;
    }
    return false;
}

bool TokenStorage::HasTokenByUser(const std::string& user, std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = user_to_token_.find(user);
    if (it != user_to_token_.end()) {
        token = it->second;
        return true;
    }
    return false;
}

void TokenStorage::RemoveToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = token_to_user_.find(token);
    if (it != token_to_user_.end()) {
        user_to_token_.erase(it->second);
        token_to_user_.erase(it);
    }
}
