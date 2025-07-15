#include "password_hasher.h"


std::string PasswordHasher::HashPassword(const std::string& password) {
    // Для хэша используем 256 бит = 32 байта
    std::array<unsigned char, SHA256_DIGEST_LENGTH> hash{};
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash.data());

    // Байты в HEX строку
    std::ostringstream oss;
    for (auto byte : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }

    return oss.str();
}
