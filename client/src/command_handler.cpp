#include "command_handler.h"

Command ParseCommand(const std::string& input) {
    static const std::unordered_map<std::string, Command> map = {
        {"register", Command::Register},
        {"login",    Command::Login},
        {"send",     Command::SendMessage},
        {"upload",   Command::UploadMessage},
        {"recent",   Command::GetRecentMessages},
        {"list",     Command::List},
        {"logout",   Command::Logout},
        {"exit",     Command::Exit},
        {"create",   Command::CreateRoom},
        {"join",     Command::JoinRoom},
        {"leave",    Command::LeaveRoom},
        {"rooms",    Command::ListRooms},
        {"room",     Command::CurrentRoom},
        {"users",    Command::UsersInRoom}
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
        case Command::SendMessage: {
            std::string text;
            cin.ignore();
            cout << "Message: "; getline(cin, text);
            client.SendMessage(text); // 1. отправляем
            client.UploadMessage(text); // 2. сохраняем в БД
            break;
        }
        case Command::UploadMessage: {
            std::string text;
            cin.ignore();
            cout << "Message to save: "; getline(cin, text);
            client.UploadMessage(text);
            break;
        }
        case Command::GetRecentMessages: {
            std::string room;
            int max_items;
            cin.ignore();
            cout << "Room name: "; getline(cin, room);
            cout << "Max messages: "; cin >> max_items;
            client.GetRecentMessages(room, max_items);
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
            if (client.JoinRoom(name)) {
                client.GetRecentMessages(name, 10); // при входе в комнату получаем историю 10 последних сообщений
            }
            break;
        }
        case Command::LeaveRoom: {
            if (client.LeaveRoom()) {
                client.GetRecentMessages("general", 10); // после выхода получаем последние сообщения из общей комнаты
            }
            break;
        }
        case Command::ListRooms:
            client.ListRooms();
            break;
        case Command::CurrentRoom:
            client.GetCurrentRoom();
            break;
        case Command::UsersInRoom: {
            std::string room = client.GetCurrentRoomName();
            if (room.empty()) {
                std::cout << "You are not in any room\n";
            } else {
                client.GetUsersInRoom(room);
            }
            break;
        }
        case Command::Unknown:
        default:
            std::cout << "Unknown command\n";
            break;
    }
    return true;
}
