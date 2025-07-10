#include "logger_plugin.h"


void LoggerPlugin::initAndStart(const Json::Value &config) {
    LOG_INFO << "[LoggerPlugin] Plugin started";

    std::string level_str = config.get("log_level", "info").asString();
    if (level_str == "debug")
        log_level_ = trantor::Logger::kDebug;
    else if (level_str == "warn")
        log_level_ = trantor::Logger::kWarn;
    else if (level_str == "error")
        log_level_ = trantor::Logger::kError;
    else
        log_level_ = trantor::Logger::kInfo;

    LOG_INFO << "[LoggerPlugin] Log level: " << level_str;
}

void LoggerPlugin::shutdown() {
    LOG_INFO << "[LoggerPlugin] Plugin shutting down";
}

bool LoggerPlugin::ShouldLog(trantor::Logger::LogLevel log_level) const {
    return static_cast<int>(log_level) >= static_cast<int>(log_level_);
}

void LoggerPlugin::Log(trantor::Logger::LogLevel log_level, const std::string &text) {
    if (!ShouldLog(log_level)) return;

    switch (log_level) {
        case trantor::Logger::kDebug:
            LOG_DEBUG << text;
            break;
        case trantor::Logger::kInfo:
            LOG_INFO << text;
            break;
        case trantor::Logger::kWarn:
            LOG_WARN << text;
            break;
        case trantor::Logger::kError:
            LOG_ERROR << text;
            break;
    }
}

void LoggerPlugin::LogResponse(const std::string& text) {
    Log(trantor::Logger::kInfo, "[Response] " + text);
}

void LoggerPlugin::LogWebSocketEvent(const std::string& text) {
    Log(trantor::Logger::kDebug, "[WebSocket] " + text);
}