#!/bin/bash

declare -A tokens

users=("a" "b" "c" "d" "e")

# Регистрация всех пользователей
echo -e "\n=== Регистрация всех пользователей ==="
for user in "${users[@]}"; do
  curl -s -X POST http://localhost:8080/api/v1/auth/register \
    -H "Content-Type: application/json" \
    -d "{\"login\":\"$user\", \"password\":\"1234\"}" >/dev/null
done

# Вход всех пользователей
echo -e "\n=== Логин всех пользователей ==="
for user in "${users[@]}"; do
  echo "Логин пользователя $user"
  response=$(curl -s -X POST http://localhost:8080/api/v1/auth/login \
    -H "Content-Type: application/json" \
    -d "{\"login\":\"$user\", \"password\":\"1234\"}")

  token=$(echo "$response" | grep -oP '"token"\s*:\s*"\K[^"]+')
  echo "Токен для $user: $token"

  tokens["$user"]="$token"
done

# Отправка сообщений
echo -e "\n=== Отправка сообщений ==="
for user in "${users[@]}"; do
  token="${tokens[$user]}"
  curl -s -X POST http://localhost:8080/api/v1/messages/send \
    -H "Authorization: Bearer $token" \
    -H "Content-Type: application/json" \
    -d "{\"text\":\"Сообщение от $user\"}" &
done
wait

# Запрос списка всех комнат
echo -e "\n=== Запрос списка всех комнат ==="
for user in "${users[@]}"; do
  token="${tokens[$user]}"
  curl -s -X GET http://localhost:8080/api/v1/room/list \
    -H "Authorization: Bearer $token" &
done
wait

# Создание комнаты new_room пользователем a
echo -e "\n=== Создание комнаты new_room пользователем a ==="
curl -s -X POST http://localhost:8080/api/v1/room/create \
  -H "Authorization: Bearer ${tokens["a"]}" \
  -H "Content-Type: application/json" \
  -d '{"name":"new_room"}'
echo

# Переход всех пользователей в new_room
echo -e "\n=== Переход всех пользователей в new_room ==="
for user in "${users[@]}"; do
  token="${tokens[$user]}"
  curl -s -X POST http://localhost:8080/api/v1/room/join \
    -H "Authorization: Bearer $token" \
    -H "Content-Type: application/json" \
    -d '{"name":"new_room"}' &
done
wait

# Возврат пользователей a и b
echo -e "\n=== Возврат пользователей a и b в general ==="
for user in "a" "b"; do
  token="${tokens[$user]}"
  curl -s -X POST http://localhost:8080/api/v1/room/leave \
    -H "Authorization: Bearer $token" \
    -H "Content-Type: application/json" \
    -d '{}' &
done
wait

# Пользователь c смотрит список пользователей в new_room
echo -e "\n=== Пользователь c смотрит список пользователей в new_room ==="
curl -s -X GET "http://localhost:8080/api/v1/room/users?name=new_room" \
  -H "Authorization: Bearer ${tokens["c"]}"
echo

# Пользователь a смотрит список пользователей в general
echo -e "\n=== Пользователь a смотрит список пользователей в general ==="
curl -s -X GET "http://localhost:8080/api/v1/room/users?name=general" \
  -H "Authorization: Bearer ${tokens["a"]}"
echo

# Запрос комнаты каждого пользователя
echo -e "\n=== Запрос комнаты каждого пользователя ==="
for user in "${users[@]}"; do
  token="${tokens[$user]}"
  curl -s -X GET http://localhost:8080/api/v1/room/current \
    -H "Authorization: Bearer $token"
  echo
done

# Выход всех пользователей
echo -e "\n=== Выход всех пользователей ==="
for user in "${users[@]}"; do
  token="${tokens[$user]}"
  curl -s -X POST http://localhost:8080/api/v1/auth/logout \
    -H "Authorization: Bearer $token" &
done
wait
