/**
 * @file tagged.h
 * @brief Заголовочный файл, содержащий общие утилиты и шаблоны классов.
 * @details Определяет пространство имен `util` с шаблоном класса
 * `Tagged` и структурой `TaggedHasher`.
 */
#pragma once

#include <compare>


/**
 * @namespace util
 * @brief Пространство имен, содержащее общие утилиты и шаблоны классов.
 * @details Пространство имен `util` предоставляет набор шаблонов классов и функций,
 *  которые могут быть использованы в различных частях приложения для решения
 *  общих задач, таких как добавление тегов к значениям и хеширование.
 */
namespace util
{

    /**
     * @class Tagged
     * @brief Шаблон класса для добавления тега к значению.
     * @details Позволяет связать значение определенного типа с тегом (типом),
     *  что может быть полезно для различения значений одного и того же
     *  типа в разных контекстах.
     * @tparam Value Тип значения.
     * @tparam Tag Тип тега.
     */
    template <typename Value, typename Tag>
    class Tagged
    {
    public:
        /** @brief Тип значения. */
        using ValueType = Value;
        /** @brief Тип тега. */
        using TagType = Tag;

        /**
         * @brief Конструктор класса Tagged (перемещение).
         * @param v Значение для тегирования (передается по rvalue-ссылке).
         */
        explicit Tagged(Value &&v)
            : value_(std::move(v))
        {
        }
        /**
         * @brief Конструктор класса Tagged (копирование).
         * @param v Значение для тегирования (передается по константной ссылке).
         */
        explicit Tagged(const Value &v)
            : value_(v)
        {
        }

         /**
         * @brief Оператор разыменования (const).
         * @return Ссылка на константное значение.
         */
        const Value &operator*() const
        {
            return value_;
        }
        
         /**
         * @brief Оператор разыменования (не const).
         * @return Ссылка на значение.
         */
        Value &operator*()
        {
            return value_;
        }
        
         /**
         * @brief Оператор сравнения (default).
         * @param Другой объект Tagged для сравнения.
         * @return Результат сравнения.
         */
        auto operator<=>(const Tagged<Value, Tag> &) const = default;

    private:
        /** @brief Значение с тегом. */
        Value value_;
    };
    
      /**
     * @struct TaggedHasher
     * @brief Структура для хеширования объектов Tagged.
     * @details Предоставляет функциональность для вычисления хеш-кода объектов
     *  класса Tagged, используя стандартный хеш для типа значения.
     * @tparam TaggedValue Тип Tagged, для которого нужно вычислить хеш.
     */
    template <typename TaggedValue>
    struct TaggedHasher
    {
        /**
         * @brief Оператор вызова функции для вычисления хеша.
         * @param value Объект Tagged, для которого нужно вычислить хеш.
         * @return size_t Хеш-код объекта.
         */
        size_t operator()(const TaggedValue &value) const
        {
            return std::hash<typename TaggedValue::ValueType>{}(*value);
        }
    };

} // namespace util
