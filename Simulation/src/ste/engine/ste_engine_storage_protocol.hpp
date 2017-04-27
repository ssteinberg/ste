//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <boost_filesystem.hpp>

namespace ste {

class ste_engine_storage_protocol {
public:
	static boost::filesystem::path root_path() { return "."; }

	static boost::filesystem::path log_dir_path() { return root_path() / "Log"; }
	static boost::filesystem::path cache_dir_path() { return root_path() / "Cache"; }
	static boost::filesystem::path data_dir_path() { return root_path() / "Data"; }
	static boost::filesystem::path shader_module_dir_path() { return data_dir_path() / "programs"; }
	static boost::filesystem::path screenshots_dir_path() { return root_path() / "Screenshots"; }

	static boost::filesystem::path temp_dir_path() { return root_path() / "temp"; }
};

}
