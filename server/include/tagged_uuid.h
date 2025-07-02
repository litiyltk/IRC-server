/**
 * @file tagged_uuid.h
 * @brief Заголовочный файл, содержащий утилиты для работы с UUID.
 * @details Определяет пространство имен `util` с вложенным пространством имен `detail`
 *  и шаблоном класса `TaggedUUID` для работы с UUID, используя библиотеку Boost UUID.
 */
#pragma once

#include <boost/uuid/uuid.hpp>
#include <string>

#include "tagged.h"


namespace util
{

    namespace detail
    {
        /**
         * @brief Псевдоним для типа UUID, используемого из библиотеки Boost UUID.
         */
        using UUIDType = boost::uuids::uuid;
        /**
         * @brief Генерирует новый UUID.
         * @return UUIDType Новый UUID.
         */
        UUIDType NewUUID();
        /**
         * @brief Константа, представляющая нулевой UUID.
         */
        constexpr UUIDType ZeroUUID{{0}};

        /**
         * @brief Преобразует UUID в строку.
         * @param uuid UUID для преобразования.
         * @return std::string Строковое представление UUID.
         */
        std::string UUIDToString(const UUIDType &uuid);

        /**
         * @brief Преобразует строку в UUID.
         * @param str Строковое представление UUID.
         * @return UUIDType UUID, полученный из строки.
         */
        UUIDType UUIDFromString(std::string_view str);

    } // namespace detail

    /**
     * @class TaggedUUID
     * @brief Шаблон класса для представления UUID с тегом.
     * @details Наследуется от класса `Tagged`, предоставляя возможность связать UUID
     *  с определенным тегом (типом), что может быть полезно для различения UUID
     *  в разных контекстах.
     * @tparam Tag Тип тега.
     */
    template <typename Tag>
    class TaggedUUID : public Tagged<detail::UUIDType, Tag>
    {
    public:
        /** @brief Базовый класс (Tagged<detail::UUIDType, Tag>). */
        using Base = Tagged<detail::UUIDType, Tag>;
        /** @brief Конструкторы базового класса. */
        using Tagged<detail::UUIDType, Tag>::Tagged;

        /**
         * @brief Конструктор по умолчанию, создающий TaggedUUID с нулевым UUID.
         */
        TaggedUUID()
            : Base{detail::ZeroUUID}
        {
        }
        
         /**
         * @brief Создает новый TaggedUUID с новым UUID.
         * @return TaggedUUID Новый TaggedUUID.
         */
        static TaggedUUID New()
        {
            return TaggedUUID{detail::NewUUID()};
        }
        
        /**
         * @brief Создает TaggedUUID из строкового представления UUID.
         * @param uuid_as_text Строковое представление UUID.
         * @return TaggedUUID Новый TaggedUUID.
         */
        static TaggedUUID FromString(const std::string &uuid_as_text)
        {
            return TaggedUUID{detail::UUIDFromString(uuid_as_text)};
        }

        /**
         * @brief Преобразует TaggedUUID в строку.
         * @return std::string Строковое представление UUID.
         */
        std::string ToString() const
        {
            return detail::UUIDToString(**this);
        }
    };

} // namespace util
