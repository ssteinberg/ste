// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <cstring>
#include <cerrno>

#include <string>
#include <fstream>
#include <iostream>
#include <functional>
#include <map>

#include <time.h>

#include <log_class.hpp>
#include <log_entry.hpp>

namespace StE {

class log_stream_formatter {
private:
	static constexpr const char * log_template_path = R"(Data/log_template/index.html)";
	static constexpr const char * entry_boundary_start = "%%entry_begin%%";
	static constexpr const char * entry_boundary_end = "%%entry_end%%";

	using placeholder = std::string;
	using injection_map_type = std::map<placeholder, std::function<std::string(const log_entry *entry)>>;

private:
	static const char *severity(log_class c) {
		switch (c) {
		case StE::log_class::info_class_log:
			return "info";
		case StE::log_class::warn_class_log:
			return "warn";
		case StE::log_class::err_class_log:
			return "err";
		case StE::log_class::fatal_class_log:
			return "fatal";
		}
		return "";
	}

	int t_warn, t_err, t_fatal;
	std::string name;
	std::string head, tail, entry;
	injection_map_type injection_map;

	void create_injection_map() {
		injection_map["date"] = [](const log_entry *entry) {
			time_t rawtime;
			struct tm * timeinfo;
			char buffer[256];
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffer, 256, "%a %d/%b/%y %H:%M:%S", timeinfo);

			return std::string(buffer);
		};

		injection_map["time"] = [](const log_entry *entry) {
			time_t rawtime;
			struct tm * timeinfo;
			char buffer[256];
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffer, 256, "%H:%M:%S", timeinfo);

			return std::string(buffer);
		};

		injection_map["name"] = [this](const log_entry *entry) { return this->name; };
		injection_map["func"] = [](const log_entry *entry) -> std::string { return entry ? entry->data().func : ""; };
		injection_map["file"] = [](const log_entry *entry) -> std::string { return entry ? entry->data().file : ""; };
		injection_map["line"] = [](const log_entry *entry) -> std::string { return entry ? std::to_string(entry->data().line) : ""; };
		injection_map["src"] = [](const log_entry *entry) -> std::string { return "%%file%%:%%line%%"; };
		injection_map["log"] = [](const log_entry *entry) -> std::string { return entry ? entry->entry() : ""; };
		injection_map["t_warn"] = [this](const log_entry *entry) { return std::to_string(this->t_warn); };
		injection_map["t_err"] = [this](const log_entry *entry) { return std::to_string(this->t_err); };
		injection_map["t_fatal"] = [this](const log_entry *entry) { return std::to_string(this->t_fatal); };
		injection_map["severity_code"] = [](const log_entry *entry) -> std::string { return entry ? severity(entry->data().c) : ""; };
		injection_map["severity"] = [](const log_entry *entry) -> std::string { return entry ? std::string(severity(entry->data().c), 1) : ""; };
	}

	std::string inject(std::string str, const log_entry *entry = nullptr) {
		std::string::size_type p;
		int i = 0;
		while (static_cast<std::size_t>(p = str.find("%%"), i) != std::string::npos) {
			auto e = str.find("%%", p + 2);
			if (e == std::string::npos)
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
	log_stream_formatter(const std::string &name) : t_warn(0), t_err(0), t_fatal(0), name(name) {
		std::string template_data;

		std::ifstream fs(log_template_path, std::ios::in);
		if (!fs.good()) {
			std::cerr << "Error while reading log template file \"" << log_template_path << "\": " << std::strerror(errno) << std::endl;
			assert(false);
			return;
		}

		template_data = std::string((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
		fs.close();

		auto entry_begin_len = std::string(entry_boundary_start).length();
		auto entry_end_len = std::string(entry_boundary_end).length();
		auto entry_begin = template_data.find(entry_boundary_start);
		auto entry_end = template_data.find(entry_boundary_end);
		if (entry_begin == std::string::npos || entry_end == std::string::npos || entry_end < entry_begin || entry_end + entry_end_len >= template_data.length()) {
			std::cerr << "Error: Bad data in log template file.";
			assert(false);
			return;
		}

		head = template_data.substr(0, entry_begin);
		entry = template_data.substr(entry_begin + entry_begin_len, entry_end - entry_begin - entry_begin_len);
		tail = template_data.substr(entry_end + entry_end_len);

		create_injection_map();
	}

	std::string format_head() {
		return inject(head);
	}

	std::string format_line(const log_entry &e) {
		const auto &data = e.data();

		if (data.c == log_class::err_class_log) ++t_err;
		else if (data.c == log_class::warn_class_log) ++t_warn;
		else if (data.c == log_class::fatal_class_log) ++t_fatal;

		return inject(entry, &e);
	}

	std::string format_tail() {
		return inject(tail);
	}
};

}
