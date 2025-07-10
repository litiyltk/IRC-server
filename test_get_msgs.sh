#!/bin/bash

echo -e "\n=== Тест некорректных параметров recent и page ==="

# Регистрация и логин одного тестового пользователя
curl -s -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{"login":"test_user", "password":"1234"}' >/dev/null

response=$(curl -s -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"test_user", "password":"1234"}')

token=$(echo "$response" | grep -oP '"token"\s*:\s*"\K[^"]+')
echo "Токен: $token"

# Создание комнаты для тестов
curl -s -X POST http://localhost:8080/api/v1/room/create \
  -H "Authorization: Bearer ${token}" \
  -H "Content-Type: application/json" \
  -d '{"name":"new_room"}' >/dev/null

# Переход в комнату
curl -s -X POST http://localhost:8080/api/v1/room/join \
  -H "Authorization: Bearer ${token}" \
  -H "Content-Type: application/json" \
  -d '{"name":"new_room"}' >/dev/null

echo -e "\n=== Некорректный max_items - не число ==="
curl -s -X GET "http://localhost:8080/api/v1/messages/recent?room=new_room&max_items=abc" \
  -H "Authorization: Bearer $token"
echo

echo -e "\n=== Некорректный max_items - отрицательное число ==="
curl -s -X GET "http://localhost:8080/api/v1/messages/recent?room=new_room&max_items=-5" \
  -H "Authorization: Bearer $token"
echo

echo -e "\n=== Некорректный offset - не число ==="
curl -s -X GET "http://localhost:8080/api/v1/messages/page?room=new_room&offset=abc&limit=10" \
  -H "Authorization: Bearer $token"
echo

echo -e "\n=== Некорректный limit - не число ==="
curl -s -X GET "http://localhost:8080/api/v1/messages/page?room=new_room&offset=0&limit=xyz" \
  -H "Authorization: Bearer $token"
echo

echo -e "\n=== Отрицательный offset ==="
curl -s -X GET "http://localhost:8080/api/v1/messages/page?room=new_room&offset=-1&limit=10" \
  -H "Authorization: Bearer $token"
echo

echo -e "\n=== Нулевой limit ==="
curl -s -X GET "http://localhost:8080/api/v1/messages/page?room=new_room&offset=0&limit=0" \
  -H "Authorization: Bearer $token"
echo

echo -e "\n=== Корректный запрос recent ==="
curl -s -X GET "http://localhost:8080/api/v1/messages/recent?room=new_room&max_items=5" \
  -H "Authorization: Bearer $token"
echo

echo -e "\n=== Корректный запрос page ==="
curl -s -X GET "http://localhost:8080/api/v1/messages/page?room=new_room&offset=0&limit=5" \
  -H "Authorization: Bearer $token"
echo

# Выход пользователя
curl -s -X POST http://localhost:8080/api/v1/auth/logout \
  -H "Authorization: Bearer $token" >/dev/null

echo -e "\n=== Logout пользователя ==="
