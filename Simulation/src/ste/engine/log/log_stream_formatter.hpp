// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <cstring>
#include <cerrno>

#include <lib/string.hpp>
#include <fstream>
#include <iostream>
#include <functional>
#include <lib/map.hpp>

#include <time.h>

#include <log_class.hpp>
#include <log_entry.hpp>

namespace ste::log {

class log_stream_formatter {
private:
	static constexpr const char * log_template_path = R"(Data/log_template/index.html)";
	static constexpr const char * entry_boundary_start = "%%entry_begin%%";
	static constexpr const char * entry_boundary_end = "%%entry_end%%";

	using placeholder = lib::string;
	using injection_map_type = lib::map<placeholder, std::function<lib::string(const log_entry *entry)>>;

private:
	static const char *severity(log_class c) {
		switch (c) {
		case log_class::info_class_log:
			return "info";
		case log_class::warn_class_log:
			return "warn";
		case log_class::err_class_log:
			return "err";
		case log_class::fatal_class_log:
			return "fatal";
		}
		return "";
	}

	int t_warn, t_err, t_fatal;
	lib::string name;
	lib::string head, tail, entry;
	injection_map_type injection_map;

	void create_injection_map() {
		injection_map["date"] = [](const log_entry *entry) {
			time_t rawtime;
			struct tm * timeinfo;
			char buffer[256];
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffer, 256, "%a %d/%b/%y %H:%M:%S", timeinfo);

			return lib::string(buffer);
		};

		injection_map["time"] = [](const log_entry *entry) {
			time_t rawtime;
			struct tm * timeinfo;
			char buffer[256];
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffer, 256, "%H:%M:%S", timeinfo);

			return lib::string(buffer);
		};

		injection_map["name"] = [this](const log_entry *entry) { return this->name; };
		injection_map["func"] = [](const log_entry *entry) -> lib::string { return entry ? entry->data().func : ""; };
		injection_map["file"] = [](const log_entry *entry) -> lib::string { return entry ? entry->data().file : ""; };
		injection_map["line"] = [](const log_entry *entry) -> lib::string { return entry ? lib::to_string(entry->data().line) : ""; };
		injection_map["src"] = [](const log_entry *entry) -> lib::string { return "%%file%%:%%line%%"; };
		injection_map["log"] = [](const log_entry *entry) -> lib::string { return entry ? entry->entry() : ""; };
		injection_map["t_warn"] = [this](const log_entry *entry) { return lib::to_string(this->t_warn); };
		injection_map["t_err"] = [this](const log_entry *entry) { return lib::to_string(this->t_err); };
		injection_map["t_fatal"] = [this](const log_entry *entry) { return lib::to_string(this->t_fatal); };
		injection_map["severity_code"] = [](const log_entry *entry) -> lib::string { return entry ? severity(entry->data().c) : ""; };
		injection_map["severity"] = [](const log_entry *entry) -> lib::string { return entry ? lib::string(severity(entry->data().c), 1) : ""; };
	}

	lib::string inject(lib::string str, const log_entry *entry = nullptr) {
		lib::string::size_type p;
		lib::string::size_type i = 0;
		while (static_cast<std::size_t>(p = str.find("%%"), i) != lib::string::npos) {
			auto e = str.find("%%", p + 2);
			if (e == lib::string::npos)
				break;

			placeholder type = str.substr(p + 2, e - p - 2);

			injection_map_type::iterator it;
			if ((it = injection_map.find(type)) == injection_map.end()) {
				p = e + 2;
				continue;
			}
			injection_map_type::mapped_type &injector = it->second;

			auto replace = injector(entry);
			str.replace(p, e - p + 2, std::move(replace));

			i = p;
		}

		return str;
	}

public:
	log_stream_formatter(const lib::string &name) : t_warn(0), t_err(0), t_fatal(0), name(name) {
		std::ifstream fs(log_template_path, std::ios::in);
		if (!fs.good()) {
			std::cerr << "Error while reading log template file \"" << log_template_path << "\": " << std::strerror(errno) << std::endl;
			assert(false);
			return;
		}

		lib::string template_data = lib::string((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
		fs.close();

		auto entry_begin_len = lib::string(entry_boundary_start).length();
		auto entry_end_len = lib::string(entry_boundary_end).length();
		auto entry_begin = template_data.find(entry_boundary_start);
		auto entry_end = template_data.find(entry_boundary_end);
		if (entry_begin == lib::string::npos || entry_end == lib::string::npos || entry_end < entry_begin || entry_end + entry_end_len >= template_data.length()) {
			std::cerr << "Error: Bad data in log template file.";
			assert(false);
			return;
		}

		head = template_data.substr(0, entry_begin);
		entry = template_data.substr(entry_begin + entry_begin_len, entry_end - entry_begin - entry_begin_len);
		tail = template_data.substr(entry_end + entry_end_len);

		create_injection_map();
	}

	lib::string format_head() {
		return inject(head);
	}

	lib::string format_line(const log_entry &e) {
		const auto &data = e.data();

		if (data.c == log_class::err_class_log) ++t_err;
		else if (data.c == log_class::warn_class_log) ++t_warn;
		else if (data.c == log_class::fatal_class_log) ++t_fatal;

		return inject(entry, &e);
	}

	lib::string format_tail() {
		return inject(tail);
	}
};

}
