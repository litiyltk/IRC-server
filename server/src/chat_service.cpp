#include "chat_service.h"


namespace chat {

inline const std::string GENERAL_ROOM = "general"; // TODO вынести в константы

ChatService::ChatService(std::shared_ptr<IRCDBWrapper> db_wrapper,
                         std::shared_ptr<TokenManager> token_mgr)
    : db_wrapper_(std::move(db_wrapper))
    , token_manager_(std::move(token_mgr)) {
}

bool ChatService::Register(const std::string& name, const std::string& password_hash) {
    auto [ok, err] = db_wrapper_->AddUserToDB(name, password_hash);
    return ok;
}

std::optional<std::string> ChatService::Login(const std::string& name, const std::string& password_hash) {
    auto user_opt = db_wrapper_->FindUserByName(name);
    if (!user_opt || user_opt->password_hash != password_hash) {
        return std::nullopt;
    }

    // Важно!!! Если уже есть активный токен - пользователь уже залогинен. Параллельный вход невозможен.
    auto existing_token = token_manager_->GetTokenByUserId(user_opt->id);
    if (existing_token) {
        return std::nullopt;
    }

    // Создать комнату "general", если не была создана ранее
    if (!db_wrapper_->FindRoomByName(chat::GENERAL_ROOM)) {
        db_wrapper_->AddRoomToDB(chat::GENERAL_ROOM);
    }

    db_wrapper_->AddUserToRoomByName(name, chat::GENERAL_ROOM);

    std::string token = Token::GENERATOR.GenerateHEXToken();
    token_manager_->SaveToken(user_opt->id, token);
    return token;
}

bool ChatService::Logout(const std::string& token) {
    auto user_id_opt = token_manager_->GetUserIdByToken(token);
    if (!user_id_opt) {
        return false;
    }

    auto user_opt = db_wrapper_->FindUserById(user_id_opt.value());
    if (!user_opt) {
        return false;
    }

    for (const auto& room : db_wrapper_->GetAllRooms()) {
        db_wrapper_->RemoveUserFromRoomByName(user_opt->username, room.name);
    }

    token_manager_->RemoveTokenByToken(token);
    return true;
}

std::vector<std::string> ChatService::GetOnlineUserNames() const {
    std::vector<std::string> names;
    for (const auto& user_id : token_manager_->GetOnlineUserIds()) {
        auto user_opt = db_wrapper_->FindUserById(user_id);
        if (user_opt) {
            names.push_back(user_opt->username);
        }
    }
    return names;
}

bool ChatService::CreateRoom(const std::string& name) {
    auto [ok, err] = db_wrapper_->AddRoomToDB(name);
    return ok;
}

bool ChatService::JoinRoom(const std::string& token, const std::string& room_name) {
    auto user_opt = GetUserByToken(token);
    if (!user_opt) {
        return false;
    }

    for (const auto& room : db_wrapper_->GetAllRooms()) {
        db_wrapper_->RemoveUserFromRoomByName(user_opt->username, room.name);
    }

    auto [ok, err] = db_wrapper_->AddUserToRoomByName(user_opt->username, room_name);
    return ok;
}

bool ChatService::LeaveRoom(const std::string& token) {
    auto user_opt = GetUserByToken(token);
    if (!user_opt) {
        return false;
    }

    for (const auto& room : db_wrapper_->GetAllRooms()) {
        db_wrapper_->RemoveUserFromRoomByName(user_opt->username, room.name);
    }

    auto [ok, err] = db_wrapper_->AddUserToRoomByName(user_opt->username, chat::GENERAL_ROOM);
    return ok;
}

bool ChatService::HasRoom(const std::string& name) const {
    return db_wrapper_->FindRoomByName(name).has_value();
}

std::vector<std::string> ChatService::GetRoomNames() const {
    std::vector<std::string> names;
    for (const auto& room : db_wrapper_->GetAllRooms()) {
        names.push_back(room.name);
    }
    return names;
}

std::optional<std::string> ChatService::GetCurrentRoomName(const std::string& token) const {
    auto user_opt = GetUserByToken(token);
    if (!user_opt) {
        return std::nullopt;
    }

    for (const auto& room : db_wrapper_->GetAllRooms()) {
        auto members = db_wrapper_->GetRoomMembersByName(room.name);
        for (const auto& u : members) {
            if (u.username == user_opt->username) {
                return room.name;
            }
        }
    }
    
    return std::nullopt;
}

std::vector<std::string> ChatService::GetUserNamesInRoom(const std::string& room_name) const {
    std::vector<std::string> names;
    for (const auto& u : db_wrapper_->GetRoomMembersByName(room_name)) {
        names.push_back(u.username);
    }
    return names;
}

// метод для сохранения при отправке сообщения
bool ChatService::SaveMessage(const std::string& token, const std::string& text) {
    if (text.empty()) {
        return false;
    }

    auto user_opt = GetUserByToken(token);
    if (!user_opt) {
        return false;
    }

    auto room_opt = GetCurrentRoomName(token);
    if (!room_opt) {
        return false;
    }

    auto [ok, err] = db_wrapper_->AddMessage(user_opt->username, *room_opt, text);
    return ok;
}

std::vector<postgres::MessageRecord> ChatService::GetRecentMessages(const std::string& room_name, int max_items) const {
    return db_wrapper_->GetRecentMessages(room_name, max_items);
}

std::vector<postgres::MessageRecord> ChatService::GetRoomMessagesPage(const std::string& room_name, int offset, int limit) const {
    return db_wrapper_->GetRoomMessagesPage(room_name, offset, limit);
}

std::optional<postgres::UserRecord> ChatService::GetUserByToken(const std::string& token) const {
    auto user_id_opt = token_manager_->GetUserIdByToken(token);
    if (!user_id_opt) {
        return std::nullopt;
    }

    return db_wrapper_->FindUserById(user_id_opt.value());
}

std::optional<std::string> ChatService::GetTokenByUserName(const std::string& name) const {
    auto user_opt = db_wrapper_->FindUserByName(name);
    if (!user_opt) {
        return std::nullopt;
    }

    return token_manager_->GetTokenByUserId(user_opt->id);
}

std::optional<std::string> ChatService::GetUserNameById(const postgres::UserId& id) const {
    auto user_opt = db_wrapper_->FindUserById(id);
    if (user_opt) {
        return user_opt->username;
    }
    return std::nullopt;
}

}  // namespace chat
