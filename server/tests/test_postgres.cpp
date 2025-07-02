#include <gtest/gtest.h>
#include "postgres.h"
#include "db_wrapper.h"
#include <pqxx/pqxx>

// Для каждого теста подготавливается чистая база
class PostgresRepoTest : public ::testing::Test {
protected:
    void SetUp() override {
        const char* conn_str = std::getenv("IRC_CHAT_TEST_DB_URL");
        if (!conn_str) {
            throw std::runtime_error("Environment variable IRC_CHAT_DB_URL is not set!");
        }
        pool = std::make_shared<postgres::ConnectionPool>(1, [conn_str]{
            return std::make_shared<pqxx::connection>(conn_str);
        });
        db = std::make_unique<postgres::Database>(pool);

        auto conn = db->GetTransaction();
        pqxx::work txn(*conn);
        txn.exec("DELETE FROM room_members;");
        txn.exec("DELETE FROM messages;");
        txn.exec("DELETE FROM rooms;");
        txn.exec("DELETE FROM users;");
        txn.commit();
    }

    std::shared_ptr<postgres::ConnectionPool> pool;
    std::unique_ptr<postgres::Database> db;
};

// --- UsersRepository ---
TEST_F(PostgresRepoTest, UsersRepository_SaveAndLoadAndFind) {
    auto conn = db->GetTransaction();
    pqxx::work txn(*conn);
    postgres::UsersRepository repo(txn);

    repo.Save("testuser", "hash123");
    repo.Save("second", "hash456");

    auto users = repo.LoadAll();
    EXPECT_EQ(users.size(), 2);

    auto found = repo.FindByUsername("second");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->username, "second");
    EXPECT_EQ(found->password_hash, "hash456");

    // Несуществующий
    auto not_found = repo.FindByUsername("nope");
    EXPECT_FALSE(not_found.has_value());
}

