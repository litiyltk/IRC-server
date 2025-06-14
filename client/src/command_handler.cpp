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

        case Command::CreateRoom:
        case Command::JoinRoom:
        case Command::LeaveRoom:
        case Command::ListRooms:
        case Command::CurrentRoom:
        case Command::UsersInRoom: {
            if (!client.IsLoggedIn()) {
                std::cout << "You are not logged in\n";
                break;
            }

            std::string url, method = "POST";
            Json::Value json;
            Json::StreamWriterBuilder writer;
            std::string resText;

            switch (cmd) {
                case Command::CreateRoom: {
                    cin.ignore();
                    std::string room;
                    cout << "Room name: "; getline(cin, room);
                    json["name"] = room;
                    url = "/api/room/create";
                    break;
                }
                case Command::JoinRoom: {
                    cin.ignore();
                    std::string room;
                    cout << "Room name: "; getline(cin, room);
                    json["name"] = room;
                    url = "/api/room/join";
                    break;
                }
                case Command::LeaveRoom:
                    url = "/api/room/leave";
                    break;
                case Command::ListRooms:
                    url = "/api/room/list";
                    method = "GET";
                    break;
                case Command::CurrentRoom:
                    url = "/api/room/current";
                    method = "GET";
                    break;
                case Command::UsersInRoom: {
                    std::string name;
                    cin.ignore();
                    cout << "Room name: "; getline(cin, name);
                    url = "/api/room/users?name=" + name;
                    method = "GET";
                    break;
                }
                default: break;
            }

            cpr::Response res;
            cpr::Header headers = {{"Authorization", "Bearer " + client.GetToken()}};
            if (method == "POST") {
                headers["Content-Type"] = "application/json";
                res = cpr::Post(cpr::Url{client.GetBaseUrl() + url},
                                headers, cpr::Body{Json::writeString(writer, json)});
            } else {
                res = cpr::Get(cpr::Url{client.GetBaseUrl() + url}, headers);
            }
            cout << res.text << "\n";
            break;
        }
        case Command::Unknown:
        default:
            std::cout << "Unknown command\n";
            break;
    }
    return true;
}