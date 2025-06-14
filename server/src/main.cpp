//#include "logger.h"

#include <drogon/drogon.h>
#include <json/json.h>
#include <csignal>
#include <iostream>
#include <fstream>
#include <thread>


int main() {
    // Инициализируем логирование
    //logger::Logger::Init(logger::Logger::Level::Debug, true, "../logs/server.log");

    // Читаем конфиг
    std::ifstream configFile("../data/config.json");
    if (!configFile.is_open()) {
        std::cerr << "[SERVER] Could not open config\n";
        return 1;
    }

    Json::Value config;
    configFile >> config;

    const std::string ip = config.get("IP", "127.0.0.1").asString();
    const uint16_t port = config.get("PORT", 8080).asUInt();

    std::cout << "[SERVER] Starting server at " << ip << ":" << port << "\n";

    /*std::signal(SIGINT, [](int) {
        std::cout << "[INFO] SIGINT received, shutting down...\n";
        drogon::app().quit();
    });*/

    // Определяем число потоков по количеству доступных ядер CPU
    const unsigned num_threads = std::thread::hardware_concurrency();

    // Настраиваем сервер
    drogon::app()
        .setThreadNum(num_threads)
        .addListener(ip, port)
        .run();
}
