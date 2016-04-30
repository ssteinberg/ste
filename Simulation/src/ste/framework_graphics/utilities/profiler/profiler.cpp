
#include "stdafx.hpp"
#include "profiler.hpp"

#include <functional>
#include <fstream>

using namespace StE::Graphics;

profiler::~profiler() {
	std::ofstream f(log_path);
	f << "[" << std::endl;

	bool first = true;
	for (auto &e : entries) {
		if (!first)
			f << "," << std::endl;
		first = false;

		auto tid = std::hash<std::string>()(e.name);
		f << "{" <<
			"\"cat\": \"GPU task\"," <<
			"\"pid\": 0," <<
			"\"tid\": " << std::to_string(tid) << "," <<
			"\"ts\": " << std::to_string(e.start) << "," <<
			"\"ph\": \"B\"," <<
			"\"name\": \"" << e.name << "\"," <<
			"\"args\": {}" <<
		"}," << std::endl;
		f << "{" <<
			"\"cat\": \"GPU task\"," <<
			"\"pid\": 0," <<
			"\"tid\": " << std::to_string(tid) << "," <<
			"\"ts\": " << std::to_string(e.end) << "," <<
			"\"ph\": \"E\"," <<
			"\"name\": \"" << e.name << "\"," <<
			"\"args\": {}" <<
		"}";
	}

	f << "]" << std::endl;
}