TEST_F(PostgresRepoTest, UsersRepository_LoadPage) {
    auto conn = db->GetTransaction();

    for (int i = 1; i <= 5; ++i) {
        pqxx::work txn(*conn);
        postgres::UsersRepository repo(txn);
        repo.Save("user" + std::to_string(i), "hash" + std::to_string(i));
        txn.commit();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    pqxx::work txn(*conn);
    postgres::UsersRepository repo(txn);
    auto page1 = repo.LoadPage(0, 2);
    auto page2 = repo.LoadPage(2, 2);
    auto page3 = repo.LoadPage(4, 2); // Последняя страница (1 элемент)
    txn.commit();

    EXPECT_EQ(page1.size(), 2);
    EXPECT_EQ(page2.size(), 2);
    EXPECT_EQ(page3.size(), 1);
    EXPECT_EQ(page1[0].username, "user5"); // DESC
    EXPECT_EQ(page1[1].username, "user4");
    EXPECT_EQ(page2[0].username, "user3");
    EXPECT_EQ(page2[1].username, "user2");
    EXPECT_EQ(page3[0].username, "user1");
}

TEST_F(PostgresRepoTest, UsersRepository_LoadUserMap) {
    auto conn = db->GetTransaction();
    pqxx::work txn(*conn);
    postgres::UsersRepository repo(txn);

    repo.Save("alice", "hash1");
    repo.Save("bob", "hash2");

    auto map = repo.LoadUserMap();
    EXPECT_EQ(map.size(), 2);
    EXPECT_EQ(map.at("alice"), "hash1");
    EXPECT_EQ(map.at("bob"), "hash2");
}

// --- RoomsRepository ---
TEST_F(PostgresRepoTest, RoomsRepository_SaveAndLoad) {
    auto conn = db->GetTransaction();
    {
        pqxx::work txn(*conn);
        postgres::RoomsRepository repo(txn);
        repo.Save("room1");
        txn.commit();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    {
        pqxx::work txn(*conn);
        postgres::RoomsRepository repo(txn);
        repo.Save("room2");
        auto rooms = repo.LoadAll();
        EXPECT_EQ(rooms.size(), 2);
        EXPECT_EQ(rooms[0].name, "room2"); // loadAll: ORDER BY created_at DESC
        EXPECT_EQ(rooms[1].name, "room1");
        txn.commit();
    }
}

TEST_F(PostgresRepoTest, RoomsRepository_LoadPage) {
    auto conn = db->GetTransaction();

    for (int i = 1; i <= 4; ++i) {
        pqxx::work txn(*conn);
        postgres::RoomsRepository repo(txn);
        repo.Save("room" + std::to_string(i));
        txn.commit();
    }
    pqxx::work txn(*conn);
    postgres::RoomsRepository repo(txn);
    auto page1 = repo.LoadPage(0, 2);
    auto page2 = repo.LoadPage(2, 2);
    txn.commit();

    EXPECT_EQ(page1.size(), 2);
    EXPECT_EQ(page2.size(), 2);
    EXPECT_EQ(page1[0].name, "room4");
    EXPECT_EQ(page1[1].name, "room3");
    EXPECT_EQ(page2[0].name, "room2");
    EXPECT_EQ(page2[1].name, "room1");
}

// --- MessagesRepository ---
TEST_F(PostgresRepoTest, MessagesRepository_SaveAndLoadRecent) {
    std::optional<postgres::UserRecord> user;
    postgres::RoomRecord room;
    auto conn = db->GetTransaction();
    // Для сообщения нужен пользователь и комната
    {
        pqxx::work txn(*conn);
        postgres::UsersRepository users(txn);
        users.Save("msguser", "phash");
        user = users.FindByUsername("msguser");
        ASSERT_TRUE(user.has_value());
        txn.commit();
    }
    {
        pqxx::work txn(*conn);
        postgres::RoomsRepository rooms(txn);
        rooms.Save("msgroom");
        room = rooms.LoadAll().front();
        txn.commit();
    }
    {
        pqxx::work txn(*conn);
        postgres::MessagesRepository msgs(txn);
        msgs.Save(user->id, room.id, "Hello!");
        txn.commit();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    {
        pqxx::work txn(*conn);
        postgres::MessagesRepository msgs(txn);
        msgs.Save(user->id, room.id, "Second message");
        auto loaded = msgs.LoadRecent(room.id, 5);
        ASSERT_EQ(loaded.size(), 2);
        EXPECT_EQ(loaded[0].message, "Second message");
        EXPECT_EQ(loaded[1].message, "Hello!");
        // Ограничение выборки
        auto one = msgs.LoadRecent(room.id, 1);
        ASSERT_EQ(one.size(), 1);
        EXPECT_EQ(one[0].message, "Second message");
        txn.commit();
    }
}

TEST_F(PostgresRepoTest, MessagesRepository_LoadPage) {
    auto conn = db->GetTransaction();

    pqxx::work txn1(*conn);
    postgres::UsersRepository users(txn1);
    users.Save("alice", "hashA");
    auto user = users.FindByUsername("alice");
    ASSERT_TRUE(user.has_value());

    postgres::RoomsRepository rooms(txn1);
    rooms.Save("main");
    auto room = rooms.LoadAll().front();
    txn1.commit();

    for (int i = 1; i <= 6; ++i) {
        pqxx::work txn(*conn);
        postgres::MessagesRepository msgs(txn);
        msgs.Save(user->id, room.id, "msg" + std::to_string(i));
        txn.commit();
    }

    pqxx::work txn2(*conn);
    postgres::MessagesRepository msgs(txn2);
    auto page1 = msgs.LoadPage(room.id, 0, 3);
    auto page2 = msgs.LoadPage(room.id, 3, 3);
    txn2.commit();

    EXPECT_EQ(page1.size(), 3);
    EXPECT_EQ(page2.size(), 3);
    EXPECT_EQ(page1[0].message, "msg6");
    EXPECT_EQ(page1[1].message, "msg5");
    EXPECT_EQ(page1[2].message, "msg4");
    EXPECT_EQ(page2[0].message, "msg3");
    EXPECT_EQ(page2[1].message, "msg2");
    EXPECT_EQ(page2[2].message, "msg1");
}

// --- RoomMembersRepository ---
TEST_F(PostgresRepoTest, RoomMembersRepository_SaveLoadRemove) {
    auto conn = db->GetTransaction();
    pqxx::work txn(*conn);

    postgres::UsersRepository users(txn);
    users.Save("memberuser", "phash");
    auto user = users.FindByUsername("memberuser");
    ASSERT_TRUE(user.has_value());

    postgres::RoomsRepository rooms(txn);
    rooms.Save("roomA");
    auto room = rooms.LoadAll().front();

    postgres::RoomMembersRepository members(txn);

    // Добавить в комнату
    members.Save(user->id, room.id);
    auto in_room = members.LoadMembers(room.id);
    EXPECT_EQ(in_room.size(), 1);
    EXPECT_EQ(in_room[0].user_id.ToString(), user->id.ToString());

    // Повторное добавление (ON CONFLICT) — не будет дублей
    members.Save(user->id, room.id);
    auto after_conflict = members.LoadMembers(room.id);
    EXPECT_EQ(after_conflict.size(), 1);

    // Удалить из комнаты
    members.Remove(user->id, room.id);
    auto after_remove = members.LoadMembers(room.id);
    EXPECT_EQ(after_remove.size(), 0);
}

TEST_F(PostgresRepoTest, RoomMembersRepository_LoadRoomsByUser) {
    std::optional<postgres::UserRecord> alice;
    postgres::RoomRecord r1;
    postgres::RoomRecord r2;
    auto conn = db->GetTransaction();
    {
        pqxx::work txn(*conn);
        postgres::UsersRepository users(txn);

        users.Save("alice", "hash1");
        alice = users.FindByUsername("alice");
        txn.commit();
        ASSERT_TRUE(alice.has_value());
    }
    {
        pqxx::work txn(*conn);
        postgres::RoomsRepository rooms(txn);

        rooms.Save("r1");
        txn.commit();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    {
        pqxx::work txn(*conn);
        postgres::RoomsRepository rooms(txn);

        rooms.Save("r2");
        r1 = rooms.LoadAll()[1]; // r1 - старее, r2 - новее
        r2 = rooms.LoadAll()[0];
        txn.commit();
    }
    {
        pqxx::work txn(*conn);
        postgres::RoomMembersRepository members(txn);

        members.Save(alice->id, r1.id);
        txn.commit();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    {
        pqxx::work txn(*conn);
        postgres::RoomMembersRepository members(txn);

        members.Save(alice->id, r2.id);
        auto rooms_by_user = members.LoadRooms(alice->id);
        txn.commit();
        EXPECT_EQ(rooms_by_user.size(), 2);
        EXPECT_EQ(rooms_by_user[0].room_id.ToString(), r2.id.ToString());
        EXPECT_EQ(rooms_by_user[1].room_id.ToString(), r1.id.ToString());
    }
}

// --- IRCDBWrapper: AddUserToDB и GetAllUsers ---
TEST_F(PostgresRepoTest, Wrapper_AddUserToDB_And_GetAllUsers) {
    const char* conn_str = std::getenv("IRC_CHAT_TEST_DB_URL");
    ASSERT_NE(conn_str, nullptr) << "Environment variable IRC_CHAT_DB_URL is not set!";
    IRCDBWrapper wrapper(conn_str);

    auto [ok1, err1] = wrapper.AddUserToDB("vasya", "pass_111");
    EXPECT_TRUE(ok1);
    EXPECT_EQ(err1, "");

    auto [ok2, err2] = wrapper.AddUserToDB("petya", "pass_222");
    EXPECT_TRUE(ok2);

    // Попытка добавить того же пользователя
    auto [ok3, err3] = wrapper.AddUserToDB("vasya", "pass_333");
    EXPECT_FALSE(ok3);
    EXPECT_EQ(err3, "User already exists");

    auto users = wrapper.GetAllUsers();
    EXPECT_EQ(users.size(), 2);
    EXPECT_EQ(users.at("vasya"), "pass_111");
    EXPECT_EQ(users.at("petya"), "pass_222");
}

// --- IRCDBWrapper: User find by id/name, remove by name ---
TEST_F(PostgresRepoTest, Wrapper_FindUserByIdAndByName_And_DeleteUser) {
    const char* conn_str = std::getenv("IRC_CHAT_TEST_DB_URL");
    ASSERT_NE(conn_str, nullptr) << "Environment variable IRC_CHAT_DB_URL is not set!";
    IRCDBWrapper wrapper(conn_str);

    wrapper.AddUserToDB("u1", "h1");
    wrapper.AddUserToDB("u2", "h2");
    auto user = wrapper.FindUserByName("u1");
    ASSERT_TRUE(user);
    auto user_by_id = wrapper.FindUserById(user->id);
    ASSERT_TRUE(user_by_id);
    EXPECT_EQ(user_by_id->username, "u1");

    // Delete
    bool deleted = wrapper.DeleteUserByName("u1");
    EXPECT_TRUE(deleted);
    EXPECT_FALSE(wrapper.FindUserByName("u1"));
    EXPECT_FALSE(wrapper.DeleteUserByName("u1")); // Повторно — не найден
}

// --- IRCDBWrapper: Room create/find/delete, get all, page ---
TEST_F(PostgresRepoTest, Wrapper_AddRoom_FindRoom_DeleteRoom_GetAllRooms_Pagination) {
    const char* conn_str = std::getenv("IRC_CHAT_TEST_DB_URL");
    ASSERT_NE(conn_str, nullptr) << "Environment variable IRC_CHAT_DB_URL is not set!";
    IRCDBWrapper wrapper(conn_str);

    // Add two rooms
    auto [ok1, err1] = wrapper.AddRoomToDB("r1");
    EXPECT_TRUE(ok1);
    EXPECT_EQ(err1, "");
    auto [ok2, err2] = wrapper.AddRoomToDB("r2");
    EXPECT_TRUE(ok2);

    // Повтор — комната уже есть
    auto [ok3, err3] = wrapper.AddRoomToDB("r1");
    EXPECT_FALSE(ok3);

    // Поиск по имени
    auto room = wrapper.FindRoomByName("r2");
    ASSERT_TRUE(room);
    EXPECT_EQ(room->name, "r2");

    // Get all
    auto rooms = wrapper.GetAllRooms();
    EXPECT_EQ(rooms.size(), 2);

    // Пагинация
    auto page = wrapper.GetRoomsPage(0, 1);
    EXPECT_EQ(page.size(), 1);
    EXPECT_EQ(page[0].name, "r2"); // Последняя созданная (DESC)

    // Удаление
    auto [okdel, errdel] = wrapper.DeleteRoomByName("r2");
    EXPECT_TRUE(okdel);
    EXPECT_FALSE(wrapper.FindRoomByName("r2"));
}

// --- IRCDBWrapper: User add/remove to/from room by name ---
TEST_F(PostgresRepoTest, Wrapper_AddAndRemoveUserToRoomByName) {
    const char* conn_str = std::getenv("IRC_CHAT_TEST_DB_URL");
    ASSERT_NE(conn_str, nullptr) << "Environment variable IRC_CHAT_DB_URL is not set!";
    IRCDBWrapper wrapper(conn_str);
    wrapper.AddUserToDB("alice", "pa");
    wrapper.AddRoomToDB("main");

    auto [addok, adderr] = wrapper.AddUserToRoomByName("alice", "main");
    EXPECT_TRUE(addok);

    // Повтор — не вызывает ошибку (ON CONFLICT DO NOTHING)
    auto [addok2, adderr2] = wrapper.AddUserToRoomByName("alice", "main");
    EXPECT_TRUE(addok2);

    // Удаление
    auto [remok, remerr] = wrapper.RemoveUserFromRoomByName("alice", "main");
    EXPECT_TRUE(remok);

    // Повторное удаление — не ошибка (просто нет связи)
    auto [remok2, remerr2] = wrapper.RemoveUserFromRoomByName("alice", "main");
    EXPECT_TRUE(remok2);
}

// --- IRCDBWrapper: Add and get messages, pagination, delete message ---
TEST_F(PostgresRepoTest, Wrapper_AddMessage_GetMessages_DeleteMessage) {
    const char* conn_str = std::getenv("IRC_CHAT_TEST_DB_URL");
    ASSERT_NE(conn_str, nullptr) << "Environment variable IRC_CHAT_DB_URL is not set!";
    IRCDBWrapper wrapper(conn_str);
    wrapper.AddUserToDB("bob", "p1");
    wrapper.AddRoomToDB("talk");
    wrapper.AddUserToRoomByName("bob", "talk");

    // Несколько сообщений — в разных транзакциях, чтобы метки времени отличались!
    std::vector<postgres::MessageId> ids;
    for (int i = 0; i < 3; ++i) {
        auto [ok, err] = wrapper.AddMessage("bob", "talk", "msg" + std::to_string(i + 1));
        EXPECT_TRUE(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    auto msgs = wrapper.GetRecentMessages("talk", 10);
    ASSERT_EQ(msgs.size(), 3);
    EXPECT_EQ(msgs[0].message, "msg3");
    EXPECT_EQ(msgs[1].message, "msg2");
    EXPECT_EQ(msgs[2].message, "msg1");

    // Пагинация
    auto page = wrapper.GetRoomMessagesPage("talk", 1, 2);
    EXPECT_EQ(page.size(), 2);
    EXPECT_EQ(page[0].message, "msg2");
    EXPECT_EQ(page[1].message, "msg1");

    // Удаление сообщения
    auto id = msgs[1].id; // id второго сообщения (msg2)
    EXPECT_TRUE(wrapper.DeleteMessageById(id));
    auto after_del = wrapper.GetRecentMessages("talk", 10);
    ASSERT_EQ(after_del.size(), 2);
    EXPECT_EQ(after_del[0].message, "msg3");
    EXPECT_EQ(after_del[1].message, "msg1");
}

TEST_F(PostgresRepoTest, Wrapper_GetRoomMembersByName) {
    const char* conn_str = std::getenv("IRC_CHAT_TEST_DB_URL");
    ASSERT_NE(conn_str, nullptr) << "Environment variable IRC_CHAT_DB_URL is not set!";
    IRCDBWrapper wrapper(conn_str);
    wrapper.AddUserToDB("alice", "hash1");
    wrapper.AddUserToDB("bob", "hash2");
    wrapper.AddUserToDB("charlie", "hash3");
    wrapper.AddRoomToDB("general");

    wrapper.AddUserToRoomByName("alice", "general");
    wrapper.AddUserToRoomByName("bob", "general");

    auto members = wrapper.GetRoomMembersByName("general");
    ASSERT_EQ(members.size(), 2);

    std::vector<std::string> names;
    for (const auto& u : members) names.push_back(u.username);
    EXPECT_NE(std::find(names.begin(), names.end(), "alice"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "bob"), names.end());
    EXPECT_EQ(std::find(names.begin(), names.end(), "charlie"), names.end());
}


// --- TaggedUUID ---
TEST(TaggedUUIDTest, UUIDGenerateAndStringRoundtrip) {
    auto uuid = util::TaggedUUID<struct TestTag>::New();
    std::string str = uuid.ToString();
    auto from_str = util::TaggedUUID<struct TestTag>::FromString(str);
    EXPECT_EQ(uuid.ToString(), from_str.ToString());
}

TEST(TaggedUUIDTest, ZeroUUID) {
    util::TaggedUUID<struct TestTag> zero;
    EXPECT_EQ(zero.ToString(), util::detail::UUIDToString(util::detail::ZeroUUID));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
