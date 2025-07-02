/**
 * @file postgres.h
 * @brief Заголовочный файл, определяющий интерфейс для работы с базой данных PostgreSQL.
 * @details Содержит объявления пространств имен, структур данных и классов,
 *  предназначенных для организации взаимодействия с базой данных PostgreSQL,
 *  включая пулы соединений, репозитории для пользователей, комнат, сообщений
 *  и участников комнат.
 */
#pragma once

#include <condition_variable>
#include <pqxx/pqxx>
#include <memory>
#include <vector>
#include <optional>
#include <string>
#include <mutex>

#include "tagged_uuid.h"


/**
 * @namespace postgres
 * @brief Пространство имен, содержащее классы и структуры для работы с базой данных PostgreSQL.
 * @details Это пространство имен содержит классы для управления пулом соединений,
 *  а также репозитории для доступа к данным, таким как пользователи, комнаты
 *  и сообщения.
 */
namespace postgres
{

    // ---- Идентификаторы через TaggedUUID ----
    namespace detail
    {
        /**
         * @struct UserIdTag
         * @brief Тег для идентификатора пользователя.
         */
        struct UserIdTag
        {
        };
        /**
         * @struct RoomIdTag
         * @brief Тег для идентификатора комнаты.
         */
        struct RoomIdTag
        {
        };
        /**
         * @struct MessageIdTag
         * @brief Тег для идентификатора сообщения.
         */
        struct MessageIdTag
        {
        };
        /**
         * @struct RoomMemberIdTag
         * @brief Тег для идентификатора участника комнаты.
         */
        struct RoomMemberIdTag
        {
        };
    }

    /** @brief Идентификатор пользователя. */
    using UserId = util::TaggedUUID<detail::UserIdTag>;
    /** @brief Идентификатор комнаты. */
    using RoomId = util::TaggedUUID<detail::RoomIdTag>;
    /** @brief Идентификатор сообщения. */
    using MessageId = util::TaggedUUID<detail::MessageIdTag>;
    /** @brief Идентификатор участника комнаты. */
    using RoomMemberId = util::TaggedUUID<detail::RoomMemberIdTag>;

    // ---- Структуры записей ----
    /**
     * @struct UserRecord
     * @brief Структура, представляющая запись пользователя.
     */
    struct UserRecord
    {
        /** @brief Идентификатор пользователя. */
        UserId id;
        /** @brief Имя пользователя. */
        std::string username;
        /** @brief Хэш пароля пользователя. */
        std::string password_hash;
        /** @brief Дата регистрации пользователя. */
        std::string registered_at;
    };

    /**
     * @struct RoomRecord
     * @brief Структура, представляющая запись комнаты.
     */
    struct RoomRecord
    {
        /** @brief Идентификатор комнаты. */
        RoomId id;
        /** @brief Название комнаты. */
        std::string name;
        /** @brief Дата создания комнаты. */
        std::string created_at;
    };

    /**
     * @struct MessageRecord
     * @brief Структура, представляющая запись сообщения.
     */
    struct MessageRecord
    {
        /** @brief Идентификатор сообщения. */
        MessageId id;
        /** @brief Идентификатор пользователя, отправившего сообщение. */
        UserId user_id;
        /** @brief Идентификатор комнаты, в которой отправлено сообщение. */
        RoomId room_id;
        /** @brief Текст сообщения. */
        std::string message;
        /** @brief Дата отправки сообщения. */
        std::string sent_at;
    };

    /**
     * @struct RoomMemberRecord
     * @brief Структура, представляющая запись участника комнаты.
     */
    struct RoomMemberRecord
    {
        /** @brief Идентификатор участника комнаты. */
        RoomMemberId id;
        /** @brief Идентификатор комнаты. */
        RoomId room_id;
        /** @brief Идентификатор пользователя. */
        UserId user_id;
        /** @brief Дата присоединения к комнате. */
        std::string joined_at;
    };

    // ---- Пул соединений ----
    /**
     * @class ConnectionPool
     * @brief Класс для управления пулом соединений с базой данных.
     */
    class ConnectionPool
    {
        using PoolType = ConnectionPool;
        using ConnectionPtr = std::shared_ptr<pqxx::connection>;

