// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>

namespace ste {
namespace graphics {

class hdr_compute_minmax_fragment : public gl::fragment_compute {
	using Base = gl::fragment_compute;

public:
	hdr_compute_minmax_fragment(const gl::rendering_system &rs)
		: Base(rs,
			   "hdr_compute_minmax.comp")
	{}
	~hdr_compute_minmax_fragment() noexcept {}

	static const std::string& name() { return "hdr_compute_minmax"; }

	void record(gl::command_recorder &recorder) override final {

	}
};

}
}
