/**
 * @file db_wrapper.h
 * @brief Заголовочный файл для класса IRCDBWrapper, предоставляющего интерфейс 
 * для взаимодействия с базой данных IRC.
 */
#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "postgres.h"


/**
 * @class IRCDBWrapper
 * @brief Предоставляет методы для взаимодействия с базой данных IRC, включая управление пользователями, комнатами и сообщениями.
 * @details Этот класс инкапсулирует пул соединений с базой данных PostgreSQL и предоставляет
 *  методы для выполнения операций CRUD (Create, Read, Update, Delete) для
 *  пользователей, комнат и сообщений. Класс предназначен для упрощения и
 *  централизации доступа к базе данных в приложении IRC.
 */
class IRCDBWrapper {
public:
    /**
     * @brief Конструктор класса IRCDBWrapper.
     * @param conn_str Строка подключения к базе данных.
     * @param pool_size Размер пула соединений (по умолчанию: 1).
     */
    IRCDBWrapper(const std::string& conn_str, size_t pool_size = 1);

     /**
     * @brief Добавляет пользователя в базу данных.
     * @param name Имя пользователя.
     * @param pass_hash Хэш пароля пользователя.
     * @return std::pair<bool, std::string>  `true` если пользователь добавлен успешно, `false` в противном случае, а также сообщение об ошибке.
     */
    std::pair<bool, std::string> AddUserToDB(const std::string& name, const std::string& pass_hash);

    /**
     * @brief Находит пользователя по его идентификатору.
     * @param id Идентификатор пользователя.
     * @return std::optional<postgres::UserRecord> Запись пользователя, если найдена, `std::nullopt` в противном случае.
     */
    std::optional<postgres::UserRecord> FindUserById(const postgres::UserId& id);

    /**
     * @brief Находит пользователя по имени пользователя.
     * @param username Имя пользователя.
     * @return std::optional<postgres::UserRecord> Запись пользователя, если найдена, `std::nullopt` в противном случае.
     */
    std::optional<postgres::UserRecord> FindUserByName(const std::string& username);

    /**
     * @brief Удаляет пользователя из базы данных по имени пользователя.
     * @param username Имя пользователя.
     * @return true, если пользователь был успешно удален, false в противном случае.
     */
    bool DeleteUserByName(const std::string& username);

     /**
     * @brief Получает карту всех пользователей (имя пользователя -> хэш пароля).
     * @return std::unordered_map<std::string, std::string> Карта всех пользователей, хранящихся в базе данных.
     */
    std::unordered_map<std::string, std::string> GetAllUsers();

      /**
     * @brief Находит комнату по ее имени.
     * @param room_name Имя комнаты.
     * @return std::optional<postgres::RoomRecord> Запись комнаты, если найдена, `std::nullopt` в противном случае.
     */
    std::optional<postgres::RoomRecord> FindRoomByName(const std::string& room_name);

    /**
     * @brief Добавляет комнату в базу данных.
     * @param room_name Имя комнаты.
     * @return std::pair<bool, std::string> `true` если комната добавлена успешно, `false` в противном случае, а также сообщение об ошибке.
     */
    std::pair<bool, std::string> AddRoomToDB(const std::string& room_name);

    /**
     * @brief Получает список всех комнат.
     * @return std::vector<postgres::RoomRecord> Список записей всех комнат, хранящихся в базе данных.
     */
    std::vector<postgres::RoomRecord> GetAllRooms();

    /**
     * @brief Добавляет пользователя в комнату по имени пользователя и имени комнаты.
     * @param username Имя пользователя.
     * @param roomname Имя комнаты.
     * @return std::pair<bool, std::string> `true` если пользователь добавлен в комнату успешно, `false` в противном случае, а также сообщение об ошибке.
     */
    std::pair<bool, std::string> AddUserToRoomByName(const std::string& username, const std::string& roomname);

     /**
     * @brief Удаляет пользователя из комнаты по имени пользователя и имени комнаты.
     * @param username Имя пользователя.
     * @param roomname Имя комнаты.
     * @return std::pair<bool, std::string> `true` если пользователь удален из комнаты успешно, `false` в противном случае, а также сообщение об ошибке.
     */
    std::pair<bool, std::string> RemoveUserFromRoomByName(const std::string& username, const std::string& roomname);

    /**
     * @brief Получает страницу записей комнат.
     * @param offset Смещение для загрузки страницы.
     * @param limit Количество записей на странице.
     * @return std::vector<postgres::RoomRecord> Список записей комнат на странице.
     */
    std::vector<postgres::RoomRecord> GetRoomsPage(int offset, int limit);

    /**
     * @brief Получает список участников комнаты по ее имени.
     * @param roomname Имя комнаты.
     * @return std::vector<postgres::UserRecord> Список записей пользователей, являющихся участниками комнаты.
     */
    std::vector<postgres::UserRecord> GetRoomMembersByName(const std::string& roomname);

    /**
     * @brief Удаляет комнату по ее имени.
     * @param roomname Имя комнаты.
     * @return std::pair<bool, std::string> `true` если комната удалена успешно, `false` в противном случае, а также сообщение об ошибке.
     */
    std::pair<bool, std::string> DeleteRoomByName(const std::string& roomname);

    /**
     * @brief Добавляет сообщение в базу данных.
     * @param username Имя пользователя, отправившего сообщение.
     * @param roomname Имя комнаты, в которую отправлено сообщение.
     * @param text Текст сообщения.
     * @return std::pair<bool, std::string> `true` если сообщение добавлено успешно, `false` в противном случае, а также сообщение об ошибке.
     */
    std::pair<bool, std::string> AddMessage(const std::string& username, const std::string& roomname, const std::string& text);

    /**
     * @brief Получает последние сообщения из комнаты.
     * @param roomname Имя комнаты.
     * @param max_items Максимальное количество сообщений для загрузки.
     * @return std::vector<postgres::MessageRecord> Список записей сообщений.
     */
    std::vector<postgres::MessageRecord> GetRecentMessages(const std::string& roomname, int max_items);

     /**
     * @brief Получает страницу сообщений из комнаты.
     * @param roomname Имя комнаты.
     * @param offset Смещение для загрузки страницы.
     * @param limit Количество записей на странице.
     * @return std::vector<postgres::MessageRecord> Список записей сообщений на странице.
     */
    std::vector<postgres::MessageRecord> GetRoomMessagesPage(const std::string& roomname, int offset, int limit);

    /**
     * @brief Удаляет сообщение по его идентификатору.
     * @param id Идентификатор сообщения.
     * @return true, если сообщение было успешно удалено, false в противном случае.
     */
    bool DeleteMessageById(const postgres::MessageId& id);

private:
     /// @brief Пул соединений с базой данных.
    std::shared_ptr<postgres::ConnectionPool> pool_;
};