    public:
        /**
         * @class ConnectionWrapper
         * @brief Класс-обертка для управления временем жизни соединения.
         */
        class ConnectionWrapper
        {
        public:
            /**
             * @brief Конструктор класса ConnectionWrapper.
             * @param conn Указатель на соединение.
             * @param pool Ссылка на пул соединений.
             */
            ConnectionWrapper(std::shared_ptr<pqxx::connection> &&conn, PoolType &pool) noexcept;

            ConnectionWrapper(const ConnectionWrapper &) = delete;
            ConnectionWrapper &operator=(const ConnectionWrapper &) = delete;
            ConnectionWrapper(ConnectionWrapper &&) = default;
            ConnectionWrapper &operator=(ConnectionWrapper &&) = default;

            /**
             * @brief Оператор разыменования (const).
             * @return Ссылка на соединение.
             */
            pqxx::connection &operator*() const & noexcept;
            pqxx::connection &operator*() const && = delete;
            /**
             * @brief Оператор -> (const).
             * @return Указатель на соединение.
             */
            pqxx::connection *operator->() const & noexcept;

            ~ConnectionWrapper();

        private:
            /** @brief Указатель на соединение. */
            std::shared_ptr<pqxx::connection> conn_;
            /** @brief Указатель на пул соединений. */
            PoolType *pool_;
        };

        /**
         * @brief Конструктор класса ConnectionPool.
         * @param capacity Максимальное количество соединений в пуле.
         * @param connection_factory Фабрика соединений.
         */
        template <typename ConnectionFactory>
        ConnectionPool(size_t capacity, ConnectionFactory &&connection_factory);

        /**
         * @brief Получает соединение из пула.
         * @return ConnectionWrapper Обертка для соединения.
         */
        ConnectionWrapper GetConnection();

        /**
         * @brief Получает соединение из пула с таймаутом.
         * @param timeout Таймаут ожидания соединения.
         * @return std::optional<ConnectionWrapper> Обертка для соединения, если удалось получить, иначе `std::nullopt`.
         */
        std::optional<ConnectionWrapper> GetConnection(std::chrono::milliseconds timeout);

    private:
        /**
         * @brief Возвращает соединение в пул.
         * @param conn Указатель на соединение.
         */
        void ReturnConnection(ConnectionPtr &&conn);

        /** @brief Мьютекс для обеспечения потокобезопасности. */
        std::mutex mutex_;
        /** @brief Условная переменная для ожидания соединения. */
        std::condition_variable cond_var_;
        /** @brief Вектор для хранения соединений. */
        std::vector<ConnectionPtr> pool_;
        /** @brief Количество используемых соединений. */
        size_t used_connections_ = 0;
    };

