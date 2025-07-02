#include "logger_plugin.h"


void LoggerPlugin::initAndStart(const Json::Value &config) {
    LOG_INFO << "[LoggerPlugin] Plugin started";
    if (config.isMember("verbose") && config["verbose"].asBool()) {
        verbose_ = true; // включаем вывод дополнительной инфо в сообщениях логов
        LOG_INFO << "[LoggerPlugin] Verbose mode ON";
    }
}

void LoggerPlugin::shutdown() {
    LOG_INFO << "[LoggerPlugin] Plugin shutting down";
}

void LoggerPlugin::LogResponse(const std::string& text) {
    LOG_INFO << "[Response] " << text;
}

void LoggerPlugin::LogWebSocketEvent(const std::string& text) {
    LOG_INFO << "[ChatWS] " << text;
}