#pragma once
#include <string_view>

namespace api {

// REST API для пинга
inline constexpr std::string_view PING = "/api/v1/ping";

// REST API для аутентификации
inline constexpr std::string_view AUTH_REGISTER = "/api/v1/auth/register";
inline constexpr std::string_view AUTH_LOGIN = "/api/v1/auth/login";
inline constexpr std::string_view AUTH_LOGOUT = "/api/v1/auth/logout";

// REST API для работы с пользователями: список всех клиентов
inline constexpr std::string_view USERS_ONLINE = "/api/v1/users/online";

// REST API для отправки сообщения
inline constexpr std::string_view MESSAGE_SEND = "/api/v1/messages/send";
inline constexpr std::string_view MESSAGE_UPLOAD = "/api/v1/messages/upload";
inline constexpr std::string_view MESSAGE_RECENT = "/api/v1/messages/recent";
inline constexpr std::string_view MESSAGE_PAGE = "/api/v1/messages/page";

// REST API для работы с комнатами
inline constexpr std::string_view ROOM_CREATE = "/api/v1/room/create";
inline constexpr std::string_view ROOM_JOIN = "/api/v1/room/join";
inline constexpr std::string_view ROOM_LEAVE = "/api/v1/room/leave";
inline constexpr std::string_view ROOM_LIST = "/api/v1/room/list";
inline constexpr std::string_view ROOM_CURRENT = "/api/v1/room/current";
inline constexpr std::string_view ROOM_USERS = "/api/v1/room/users";

} // namespace api
