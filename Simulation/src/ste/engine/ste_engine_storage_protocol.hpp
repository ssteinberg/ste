//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <string>

namespace StE {

class ste_engine_storage_protocol {
public:
	static std::string log_dir_path() { return "Log"; }
	static std::string cache_dir_path() { return "Cache"; }
	static std::string data_dir_path() { return "Data"; }
	static std::string screenshots_dir_path() { return "Screenshots"; }
};

}
