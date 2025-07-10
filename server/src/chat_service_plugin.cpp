#include "chat_service_plugin.h"


chat::ChatService* ChatServicePlugin::chat_service_ = nullptr;

/*namespace {
    constexpr double TOKEN_CLEANUP_INTERVAL_SECONDS = 30.0;
    constexpr std::chrono::minutes TOKEN_TIMEOUT = std::chrono::minutes(1);
}*/

void ChatServicePlugin::initAndStart(const Json::Value &) {
    auto db = drogon::app().getPlugin<DatabasePlugin>()->GetDB();

    // Создаём ChatService как static локальный объект, живущий весь runtime.
    static chat::ChatService service(*db);
    chat_service_ = &service;

    // Очистка просроченных токенов каждые TOKEN_CLEANUP_INTERVAL_SECONDS
    /*drogon::app().getLoop()->runEvery(TOKEN_CLEANUP_INTERVAL_SECONDS, []() {
        if (chat_service_) {
            chat_service_->RemoveExpiredTokens(TOKEN_TIMEOUT);
        }
    });*/

}

void ChatServicePlugin::shutdown() {
    chat_service_ = nullptr;
}

chat::ChatService* ChatServicePlugin::GetService() {
    return chat_service_;
}