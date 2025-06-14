#include "token_generator.h"


std::string TokenGenerator::UintToHEX(uint64_t number) {
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << number;
    return ss.str();
}

std::string TokenGenerator::GenerateHEXToken() {
    return UintToHEX(generator1_()) + UintToHEX(generator2_());
}

TokenGenerator Token::GENERATOR;
