#pragma once

#include "chat_ws_controller.h"


/**
 * @brief Прослойка для REST-контроллеров, предоставляющая доступ к уведомлениям пользователей.
 * @details Внутри работает через зарегистрированную реализацию, по умолчанию — WebSocket.
 */
class ChatNotifier {
public:
    /**
     * @brief Рассылает сообщение всем пользователям в комнате.
     * @param token Токен аутентификации пользователя.
     * @param message Сообщение, полученное от клиента.
     */
    static void Broadcast(const std::string& token, const std::string& message);

    /**
     * @brief Возвращает список подключённых пользователей.
     * @return std::vector<std::string> Список подключённых пользователей.
     */
    static std::vector<std::string> GetConnectedUsers();

    /**
     * @brief Принудительно отключает пользователей с недействительными токенами.
     * @details Вызывается, например, при периодической проверке активности токенов.
     */
    static void DisconnectInvalidUsers();
};
