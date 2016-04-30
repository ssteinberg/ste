// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include <vector>
#include <string>

namespace StE {
namespace Graphics {

struct profiler_entry {
	std::string name;
	std::uint64_t start, end;
};

class profiler {
private:
	static constexpr const char * log_path = R"(Log/profiler_output.json)";

private:
	std::vector<profiler_entry> entries;

public:
	profiler() = default;
	~profiler() noexcept;

	void add_entry(const profiler_entry &e) { entries.push_back(e); }
};

}
}
