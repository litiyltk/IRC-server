## Запросы к API

### REST API для аутентификации

#### Регистрация
```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"login":"alice", "password":"1234"}'
```

Возможные ответы

Успешная регистрация  
**HTTP 201 Created**
```json
{
  "info":"Registration successful"
}
```

Пользователь уже существует  
**HTTP 409 Conflict**
```json
{
  "error":"User already exists"
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
  "error":"Fields 'login' and 'password' must be non-empty"
}
```

#### Вход (выдает новый токен)
```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"alice", "password":"1234"}'
```

Возможные ответы // TODO проверить и дополнить ответы

Успешный вход
**HTTP 200**
```json
{
  "token":"96b1f594eb48c7026d90a1601eca4ce7"
}
```

Невалидный json
**HTTP 400 Bad Request**
```json
{
  "error":"Missing login or password" // TODO заменить на Invalid JSON format
}
```

ответ при некорректном логине или пароле "Invalid login or password"

#### Выход (токен аннулируется)
```bash
curl -X POST http://localhost:8080/api/auth/logout \
  -H "Authorization: Bearer 96b1f594eb48c7026d90a1601eca4ce7"
```

ответ Logged out
ответ при некорректном токене Invalid token

---

### REST API для работы с пользователями

#### Получить список активных участников
```bash
curl -X GET http://localhost:8080/api/users/online \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a"
```

---

### REST API для работы с комнатами

#### Создать новую комнату
```bash
curl -X POST http://localhost:8080/api/room/create \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a" \
  -H "Content-Type: application/json" \
  -d '{"name":"new_room"}'
```
- Нельзя создать комнату с уже существующим именем

#### Присоединиться к комнате чата
```bash
curl -X POST http://localhost:8080/api/room/join \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a" \
  -H "Content-Type: application/json" \
  -d '{"name":"new_room"}'
```
- Нельзя перейти в ту же самую комнату  
- Нельзя перейти в несуществующую комнату (получаем 400 и сообщение "Room does not exist")

#### Выйти в общую комнату (перейти в "general")
```bash
curl -X POST http://localhost:8080/api/room/join \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a" \
  -H "Content-Type: application/json" \
  -d '{"name":"general"}'
```

#### Получить название текущей комнаты
```bash
curl -X GET http://localhost:8080/api/room/current \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a"
```

#### Получить список всех комнат
```bash
curl -X GET http://localhost:8080/api/room/list \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a"
```

#### Получить список участников комнаты
```bash
curl -X GET "http://localhost:8080/api/room/users?name=general" \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a"

curl -X GET "http://localhost:8080/api/room/users?name=new_room" \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a"
```
- Нельзя получить список участников несуществующей комнаты (получаем 400)

---

### REST API для отправки сообщений

#### Отправка сообщения (REST)
```bash
curl -X POST http://localhost:8080/api/messages \
  -H "Authorization: Bearer 2a3d8e712bcebc2da7416dc67cf9103a" \
  -H "Content-Type: application/json" \
  -d '{"text":"Привет, чат!", "to":""}'
```

#### Отправка сообщения через WebSocket
```bash
wscat -c "ws://localhost:8080/chat?token=2a3d8e712bcebc2da7416dc67cf9103a"
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