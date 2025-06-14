#include "server_loader.h"


std::vector<ServerInfo> LoadServerList(const std::string& path) {
    std::vector<ServerInfo> servers;

    std::ifstream file(path);
    if (!file.is_open()) {
        servers.push_back({ "Default Localhost", "127.0.0.1", 8080 });
        return servers;
    }

    Json::Value root;
    file >> root;
    for (const auto& srv : root) {
        servers.push_back({
            srv.get("name", "Unnamed").asString(),
            srv.get("ip", "127.0.0.1").asString(),
            srv.get("port", 8080).asUInt()
        });
    }

    return servers;
}
