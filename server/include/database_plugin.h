/**
 * @file database_plugin.h
 * @brief Заголовочный файл для класса DatabasePlugin, реализующего плагин для подключения к базе данных.
 * @details Определяет класс DatabasePlugin, который отвечает за инициализацию,
 *  запуск, завершение работы и предоставление доступа к экземпляру
 *  IRCDBWrapper.
 */
#pragma once

#include "db_wrapper.h"

#include <drogon/plugins/Plugin.h>


/**
 * @class DatabasePlugin
 * @brief Плагин для управления подключением к базе данных.
 * @details Этот плагин отвечает за инициализацию подключения к базе данных,
 *  используя предоставленную строку подключения и размер пула.
 *  Он также предоставляет статический метод для доступа к экземпляру
 *  IRCDBWrapper.
 */
class DatabasePlugin : public drogon::Plugin<DatabasePlugin> {
public:
        /**
     * @brief Инициализирует и запускает плагин.
     * @details Этот метод вызывается при запуске плагина.  Он извлекает строку
     *  подключения из переменной окружения `IRC_CHAT_DB_URL`, создает
     *  экземпляр `IRCDBWrapper` и настраивает пул соединений.
     * @param config Конфигурация плагина.  В настоящее время используется
     *               только для настройки размера пула соединений.
     */
    void initAndStart(const Json::Value &config) override;
    
    /**
     * @brief Выполняет завершение работы плагина.
     * @details Этот метод вызывается при завершении работы плагина.  Он освобождает
     *  ресурсы, освобождая экземпляр `IRCDBWrapper`.
     */
    void shutdown() override;

    /**
     * @brief Получает доступ к экземпляру IRCDBWrapper.
     * @return std::shared_ptr<IRCDBWrapper> Указатель на экземпляр IRCDBWrapper.
     */
    static std::shared_ptr<IRCDBWrapper> GetDB();

private:
    /** @brief Указатель на экземпляр IRCDBWrapper. */
    static std::shared_ptr<IRCDBWrapper> db_wrapper_;
};
