// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <trace.hpp>

#include <string>
#include <condition_variable>

#include <iostream>

#include <log_entry.hpp>

namespace StE {

class log_sink {
private:
	std::shared_ptr<std::condition_variable> notifier;
	std::shared_ptr<concurrent_queue<log_entry>> queue;
	log_entry_data data;

public:
	log_sink(const log_entry_data &data, std::shared_ptr<std::condition_variable> notifier, std::shared_ptr<concurrent_queue<log_entry>> q) : notifier(notifier), queue(q), data(data) {}
	virtual ~log_sink() {}

	log_sink &operator<<(const std::string &str) {
		if (data.c != log_class::info_class_log) {
			TRACE((str + "\n").data());
		}

		queue->push(log_entry(data, str));
		notifier->notify_one();

		return *this;
	}
};

}
