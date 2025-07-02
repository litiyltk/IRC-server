# IRC-Chat  
**IRC-подобный мини-чат с комнатами, авторизацией и API на C++**

---

## Запросы к API

#### Ping сервера

```bash
curl -X GET http://localhost:8080/api/v1/ping
```

Успешный ответ  
**HTTP 200**
```json
{
  "info":"Ping success"
}
```

### REST API для аутентификации

#### Регистрация
```bash
curl -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{"login":"alice", "password":"1234"}'
```

Успешная регистрация  
**HTTP 201 Created**
```json
{
  "info":"Registration successful: alice"
}
```

Пользователь уже существует  
**HTTP 409 Conflict**
```json
{
  "error":"User alice already registered"
}
```

Невалидный json
**HTTP 400 Bad Request**
```json
{
  "error":"Invalid JSON format"
}
```

При отсутствии имени или пароля
**HTTP 400 Bad Request**
```json
{
  "error":"Empty login or password"
}
```

#### Вход (выдает новый токен)
```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"alice", "password":"1234"}'
```

Успешный вход
**HTTP 200**
```json
{
  "token":"96b1f594eb48c7026d90a1601eca4ce7",
  "user": "alice"
}
```

Невалидный json
**HTTP 400 Bad Request**
```json
{
  "error":"Invalid JSON format"
}
```

При отсутствии регистрации пользователя, либо при уже выполненном входе
**HTTP 401 Unauthorized**
```json
{
  "error":"User is not registered or User already logged in"
}
```

#### Выход (токен аннулируется)
```bash
curl -X POST http://localhost:8080/api/v1/auth/logout \
  -H "Authorization: Bearer 96b1f594eb48c7026d90a1601eca4ce7"
```

Успешный выход
**HTTP 200**
```json
{
  "info":"Logged out"
}
```

---

### REST API для работы с пользователями

#### Получить список всех подключенных клиентов WebSocket
```bash
curl -X GET http://localhost:8080/api/v1/users/online \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a"
```

Возврат массива json при успешном запросе
**HTTP 200**
```json
[
  "alice",
  "bob"
]
```

---

### REST API для работы с комнатами

#### Создать новую комнату
```bash
curl -X POST http://localhost:8080/api/v1/room/create \
  -H "Authorization: Bearer 536942b7d3c7b5a91fa34e8f4b24439a" \
  -H "Content-Type: application/json" \
  -d '{"name":"new_room"}'
```

Возврат при успешном запросе
**HTTP 201 Created**
```json
{
  "info":"Room created"
}
```

Комната уже существует  
**HTTP 409 Conflict**
```json
{
  "error":"Room already created"
}
```

Невалидный json
**HTTP 400 Bad Request**
```json
{
  "error":"Invalid JSON format"
}
```

При отсутствии имени комнаты
**HTTP 400 Bad Request**
```json
{
  "error":"Empty room name"
}
```

#### Присоединиться к комнате чата
```bash
curl -X POST http://localhost:8080/api/v1/room/join \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a" \
  -H "Content-Type: application/json" \
  -d '{"name":"new_room"}'
```

Возврат при успешном запросе перехода в "room_1" или уже этой в комнате
**HTTP 200**
```json
{
  "info": "room_1" 
}
```

Невалидный json
**HTTP 400 Bad Request**
```json
{
  "error":"Invalid JSON format"
}
```

При отсутствии имени комнаты
**HTTP 400 Bad Request**
```json
{
  "error":"Empty room name"
}
```

Комната не найдена
**HTTP 404 Not Found**
```json
{
  "error":"Invalid room join"
}
```

#### Выйти в общую комнату (перейти в комнату GENERAL_ROOM)
```bash
curl -X POST http://localhost:8080/api/v1/room/leave \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a" \
  -H "Content-Type: application/json" \
  -d '{}'
```

Возврат при успешном запросе
**HTTP 200**
```json
{
  "info": "Left to general room"
}
```

Комната general не найдена
**HTTP 404 Not Found**
```json
{
  "error":"Room general not founded"
}
```

#### Получить название текущей комнаты
```bash
curl -X GET http://localhost:8080/api/v1/room/current \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a"
```

Возврат json с название комнаты по полю room
**HTTP 200**
```json
{
  "room": "room_name"
}
```

#### Получить список всех комнат
```bash
curl -X GET http://localhost:8080/api/v1/room/list \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a"
```

Возврат массива json при успешном запросе
**HTTP 200**
```json
[
  "general",
  "new_room"
]
```

#### Получить список участников комнаты
```bash
curl -X GET "http://localhost:8080/api/v1/room/users?name=general" \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a"
```

Комната не найдена  
**HTTP 404 Not Found**
```json
{
  "error":"Room not found"
}
```

