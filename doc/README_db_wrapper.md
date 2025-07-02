# IRCDBWrapper — упрощённый интерфейс для работы с БД IRC-чата

## 📝 Описание

`IRCDBWrapper` — это высокоуровневая обёртка для работы с базой данных IRC-чата, позволяющая выполнять все базовые операции с БД *без прямого доступа к репозиториям и транзакциям*.  
Обеспечивает быстрый старт, простое взаимодействие и чистый API для серверной логики.

---

## 🚀 Быстрый старт

```cpp
#include "db_wrapper.h"

const char* conn_str = std::getenv("IRC_CHAT_DB_URL");
if (!conn_str) {
    throw std::runtime_error("Environment variable IRC_CHAT_DB_URL is not set!");
}

IRCDBWrapper db(conn_str);

// Добавление пользователя
auto [ok, err] = db.AddUserToDB("alice", "hashed_password");
if (!ok) std::cout << "Ошибка: " << err << std::endl;

// Получение всех пользователей (имя -> хеш)
auto users = db.GetAllUsers();
for (const auto& [name, hash] : users) {
    std::cout << name << ": " << hash << std::endl;
}
```

---

## 🛠️ Методы класса

### Пользователи

- **`AddUserToDB(const std::string& name, const std::string& pass_hash)`**  
  Добавляет пользователя (имя, хеш пароля).  
  Возвращает `{true, ""}` если успешно, `{false, "User already exists"}` — если пользователь был.

- **`std::unordered_map<std::string, std::string> GetAllUsers()`**  
  Получить всех пользователей (имя → хеш).

- **`std::optional<postgres::UserRecord> FindUserByName(const std::string& username)`**  
  Поиск пользователя по имени.

- **`std::optional<postgres::UserRecord> FindUserById(const postgres::UserId& id)`**  
  Поиск пользователя по id.

- **`bool DeleteUserByName(const std::string& username)`**  
  Удаление пользователя по имени. Возвращает `true`, если пользователь был удалён.

---

### Комнаты

- **`std::pair<bool, std::string> AddRoomToDB(const std::string& room_name)`**  
  Создание комнаты. `{false, "Room already exists"}` — если комната уже есть.

- **`std::optional<postgres::RoomRecord> FindRoomByName(const std::string& room_name)`**  
  Поиск комнаты по имени.

- **`std::pair<bool, std::string> DeleteRoomByName(const std::string& room_name)`**  
  Удаление комнаты по имени.

- **`std::vector<postgres::RoomRecord> GetAllRooms()`**  
  Получить все комнаты.

- **`std::vector<postgres::RoomRecord> GetRoomsPage(int offset, int limit)`**  
  Пагинация по списку комнат (offset, limit).

---

### Участие в комнатах

- **`std::pair<bool, std::string> AddUserToRoomByName(const std::string& username, const std::string& roomname)`**  
  Добавить пользователя в комнату по именам.

- **`std::vector<postgres::UserRecord> GetRoomMembersByName(const std::string& roomname)`**  
  Получить список всех пользователей, состоящих в комнате (по имени комнаты).

- **`std::pair<bool, std::string> RemoveUserFromRoomByName(const std::string& username, const std::string& roomname)`**  
  Удалить пользователя из комнаты по именам.

---

### Сообщения

- **`std::pair<bool, std::string> AddMessage(const std::string& username, const std::string& roomname, const std::string& text)`**  
  Добавить сообщение от пользователя в комнату.

- **`std::vector<postgres::MessageRecord> GetRecentMessages(const std::string& roomname, int max_items)`**  
  Получить последние N сообщений комнаты (новые сверху).

- **`std::vector<postgres::MessageRecord> GetRoomMessagesPage(const std::string& roomname, int offset, int limit)`**  
  Получить сообщения комнаты с пагинацией.

- **`bool DeleteMessageById(const postgres::MessageId& id)`**  
  Удалить сообщение по id.

---

## 💡 Примеры использования

```cpp
IRCDBWrapper db("host=localhost dbname=test_db user=test_user password=test_password");

// --- Пользователи ---
db.AddUserToDB("vasya", "pass_hash");
auto user = db.FindUserByName("vasya");
if (user) std::cout << user->username << std::endl;
db.DeleteUserByName("vasya");

// --- Комнаты ---
db.AddRoomToDB("main");
auto room = db.FindRoomByName("main");
db.DeleteRoomByName("main");
auto all_rooms = db.GetAllRooms();

// --- Участие в комнатах ---
db.AddUserToDB("alice", "hash");
db.AddRoomToDB("general");
db.AddUserToRoomByName("alice", "general");
auto members = db.GetRoomMembersByName("general");
db.RemoveUserFromRoomByName("alice", "general");

// --- Сообщения ---
db.AddMessage("alice", "general", "Привет, чат!");
auto msgs = db.GetRecentMessages("general", 10);
if (!msgs.empty()) std::cout << msgs[0].message << std::endl;
if (!msgs.empty()) db.DeleteMessageById(msgs[0].id);
```

---

## 📝 Примечания

- Все методы возвращают пару `{ok, error}` либо `std::optional`, чтобы обработать ошибки или отсутствие объекта.
- Для корректной сортировки по времени (например, сообщений), вставляйте новые записи в отдельных транзакциях с задержкой, если нужен разный timestamp (актуально для тестов и сортировки по времени).
- Пароли должны передаваться **в виде хеша** (например, SHA256), а не открытым текстом!
- Важно: если вы хотите, чтобы при удалении пользователя или комнаты автоматически удалялись все их сообщения — в вашей БД должны быть настроены внешние ключи с `ON DELETE CASCADE`.

---

## 🧪 Тестирование

- Для автоматизированных тестов используйте файл `test_postgres.cpp` — он покрывает все методы репозиториев и IRCDBWrapper, включая:
    - Добавление, поиск, удаление пользователей и комнат
    - Добавление/удаление пользователей в/из комнат
    - Вставку, получение и удаление сообщений
    - Пагинацию и проверку сортировки
- Для корректного тестирования сортировки между вставками данных делайте паузу (`std::this_thread::sleep_for`).

---

## 📂 Связанные файлы

- `db_wrapper.h/.cpp` — обёртка и описание класса
- `postgres.h/.cpp` — низкоуровневые репозитории и инфраструктура
- `test_postgres.cpp` — автотесты
- `README_irc_chat_db.md` — общая документация по структуре БД IRC-чата

---
