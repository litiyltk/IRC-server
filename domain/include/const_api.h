#pragma once
#include <string_view>

namespace api {

// REST API для аутентификации
inline constexpr std::string_view AUTH_REGISTER = "/api/auth/register";
inline constexpr std::string_view AUTH_LOGIN = "/api/auth/login";
inline constexpr std::string_view AUTH_LOGOUT = "/api/auth/logout";

// REST API для работы с пользователями: список всех клиентов
inline constexpr std::string_view USERS_ONLINE = "/api/users/online";

// REST API для отправки сообщения
inline constexpr std::string_view MESSAGE_SEND = "/api/messages";

// REST API для работы с комнатами
inline constexpr std::string_view ROOM_CREATE = "/api/room/create";
inline constexpr std::string_view ROOM_JOIN = "/api/room/join";
inline constexpr std::string_view ROOM_LEAVE = "/api/room/leave";
inline constexpr std::string_view ROOM_LIST = "/api/room/list";
inline constexpr std::string_view ROOM_CURRENT = "/api/room/current";
inline constexpr std::string_view ROOM_USERS = "/api/room/users";

} // namespace api
