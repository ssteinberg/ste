// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <ostream>
#include <chrono>

namespace StE {

class _Logger {
	friend class Log;
	void *d;
	_Logger(void *p) : d(p) {}
public:
	std::ostream& logger();
	~_Logger();
};

class Log {
private:
	~Log() {}

public:
	Log();

	static Log &instance() {
		static Log inst;
		return inst;
	}

	_Logger log_info(const char *file, const char *func, int line);
	_Logger log_warn(const char *file, const char *func, int line);
	_Logger log_err(const char *file, const char *func, int line);
	_Logger log_fatal(const char *file, const char *func, int line);
};

}

#define ___STE_LOG_EVERY_VAR(x,l) ___STE_LOG_VAR ## x ## l
#define ___STE_LOG_EVERY(func, n) static std::chrono::time_point< std::chrono::high_resolution_clock> ___STE_LOG_EVERY_VAR(1,__LINE__); bool ___STE_LOG_EVERY_VAR(2,__LINE__) = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - ___STE_LOG_EVERY_VAR(1,__LINE__)) > static_cast<std::chrono::milliseconds>(n); if (___STE_LOG_EVERY_VAR(2,__LINE__)) ___STE_LOG_EVERY_VAR(1,__LINE__) = std::chrono::high_resolution_clock::now(); if (___STE_LOG_EVERY_VAR(2,__LINE__)) func

#define ste_log()		(StE::Log::instance().log_info(__FILE__,__func__,__LINE__).logger())
#define ste_log_warn()	(StE::Log::instance().log_warn(__FILE__,__func__,__LINE__).logger())
#define ste_log_error()	(StE::Log::instance().log_err(__FILE__,__func__,__LINE__).logger())
#define ste_log_fatal() (StE::Log::instance().log_fatal(__FILE__,__func__,__LINE__).logger())

// Write to info log, limit log to one per n milliseconds
#define ste_log_every(n)		___STE_LOG_EVERY((StE::Log::instance().log_info(__FILE__,__func__,__LINE__).logger()),n)
// Write to warning log, limit log to one per n milliseconds
#define ste_log_every_warn(n)	___STE_LOG_EVERY((StE::Log::instance().log_warn(__FILE__,__func__,__LINE__).logger()),n)
// Write to error log, limit log to one per n milliseconds
#define ste_log_every_error(n)	___STE_LOG_EVERY((StE::Log::instance().log_err(__FILE__,__func__,__LINE__).logger()),n)
