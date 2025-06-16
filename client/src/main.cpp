#include "server_loader.h"
#include "command_handler.h"
#include "chat_client.h"
#include <iostream>


int main() {
    auto servers = LoadServerList("../data/servers.json");

    std::cout << "Servers:\n";
    for (size_t i = 0; i < servers.size(); ++i) {
        std::cout << i << ") " << servers[i].name << " [" << servers[i].ip << ":" << servers[i].port << "]\n";
    }

    size_t index = 0;
    std::cout << "Select server by index: ";
    std::string input;
    std::getline(std::cin, input);
    
    if (!input.empty()) {
        index = std::stoi(input);
    }

    ChatClient client(servers[index].ip, servers[index].port);

    while (true) {
        std::cout << "\nКоманды (register/login/send/list/logout/exit/create-room/join-room/leave-room/rooms/room/list-room): ";
        std::cin >> input;

        Command cmd = ParseCommand(input);
        if (!HandleCommand(cmd, client)) {
            break;
        }
    }
}
