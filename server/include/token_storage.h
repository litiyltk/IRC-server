#pragma once

#include <mutex>
#include <string>
#include <unordered_map>


class TokenStorage {
public:
    static TokenStorage& instance();

    void SaveToken(const std::string& token, const std::string& user);
    bool HasUserByToken(const std::string& token, std::string& user);
    bool HasTokenByUser(const std::string& user, std::string& token);
    void RemoveToken(const std::string& token);

private:
    std::unordered_map<std::string, std::string> user_to_token_;
    std::unordered_map<std::string, std::string> token_to_user_;
    std::mutex mutex_;
};
