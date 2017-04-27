// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage.hpp>

namespace ste {
namespace gl {

struct ste_shader_properties {
	std::uint32_t version_major, version_minor;
};

struct ste_shader_blob_header {
	static constexpr std::uint32_t header_magic_value = 0x058ADE20;

	std::uint32_t magic;
	ste_shader_stage type;
	ste_shader_properties properties;
};

}
}
