/**
 * @file logger_plugin.h
 * @brief Заголовочный файл для класса LoggerPlugin, реализующего плагин для логирования событий чата.
 *
 *  Определяет класс LoggerPlugin, который предоставляет методы для логирования
 *  событий сервера, REST API и WebSocket.
 */
#pragma once

#include <drogon/plugins/Plugin.h>
#include <drogon/drogon.h>


/**
 * @class LoggerPlugin
 * @brief Плагин для логирования событий чата.
 * @details Этот плагин предоставляет методы для логирования событий сервера, REST API
 *  и WebSocket.  Он может быть настроен для вывода дополнительной информации
 *  в сообщениях логов с помощью флага verbose_.
 */
class LoggerPlugin : public drogon::Plugin<LoggerPlugin> {
public:
    // События сервера
       /**
     * @brief Инициализирует и запускает плагин.
     * @details Этот метод вызывается при запуске плагина.  Он устанавливает флаг
     *  verbose_ на основе конфигурации и выводит сообщение в лог.
     * @param config Конфигурация плагина.
     */
    void initAndStart(const Json::Value &config) override;
    /**
     * @brief Выполняет завершение работы плагина.
     * @details Этот метод вызывается при завершении работы плагина.  Он выводит
     *  сообщение в лог.
     */
    void shutdown() override;

    // События REST API
      /**
     * @brief Логирует событие REST API.
     * @param text Текст сообщения для логирования.
     */
    void LogResponse(const std::string& text);

    // События WebSocket рассылки сообщений
     /**
     * @brief Логирует событие WebSocket рассылки сообщений.
     * @param text Текст сообщения для логирования.
     */
    void LogWebSocketEvent(const std::string& text);

private:
    /** @brief Флаг для вывода дополнительной информации в сообщениях логов. */
    bool verbose_ = false;
};