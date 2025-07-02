# IRC Chat Database Layer (C++ / PostgreSQL)

## 📦 Описание

Модуль реализует слой работы с базой данных для IRC-чата с поддержкой PostgreSQL: пул соединений, идентификаторы на основе UUID, репозитории для пользователей, комнат, сообщений и участников.  
Вся бизнес-логика отделена от слоя хранения данных (реализация *Repository Pattern*).

---

## 🗃️ Структуры и классы

### 1. `util::Tagged`, `util::TaggedUUID`

- **Назначение:** Типобезопасные идентификаторы на основе UUID (например, `UserId`, `RoomId`).
- **Преимущество:** Исключает смешение идентификаторов разных сущностей на этапе компиляции.

---

### 2. `postgres::ConnectionPool`

- **Назначение:** Thread-safe пул соединений с PostgreSQL для работы из разных потоков.
- **Основные методы:**
  - `GetConnection()`
  - `GetConnection(std::chrono::milliseconds timeout)`

---

### 3. `UserRecord`, `RoomRecord`, `MessageRecord`, `RoomMemberRecord`

- **Назначение:** Структуры данных, описывающие сущности для работы с базой (пользователь, комната, сообщение, членство).
- **Поля:** UUID, имена, текст сообщений, временные метки и др.

---

### 4. Репозитории

Каждый репозиторий отвечает за одну сущность:

#### `UsersRepository`
- **Методы:**
  - `Save(const std::string& username, const std::string& password_hash)`
  - `std::vector<UserRecord> LoadAll() const`
  - `std::optional<UserRecord> FindByUsername(const std::string&) const`
  - `std::optional<UserRecord> FindById(const UserId&) const`
  - `void DeleteByUsername(const std::string&) const`
  - `std::vector<UserRecord> LoadPage(int offset, int limit) const;`
  - `std::unordered_map<std::string, std::string> LoadUserMap() const;`

#### `RoomsRepository`
- **Методы:**
  - `Save(const std::string&)`
  - `std::vector<RoomRecord> LoadAll() const`
  - `std::optional<RoomRecord> FindByName(const std::string&) const`
  - `void DeleteById(const RoomId&) const`
  - `std::vector<RoomRecord> LoadPage(int offset, int limit) const`

#### `MessagesRepository`
- **Методы:**
  - `Save(const UserId&, const RoomId&, const std::string&)`
  - `std::vector<MessageRecord> LoadRecent(const RoomId&, int max_items) const`
  - `std::vector<MessageRecord> LoadPage(const RoomId&, int offset, int limit) const`
  - `void DeleteById(const MessageId&) const`

#### `RoomMembersRepository`
- **Методы:**
  - `Save(const UserId&, const RoomId&)`
  - `Remove(const UserId&, const RoomId&)`
  - `LoadMembers(const RoomId&)`
  - `LoadRooms(const UserId&)`

---

### 5. `postgres::Database`

- **Назначение:** Инициализация и миграция базы данных (создание таблиц), предоставление транзакций.
- **Метод:** `GetTransaction()` — получить соединение для транзакции.

---

### 6. `IRCDBWrapper` (высокоуровневая обёртка для работы с БД)

- **Назначение:** Класс для базовых операций с БД без прямого взаимодействия с репозиториями.
- **Основные методы:**

#### Работа с пользователями
- `AddUserToDB(name, pass_hash)` — добавить пользователя.
- `GetAllUsers()` — получить map<имя, хеш>.
- `FindUserByName(username)` — найти пользователя по имени.
- `FindUserById(id)` — найти пользователя по id.
- `DeleteUserByName(username)` — удалить пользователя по имени.

#### Работа с комнатами
- `AddRoomToDB(room_name)` — создать комнату.
- `FindRoomByName(room_name)` — найти комнату по имени.
- `DeleteRoomByName(room_name)` — удалить комнату по имени.
- `GetAllRooms()` — все комнаты.
- `GetRoomsPage(offset, limit)` — пагинация комнат.

#### Участие в комнатах
- `AddUserToRoomByName(username, roomname)` — добавить пользователя в комнату по именам.
- `GetRoomMembersByName(roomname)` — получить список пользователей в комнате по её имени.
- `RemoveUserFromRoomByName(username, roomname)` — удалить пользователя из комнаты по именам.

