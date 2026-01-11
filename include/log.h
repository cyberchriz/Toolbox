#ifndef LOG_H
#define LOG_H

#ifdef NDEBUG
#define DEFAULT_LOG_LEVEL LogLevel::LEVEL_ERROR
//#define DEBUG_LOG_IN_RELEASE						// <-- uncomment this line to enable Log::debug(...) in RELEASE
//#define HEAP_MEMLOG								// <-- uncomment this line to log memory allocations on the heap (overrides 'new' & 'delete')
#else
#define DEFAULT_LOG_LEVEL LogLevel::LEVEL_WARNING
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
#include <thread>

#ifdef _MEMLOG
#include <unordered_map>
#endif

// macro shortcuts
#define TIMER_START Log::Timer timer(LEVEL_DEBUG, __FUNCTION__);	// start timer (first start within scope)
#define TIMER_STOP timer.stop();									// stop timer and log elapsed time (since start or last stop)
#define TIMER_SILENT_STOP timer.silent_stop();						// stop the timer without logging (thus silencing output when the Timer instance goes out of scope)
#define TIMER_RESTART timer.restart();								// restart timer
#define TIMER_STOP_RESTART timer.stop(); timer.restart();			// log elapsed time and directly restart the timer
#define TIMER_ELAPSED_SEC timer.elapsed_sec()						// get elapsed time in seconds

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
	template <typename... Args> static void force(Args&&... args);
	
	// Ignore debug logs in RELEASE:
#if defined(NDEBUG) && !defined(DEBUG_LOG_IN_RELEASE)
	// In Release: Define as an empty inline function. The compiler will typically "optimize away" the call and its arguments.
	template <typename... Args>	static inline void debug(Args&&...) {}
#else
	// In Debug: Keep the standard declaration.
	template <typename... Args>	static void debug(Args&&... args);
#endif

	static void set_level(LogLevel level);
	static void set_filepath(const std::string& filepath);
	static void to_console(bool active = true);
	static void to_file(bool active = true);
	static LogLevel get_level();
	static void enable_exit_on_error(bool active = true);
	static void enable_exit_on_warning(bool active = true);

	// nested class for time logging
	class Timer {
	public:
		Timer(LogLevel level = LEVEL_FORCE, std::string caller_function = "");
		~Timer();
		void stop();
		void silent_stop();
		void restart();
		double elapsed_sec();
		static void sleep(int64_t nanosec, LogLevel level = LEVEL_DEBUG);
	private:
		LogLevel log_level;
		std::chrono::high_resolution_clock::time_point begin, end;
		bool stopped = false;
		std::string caller_function = "";
	};

private:
	Log() {}
	~Log() {}
	template <typename... Args> static void log(LogLevel level, Args&&... args);
	static void write_log(std::string log_message);
	static LogLevel log_level;
	static bool log_to_console;
	static bool log_to_file;
	static bool exit_on_error;
	static bool exit_on_warning;
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
	if (exit_on_warning) {
		exit(EXIT_FAILURE);
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
#ifndef __ANDROID__
	log_to_console = active;
#endif
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

void Log::enable_exit_on_warning(bool active) {
	exit_on_warning = active;
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
LogLevel Log::log_level = DEFAULT_LOG_LEVEL;
#if defined(__ANDROID__)
bool Log::log_to_console = false;
bool Log::log_to_file = true;
#else
bool Log::log_to_console = true;
bool Log::log_to_file = false;
#endif
bool Log::exit_on_error = true;
bool Log::exit_on_warning = false;
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

void Log::Timer::silent_stop() {
	stopped = true;
	begin = std::chrono::high_resolution_clock::now();
}

void Log::Timer::restart() {
	begin = std::chrono::high_resolution_clock::now();
	stopped = false;
}

void Log::Timer::sleep(int64_t nanosec, LogLevel level) {
	Log::log(level, "'sleeping' for ", nanosec, " nanosec");
	std::this_thread::sleep_for(std::chrono::nanoseconds(nanosec));
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
// in order to use this, simply use a "#define HEAL_MEMLOG" flag as a preprocessor directive before(!) including this file
#ifdef HEAP_MEMLOG

// global variables
std::unordered_map<void*, std::size_t> allocated_memory;
int total_allocation = 0;

// operator 'new' override
void* operator new(std::size_t size) {
	void* ptr = std::malloc(size);
	allocated_memory.insert(ptr, size);
	total_allocation += size;
	Log::force("HEAP MEMORY: allocated ", size, " bytes at address ", ptr, " [updated total allocation: ", total_allocation, " bytes]");
	return ptr;
}

// operator 'delete' override
void operator delete(void* ptr) noexcept {
	std::size_t size = allocated_memory[ptr];
	total_allocation -= size;
	allocated_memory.erase(ptr);
	std::free(ptr);
	Log::force("HEAP MEMORY: freed ", size, " bytes at address ", ptr, " [remaining total allocation: ", total_allocation, " bytes]");
}
#endif

#endif