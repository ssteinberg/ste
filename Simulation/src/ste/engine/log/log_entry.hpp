// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <lib/string.hpp>
#include <lib/unique_ptr.hpp>

#include <log_class.hpp>

namespace ste::log {

struct log_entry_data {
	const char *file;
	const char *func;
	int line;
	log_class c;

	log_entry_data(const char *file, const char *func, int line, log_class c) : file(file), func(func), line(line), c(c) {}
};

class log_entry {
private:
	log_entry_data entry_data;
	lib::string str;

public:
	log_entry(const log_entry_data &entry_data, const lib::string &line) : entry_data(entry_data), str(line) {}
	virtual ~log_entry() {}

	log_entry_data data() const { return entry_data; }
	lib::string entry() const { return str; }
};

}
