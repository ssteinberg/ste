// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <chrono>
#include <ctime>

#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <concurrent_queue.hpp>
#include <log_ostream.hpp>
#include <log_stream_formatter.hpp>
#include <log_sink.hpp>
#include <log_class.hpp>

namespace ste::log {

class _logger {
private:
	friend class log;

private:
	_logger(std::unique_ptr<log_sink> &&sink, bool force_flush = false) : stream(std::move(sink), force_flush) {}
	_logger(_logger &&) = default;

	log_ostream stream;

public:
	log_ostream &logger() { return stream; }
	~_logger() {}
};

class log {
private:
	using queue_type = concurrent_queue<log_entry>;

private:
	log_stream_formatter formatter;

	std::condition_variable notifier;
	queue_type queue;
	std::mutex m;
	std::thread t;
	std::atomic<bool> finish;

	std::string file_path;

	static std::string file_name(const std::string &title) {
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[256];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, 256, "%a %d%b%y %H.%M.%S", timeinfo);

		return title + " " + buffer;
	}

	void dump() {
		queue_type::stored_ptr entry;
		while ((entry = queue.pop()) != nullptr)
			write_entry(std::move(entry));
	}

	void write_entry(queue_type::stored_ptr &&entry) {
		auto fs = std::ofstream();
		fs.open(file_path, std::ofstream::app);
		if (!fs) {
			assert(false);
			return;
		}

		auto line = formatter.format_line(*entry);
		fs << line << std::endl;
	}

	void write_head() {
		std::ofstream fs;
		fs.open(file_path, std::ofstream::trunc);
		if (!fs) {
			assert(false);
			return;
		}

		fs << formatter.format_head();
	}

	void write_tail() {
		std::ofstream fs;
		fs.open(file_path, std::ofstream::app);
		if (!fs) {
			assert(false);
			return;
		}

		fs << formatter.format_tail();
	}

	std::streambuf *cout_strm_buffer;
	std::streambuf *cerr_strm_buffer;
	std::unique_ptr<_logger> cout_logger;
	std::unique_ptr<_logger> cerr_logger;

public:
	log(const std::string &title, const std::string path_prefix = R"(Log/)", const std::string path_extension = ".html") :
			formatter(title),
			finish(false),
			cout_strm_buffer(nullptr),
			cerr_strm_buffer(nullptr) {
		file_path = path_prefix + file_name(title) + path_extension;

		write_head();

		t = std::thread([this] {
			for (;;) {
				std::unique_lock<std::mutex> ul(m);
				dump();

				notifier.wait(ul);
				if (finish.load()) {
					finish.store(false);
					return;
				}
			}
		});
	}

	~log() {
		std::unique_lock<std::mutex> ul(m);

		if (cout_logger != nullptr) cout_logger->stream.flush();
		if (cerr_logger != nullptr) cerr_logger->stream.flush();
		dump();

		// Notify worker that we are done
		finish.store(true);
		ul.unlock();
		// Wait for worker to roger that
		while (finish.load())
			notifier.notify_one();

		write_tail();

		if (cout_strm_buffer) std::cout.rdbuf(cout_strm_buffer);
		if (cerr_strm_buffer) std::cerr.rdbuf(cerr_strm_buffer);
		cout_strm_buffer = cerr_strm_buffer = nullptr;

		if (t.joinable())
			t.join();
	}

	void redirect_std_outputs() {
		cout_strm_buffer = std::cout.rdbuf();
		cerr_strm_buffer = std::cerr.rdbuf();
		cout_logger = std::unique_ptr<_logger>(new _logger(std::make_unique<log_sink>(log_entry_data("std", "std::cout", 0, log_class::info_class_log), &notifier, &queue)));
		cerr_logger = std::unique_ptr<_logger>(new _logger(std::make_unique<log_sink>(log_entry_data("std", "std::cerr", 0, log_class::err_class_log), &notifier, &queue), true));

		std::cout.rdbuf(cout_logger->logger().rdbuf());
		std::cerr.rdbuf(cerr_logger->logger().rdbuf());
	}

	_logger log_info(const char *file, const char *func, int line) {
		return _logger(std::make_unique<log_sink>(log_entry_data(file, func, line, log_class::info_class_log), &notifier, &queue));
	}
	_logger log_warn(const char *file, const char *func, int line) {
		return _logger(std::make_unique<log_sink>(log_entry_data(file, func, line, log_class::warn_class_log), &notifier, &queue));
	}
	_logger log_err(const char *file, const char *func, int line) {
		return _logger(std::make_unique<log_sink>(log_entry_data(file, func, line, log_class::err_class_log), &notifier, &queue), true);
	}
	_logger log_fatal(const char *file, const char *func, int line) {
		return _logger(std::make_unique<log_sink>(log_entry_data(file, func, line, log_class::fatal_class_log), &notifier, &queue), true);
	}
};

extern log *ste_global_logger;

void inline ste_log_set_global_logger(log *ptr) {
	ste_global_logger = ptr;
}

}

#define ___STE_LOG_EVERY_VAR(x,l) ___STE_LOG_VAR ## x ## l
#define ___STE_LOG_EVERY(func, n) static std::chrono::time_point< std::chrono::high_resolution_clock> ___STE_LOG_EVERY_VAR(1,__LINE__); bool ___STE_LOG_EVERY_VAR(2,__LINE__) = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - ___STE_LOG_EVERY_VAR(1,__LINE__)) > static_cast<std::chrono::milliseconds>(n); if (___STE_LOG_EVERY_VAR(2,__LINE__)) ___STE_LOG_EVERY_VAR(1,__LINE__) = std::chrono::high_resolution_clock::now(); if (___STE_LOG_EVERY_VAR(2,__LINE__)) func

#define __ste_log_null	(std::cout)

#define ste_log()		(::ste::log::ste_global_logger!=nullptr ? ::ste::log::ste_global_logger->log_info(__FILE__,__func__,__LINE__).logger() : __ste_log_null)
#define ste_log_warn()	(::ste::log::ste_global_logger!=nullptr ? ::ste::log::ste_global_logger->log_warn(__FILE__,__func__,__LINE__).logger() : __ste_log_null)
#define ste_log_error()	(::ste::log::ste_global_logger!=nullptr ? ::ste::log::ste_global_logger->log_err(__FILE__,__func__,__LINE__).logger() : __ste_log_null)
#define ste_log_fatal() (::ste::log::ste_global_logger!=nullptr ? ::ste::log::ste_global_logger->log_fatal(__FILE__,__func__,__LINE__).logger() : __ste_log_null)

// Write to info log, limit log to one per n milliseconds
#define ste_log_every(n)		___STE_LOG_EVERY((::ste::log::ste_global_logger!=nullptr ? ::ste::log::ste_global_logger->log_info(__FILE__,__func__,__LINE__).logger() : __ste_log_null),n)
// Write to warning log, limit log to one per n milliseconds
#define ste_log_every_warn(n)	___STE_LOG_EVERY((::ste::log::ste_global_logger!=nullptr ? ::ste::log::ste_global_logger->log_warn(__FILE__,__func__,__LINE__).logger() : __ste_log_null),n)
// Write to error log, limit log to one per n milliseconds
#define ste_log_every_error(n)	___STE_LOG_EVERY((::ste::log::ste_global_logger!=nullptr ? ::ste::log::ste_global_logger->log_err(__FILE__,__func__,__LINE__).logger() : __ste_log_null),n)
