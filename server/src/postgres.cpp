#include "postgres.h"
#include "tagged_uuid.h"

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

// ---- ConnectionPool реализация ----

ConnectionPool::ConnectionWrapper::ConnectionWrapper(std::shared_ptr<pqxx::connection> &&conn, PoolType &pool) noexcept
: conn_{std::move(conn)}
, pool_{&pool} {}

pqxx::connection & ConnectionPool::ConnectionWrapper::operator*() const & noexcept {
return *conn_;
}

pqxx::connection * ConnectionPool::ConnectionWrapper::operator->() const & noexcept {
return conn_.get();
}

ConnectionPool::ConnectionWrapper::~ConnectionWrapper() {
    if (conn_) {
        pool_->ReturnConnection(std::move(conn_));
    }
}

ConnectionPool::ConnectionWrapper ConnectionPool::GetConnection() {
    std::unique_lock lock{mutex_};
    cond_var_.wait(lock, [this] {
        return used_connections_ < pool_.size();
    });
    return {std::move(pool_[used_connections_++]), *this};
}

std::optional<ConnectionPool::ConnectionWrapper> ConnectionPool::GetConnection(std::chrono::milliseconds timeout) {
    std::unique_lock lock{mutex_};
    if (!cond_var_.wait_for(lock, timeout, [this] {
        return used_connections_ < pool_.size();
    })) {
        return std::nullopt;
    }
    return ConnectionWrapper{std::move(pool_[used_connections_++]), *this};
}

void ConnectionPool::ReturnConnection(ConnectionPtr &&conn) {
    {
        std::lock_guard lock{mutex_};
        assert(used_connections_ != 0);
        pool_[--used_connections_] = std::move(conn);
    }
    cond_var_.notify_one();
}

// ---- UsersRepository ----

UsersRepository::UsersRepository(pqxx::work &transaction) : transaction_(transaction) {}

void UsersRepository::Save(const std::string &username, const std::string &password_hash) const {
    auto id = UserId::New().ToString();
    transaction_.exec(
            R"(INSERT INTO users (id, username, password_hash, registered_at) VALUES ($1, $2, $3, NOW());)"_zv,
            pqxx::params(id, username, password_hash)
    );
}

std::vector<UserRecord> UsersRepository::LoadAll() const {
    std::vector<UserRecord> result;
    const pqxx::result query_result = transaction_.exec(
            R"(SELECT id, username, password_hash, registered_at FROM users ORDER BY registered_at DESC;)"_zv
    );
    for (const auto& row : query_result) {
        result.push_back(UserRecord{
                UserId::FromString(row[0].as<std::string>()),
                row[1].as<std::string>(),
                row[2].as<std::string>(),
                row[3].as<std::string>()
        });
    }
    return result;
}

std::optional<UserRecord> UsersRepository::FindById(const UserId &id) const
{
    const pqxx::result res = transaction_.exec(
        "SELECT id, username, password_hash, registered_at FROM users WHERE id = $1;",
        pqxx::params(id.ToString())
    );
    if (res.empty())
        return std::nullopt;
    const auto& row = res[0];
    return UserRecord{
        UserId::FromString(row[0].as<std::string>()),
        row[1].as<std::string>(),
        row[2].as<std::string>(),
        row[3].as<std::string>()
    };
}

std::optional<UserRecord> UsersRepository::FindByUsername(const std::string &username) const
{
    const pqxx::result res = transaction_.exec(
        R"(SELECT id, username, password_hash, registered_at FROM users WHERE username = $1;)", pqxx::params(username)
    );
    if (res.empty())
        return std::nullopt;
    const auto& row = res[0];
    return UserRecord{
        UserId::FromString(row[0].as<std::string>()),
        row[1].as<std::string>(),
        row[2].as<std::string>(), // password_hash
        row[3].as<std::string>()
    };
}

bool UsersRepository::DeleteByUsername(const std::string &username) const
{
    auto res = transaction_.exec(
        "DELETE FROM users WHERE username = $1;",
        pqxx::params(username)
    );
    return res.affected_rows() > 0;
}

std::vector<UserRecord> UsersRepository::LoadPage(int offset, int limit) const
{
    std::vector<UserRecord> result;
    const pqxx::result query_result = transaction_.exec(
        R"(SELECT id, username, password_hash, registered_at FROM users
            ORDER BY registered_at DESC
            LIMIT $1 OFFSET $2;)",
        pqxx::params(limit, offset)
        );
    for (const auto& row : query_result) {
        result.push_back(UserRecord{
            UserId::FromString(row[0].as<std::string>()),
            row[1].as<std::string>(),
            row[2].as<std::string>(),
            row[3].as<std::string>()
        });
    }
    return result;
}

