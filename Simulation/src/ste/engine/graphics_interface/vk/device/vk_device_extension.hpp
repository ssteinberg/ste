//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <lib/string.hpp>
#include <lib/flat_set.hpp>

namespace ste {
namespace gl {

namespace vk {

struct vk_device_extension {
	lib::string name;
	std::uint32_t spec_version;

	vk_device_extension() = default;
	vk_device_extension(lib::string name, std::uint32_t v) : name(name), spec_version(v) {}

	bool operator==(const vk_device_extension &rhs) const { return name == rhs.name; }
	bool operator==(const lib::string &rhs) const { return name == rhs; }
	bool operator<(const vk_device_extension &rhs) const { return name < rhs.name; }
};

/**
 *	@brief	Helper structure that keeps hold of all available extensions and allows look-up
 */
class vk_device_extensions {
private:
	lib::flat_set<vk_device_extension> extensions;

public:
	vk_device_extensions() = default;
	vk_device_extensions(lib::flat_set<vk_device_extension> &&list) : extensions(std::move(list)) {}

	vk_device_extensions(vk_device_extensions &&) = default;
	vk_device_extensions &operator=(vk_device_extensions &&) = default;
	vk_device_extensions(const vk_device_extensions &) = default;
	vk_device_extensions &operator=(const vk_device_extensions &) = default;

	bool is_supported(lib::string n) const {
		return extensions.find({ n,0 }) != extensions.end();
	}

	auto &list() const { return extensions; }
};

}

}
}
