/**
 * @file http_utils.h
 * @brief Заголовочный файл, содержащий вспомогательные функции для обработки HTTP-запросов и ответов.
 * @details Определяет набор inline-функций для упрощения создания и отправки JSON-ответов,
 *  извлечения токенов авторизации из заголовков запросов и логирования запросов и ответов.
 */
#pragma once

#include "logger_plugin.h"

#include <drogon/HttpResponse.h>
#include <json/json.h>
#include <functional>
#include <optional>
#include <string>
#include <vector>


/**
 * @namespace http_utils
 * @brief Пространство имен, содержащее вспомогательные функции для обработки HTTP-запросов и ответов.
 * @details Пространство имен `http_utils` предоставляет набор inline-функций, предназначенных для упрощения
 *  процесса создания и отправки HTTP-ответов в различных форматах (JSON, успех, ошибка), извлечения
 *  токенов авторизации из заголовков запросов, логирования действий и выполнения других
 *  вспомогательных операций, связанных с HTTP-запросами и ответами в веб-приложении.
 */
namespace http_utils {

/**
 * @brief Создает HTTP-ответ в формате JSON.
 * @param body Объект Json::Value, представляющий тело ответа.
 * @param code Код состояния HTTP (drogon::HttpStatusCode).
 * @return drogon::HttpResponsePtr Указатель на созданный HTTP-ответ.
 */
drogon::HttpResponsePtr MakeJsonResponse(const Json::Value& body, drogon::HttpStatusCode code);

/**
 * @brief Отправляет успешный HTTP-ответ с сообщением в формате JSON.
 * @param message Сообщение для включения в тело ответа.
 * @param code Код состояния HTTP (drogon::HttpStatusCode).
 * @param callback Функция обратного вызова для отправки HTTP-ответа.
 */
void RespondWithSuccess(const std::string& message,
                               drogon::HttpStatusCode code,
                               const std::function<void(const drogon::HttpResponsePtr&)>& callback);

/**
 * @brief Отправляет HTTP-ответ с ошибкой в формате JSON.
 * @param message Сообщение об ошибке для включения в тело ответа.
 * @param code Код состояния HTTP (drogon::HttpStatusCode).
 * @param callback Функция обратного вызова для отправки HTTP-ответа.
 */
void RespondWithError(const std::string& message,
                             drogon::HttpStatusCode code,
                             const std::function<void(const drogon::HttpResponsePtr&)>& callback);

/**
 * @brief Пытается извлечь токен авторизации из заголовка запроса.
 * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
 * @return std::optional<std::string> Токен авторизации, если он найден в заголовке, иначе std::nullopt.
 */
std::optional<std::string> TryExtractToken(const drogon::HttpRequestPtr& req);

/**
 * @brief Отправляет успешный HTTP-ответ с данными аутентификации (имя пользователя и токен) в формате JSON.
 * @param username Имя пользователя.
 * @param token Токен авторизации.
 * @param callback Функция обратного вызова для отправки HTTP-ответа.
 */
void RespondAuthSuccess(const std::string& username,
                               const std::string& token,
                               const std::function<void(const drogon::HttpResponsePtr&)>& callback);

/**
 * @brief Отправляет успешный HTTP-ответ с массивом строк в формате JSON.
 * @param message Сообщение для логирования.
 * @param values Вектор строк для включения в тело ответа.
 * @param callback Функция обратного вызова для отправки HTTP-ответа.
 */
void RespondWithStringArray(const std::string& message,
                                   const std::vector<std::string>& values,
                                   const std::function<void(const drogon::HttpResponsePtr&)>& callback);

/**
 * @brief Отправляет HTTP-ответ с произвольным JSON-объектом.
 * @param body Объект Json::Value, представляющий тело ответа.
 * @param message Сообщение для логирования.
 * @param code Код состояния HTTP (drogon::HttpStatusCode).
 * @param callback Функция обратного вызова для отправки HTTP-ответа.
 */
void RespondWithJson(const Json::Value& body,
                            const std::string& message,
                            drogon::HttpStatusCode code,
                            const std::function<void(const drogon::HttpResponsePtr&)>& callback);

}  // namespace http_utils