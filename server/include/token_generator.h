#pragma once

#include <iomanip>
#include <random>
#include <sstream>
#include <string>


class TokenGenerator {
public:
    std::string GenerateHEXToken();

private:
    std::string UintToHEX(uint64_t number);

    std::random_device random_device_;
    std::mt19937_64 generator1_{[this]() {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};

    std::mt19937_64 generator2_{[this]() {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
};

class Token {
public:
    static TokenGenerator GENERATOR;
};
