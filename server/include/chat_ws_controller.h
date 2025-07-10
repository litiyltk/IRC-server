/**
 * @file chat_ws_controller.h
 * @brief Заголовочный файл для класса ChatWebSocket, реализующего WebSocket-соединение для чата.
 * @details Определяет класс ChatWebSocket, который обрабатывает WebSocket-соединения
 *  для рассылки сообщений в чате и управления соединениями. Также предоставляет
 *  вспомогательные методы для отключения клиентов с истекшими токенами.
 */
#pragma once

#include "chat_service_plugin.h"
#include "logger_plugin.h"

#include <drogon/WebSocketController.h>
#include <json/json.h>
#include <mutex>
#include <unordered_map>

//#include <iostream>


/**
 * @class ChatWebSocket
 * @brief Класс, реализующий WebSocket-соединение для чата.
 * @details Этот класс обрабатывает входящие сообщения от клиентов, устанавливает новые
 *  соединения, обрабатывает закрытие соединений и рассылает сообщения между
 *  подключенными клиентами. Он использует мьютекс для обеспечения
 *  потокобезопасности и предоставляет интерфейс для управления WS-сессиями.
 */
class ChatWebSocket : public drogon::WebSocketController<ChatWebSocket> {
public:
    /**
     * @brief Обрабатывает входящее сообщение от клиента через WebSocket.
     * @param ws_conn Указатель на объект WebSocketConnection, представляющий соединение.
     * @param message Сообщение, полученное от клиента.
     * @param type Тип сообщения.
     */
    void handleNewMessage(const drogon::WebSocketConnectionPtr &ws_conn, std::string &&message, const drogon::WebSocketMessageType &type) override;

    /**
     * @brief Обрабатывает новое WebSocket-соединение по HTTP запросу.
     * @param req Указатель на объект HttpRequest, представляющий HTTP-запрос.
     * @param conn Указатель на объект WebSocketConnection, представляющий соединение.
     */
    void handleNewConnection(const drogon::HttpRequestPtr &req, const drogon::WebSocketConnectionPtr &conn) override;

    /**
     * @brief Обрабатывает закрытие WebSocket-соединения.
     * @param conn Указатель на объект WebSocketConnection, представляющий соединение.
     */
    void handleConnectionClosed(const drogon::WebSocketConnectionPtr &conn) override;

    /**
     * @brief Возвращает всех клиентов, подключенных по WebSocket-соединениям.
     * @return std::vector<std::string> Список имен пользователей, подключенных к WebSocket.
     */
    static std::vector<std::string> GetConnectedUsers();

    /**
     * @brief Рассылает сообщения всем клиентам в комнате отправителя.
     * @param token Токен пользователя, отправившего сообщение.
     * @param text Текст сообщения.
     */
    static void Broadcast(const std::string& token, const std::string& text);

    /**
     * @brief Принудительно разрывает соединение пользователя по имени.
     * @param username Имя пользователя, чьё WebSocket-соединение требуется разорвать.
     * @details Используется для отключения пользователей, чьи токены истекли или были отозваны.
     */
    static void ForceDisconnectByUsername(const std::string& username);

    /**
     * @brief Отключает всех пользователей, у которых токены стали недействительными.
     * @details Вызывается периодически через TokenMonitorPlugin.
     */
    static void DisconnectInvalidUsers();

    /**
     * @brief Макрос для определения списка WebSocket-путей, обрабатываемых контроллером.
     */
    WS_PATH_LIST_BEGIN
        /**
         * @brief Регистрирует путь "/ws/chat" для обработки WebSocket-соединений.
         */
        WS_PATH_ADD("/ws/chat", drogon::Get);
    WS_PATH_LIST_END

private:
    /** @brief Мьютекс для обеспечения потокобезопасности. */
    static std::mutex conn_mutex_;
    /** @brief Карта, связывающая имя пользователя с WebSocket-соединением. */
    static std::unordered_map<std::string, drogon::WebSocketConnectionPtr> connections_;
    /** @brief Карта, связывающая WebSocket-соединение с именем пользователя. */
    static std::unordered_map<drogon::WebSocketConnectionPtr, std::string> conn_to_user_;
};