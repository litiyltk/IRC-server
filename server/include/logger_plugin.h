#pragma once

#include <drogon/plugins/Plugin.h>
#include <drogon/drogon.h>


// Плагин для логирования событий чата
class LoggerPlugin : public drogon::Plugin<LoggerPlugin> {
public:
    // События сервера
    void initAndStart(const Json::Value &config) override;
    void shutdown() override;

    // События REST API
    void LogResponse(const std::string& text);

private:
    bool verbose_ = false; // флаг для вывода дополнительной инфо в сообщениях логов
};