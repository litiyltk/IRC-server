/**
 * @file token_generator.h
 * @brief Заголовочный файл, содержащий классы для генерации токенов.
 * @details Определяет классы `TokenGenerator` и `Token`, используемые для генерации
 *  уникальных токенов, например, для аутентификации.
 */
#pragma once

#include <iomanip>
#include <random>
#include <sstream>
#include <string>


namespace chat
{
    /**
     * @class TokenGenerator
     * @brief Класс для генерации шестнадцатеричных токенов.
     * @details Предоставляет функциональность для создания случайных шестнадцатеричных токенов.
     */
    class TokenGenerator
    {
    public:
        /**
         * @brief Генерирует случайный шестнадцатеричный токен.
         * @return std::string Случайный шестнадцатеричный токен.
         */
        std::string GenerateHEXToken();

    private:
        /**
         * @brief Преобразует 64-битное целое число без знака в шестнадцатеричную строку.
         * @param number 64-битное целое число без знака.
         * @return std::string Шестнадцатеричное представление числа.
         */
        std::string UintToHEX(uint64_t number);

        /** @brief Генератор случайных чисел (random_device). */
        std::random_device random_device_;
        /** @brief Первый генератор случайных чисел (mersenne_twister_engine). */
        std::mt19937_64 generator1_{[this]()
                                    {
                                        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
                                        return dist(random_device_);
                                    }()};
        /** @brief Второй генератор случайных чисел (mersenne_twister_engine). */
        std::mt19937_64 generator2_{[this]()
                                    {
                                        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
                                        return dist(random_device_);
                                    }()};
    };

    /**
     * @class Token
     * @brief Класс, предоставляющий статический генератор токенов.
     */
    class Token
    {
    public:
        /** @brief Статический объект TokenGenerator.
         * @details GENERATOR.GenerateHEXToken() сгенерирует рандомный новый токен
         */
        static TokenGenerator GENERATOR;
    };

} // namespace chat
