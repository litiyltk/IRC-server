#include "token_monitor_plugin.h"


void TokenMonitorPlugin::initAndStart(const Json::Value &config) {
    if (config.isMember("cleanup_interval_seconds")) {
        cleanup_period_seconds_ = config["cleanup_interval_seconds"].asDouble();
    }
    if (config.isMember("token_timeout_minutes")) {
        token_timeout_ = std::chrono::minutes(config["token_timeout_minutes"].asInt());
    }

    drogon::app().getLoop()->runEvery(cleanup_period_seconds_, [this]() {
        auto chat_service = ChatServicePlugin::GetService();
        if (chat_service) {
            chat_service->RemoveExpiredTokens(token_timeout_);
            ChatNotifier::DisconnectInvalidUsers();
        }
    });

    //std::cout << "TokenMonitorPlugin started (cleanup every " << cleanup_period_seconds_ << "s, timeout: " << token_timeout_.count() << "min)\n";
}

void TokenMonitorPlugin::shutdown() {
    //std::cout << "TokenMonitorPlugin shutting down.\n";
}
