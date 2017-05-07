
#include <stdafx.hpp>
#include <profiler.hpp>

#include <functional>
#include <fstream>
#include <cstring>

using namespace ste::graphics;

profiler::profiler() {
	last_times_per_frame.resize(500);
	std::memset(last_times_per_frame.data(), 0, sizeof(float) * 500);
}

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

void profiler::record_frame(float t) {
	std::memmove(&last_times_per_frame[0], &last_times_per_frame[1], (last_times_per_frame.size() - 1) * sizeof(float));
	last_times_per_frame.back() = t;
}
