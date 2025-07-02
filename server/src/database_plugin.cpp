#include "database_plugin.h"


std::shared_ptr<IRCDBWrapper> DatabasePlugin::db_wrapper_ = nullptr;

void DatabasePlugin::initAndStart(const Json::Value &config) {
    std::string conn_str = config["conn_str"].asString();
    size_t pool_size = config.get("pool_size", 4).asUInt();
    db_wrapper_ = std::make_shared<IRCDBWrapper>(conn_str, pool_size);
}

/*void DatabasePlugin::initAndStart(const Json::Value &config) {
    // std::string conn_str = config["conn_str"].asString();
    const char* conn_str = std::getenv("IRC_CHAT_DB_URL");
    if (!conn_str) {
        throw std::runtime_error("Environment variable IRC_CHAT_DB_URL is not set!");
    }
    size_t pool_size = config.get("pool_size", 4).asUInt();
    db_wrapper_ = std::make_shared<IRCDBWrapper>(conn_str, pool_size);
}*/

void DatabasePlugin::shutdown() {
    db_wrapper_.reset();
}

std::shared_ptr<IRCDBWrapper> DatabasePlugin::GetDB() {
    return db_wrapper_;
}
