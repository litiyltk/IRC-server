#include "chat_service_plugin.h"


std::shared_ptr<chat::ChatService> ChatServicePlugin::chat_service_ = nullptr;

void ChatServicePlugin::initAndStart(const Json::Value &) {
    auto db = drogon::app().getPlugin<DatabasePlugin>()->GetDB();
    auto token_mgr = std::make_shared<chat::TokenManager>();

    chat_service_ = std::make_shared<chat::ChatService>(db, token_mgr);
}

void ChatServicePlugin::shutdown() {
    chat_service_.reset();
}

std::shared_ptr<chat::ChatService> ChatServicePlugin::GetService() {
    return chat_service_;
}