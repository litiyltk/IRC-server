#include "command_handler.h"

Command ParseCommand(const std::string& input) {
    static const std::unordered_map<std::string, Command> map = {
        {"register", Command::Register},
        {"login",    Command::Login},
        {"send",     Command::Send},
        {"list",     Command::List},
        {"logout",   Command::Logout},
        {"exit",     Command::Exit},
        {"create-room", Command::CreateRoom},
        {"join-room",   Command::JoinRoom},
        {"leave-room",  Command::LeaveRoom},
        {"rooms",       Command::ListRooms},
        {"room",        Command::CurrentRoom},
        {"list-room",   Command::UsersInRoom}
    };

    auto it = map.find(input);
    return (it != map.end()) ? it->second : Command::Unknown;
}

bool HandleCommand(Command cmd, ChatClient& client) {
    using std::cout;
    using std::cin;
    using std::getline;

    switch (cmd) {
        case Command::Register: {
            std::string login, pass;
            cout << "Login: ";
            cin >> login;
            cout << "Password: ";
            cin >> pass;
            client.RegisterUser(login, pass);
            break;
        }
        case Command::Login: {
            std::string login, pass;
            cout << "Login: ";
            cin >> login;
            cout << "Password: ";
            cin >> pass;
            client.LoginUser(login, pass);
            break;
        }
        case Command::Send: {
            std::string to, text;
            cin.ignore();
            cout << "To (empty = Broadcast): ";
            getline(cin, to);
            cout << "Message: "; getline(cin, text);
            client.SendMessage(text, to);
            break;
        }
        case Command::List:
            client.GetOnlineUsers();
            break;
        case Command::Logout:
            client.LogoutUser();
            break;
        case Command::Exit:
            client.LogoutUser();
            return false;
        case Command::CreateRoom: {
            cin.ignore();
            std::string name;
            cout << "Room name: "; getline(cin, name);
            client.CreateRoom(name);
            break;
        }
        case Command::JoinRoom: {
            cin.ignore();
            std::string name;
            cout << "Room name: "; getline(cin, name);
            client.JoinRoom(name);
            break;
        }
        case Command::LeaveRoom:
            client.LeaveRoom();
            break;
        case Command::ListRooms:
            client.ListRooms();
            break;
        case Command::CurrentRoom:
            client.GetCurrentRoom();
            break;
        case Command::UsersInRoom: {
            cin.ignore();
            std::string name;
            cout << "Room name: "; getline(cin, name);
            client.GetUsersInRoom(name);
            break;
        }
        case Command::Unknown:
        default:
            std::cout << "Unknown command\n";
            break;
    }
    return true;
}
