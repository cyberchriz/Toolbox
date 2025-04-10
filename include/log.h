#ifndef LOG_H
#define LOG_H
#pragma once

#ifdef _RELEASE
#define DEFAULT_LEVEL LogLevel::LEVEL_ERROR
#else
#define DEFAULT_LEVEL LogLevel::LEVEL_WARNING
#endif

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <sstream>
#include <string>

enum LogLevel {
    LEVEL_ERROR,
    LEVEL_WARNING,
    LEVEL_INFO,
    LEVEL_DEBUG,
    LEVEL_FORCE,
    LEVEL_SILENT
};

class Log {
public:
    template <typename... Args> static void error(Args&&... args);
    template <typename... Args> static void warning(Args&&... args);
    template <typename... Args> static void info(Args&&... args);
    template <typename... Args> static void debug(Args&&... args);
    static void set_level(LogLevel level);
    static void set_filepath(const std::string& filepath);
    static void to_console(bool active = true);
    static void to_file(bool active = true);
    static LogLevel get_level();
private:
    Log() {}
    ~Log() {}
    static void write_log(std::string log_message);
    static LogLevel log_level;
    static bool log_to_console;
    static bool log_to_file;
    static std::string log_filepath;
    template <typename Arg> static void concatArgs(std::stringstream& stream, Arg&& arg);
    template <typename First, typename... Args> static void concatArgs(std::stringstream& stream, First&& first, Args&&... args);
};


// +-----------------------------------+
// |  Definitions of member functions  |
// +-----------------------------------+

template <typename... Args>
static void Log::error(Args&&... args) {
    if (log_level == LogLevel::LEVEL_SILENT) { return; }
    std::stringstream stream;
    concatArgs(stream, std::forward<Args>(args)...);
    std::string log_message = "[ERROR]:   \033[31m" + stream.str() + "\033[0m"; // red
    write_log(log_message);
    exit(EXIT_FAILURE);
}

template <typename... Args>
static void Log::warning(Args&&... args) {
    if (log_level == LogLevel::LEVEL_SILENT) { return; }
    else if (log_level >= LogLevel::LEVEL_WARNING) {
        std::stringstream stream;
        concatArgs(stream, std::forward<Args>(args)...);
        std::string log_message = "[WARNING]: \033[33m" + stream.str() + "\033[0m"; // yellow
        write_log(log_message);
    }
}

template <typename... Args>
static void Log::info(Args&&... args) {
    if (log_level == LogLevel::LEVEL_SILENT) { return; }
    else if (log_level >= LogLevel::LEVEL_INFO) {
        std::stringstream stream;
        concatArgs(stream, std::forward<Args>(args)...);
        std::string log_message = "[INFO]:    \033[32m" + stream.str() + "\033[0m"; // green
        write_log(log_message);
    }
}

template <typename... Args>
static void Log::debug(Args&&... args) {
    if (log_level == LogLevel::LEVEL_SILENT) { return; }
    else if (log_level >= LogLevel::LEVEL_DEBUG) {
        std::stringstream stream;
        concatArgs(stream, std::forward<Args>(args)...);
        std::string log_message = "[ERROR]:   \033[34m" + stream.str() + "\033[0m"; // blue
        write_log(log_message);
    }
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

void Log::to_console(bool active) {
    log_to_console = active;
}

void Log::to_file(bool active) {
    log_to_file = active;
}

void Log::write_log(std::string log_message) {
    if (log_to_file) {
        std::ofstream file_stream(log_filepath, std::ios_base::app);
        if (file_stream.good()) {
            file_stream << log_message << std::endl;
            file_stream.close();
        }
        else {
            #ifndef _RELEASE
            std::cout << "unable to open log file" << std::endl;
            #endif
        }
    }

    if (log_to_console) {
        std::cout << log_message << std::endl;
    }
}

LogLevel Log::get_level() {
    return log_level;
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
LogLevel Log::log_level = DEFAULT_LEVEL;
bool Log::log_to_console = true;
bool Log::log_to_file = false;
std::string Log::log_filepath = "../logs/";

#endif