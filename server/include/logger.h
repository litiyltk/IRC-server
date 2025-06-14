#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/attributes/current_thread_id.hpp>


// Логер для сервера
// TODO разбить на *.h/*.cpp 
namespace logger {

class Logger {
public:
    enum class Level {
        Debug, // отправка сообщений, выполнение команд
        Info, // подключение, отключение клиента, аутентификация
        Warning, // разница?
        Error, // разница?
        Fatal // ошибки сервера - критические ошибки
    };

    static Logger& Get() {
        static Logger instance; // существует в единственном экземпляре!
        return instance;
    }

    // Иницализация логирования, флаг to_file для сохранения в файл, по умолчанию - вывод в консоль
    static void Init(Level level = Logger::Level::Info, bool to_file = false, const std::string& path_to_file = "") { 
        namespace logging = boost::log;
        namespace keywords = boost::log::keywords;

        // Настройка sink: в файл или консоль
        if (to_file) {
            logging::add_file_log(
                keywords::file_name = path_to_file,
                keywords::auto_flush = true,
                keywords::format = "[%TimeStamp%] [%Severity%] [Thread %ThreadID%] %Message%"
            );
        } else {
            logging::add_console_log(
                std::cout, // потокобезопасно Boost.Log, собственные мьютексы!
                keywords::format = "[%TimeStamp%] [%Severity%] [Thread %ThreadID%] %Message%"
            );
        }

        static bool initialized = false;
        if (initialized) {
            return; // нельзя инициализовать повторно!
        }

        initialized = true;

        // Выбор уровня логирования
        logging::trivial::severity_level log_level;
        switch (level) {
            case Level::Debug:
                log_level = logging::trivial::debug;
                break;
            case Level::Info:
                log_level = logging::trivial::info;
                break;
            case Level::Warning:
                log_level = logging::trivial::warning;
                break;
            case Level::Error:
                log_level = logging::trivial::error;
                break;
            case Level::Fatal:
                log_level = logging::trivial::fatal;
                break;
            default:
                log_level = logging::trivial::info; // заглушка по умолчанию
                break;
        }

        // Выбор уровней логирования
        logging::core::get()->set_filter(logging::trivial::severity >= log_level);

        // Базовые атрибуты лога время и поток
        logging::add_common_attributes();
        logging::core::get()->add_global_attribute("ThreadID", logging::attributes::current_thread_id());
    }

    // Методы логирования по уровням

    void Debug(const std::string& message) {
        BOOST_LOG_TRIVIAL(debug) << message;
    }

    void Info(const std::string& message) {
        BOOST_LOG_TRIVIAL(info) << message;
    }

    void Warning(const std::string& message) {
        BOOST_LOG_TRIVIAL(warning) << message;
    }

    void Error(const std::string& message) {
        BOOST_LOG_TRIVIAL(error) << message;
    }

    void Fatal(const std::string& message) {
        BOOST_LOG_TRIVIAL(fatal) << message;
    }
};

} // namespace logger