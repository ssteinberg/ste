// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <string>
#include <condition_variable>

#include "log_entry.h"

namespace StE {

class log_sink {
private:
	std::shared_ptr<std::condition_variable> notifier;
	std::shared_ptr<concurrent_queue<log_entry>> queue;
	log_entry_data data;

public:
	log_sink(const log_entry_data &data, std::shared_ptr<std::condition_variable> notifier, std::shared_ptr<concurrent_queue<log_entry>> q) : data(data), notifier(notifier), queue(q) {}
	virtual ~log_sink() {}

	log_sink &operator<<(const std::string &str) {
		queue->push(log_entry(data, str));
		notifier->notify_one();

		return *this;
	}
};

}