std::unordered_map<std::string, std::string> UsersRepository::LoadUserMap() const
{
    std::unordered_map<std::string, std::string> result;
    const pqxx::result query_result = transaction_.exec(
        "SELECT username, password_hash FROM users"
        );
    for (const auto& row : query_result) {
        result[row[0].as<std::string>()] = row[1].as<std::string>();
    }
    return result;
}

// ---- RoomsRepository ----

RoomsRepository::RoomsRepository(pqxx::work &transaction) : transaction_(transaction) {}

void RoomsRepository::Save(const std::string &name) const {
    auto id = RoomId::New().ToString();
    transaction_.exec(
            R"(INSERT INTO rooms (id, name, created_at) VALUES ($1, $2, NOW());)"_zv, pqxx::params(id, name)
    );
}

std::optional<RoomRecord> RoomsRepository::FindByName(const std::string &name) const
{
    const pqxx::result res = transaction_.exec(
        "SELECT id, name, created_at FROM rooms WHERE name = $1;",
        pqxx::params(name)
        );
    if (res.empty())
        return std::nullopt;
    const auto& row = res[0];
    return RoomRecord{
        RoomId::FromString(row[0].as<std::string>()),
        row[1].as<std::string>(),
        row[2].as<std::string>()
    };
}

std::vector<RoomRecord> RoomsRepository::LoadAll() const {
    std::vector<RoomRecord> result;
    const pqxx::result query_result = transaction_.exec(
            R"(SELECT id, name, created_at FROM rooms ORDER BY created_at DESC;)"_zv
    );
    for (const auto& row : query_result) {
        result.push_back(RoomRecord{
                RoomId::FromString(row[0].as<std::string>()),
                row[1].as<std::string>(),
                row[2].as<std::string>()
        });
    }
    return result;
}

std::vector<RoomRecord> RoomsRepository::LoadPage(int offset, int limit) const {
    std::vector<RoomRecord> result;
    const pqxx::result query_result = transaction_.exec(
        R"(SELECT id, name, created_at FROM rooms
            ORDER BY created_at DESC
            LIMIT $1 OFFSET $2;)",
        pqxx::params(limit, offset)
        );
    for (const auto& row : query_result) {
        result.push_back(RoomRecord{
            RoomId::FromString(row[0].as<std::string>()),
            row[1].as<std::string>(),
            row[2].as<std::string>()
        });
    }
    return result;
}

void RoomsRepository::DeleteById(const RoomId &id) const
{
    transaction_.exec(
        "DELETE FROM rooms WHERE id = $1;",
        pqxx::params(id.ToString())
    );
}

// ---- MessagesRepository ----

MessagesRepository::MessagesRepository(pqxx::work &transaction) : transaction_(transaction) {}

void MessagesRepository::Save(const UserId& user_id, const RoomId& room_id, const std::string& message) const {
    auto id = MessageId::New().ToString();
    transaction_.exec(
            R"(INSERT INTO messages (id, user_id, room_id, message, sent_at) VALUES ($1, $2, $3, $4, NOW());)"_zv,
            pqxx::params(id, user_id.ToString(), room_id.ToString(), message)
    );
}

std::vector<MessageRecord> MessagesRepository::LoadRecent(const RoomId& room_id, int max_items) const {
    std::vector<MessageRecord> result;
    const pqxx::result query_result = transaction_.exec(
            R"(SELECT id, user_id, room_id, message, sent_at FROM messages
                WHERE room_id = $1
                ORDER BY sent_at DESC, id ASC LIMIT $2;)"_zv,
            pqxx::params(room_id.ToString(), max_items)
    );
    for (const auto& row : query_result) {
        result.push_back(MessageRecord{
                MessageId::FromString(row[0].as<std::string>()),
                UserId::FromString(row[1].as<std::string>()),
                RoomId::FromString(row[2].as<std::string>()),
                row[3].as<std::string>(),
                row[4].as<std::string>()
        });
    }
    return result;
}

std::vector<MessageRecord> MessagesRepository::LoadPage(const RoomId& room_id, int offset, int limit) const {
    std::vector<MessageRecord> result;
    const pqxx::result query_result = transaction_.exec(
        R"(SELECT id, user_id, room_id, message, sent_at FROM messages
            WHERE room_id = $1
            ORDER BY sent_at DESC, id ASC
            LIMIT $2 OFFSET $3;)",
        pqxx::params(room_id.ToString(), limit, offset)
        );
    for (const auto& row : query_result) {
        result.push_back(MessageRecord{
            MessageId::FromString(row[0].as<std::string>()),
            UserId::FromString(row[1].as<std::string>()),
            RoomId::FromString(row[2].as<std::string>()),
            row[3].as<std::string>(),
            row[4].as<std::string>()
        });
    }
    return result;
}

void MessagesRepository::DeleteById(const MessageId &id) const
{
    transaction_.exec(
        "DELETE FROM messages WHERE id = $1;",
        pqxx::params(id.ToString())
    );
}

