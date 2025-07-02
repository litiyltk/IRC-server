/**
 * @file token_manager.h
 * @brief Заголовочный файл для класса TokenManager, управляющего токенами аутентификации.
 * @details Определяет класс TokenManager, который предоставляет методы для сохранения,
 *  удаления и получения токенов аутентификации, а также для получения списка
 *  идентификаторов онлайн-пользователей. Использует мьютекс для обеспечения
 *  потокобезопасности.
 */
#pragma once

#include "postgres.h"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace chat {

/**
 * @class TokenManager
 * @brief Класс для управления токенами аутентификации.
 * @details Предоставляет методы для сохранения, удаления и получения токенов
 *  аутентификации, а также для получения списка идентификаторов
 *  онлайн-пользователей.  Использует мьютекс для обеспечения
 *  потокобезопасности.
 */
class TokenManager {
public:
    
    /**
     * @brief Сохраняет токен для указанного идентификатора пользователя.
     * @param user_id Идентификатор пользователя.
     * @param token Токен, который необходимо сохранить.
     */
    void SaveToken(postgres::UserId user_id, const std::string& token);
     /**
     * @brief Удаляет токен, связанный с указанным идентификатором пользователя.
     * @param user_id Идентификатор пользователя.
     */
    void RemoveTokenByUserId(const postgres::UserId& user_id);
    /**
     * @brief Удаляет токен по его значению.
     * @param token Токен, который необходимо удалить.
     */
    void RemoveTokenByToken(const std::string& token);
    
      /**
     * @brief Получает токен, связанный с указанным идентификатором пользователя.
     * @param user_id Идентификатор пользователя.
     * @return std::optional<std::string> Токен, если он найден, в противном случае `std::nullopt`.
     */
    std::optional<std::string> GetTokenByUserId(const postgres::UserId& user_id) const;
    
    /**
     * @brief Получает идентификатор пользователя, связанный с указанным токеном.
     * @param token Токен, который необходимо найти.
     * @return std::optional<postgres::UserId> Идентификатор пользователя, если он найден, в противном случае `std::nullopt`.
     */
    std::optional<postgres::UserId> GetUserIdByToken(const std::string& token) const;

      /**
     * @brief Получает список идентификаторов пользователей, у которых есть активные токены.
     * @return std::vector<postgres::UserId> Список идентификаторов онлайн-пользователей.
     */
    std::vector<postgres::UserId> GetOnlineUserIds() const;

private:
     /** @brief Мьютекс для обеспечения потокобезопасности. */
    mutable std::mutex mutex_;
    /** @brief Карта, связывающая идентификатор пользователя с его токеном. */
    std::unordered_map<postgres::UserId, std::string, util::TaggedHasher<postgres::UserId>> id_to_token_;
    /** @brief Карта, связывающая токен с идентификатором пользователя. */
    std::unordered_map<std::string, postgres::UserId> token_to_id_;
};

}  // namespace chat
