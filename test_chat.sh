#!/bin/bash

declare -A tokens
declare -A ws_pids

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

  echo "Открытие WebSocket для $user"
  # WebSocket-подключение с использованием websocat
  websocat "ws://localhost:8080/ws/chat?token=${token}" >/dev/null 2>&1 &
  ws_pids["$user"]=$!
done

sleep 1

# Отправка сообщений в general от пользователей a и b
echo -e "\n=== Пользователи a и b отправляют сообщения в general ==="
for user in "a" "b"; do
  token="${tokens[$user]}"
  curl -s -X POST http://localhost:8080/api/v1/messages/send \
    -H "Authorization: Bearer $token" \
    -H "Content-Type: application/json" \
    -d "{\"text\":\"Сообщение в general от $user\"}" &
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

# Возврат пользователей a и b в general
echo -e "\n=== Возврат пользователей a и b в general ==="
for user in "a" "b"; do
  token="${tokens[$user]}"
  curl -s -X POST http://localhost:8080/api/v1/room/leave \
    -H "Authorization: Bearer $token" \
    -H "Content-Type: application/json" \
    -d '{}' &
done
wait

# Пользователи c, d, e отправляют сообщения в new_room
echo -e "\n=== Пользователи c, d, e отправляют сообщения в new_room ==="
for user in "c" "d" "e"; do
  token="${tokens[$user]}"
  curl -s -X POST http://localhost:8080/api/v1/messages/send \
    -H "Authorization: Bearer $token" \
    -H "Content-Type: application/json" \
    -d "{\"text\":\"Сообщение в new_room от $user\"}" &
done
wait

# Выход всех пользователей
echo -e "\n=== Выход всех пользователей ==="
for user in "${users[@]}"; do
  token="${tokens[$user]}"
  curl -s -X POST http://localhost:8080/api/v1/auth/logout \
    -H "Authorization: Bearer $token" &
done
wait

# Закрытие WebSocket-соединений
echo -e "\n=== Закрытие WebSocket-соединений ==="
for user in "${users[@]}"; do
  kill "${ws_pids[$user]}" 2>/dev/null
done
