
#include "stdafx.h"
#include "Log.h"

#include <glog/logging.h>

#include <string>

using namespace StE;
using namespace google;


_Logger::~_Logger() {
	delete static_cast<google::LogMessage*>(this->d);
}

std::ostream& _Logger::logger() {
	return static_cast<google::LogMessage*>(this->d)->stream();
}

Log::Log() {
	google::SetLogDestination(GLOG_INFO, "./Log/");
	google::InitGoogleLogging("./StE");
}

_Logger Log::log_info(const char *file, const char *func, int line) {
	return _Logger(new google::LogMessage((std::string(file) + " (" + func + ")").data(), line, GLOG_INFO));
}

_Logger Log::log_warn(const char *file, const char *func, int line) {
	return _Logger(new google::LogMessage((std::string(file) + " (" + func + ")").data(), line, GLOG_WARNING));
}

_Logger Log::log_err(const char *file, const char *func, int line) {
	return _Logger(new google::LogMessage((std::string(file) + " (" + func + ")").data(), line, GLOG_ERROR));
}

_Logger Log::log_fatal(const char *file, const char *func, int line) {
	return _Logger(new google::LogMessage((std::string(file) + " (" + func + ")").data(), line, GLOG_FATAL));
}
