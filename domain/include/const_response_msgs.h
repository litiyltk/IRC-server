#pragma once
#include <string>


namespace response_msg { //TODO дополнить и сделать замены

// REST API для аутентификации
// ошибки
inline const std::string INVALID_JSON = "Invalid JSON format";
inline const std::string EMPTY_LOGIN_OR_PASSWORD = "Empty login or password";
inline const std::string USED_ALREADY_REGISTERED = "User already registered";
// успех
inline const std::string REGISTRATION_SUCCESSFUL = "Registration successful";

// REST API для работы с пользователями: список всех клиентов

// REST API для отправки сообщения

// REST API для работы с комнатами


} // response_msg
