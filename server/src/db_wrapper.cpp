#include "db_wrapper.h"
#include <pqxx/pqxx>
#include <sstream>

IRCDBWrapper::IRCDBWrapper(const std::string& conn_str, size_t pool_size)
{
    pool_ = std::make_shared<postgres::ConnectionPool>(pool_size, [conn_str] {
        return std::make_shared<pqxx::connection>(conn_str);
    });
    postgres::Database db(pool_);
}

std::pair<bool, std::string> IRCDBWrapper::AddUserToDB(const std::string& name, const std::string& pass_hash)
try
{
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::UsersRepository repo(txn);

    if (repo.FindByUsername(name)) {
        return {false, "User already exists"};
    }

    repo.Save(name, pass_hash);
    txn.commit();
    return {true, ""};
}
catch (const std::exception& ex)
{
    return {false, ex.what()};
}

std::optional<postgres::UserRecord> IRCDBWrapper::FindUserById(const postgres::UserId &id)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::UsersRepository repo(txn);
    return repo.FindById(id);
} catch (...) {
    return std::nullopt;
}

std::optional<postgres::UserRecord> IRCDBWrapper::FindUserByName(const std::string &username)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::UsersRepository repo(txn);
    return repo.FindByUsername(username);
} catch (...) {
    return std::nullopt;
}

bool IRCDBWrapper::DeleteUserByName(const std::string &username)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::UsersRepository repo(txn);

    bool deleted = repo.DeleteByUsername(username);
    txn.commit();
    return deleted;
} catch (...) {
    return false;
}

std::unordered_map<std::string, std::string> IRCDBWrapper::GetAllUsers()
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::UsersRepository repo(txn);
    return repo.LoadUserMap();
} catch (...) {
    // Можно добавить лог
    return {};
}

std::optional<postgres::RoomRecord> IRCDBWrapper::FindRoomByName(const std::string &room_name)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::RoomsRepository repo(txn);
    return repo.FindByName(room_name);
} catch (...) {
    return std::nullopt;
}

std::pair<bool, std::string> IRCDBWrapper::AddRoomToDB(const std::string &room_name)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::RoomsRepository repo(txn);

    if (repo.FindByName(room_name)) {
        return {false, "Room already exists"};
    }

    repo.Save(room_name);
    txn.commit();
    return {true, ""};
} catch (const std::exception& ex) {
    return {false, ex.what()};
}

std::vector<postgres::RoomRecord> IRCDBWrapper::GetAllRooms()
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::RoomsRepository repo(txn);
    return repo.LoadAll();
} catch (...) {
    return {};
}

std::pair<bool, std::string> IRCDBWrapper::AddUserToRoomByName(const std::string &username, const std::string &roomname)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);

    postgres::UsersRepository users_repo(txn);
    auto user = users_repo.FindByUsername(username);
    if (!user) return {false, "User not found"};

    postgres::RoomsRepository rooms_repo(txn);
    auto room = rooms_repo.FindByName(roomname);
    if (!room) return {false, "Room not found"};

    postgres::RoomMembersRepository members_repo(txn);
    members_repo.Save(user->id, room->id);

    txn.commit();
    return {true, ""};
} catch (const std::exception& ex) {
    return {false, ex.what()};
}

std::pair<bool, std::string> IRCDBWrapper::RemoveUserFromRoomByName(const std::string &username, const std::string &roomname)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);

    postgres::UsersRepository users_repo(txn);
    auto user = users_repo.FindByUsername(username);
    if (!user) return {false, "User not found"};

    postgres::RoomsRepository rooms_repo(txn);
    auto room = rooms_repo.FindByName(roomname);
    if (!room) return {false, "Room not found"};

    postgres::RoomMembersRepository members_repo(txn);
    members_repo.Remove(user->id, room->id);

    txn.commit();
    return {true, ""};
} catch (const std::exception& ex) {
    return {false, ex.what()};
}

std::vector<postgres::RoomRecord> IRCDBWrapper::GetRoomsPage(int offset, int limit)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::RoomsRepository repo(txn);
    return repo.LoadPage(offset, limit);
} catch (...) {
    return {};
}

std::vector<postgres::UserRecord> IRCDBWrapper::GetRoomMembersByName(const std::string& roomname)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);

    postgres::RoomsRepository rooms_repo(txn);
    auto room = rooms_repo.FindByName(roomname);
    if (!room) return {};

    postgres::RoomMembersRepository members_repo(txn);
    auto members = members_repo.LoadMembers(room->id);

    postgres::UsersRepository users_repo(txn);
    std::vector<postgres::UserRecord> result;
    for (const auto& m : members) {
        auto user = users_repo.FindById(m.user_id);
        if (user) result.push_back(*user);
    }
    return result;
} catch (...) {
    return {};
}

std::pair<bool, std::string> IRCDBWrapper::DeleteRoomByName(const std::string &roomname)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::RoomsRepository repo(txn);

    auto room = repo.FindByName(roomname);
    if (!room) {
        return {false, "Room not found"};
    }

    repo.DeleteById(room->id);
    txn.commit();
    return {true, ""};
} catch (const std::exception& ex) {
    return {false, ex.what()};
}

std::pair<bool, std::string> IRCDBWrapper::AddMessage(const std::string &username, const std::string &roomname, const std::string &text)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);

    postgres::UsersRepository users_repo(txn);
    auto user = users_repo.FindByUsername(username);
    if (!user) return {false, "User not found"};

    postgres::RoomsRepository rooms_repo(txn);
    auto room = rooms_repo.FindByName(roomname);
    if (!room) return {false, "Room not found"};

    postgres::MessagesRepository messages_repo(txn);
    messages_repo.Save(user->id, room->id, text);

    txn.commit();
    return {true, ""};
} catch (const std::exception& ex) {
    return {false, ex.what()};
}

std::vector<postgres::MessageRecord> IRCDBWrapper::GetRecentMessages(const std::string &roomname, int max_items)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);

    postgres::RoomsRepository rooms_repo(txn);
    auto room = rooms_repo.FindByName(roomname);
    if (!room) return {};

    postgres::MessagesRepository messages_repo(txn);
    return messages_repo.LoadRecent(room->id, max_items);
} catch (...) {
    return {};
}

std::vector<postgres::MessageRecord> IRCDBWrapper::GetRoomMessagesPage(const std::string &roomname, int offset, int limit)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);

    postgres::RoomsRepository rooms_repo(txn);
    auto room = rooms_repo.FindByName(roomname);
    if (!room) return {};

    postgres::MessagesRepository messages_repo(txn);
    return messages_repo.LoadPage(room->id, offset, limit);
} catch (...) {
    return {};
}

bool IRCDBWrapper::DeleteMessageById(const postgres::MessageId &id)
try {
    auto conn = pool_->GetConnection();
    pqxx::work txn(*conn);
    postgres::MessagesRepository messages_repo(txn);

    messages_repo.DeleteById(id);
    txn.commit();
    return true;
} catch (...) {
    return false;
}
