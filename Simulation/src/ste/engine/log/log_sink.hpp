// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <trace.hpp>

#include <lib/string.hpp>
#include <condition_variable>

#include <iostream>

#include <log_entry.hpp>

namespace ste::log {

class log_sink {
private:
	std::condition_variable *notifier;
	lib::concurrent_queue<log_entry> *queue;
	log_entry_data data;

public:
	log_sink(const log_entry_data &data, std::condition_variable *notifier, lib::concurrent_queue<log_entry> *q) : notifier(notifier), queue(q), data(data) {}
	virtual ~log_sink() {}

	log_sink &operator<<(const lib::string &str) {
		if (data.c != log_class::info_class_log) {
			TRACE((str + "\n").data());
		}

		queue->push(log_entry(data, str));
		notifier->notify_one();

		return *this;
	}
};

}
