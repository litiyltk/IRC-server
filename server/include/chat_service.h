/**
 * @file chat_service.h
 * @brief Заголовочный файл для класса ChatService, 
 * предоставляющего интерфейс для управления чатом.
 */
#pragma once

#include "token_manager.h"
#include "token_generator.h"
#include "db_wrapper.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>


/**
 * @namespace chat
 * @brief Пространство имен, содержащее классы и функции для управления чатом.
 */
namespace chat
{

     /**
     * @class ChatService
     * @brief Предоставляет интерфейс для управления чатом, 
     * включая аутентификацию, управление пользователями и комнатами.
     * @details  Класс `ChatService` предоставляет методы для 
     * аутентификации пользователей, управления комнатами чата 
     * и обработки сообщений.  Он использует `IRCDBWrapper` для доступа 
     * к базе данных и `TokenManager` для управления токенами.
     */
    class ChatService
    {
    public:
        /**
         * @brief Конструктор класса ChatService.
         * @param db_wrapper Указатель на объект `IRCDBWrapper` для доступа к базе данных.
         * @param token_mgr Указатель на объект `TokenManager` для управления токенами.
         */
        ChatService(std::shared_ptr<IRCDBWrapper> db_wrapper,
                    std::shared_ptr<TokenManager> token_mgr);

        // Аутентификация
        /**
         * @brief Регистрирует нового пользователя.
         * @param name Имя пользователя.
         * @param password_hash Хэш пароля пользователя.
         * @return `true`, если регистрация прошла успешно, `false` в противном случае.
         */
        bool Register(const std::string &name, const std::string &password_hash);

        /**
         * @brief Выполняет вход пользователя.
         * @param name Имя пользователя.
         * @param password_hash Хэш пароля пользователя.
         * @return std::optional<std::string> Токен аутентификации, если вход выполнен успешно, `std::nullopt` в противном случае.
         */
        std::optional<std::string> Login(const std::string &name, const std::string &password_hash);

        /**
         * @brief Выполняет выход пользователя.
         * @param token Токен аутентификации пользователя.
         * @return `true`, если выход выполнен успешно, `false` в противном случае.
         */
        bool Logout(const std::string &token);

        // Пользователи
        /**
         * @brief Получает список имен пользователей, находящихся в сети.
         * @return std::vector<std::string> Список имен пользователей, находящихся в сети.
         */
        std::vector<std::string> GetOnlineUserNames() const;

        // Комнаты
        /**
         * @brief Создает новую комнату.
         * @param name Имя комнаты.
         * @return `true`, если комната создана успешно, `false` в противном случае.
         */
        bool CreateRoom(const std::string &name);

        /**
         * @brief Присоединяет пользователя к комнате.
         * @param token Токен аутентификации пользователя.
         * @param room_name Имя комнаты.
         * @return `true`, если пользователь успешно присоединен к комнате, `false` в противном случае.
         */
        bool JoinRoom(const std::string &token, const std::string &room_name);

        /**
         * @brief Позволяет пользователю покинуть текущую комнату.
         * @param token Токен аутентификации пользователя.
         * @return `true`, если пользователь успешно покинул комнату, `false` в противном случае.
         */
        bool LeaveRoom(const std::string &token);

        /**
         * @brief Проверяет, существует ли комната с указанным именем.
         * @param name Имя комнаты.
         * @return `true`, если комната существует, `false` в противном случае.
         */
        bool HasRoom(const std::string &name) const;

        /**
         * @brief Получает список имен всех комнат.
         * @return std::vector<std::string> Список имен всех комнат.
         */
        std::vector<std::string> GetRoomNames() const;

        /**
         * @brief Получает имя текущей комнаты, в которой находится пользователь.
         * @param token Токен аутентификации пользователя.
         * @return std::optional<std::string> Имя комнаты, если пользователь находится в комнате, `std::nullopt` в противном случае.
         */
        std::optional<std::string> GetCurrentRoomName(const std::string &token) const;

        /**
         * @brief Получает список имен пользователей, находящихся в указанной комнате.
         * @param room_name Имя комнаты.
         * @return std::vector<std::string> Список имен пользователей, находящихся в указанной комнате.
         */
        std::vector<std::string> GetUserNamesInRoom(const std::string &room_name) const;

        // Сообщения
        /**
         * @brief Сохраняет сообщение в базе данных.
         * @param token Токен аутентификации пользователя.
         * @param text Текст сообщения.
         * @return `true`, если сообщение успешно сохранено, `false` в противном случае.
         */
        bool SaveMessage(const std::string &token, const std::string &text); // Сохраняет сообщение в БД, применяется при отправке

        /**
         * @brief Получает последние сообщения из указанной комнаты.
         * @param room_name Имя комнаты.
         * @param max_items Максимальное количество сообщений для загрузки.
         * @return std::vector<postgres::MessageRecord> Список записей сообщений.
         */
        std::vector<postgres::MessageRecord> GetRecentMessages(const std::string &room_name, int max_items) const;

        /**
         * @brief Получает страницу сообщений из указанной комнаты.
         * @param room_name Имя комнаты.
         * @param offset Смещение для загрузки страницы.
         * @param limit Количество записей на странице.
         * @return std::vector<postgres::MessageRecord> Список записей сообщений на странице.
         */
        std::vector<postgres::MessageRecord> GetRoomMessagesPage(const std::string &room_name, int offset, int limit) const;

        /**
         * @brief Получает пользователя по токену аутентификации.
         * @param token Токен аутентификации пользователя.
         * @return std::optional<postgres::UserRecord> Запись пользователя, если токен действителен, `std::nullopt` в противном случае.
         */
        std::optional<postgres::UserRecord> GetUserByToken(const std::string &token) const;

        /**
         * @brief Получает токен аутентификации по имени пользователя.
         * @param name Имя пользователя.
         * @return std::optional<std::string> Токен аутентификации, если пользователь найден, `std::nullopt` в противном случае.
         */
        std::optional<std::string> GetTokenByUserName(const std::string &name) const;

        /**
         * @brief Получает имя пользователя по его идентификатору.
         * @param id Идентификатор пользователя.
         * @return std::optional<std::string> Имя пользователя, если пользователь найден, `std::nullopt` в противном случае.
         */
        std::optional<std::string> GetUserNameById(const postgres::UserId &id) const;

    private:
        /// @brief Указатель на объект `IRCDBWrapper` для доступа к базе данных.
        std::shared_ptr<IRCDBWrapper> db_wrapper_;
        /// @brief Указатель на объект `TokenManager` для управления токенами.
        std::shared_ptr<TokenManager> token_manager_; // Токены сбрасываются при падении сервера, требуется новый вход по сохраненным в БД логину и хэшу пароля
    };

} // namespace chat
