// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "trace.h"

#include <string>
#include <condition_variable>

#include <iostream>

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
		if (data.c != log_class::info_class_log)
			TRACE((str + "\n").data());

		queue->push(log_entry(data, str));
		notifier->notify_one();

		return *this;
	}
};

}
