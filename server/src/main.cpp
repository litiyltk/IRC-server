#include "logger_plugin.h"

#include <drogon/drogon.h>
#include <thread>

int main() {
    const unsigned num_threads = std::thread::hardware_concurrency();

    drogon::app()
        .loadConfigFile("../data/config.json") // drogon формат конфига
        .setThreadNum(num_threads);

    drogon::app().run();
}