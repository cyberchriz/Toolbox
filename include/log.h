#ifndef LOG_H
#define LOG_H

#ifdef _RELEASE
#define DEFAULT_LEVEL LogLevel::LEVEL_ERROR
#else
#define DEFAULT_LEVEL LogLevel::LEVEL_WARNING
#endif

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#ifdef _MEMLOG
#include <unordered_map>
#endif

// macro shortcuts
#define TIMER_START Log::Timer timer(LEVEL_FORCE, __FUNCTION__); // start timer
#define TIMER_ELAPSED_MS timer.elapsed_microsec()   // get elapsed time in ms
#define TIMER_STOP timer.stop();                    // stop timer and log elapsed time (since start or last stop)
#define TIMER_RESTART timer.restart();              // restart timer

enum LogLevel {
	LEVEL_ERROR,
	LEVEL_WARNING,
	LEVEL_INFO,
	LEVEL_DEBUG,
	LEVEL_FORCE,
	LEVEL_SILENT
};

// +-----------------------------------+
// |  Log class declaration            |
// +-----------------------------------+
class Log {
public:
	template <typename... Args> static void error(Args&&... args);
	template <typename... Args> static void warning(Args&&... args);
	template <typename... Args> static void info(Args&&... args);
	template <typename... Args> static void debug(Args&&... args);
	template <typename... Args> static void force(Args&&... args);
	template <typename... Args> static void log(LogLevel level, Args&&... args);
	static void set_level(LogLevel level);
	static void set_filepath(const std::string& filepath);
	static void to_console(bool active = true);
	static void to_file(bool active = true);
	static LogLevel get_level();
	static void enable_exit_on_error(bool active = true);

	class Timer {
	public:
		Timer(LogLevel level = LEVEL_FORCE, std::string caller_function = "");
		~Timer();
		void stop();
		void restart();
		double elapsed_sec();
	private:
		LogLevel log_level;
		std::chrono::high_resolution_clock::time_point begin, end;
		bool stopped = false;
		std::string caller_function = "";
	};

private:
	Log() {}
	~Log() {}
	static void write_log(std::string log_message);
	static LogLevel log_level;
	static bool log_to_console;
	static bool log_to_file;
	static bool exit_on_error;
	static std::string log_filepath;
	template <typename Arg> static void concatArgs(std::stringstream& stream, Arg&& arg);
	template <typename First, typename... Args> static void concatArgs(std::stringstream& stream, First&& first, Args&&... args);
};


// +-----------------------------------+
// |  Definitions of Log members       |
// +-----------------------------------+

template <typename... Args>
static void Log::error(Args&&... args) {
	if (log_level == LogLevel::LEVEL_SILENT) { return; }
	std::stringstream stream;
	concatArgs(stream, std::forward<Args>(args)...);
	std::string log_message = "[ERROR]:   \033[31m" + stream.str() + "\033[0m"; // red
	write_log(log_message);
	if (exit_on_error) {
		exit(EXIT_FAILURE);
	}
	else {
		throw std::runtime_error(log_message);
	}
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
		std::string log_message = "[DEBUG]:   \033[34m" + stream.str() + "\033[0m"; // blue
		write_log(log_message);
	}
}

template <typename... Args>
static void Log::force(Args&&... args) {
	std::stringstream stream;
	concatArgs(stream, std::forward<Args>(args)...);
	std::string log_message = stream.str();
	write_log(log_message);
}

template <typename... Args>
static void Log::log(LogLevel level, Args&&... args) {
	switch (level) {
	case LEVEL_ERROR:	Log::error(std::forward<Args>(args)...);	break;
	case LEVEL_WARNING: Log::warning(std::forward<Args>(args)...);	break;
	case LEVEL_INFO:	Log::info(std::forward<Args>(args)...);		break;
	case LEVEL_DEBUG:	Log::debug(std::forward<Args>(args)...);	break;
	case LEVEL_FORCE:	Log::force(std::forward<Args>(args)...);	break;
	case LEVEL_SILENT:	/* do nothing */							break;
	default:			Log::force(std::forward<Args>(args)...);
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

void Log::enable_exit_on_error(bool active) {
	exit_on_error = active;
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
bool Log::exit_on_error = true;
std::string Log::log_filepath = "../logs/";


// +-----------------------------------+
// |  Definitions of Log::Timer members|
// +-----------------------------------+
double Log::Timer::elapsed_sec() {
	end = std::chrono::high_resolution_clock::now();
	return (std::chrono::duration_cast<std::chrono::duration<double>>(end - begin)).count();
}

// constructor
Log::Timer::Timer(LogLevel level, std::string caller_function) : caller_function(caller_function), log_level(level) {
	begin = std::chrono::high_resolution_clock::now();
	if (caller_function == "") {
		Log::log(log_level, "timer started");
	}
	else {
		Log::log(log_level, "timer started in scope ", caller_function);
	}
}

void Log::Timer::stop() {
	double elapsed = elapsed_sec();
	if (elapsed > 60) {
		Log::log(log_level, "timer in scope ", caller_function == "" ? "<unknown>" : caller_function,
			" stopped after ", elapsed / 60.0, " minutes");
	}
	else if (elapsed > 0.01) {
		Log::log(log_level, "timer in scope ", caller_function == "" ? "<unknown>" : caller_function,
			" stopped after ", elapsed, " seconds");
	}
	else {
		// convert to milliseconds
		elapsed *= 1000;
		if (elapsed > 0.01) {
			Log::log(log_level, "timer in scope ", caller_function == "" ? "<unknown>" : caller_function,
				" stopped after ", elapsed, " msec");
		}
		else {
			// convert to microseconds
			elapsed *= 1000;
			if (elapsed > 0.01) {
				Log::log(log_level, "timer in scope ", caller_function == "" ? "<unknown>" : caller_function,
					" stopped after ", elapsed, " µsec");
			}
			else {
				// convert to nanoseconds
				elapsed *= 1000;
				Log::log(log_level, "timer in scope ", caller_function == "" ? "<unknown>" : caller_function,
					" stopped after ", elapsed, " nanosec");
			}
		}
	}
	stopped = true;
	begin = std::chrono::high_resolution_clock::now();
}

void Log::Timer::restart() {
	begin = std::chrono::high_resolution_clock::now();
	stopped = false;
}

// destructor
Log::Timer::~Timer() {
	// automatically trigger the stop() method when the Timer object goes out of scope
	if (!stopped) { stop(); }
}


// +-----------------------------------+
// | MEMLOG Macro                      |
// +-----------------------------------+

// this code logs any heap memory allocations to the console
// by overriding the `new` and 'delete' operators;
// in order to use this, simply use a "#define _MEMLOG" flag as a preprocessor directive before(!) including this file
#ifdef _MEMLOG

// global variables
std::unordered_map<void*, std::size_t> allocated_memory;
int total_allocation = 0;

// operator 'new' override
void* operator new(std::size_t size) {
	void* ptr = std::malloc(size);
	allocated_memory.insert(ptr, size);
	total_allocation += size;
	std::cout << "HEAP MEMORY: allocated " << size << " bytes at address " << ptr << " [total allocation: " << total_allocation << " bytes]" << std::endl;
	return ptr;
}

// operator 'delete' override
void operator delete(void* ptr) noexcept {
	std::size_t size = allocated_memory[ptr];
	total_allocation -= size;
	std::cout << "HEAP MEMORY: freed " << size << " bytes at address " << ptr << " [total allocation: " << total_allocation << " bytes]" << std::endl;
	allocated_memory.erase(ptr);
	std::free(ptr);
}
#endif

#endif