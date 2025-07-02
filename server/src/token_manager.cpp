#include "token_manager.h"


namespace chat {

void TokenManager::SaveToken(postgres::UserId user_id, const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    id_to_token_[user_id] = token;
    token_to_id_[token] = user_id;
}

void TokenManager::RemoveTokenByUserId(const postgres::UserId& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = id_to_token_.find(user_id);
    if (it != id_to_token_.end()) {
        token_to_id_.erase(it->second);
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
        return it->second;
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
    std::vector<postgres::UserId> ids;
    ids.reserve(id_to_token_.size());
    for (const auto& [id, _] : id_to_token_) {
        ids.push_back(id);
    }
    return ids;
}

}  // namespace chat