// ---- RoomMembersRepository ----

RoomMembersRepository::RoomMembersRepository(pqxx::work& transaction)
        : transaction_(transaction) {}

void RoomMembersRepository::Save(const UserId& user_id, const RoomId& room_id) const {
    auto id = RoomMemberId::New().ToString();
    transaction_.exec(
            R"(INSERT INTO room_members (id, room_id, user_id, joined_at)
            VALUES ($1, $2, $3, NOW())
            ON CONFLICT DO NOTHING;)",
            pqxx::params(id, room_id.ToString(), user_id.ToString())
    );
}

void RoomMembersRepository::Remove(const UserId& user_id, const RoomId& room_id) const {
    transaction_.exec(
            R"(DELETE FROM room_members WHERE room_id = $1 AND user_id = $2;)",
            pqxx::params(room_id.ToString(), user_id.ToString())
    );
}

std::vector<RoomMemberRecord> RoomMembersRepository::LoadMembers(const RoomId& room_id) const {
    std::vector<RoomMemberRecord> result;
    const pqxx::result query_result = transaction_.exec(
            R"(SELECT id, room_id, user_id, joined_at FROM room_members WHERE room_id = $1
                    ORDER BY joined_at DESC;)",
            pqxx::params(room_id.ToString())
    );
    for (const auto& row : query_result) {
        result.push_back(RoomMemberRecord{
                RoomMemberId::FromString(row[0].as<std::string>()),
                RoomId::FromString(row[1].as<std::string>()),
                UserId::FromString(row[2].as<std::string>()),
                row[3].as<std::string>()
        });
    }
    return result;
}

std::vector<RoomMemberRecord> RoomMembersRepository::LoadRooms(const UserId& user_id) const {
    std::vector<RoomMemberRecord> result;
    const pqxx::result query_result = transaction_.exec(
            R"(SELECT id, room_id, user_id, joined_at FROM room_members WHERE user_id = $1
                    ORDER BY joined_at DESC;)",
            pqxx::params(user_id.ToString())
    );
    for (const auto& row : query_result) {
        result.push_back(RoomMemberRecord{
                RoomMemberId::FromString(row[0].as<std::string>()),
                RoomId::FromString(row[1].as<std::string>()),
                UserId::FromString(row[2].as<std::string>()),
                row[3].as<std::string>()
        });
    }
    return result;
}

// ---- Database ----

Database::Database(std::shared_ptr<ConnectionPool> pool_ptr)
        : pool_ptr_(std::move(pool_ptr)) {
    const auto conn = pool_ptr_->GetConnection();
    pqxx::work transaction{*conn};

    transaction.exec(
            R"(
            CREATE TABLE IF NOT EXISTS users (
                id UUID PRIMARY KEY,
                username VARCHAR(100) UNIQUE NOT NULL,
                password_hash VARCHAR(256) NOT NULL,
                registered_at TIMESTAMP NOT NULL DEFAULT clock_timestamp()
            );
            CREATE UNIQUE INDEX IF NOT EXISTS idx_users_username ON users(username);
            CREATE TABLE IF NOT EXISTS rooms (
                id UUID PRIMARY KEY,
                name VARCHAR(100) UNIQUE NOT NULL,
                created_at TIMESTAMP NOT NULL DEFAULT clock_timestamp()
            );
            CREATE UNIQUE INDEX IF NOT EXISTS idx_rooms_name ON rooms(name);
            CREATE INDEX IF NOT EXISTS idx_rooms_created_at ON rooms(created_at);
            CREATE TABLE IF NOT EXISTS messages (
                id UUID PRIMARY KEY,
                user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
                room_id UUID NOT NULL REFERENCES rooms(id) ON DELETE CASCADE,
                message TEXT NOT NULL,
                sent_at TIMESTAMP NOT NULL DEFAULT clock_timestamp()
            );
            CREATE INDEX IF NOT EXISTS idx_messages_room_sent_at_id ON messages(room_id, sent_at DESC, id DESC);
            CREATE INDEX IF NOT EXISTS idx_messages_user_id ON messages(user_id);
            CREATE TABLE IF NOT EXISTS room_members (
                id UUID PRIMARY KEY,
                room_id UUID NOT NULL REFERENCES rooms(id) ON DELETE CASCADE,
                user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
                joined_at TIMESTAMP NOT NULL DEFAULT clock_timestamp(),
                UNIQUE(room_id, user_id)
            );
            CREATE INDEX IF NOT EXISTS idx_room_members_room_id ON room_members(room_id);
            CREATE INDEX IF NOT EXISTS idx_room_members_user_id ON room_members(user_id);
        )"_zv
    );
    transaction.commit();
}

ConnectionPool::ConnectionWrapper Database::GetTransaction() const {
    return pool_ptr_->GetConnection();
}

} // namespace postgres
