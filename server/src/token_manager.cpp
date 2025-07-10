#include "token_manager.h"


namespace chat {

void TokenManager::SaveToken(postgres::UserId user_id, const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    id_to_token_[user_id] = {token, std::chrono::steady_clock::now()};
    token_to_id_[token] = user_id;
}

void TokenManager::RemoveTokenByUserId(const postgres::UserId& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = id_to_token_.find(user_id);
    if (it != id_to_token_.end()) {
        token_to_id_.erase(it->second.token);
        id_to_token_.erase(it);
    }
}

void TokenManager::RemoveTokenByToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = token_to_id_.find(token);
    if (it != token_to_id_.end()) {
        id_to_token_.erase(it->second);
        token_to_id_.erase(it);
    }
}

std::optional<std::string> TokenManager::GetTokenByUserId(const postgres::UserId& user_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = id_to_token_.find(user_id);
    if (it != id_to_token_.end()) {
        return it->second.token;
    }
    return std::nullopt;
}

std::optional<postgres::UserId> TokenManager::GetUserIdByToken(const std::string& token) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = token_to_id_.find(token);
    if (it != token_to_id_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<postgres::UserId> TokenManager::GetOnlineUserIds() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<postgres::UserId> users;
    for (const auto& pair : id_to_token_) {
        users.push_back(pair.first);
    }
    return users;
}

void TokenManager::UpdateActivityByToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = token_to_id_.find(token);
    if (it != token_to_id_.end()) {
        auto id_it = id_to_token_.find(it->second);
        if (id_it != id_to_token_.end()) {
            id_it->second.last_activity = std::chrono::steady_clock::now();
        }
    }
}

std::vector<std::string> TokenManager::GetExpiredTokens(std::chrono::minutes timeout) const {
    std::vector<std::string> expired;
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();

    for (const auto& [user_id, token_info] : id_to_token_) {
        if (now - token_info.last_activity > timeout) {
            expired.push_back(token_info.token);
        }
    }
    return expired;
}

} // namespace chat
