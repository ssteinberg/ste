//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <filesystem>

namespace ste {

class ste_engine_storage_protocol {
public:
	static std::experimental::filesystem::path root_path() { return "."; }

	static std::experimental::filesystem::path log_dir_path() { return root_path() / "Log"; }
	static std::experimental::filesystem::path cache_dir_path() { return root_path() / "Cache"; }
	static std::experimental::filesystem::path data_dir_path() { return root_path() / "Data"; }
	static std::experimental::filesystem::path shader_module_dir_path() { return data_dir_path() / "programs"; }
	static std::experimental::filesystem::path screenshots_dir_path() { return root_path() / "Screenshots"; }

	static std::experimental::filesystem::path temp_dir_path() { return root_path() / "temp"; }
};

}
