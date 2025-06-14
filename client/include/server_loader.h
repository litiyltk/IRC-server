#pragma once

#include <json/json.h>
#include <fstream>
#include <string>
#include <vector>


struct ServerInfo {
    std::string name;
    std::string ip;
    uint16_t port;
};

std::vector<ServerInfo> LoadServerList(const std::string& path);