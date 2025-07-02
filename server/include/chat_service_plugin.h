/**
 * @file chat_service_plugin.h
 * @brief Заголовочный файл для класса ChatServicePlugin, реализующего плагин для доступа к ChatService.
 *
 *  Определяет класс ChatServicePlugin, который предоставляет глобальный доступ
 *  к экземпляру ChatService.
 */
#pragma once

#include "chat_service.h"
#include "database_plugin.h"

#include <drogon/drogon.h>
#include <drogon/plugins/Plugin.h>


/**
 * @class ChatServicePlugin
 * @brief Плагин для доступа к сервису управления чатом (ChatService).
 * @details Этот плагин обеспечивает глобальный доступ к сервису `ChatService`,
 *  который управляет аутентификацией, пользователями, комнатами и сообщениями
 *  в чате. Плагин создает экземпляр `ChatService` при инициализации и
 *  предоставляет статический метод для доступа к этому экземпляру.
 */
class ChatServicePlugin : public drogon::Plugin<ChatServicePlugin> {
public:
     /**
     * @brief Инициализирует и запускает плагин.
     * @details Этот метод вызывается при запуске плагина. Он получает экземпляр
     *  `IRCDBWrapper` из `DatabasePlugin`, создает экземпляр
     *  `TokenManager` и, наконец, создает экземпляр `ChatService`.
     * @param config Конфигурация плагина (не используется в данном случае).
     */
    void initAndStart(const Json::Value &config) override;
    
    /**
     * @brief Выполняет завершение работы плагина.
     * @details Этот метод вызывается при завершении работы плагина. Он освобождает
     *  ресурсы, освобождая экземпляр `ChatService`.
     */
    void shutdown() override;

    /**
     * @brief Получает глобальный экземпляр ChatService.
     * @return std::shared_ptr<chat::ChatService> Указатель на экземпляр ChatService.
     */
    static std::shared_ptr<chat::ChatService> GetService();

private:
   /** @brief Указатель на глобальный экземпляр ChatService. */
    static std::shared_ptr<chat::ChatService> chat_service_;
};