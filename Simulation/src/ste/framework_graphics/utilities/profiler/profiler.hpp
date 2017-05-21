// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <lib/vector.hpp>
#include <lib/string.hpp>
#include <algorithm>

namespace ste {
namespace graphics {

struct profiler_entry {
	lib::string name;
	std::uint64_t start, end;
};

class profiler {
private:
	static constexpr const char * log_path = R"(Log/profiler_output.json)";

private:
	lib::vector<profiler_entry> entries;
	lib::vector<float> last_times_per_frame;

public:
	profiler();
	~profiler() noexcept;

	void add_entry(const profiler_entry &e) { entries.push_back(e); }
	void record_frame(float t);

	auto &get_entries() const { return entries; }
	auto &get_last_times_per_frame() const { return last_times_per_frame; }
};

}
}
