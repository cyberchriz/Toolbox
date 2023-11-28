#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

enum LogLevel {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_NONE
};

class Log {
public:
    Log(){}
    ~Log(){}
    static void error(const std::string& message);
    static void warning(const std::string& message);
    static void info(const std::string& message);
    static void debug(const std::string& message);
    static void set_level(LogLevel level);
    static void set_filepath(const std::string& filepath);
    static void enable_to_console(bool active = true);
    static void enable_to_file(bool active = true);
    static bool at_least(LogLevel level);

    template <typename... Args>
    static void log(int level, Args&&... args);

private:
    static LogLevel log_level;
    static bool log_to_console;
    static bool log_to_file;
    static std::string log_filepath;

    template <typename Arg>
    static void concatArgs(std::stringstream& stream, Arg&& arg);

    template <typename First, typename... Args>
    static void concatArgs(std::stringstream& stream, First&& first, Args&&... args);
};


// +-----------------------------------+
// |  Definitions of member functions  |
// +-----------------------------------+

void Log::error(const std::string& message) {
    log(LOG_LEVEL_ERROR, message);
}

void Log::warning(const std::string& message) {
    log(LOG_LEVEL_WARNING, message);
}

void Log::info(const std::string& message) {
    log(LOG_LEVEL_INFO, message);
}

void Log::debug(const std::string& message) {
    log(LOG_LEVEL_DEBUG, message);
}

void Log::set_level(LogLevel level) {
    log_level = level;
}

void Log::set_filepath(const std::string& filepath) {
    log_filepath = filepath;
    if (!log_filepath.empty() && log_filepath.back() != '/') {
        log_filepath += '/';
    }
    log_filepath += "log.txt";
}

void Log::enable_to_console(bool active) {
    log_to_console = active;
}

void Log::enable_to_file(bool active) {
    log_to_file = active;
}

bool Log::at_least(LogLevel level) {
    return log_level != LOG_LEVEL_NONE && log_level >= level;
}

template <typename... Args>
void Log::log(int level, Args&&... args) {
    if (level <= log_level && level < LOG_LEVEL_NONE) {
        const char* const levelStrings[] = {
            "ERROR", "WARNING", "INFO", "DEBUG"
        };
        std::stringstream stream;
        concatArgs(stream, std::forward<Args>(args)...);

        std::string log_message = "";

        // set color according to level
        if (log_level == LOG_LEVEL_DEBUG) { log_message += "\033[34m";} // blue
        if (log_level == LOG_LEVEL_INFO) { log_message += "\033[32m"; } // green
        if (log_level == LOG_LEVEL_WARNING) { log_message += "\033[33m"; } // yellow
        if (log_level == LOG_LEVEL_ERROR) { log_message += "\033[31m"; } // red

        // append message
        log_message += "[" + std::string(levelStrings[level]) + "]: " + stream.str();

        // set color back to default + add newline escape character
        log_message += "\033[0m\n";

        if (log_to_file) {
            std::ofstream file_stream(log_filepath, std::ios_base::app);
            if (file_stream.good()) {
                file_stream << log_message;
                file_stream.close();
            }
        }

        if (log_to_console && level != LogLevel::LOG_LEVEL_ERROR) {
            std::cout << log_message;
        }

        if (level == LogLevel::LOG_LEVEL_ERROR) {
            throw std::invalid_argument(log_message);
        }
    }
}

template <typename Arg>
void Log::concatArgs(std::stringstream& stream, Arg&& arg) {
    stream << std::forward<Arg>(arg);
}

template <typename First, typename... Args>
void Log::concatArgs(std::stringstream& stream, First&& first, Args&&... args) {
    stream << std::forward<First>(first);
    concatArgs(stream, std::forward<Args>(args)...);
}



// Initialization of static members (outside class)
LogLevel Log::log_level = LOG_LEVEL_WARNING;
bool Log::log_to_console = true;
bool Log::log_to_file = false;
std::string Log::log_filepath = "log.txt";

#endif