При отсутствии поля name
**HTTP 400 Bad Request**
```json
{
  "error":"Invalid parameters"
}
```

Возврат массива json при успешном запросе
**HTTP 200**
```json
[
  "alice",
  "bob"
]
```

---

### REST API для работы с сообщениями

#### Отправка сообщения (REST)
```bash
curl -X POST http://localhost:8080/api/v1/messages/send \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a" \
  -H "Content-Type: application/json" \
  -d '{"text":"Привет, чат!"}'
```

Успешная отправка
**HTTP 200**
```json
{
  "info":"Message from alice sent"
}
```

Невалидный json
**HTTP 400 Bad Request**
```json
{
  "error":"Invalid JSON format"
}
```

При отсутствии сообщения
**HTTP 400 Bad Request**
```json
{
  "error":"Empty message"
}
```

Отправка сообщения через WebSocket - пример:
```bash
wscat -c "ws://localhost:8080/chat?token=2a3d8e712bcebc2da7416dc67cf9103a"
```

#### Сохранение сообщения в БД (выполнять запрос после отправки сообщений на сервер!)
```bash
curl -X POST http://localhost:8080/api/v1/messages/upload \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a" \
  -H "Content-Type: application/json" \
  -d '{"text":"Привет, чат!"}'
```

Успешная отправка
**HTTP 200**
```json
{
  "info":"Message saved to DB"
}
```

Невалидный json
**HTTP 400 Bad Request**
```json
{
  "error":"Invalid JSON format"
}
```

При отсутствии сообщения
**HTTP 400 Bad Request**
```json
{
  "error":"Empty message"
}
```

Ошибка сохранения в БД
**HTTP 500 Internal server error**
```json
{
  "error":"Failed to save message to DB"
}
```

#### Получение последних N сообщений в комнате
```bash
curl -X GET "http://localhost:8080/api/v1/messages/recent?room=general&max_items=10" \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a"
```

Возврат массива json при успешном запросе
**HTTP 200**
```json
[
  {
    "from": "alice",
    "sent_at": "2024-06-24 20:12:45",
    "text": "Привет!"
  },
  {
    "from": "bob",
    "sent_at": "2024-06-24 20:13:10",
    "text": "Привет, Alice!"
  }
]
```

При отсутствии полей room или max_items, либо некорректное поле room
**HTTP 400 Bad Request**
```json
{
  "error":"Invalid parameters"
}
```

При некорректном поле max_items - не число
**HTTP 400 Bad Request**
```json
{
  "error":"Invalid parameter: max_items"
}
```

При некорректном поле max_items - не натуральное число
**HTTP 400 Bad Request**
```json
{
  "error":"Parameter must be positive: max_items"
}
```

Комната не найдена  
**HTTP 404 Not Found**
```json
{
  "error":"Room not found"
}
```

---

### Общие ошибки для всех запросов с авторизацией по токену

Ошибка извлечения токена 
**HTTP 401 Unauthorized**
```json
{
  "error":"Failed to extract token"
}
```

Недействительный токен, нет на сервере
**HTTP 401 Unauthorized**
```json
{
  "error":"Invalid token"
}
```

---

## Зависимости и сборка проекта

Этот проект использует [Drogon](https://github.com/drogonframework/drogon) — высокопроизводительный HTTP-фреймворк на C++. Все зависимости устанавливаются через `vcpkg`.

## Установка drogon и других библиотек

```bash
vcpkg install drogon boost-log boost-log-setup cpr jsoncpp ixwebsocket openssl
```

## Сборка

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

## Отладочная сборка, TSAN 

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_CXX_FLAGS="-fsanitize=thread -g -O1" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread"
cmake --build .
```

### Проверка

```bash
readelf -s ./Server | grep __tsan
```

# Инструкции по работе с базой данных чата (локальная сборка)

## Скрипт для создания таблиц

```bash
chmod +x setup_chat_db.sh
./setup_chat_db.sh
```

## Информация в таблицах

Подключение к базе данных:

```bash
sudo -u postgres psql -d chat_db
```

### Список пользователей с их хешами и длиной имени

```sql
SELECT id, username, password_hash, LENGTH(username) AS username_length FROM users;
```

### Список всех комнат

```sql
SELECT id, name, created_at FROM rooms;
```

### Список пользователей и их комнат

```sql
SELECT u.username, r.name AS room_name
FROM room_members rm
JOIN rooms r ON rm.room_id = r.id
JOIN users u ON rm.user_id = u.id
ORDER BY r.name, u.username;
```

Выход из psql:

```sql
\q
```

## Для удаления базы данных

```bash
sudo -u postgres dropdb chat_db
```