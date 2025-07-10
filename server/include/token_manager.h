/**
 * @file token_manager.h
 * @brief Заголовочный файл для класса TokenManager с поддержкой Idle Timeout.
 */
#pragma once

#include "postgres.h"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>


namespace chat {

/**
 * @class TokenManager
 * @brief Класс для управления токенами аутентификации с поддержкой автоматического удаления по таймауту.
 */
class TokenManager {
public:

    /**
     * @brief Сохраняет токен для указанного идентификатора пользователя.
     * @param user_id Идентификатор пользователя.
     * @param token Токен аутентификации.
     */
    void SaveToken(postgres::UserId user_id, const std::string& token);

    /**
     * @brief Удаляет токен по идентификатору пользователя.
     * @param user_id Идентификатор пользователя.
     */
    void RemoveTokenByUserId(const postgres::UserId& user_id);
    
    /**
     * @brief Удаляет токен по строковому значению токена.
     * @param token Токен аутентификации.
     */
    void RemoveTokenByToken(const std::string& token);

    /**
     * @brief Получает токен по идентификатору пользователя.
     * @param user_id Идентификатор пользователя.
     * @return Токен, если найден.
     */
    std::optional<std::string> GetTokenByUserId(const postgres::UserId& user_id) const;

    /**
     * @brief Получает идентификатор пользователя по токену.
     * @param token Токен аутентификации.
     * @return Идентификатор пользователя, если найден.
     */
    std::optional<postgres::UserId> GetUserIdByToken(const std::string& token) const;

    /**
     * @brief Получает список идентификаторов всех пользователей с активными токенами.
     * @return Вектор идентификаторов пользователей.
     */
    std::vector<postgres::UserId> GetOnlineUserIds() const;

    /**
     * @brief Обновляет время последней активности пользователя по токену.
     * @param token Токен пользователя.
     */
    void UpdateActivityByToken(const std::string& token);

    /**
     * @brief Получает список токенов, которые не обновлялись в течение заданного времени.
     * @param timeout Время бездействия, по истечении которого токены считаются просроченными.
     * @return std::vector<std::string> список токенов, которые не обновлялись в течение заданного времени.
     */
    std::vector<std::string> GetExpiredTokens(std::chrono::minutes timeout) const;

private:
    /**
     * @struct TokenInfo
     * @brief Структура, хранящая информацию о токене пользователя.
     * @details Содержит сам токен и временную метку последней активности.
     */
    struct TokenInfo {
        std::string token;  ///< Токен пользователя (HEX-строка)
        std::chrono::steady_clock::time_point last_activity;  ///< Последнее время активности пользователя
    };

    /** 
     * @brief Мьютекс для обеспечения потокобезопасного доступа к токенам.
     * @details Защищает все внутренние карты токенов от гонок и конкурентного доступа.
     */
    mutable std::mutex mutex_;

    /**
     * @brief Ассоциативная карта: ID пользователя -> информация о токене.
     * @details Используется для быстрого поиска токена и времени последней активности по идентификатору пользователя.
     */
    std::unordered_map<postgres::UserId, TokenInfo, util::TaggedHasher<postgres::UserId>> id_to_token_;

    /**
     * @brief Обратная карта: токен -> ID пользователя.
     * @details Используется для аутентификации по токену, проверки активности и поиска владельца.
     */
    std::unordered_map<std::string, postgres::UserId> token_to_id_;
};

} // namespace chat
