/**
 * @file token_monitor_plugin.h
 * @brief Плагин Drogon для централизованного мониторинга срока жизни токенов и управления WebSocket-соединениями.
 */
#pragma once

#include "chat_service_plugin.h"
#include "chat_notifier.h"

#include <drogon/drogon.h>
#include <drogon/HttpAppFramework.h>
#include <drogon/plugins/Plugin.h>
#include <chrono>

//#include <iostream>

/**
 * @class TokenMonitorPlugin
 * @brief Плагин для периодической очистки неактивных токенов и отключения соответствующих WebSocket-соединений.
 */
class TokenMonitorPlugin : public drogon::Plugin<TokenMonitorPlugin> {
public:
    /**
     * @brief Метод инициализации и запуска плагина.
     * @param config JSON-конфигурация, заданная в config.json.
     */
    void initAndStart(const Json::Value &config) override;

    /**
     * @brief Метод остановки плагина (вызывается при завершении работы приложения).
     */
    void shutdown() override;

private:
    double cleanup_period_seconds_ = 30.0;
    std::chrono::minutes token_timeout_ = std::chrono::minutes(1);
};