    template <typename ConnectionFactory>
    ConnectionPool::ConnectionPool(size_t capacity, ConnectionFactory &&connection_factory)
    {
        pool_.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i)
        {
            pool_.emplace_back(connection_factory());
        }
    }

    // ---- Репозиторий пользователей ----
    /**
     * @class UsersRepository
     * @brief Класс для управления записями пользователей в базе данных.
     * @details Предоставляет методы для сохранения, загрузки, поиска и удаления
     *  записей пользователей в базе данных.  Использует транзакцию `pqxx::work`
     *  для взаимодействия с базой данных.
     */
    class UsersRepository
    {
    public:
        /**
         * @brief Конструктор класса UsersRepository.
         * @param transaction Ссылка на транзакцию базы данных.
         */
        explicit UsersRepository(pqxx::work &transaction);

        /**
         * @brief Сохраняет запись пользователя в базе данных.
         * @param username Имя пользователя.
         * @param password_hash Хэш пароля пользователя.
         */
        void Save(const std::string &username, const std::string &password_hash) const;

        /**
         * @brief Загружает все записи пользователей из базы данных.
         * @return std::vector<UserRecord> Список записей пользователей.
         */
        std::vector<UserRecord> LoadAll() const;

        /**
         * @brief Находит запись пользователя по идентификатору.
         * @param id Идентификатор пользователя.
         * @return std::optional<UserRecord> Запись пользователя, если она найдена, иначе `std::nullopt`.
         */
        std::optional<UserRecord> FindById(const UserId &id) const;

        /**
         * @brief Находит запись пользователя по имени пользователя.
         * @param username Имя пользователя.
         * @return std::optional<UserRecord> Запись пользователя, если она найдена, иначе `std::nullopt`.
         */
        std::optional<UserRecord> FindByUsername(const std::string &username) const;

        /**
         * @brief Удаляет запись пользователя по имени пользователя.
         * @param username Имя пользователя.
         * @return true, если запись была удалена, иначе false.
         */
        bool DeleteByUsername(const std::string &username) const;
        /**
         * @brief Загружает страницу записей пользователей из базы данных.
         * @param offset Смещение для загрузки страницы.
         * @param limit Количество записей на странице.
         * @return std::vector<UserRecord> Список записей пользователей на странице.
         */
        std::vector<UserRecord> LoadPage(int offset, int limit) const;

        /**
         * @brief Загружает карту пользователей (имя пользователя -> хэш пароля).
         * @return std::unordered_map<std::string, std::string> Карта пользователей.
         */
        std::unordered_map<std::string, std::string> LoadUserMap() const;

    private:
        /// @brief Ссылка на транзакцию базы данных.
        pqxx::work &transaction_;
    };

    // ---- Репозиторий комнат ----
    /**
     * @class RoomsRepository
     * @brief Класс для управления записями комнат в базе данных.
     * @details Предоставляет методы для сохранения, загрузки, поиска и удаления
     *  записей комнат в базе данных. Использует транзакцию `pqxx::work` для
     *  взаимодействия с базой данных.
     */
    class RoomsRepository
    {
    public:
        /**
         * @brief Конструктор класса RoomsRepository.
         * @param transaction Ссылка на транзакцию базы данных.
         */
        explicit RoomsRepository(pqxx::work &transaction);

        /**
         * @brief Сохраняет запись комнаты в базе данных.
         * @param name Название комнаты.
         */
        void Save(const std::string &name) const;
        /**
         * @brief Находит запись комнаты по названию.
         * @param name Название комнаты.
         * @return std::optional<RoomRecord> Запись комнаты, если она найдена, иначе `std::nullopt`.
         */
        std::optional<RoomRecord> FindByName(const std::string &name) const;

        /**
         * @brief Загружает все записи комнат из базы данных.
         * @return std::vector<RoomRecord> Список записей комнат.
         */
        std::vector<RoomRecord> LoadAll() const;
        /**
         * @brief Загружает страницу записей комнат из базы данных.
         * @param offset Смещение для загрузки страницы.
         * @param limit Количество записей на странице.
         * @return std::vector<RoomRecord> Список записей комнат на странице.
         */
        std::vector<RoomRecord> LoadPage(int offset, int limit) const;
        /**
         * @brief Удаляет запись комнаты по идентификатору.
         * @param id Идентификатор комнаты.
         */
        void DeleteById(const RoomId &id) const;

    private:
        /// @brief Ссылка на транзакцию базы данных.
        pqxx::work &transaction_;
    };

    // ---- Репозиторий сообщений ----

    /**
     * @class MessagesRepository
     * @brief Класс для управления записями сообщений в базе данных.
     * @details Предоставляет методы для сохранения, загрузки последних сообщений,
     *  загрузки страницы сообщений и удаления записей сообщений в базе данных.
     *  Использует транзакцию `pqxx::work` для взаимодействия с базой данных.
     */
    class MessagesRepository
    {
    public:
        /**
         * @brief Конструктор класса MessagesRepository.
         * @param transaction Ссылка на транзакцию базы данных.
         */
        explicit MessagesRepository(pqxx::work &transaction);
        
         /**
         * @brief Сохраняет запись сообщения в базе данных.
         * @param user_id Идентификатор пользователя, отправившего сообщение.
         * @param room_id Идентификатор комнаты, в которой отправлено сообщение.
         * @param message Текст сообщения.
         */
        void Save(const UserId &user_id, const RoomId &room_id, const std::string &message) const;
        
         /**
         * @brief Загружает последние сообщения из комнаты.
         * @param room_id Идентификатор комнаты.
         * @param max_items Максимальное количество сообщений для загрузки.
         * @return std::vector<MessageRecord> Список записей сообщений.
         */
        std::vector<MessageRecord> LoadRecent(const RoomId &room_id, int max_items) const;
        
           /**
         * @brief Загружает страницу сообщений из базы данных.
         * @param room_id Идентификатор комнаты.
         * @param offset Смещение для загрузки страницы.
         * @param limit Количество записей на странице.
         * @return std::vector<MessageRecord> Список записей сообщений на странице.
         */
        std::vector<MessageRecord> LoadPage(const RoomId &room_id, int offset, int limit) const;
         /**
         * @brief Удаляет запись сообщения по идентификатору.
         * @param id Идентификатор сообщения.
         */
        void DeleteById(const MessageId &id) const;

    private:
        /// @brief Ссылка на транзакцию базы данных.
        pqxx::work &transaction_;
    };

    // ---- Репозиторий участников комнат ----
    /**
 * @class RoomMembersRepository
 * @brief Класс для управления записями участников комнат в базе данных.
 * @details Предоставляет методы для сохранения, удаления и загрузки информации
 *  об участниках комнат в базе данных. Использует транзакцию `pqxx::work` для
 *  взаимодействия с базой данных.
 */
    class RoomMembersRepository
    {
    public:
          /**
         * @brief Конструктор класса RoomMembersRepository.
         * @param transaction Ссылка на транзакцию базы данных.
         */
        explicit RoomMembersRepository(pqxx::work &transaction);
        
         /**
         * @brief Сохраняет запись об участии пользователя в комнате.
         * @param user_id Идентификатор пользователя.
         * @param room_id Идентификатор комнаты.
         */
        void Save(const UserId &user_id, const RoomId &room_id) const;
        
        /**
         * @brief Удаляет запись об участии пользователя в комнате.
         * @param user_id Идентификатор пользователя.
         * @param room_id Идентификатор комнаты.
         */
        void Remove(const UserId &user_id, const RoomId &room_id) const;
        
        /**
         * @brief Загружает список участников комнаты.
         * @param room_id Идентификатор комнаты.
         * @return std::vector<RoomMemberRecord> Список записей участников комнаты.
         */
        std::vector<RoomMemberRecord> LoadMembers(const RoomId &room_id) const;
         
         /**
         * @brief Загружает список комнат, в которых состоит пользователь.
         * @param user_id Идентификатор пользователя.
         * @return std::vector<RoomMemberRecord> Список записей комнат, в которых состоит пользователь.
         */
        std::vector<RoomMemberRecord> LoadRooms(const UserId &user_id) const;

    private:
        /// @brief Ссылка на транзакцию базы данных.
        pqxx::work &transaction_;
    };

    // ---- Основная база данных ----
    /**
 * @class Database
 * @brief Класс для управления подключением к базе данных.
 * @details  Этот класс предоставляет интерфейс для получения соединений с базой данных
 *  из пула соединений.  Он инкапсулирует пул соединений и предоставляет метод
 *  для получения транзакции, обернутой в `ConnectionPool::ConnectionWrapper`.
 */
    class Database
    {
    public:
        /**
         * @brief Конструктор класса Database.
         * @param pool_ptr Указатель на пул соединений.
         */
        explicit Database(std::shared_ptr<ConnectionPool> pool_ptr);

         /**
         * @brief Получает транзакцию из пула соединений.
         * @return ConnectionPool::ConnectionWrapper Обертка для соединения.
         */
        [[nodiscard]] ConnectionPool::ConnectionWrapper GetTransaction() const;

    private:
        /// @brief Указатель на пул соединений.
        std::shared_ptr<ConnectionPool> pool_ptr_;
    };

} // namespace postgres
