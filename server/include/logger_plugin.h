/**
 * @file logger_plugin.h
 * @brief Заголовочный файл для класса LoggerPlugin, реализующего плагин для логирования событий чата.
 *
 * @details Определяет класс LoggerPlugin, который предоставляет методы для логирования
 * событий сервера, REST API и WebSocket. Плагин поддерживает фильтрацию по уровням логирования,
 * заданным через конфигурационный файл.
 */
#pragma once

#include <drogon/plugins/Plugin.h>
#include <drogon/drogon.h>
#include <string>

/**
 * @class LoggerPlugin
 * @brief Плагин для логирования событий чата.
 * @details Этот плагин предоставляет методы для логирования событий сервера, REST API
 * и WebSocket. Поддерживается настройка уровня логирования (DEBUG, INFO, WARN, ERROR),
 * позволяющая контролировать степень детализации вывода.
 */
class LoggerPlugin : public drogon::Plugin<LoggerPlugin> {
public:
    /**
     * @brief Инициализирует и запускает плагин.
     * @details Этот метод вызывается при запуске плагина. Он считывает из конфигурации
     * желаемый уровень логирования и выводит информационное сообщение.
     * @param config Конфигурация плагина.
     */
    void initAndStart(const Json::Value &config) override;

    /**
     * @brief Выполняет завершение работы плагина.
     * @details Этот метод вызывается при завершении работы плагина. Он фиксирует
     * факт завершения работы в логах.
     */
    void shutdown() override;

    /**
     * @brief Логирует произвольное сообщение с указанным уровнем.
     * @param log_level Уровень логирования (DEBUG, INFO, WARN, ERROR).
     * @param text Текст сообщения для логирования.
     */
    void Log(trantor::Logger::LogLevel log_level, const std::string &text);

    /**
     * @brief Логирует событие REST API.
     * @details Используется для фиксации информации об отправке HTTP-ответов или действиях в контроллерах.
     * @param text Текст сообщения для логирования.
     */
    void LogResponse(const std::string &text);

    /**
     * @brief Логирует событие WebSocket рассылки сообщений.
     * @details Используется для вывода информации о действиях по рассылке сообщений через WebSocket.
     * @param text Текст сообщения для логирования.
     */
    void LogWebSocketEvent(const std::string &text);

private:
    /**
     * @brief Установленный минимальный уровень логирования.
     * @details Сообщения с уровнем ниже указанного не будут выводиться.
     * Значение задаётся через конфигурацию (`log_level`: "debug", "info", "warn", "error").
     */
    trantor::Logger::LogLevel log_level_{trantor::Logger::kInfo};

    /**
     * @brief Проверяет, следует ли выводить сообщение указанного уровня.
     * @param log_level Уровень логируемого сообщения.
     * @return true, если сообщение нужно логировать; false — если уровень слишком низкий.
     */
    bool ShouldLog(trantor::Logger::LogLevel log_level) const;
};