#### Работа с сообщениями
- `AddMessage(username, roomname, text)` — добавить сообщение от пользователя в комнату.
- `GetRecentMessages(roomname, max_items)` — последние N сообщений комнаты.
- `GetRoomMessagesPage(roomname, offset, limit)` — сообщения комнаты с пагинацией.
- `DeleteMessageById(message_id)` — удалить сообщение по id.

---

## 🔄 Пагинация (постраничная выборка)

В репозиториях пользователей, комнат и сообщений доступны методы постраничной выборки (пагинации):
- `UsersRepository::LoadPage(offset, limit)`
- `RoomsRepository::LoadPage(offset, limit)`
- `MessagesRepository::LoadPage(room_id, offset, limit)`

---

## 🛠️ Примеры использования

```cpp
IRCDBWrapper db("host=localhost dbname=test_db user=test_user password=test_password");

// Пользователи
db.AddUserToDB("alice", "hash");
auto user = db.FindUserByName("alice");
db.DeleteUserByName("alice");

// Комнаты
db.AddRoomToDB("main");
auto room = db.FindRoomByName("main");
db.DeleteRoomByName("main");
auto all_rooms = db.GetAllRooms();
auto page = db.GetRoomsPage(0, 2);

// Участие в комнатах
db.AddUserToRoomByName("alice", "main");
auto members = db.GetRoomMembersByName("general");
db.RemoveUserFromRoomByName("alice", "main");

// Сообщения
db.AddMessage("alice", "main", "Hello, chat!");
auto msgs = db.GetRecentMessages("main", 5);
if (!msgs.empty()) db.DeleteMessageById(msgs[0].id);
```

---

## 🏗️ Структура таблиц

```sql
CREATE TABLE IF NOT EXISTS users (
    id UUID PRIMARY KEY,
    username VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    registered_at TIMESTAMP NOT NULL DEFAULT NOW()
);
CREATE TABLE IF NOT EXISTS rooms (
    id UUID PRIMARY KEY,
    name VARCHAR(100) UNIQUE NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW()
);
CREATE TABLE IF NOT EXISTS messages (
    id UUID PRIMARY KEY,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    room_id UUID NOT NULL REFERENCES rooms(id) ON DELETE CASCADE,
    message TEXT NOT NULL,
    sent_at TIMESTAMP NOT NULL DEFAULT NOW()
);
CREATE TABLE IF NOT EXISTS room_members (
    id UUID PRIMARY KEY,
    room_id UUID NOT NULL REFERENCES rooms(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    joined_at TIMESTAMP NOT NULL DEFAULT NOW(),
    UNIQUE(room_id, user_id)
);
```

---

## 🧪 Тестирование

- Все основные методы обёртки `IRCDBWrapper` (пользователи, комнаты, участие, сообщения) покрыты автотестами в `test_postgres.cpp`.
- Тесты учитывают особенности временных меток: для корректной сортировки между вставками используется пауза (`std::this_thread::sleep_for`).
- Используется отдельная тестовая база (Docker-контейнер легко сбрасывать между тестами).
- Перед запуском тестов база должна быть “чистой” или запускаться в отдельном контейнере.

---

## 📚 Архитектурные принципы

- **Repository Pattern:** каждый репозиторий — только для своей сущности, бизнес-логика вне.
- **Thread-safe Connection Pool:** многопоточность из коробки.
- **UUID everywhere:** идентификаторы только через типизированные UUID.
- **Хеширование паролей:** не хранить пароли в открытом виде.

---

## ⚡ Рекомендации

- Для реальных проектов используйте bcrypt/argon2 для паролей.
- Логику аутентификации выносить в отдельный сервис (auth service).
- Код легко масштабируется и thread-safe.

---

## 📎 Связанные файлы

- `server/tests/test_postgres.cpp` — unit-тесты репозиториев и базы
- `tagged.h` — шаблон типобезопасных идентификаторов
- `tagged_uuid.h` — UUID для сущностей
- `postgres.h/.cpp` — репозитории, пул соединений, Database
- `db_wrapper.h/.cpp` — обёртка для высокоуровневого доступа к пользователям ([см. отдельный README](README_db_wrapper.md))

---
