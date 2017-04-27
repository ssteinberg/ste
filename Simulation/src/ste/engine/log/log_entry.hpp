// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <string>
#include <memory>

#include <log_class.hpp>

namespace ste {

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
	std::string str;

public:
	log_entry(const log_entry_data &entry_data, const std::string &line) : entry_data(entry_data), str(line) {}
	virtual ~log_entry() {}

	log_entry_data data() const { return entry_data; }
	std::string entry() const { return str; }
};

}
