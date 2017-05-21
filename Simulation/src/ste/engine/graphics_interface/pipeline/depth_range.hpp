//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

struct depth_range {
	float min_depth;
	float max_depth;

	depth_range(float min_depth,
				float max_depth) : min_depth(min_depth), max_depth(max_depth) {}

	depth_range(depth_range&&) = default;
	depth_range(const depth_range&) = default;
	depth_range &operator=(depth_range&&) = default;
	depth_range &operator=(const depth_range&) = default;

	static auto zero_to_one() {
		return depth_range{ .0f, 1.f };
	}
	static auto one_to_zero() {
		return depth_range{ 1.f, .0f };
	}
	static auto minus_one_to_one() {
		return depth_range{ -1.f, 1.f };
	}
};

}
}
