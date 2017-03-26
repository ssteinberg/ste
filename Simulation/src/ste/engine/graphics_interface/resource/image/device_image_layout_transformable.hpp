//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image_layout.hpp>

namespace StE {
namespace GL {

class device_resource_queue_transferable;

template <int dimensions, class allocation_policy>
class device_image;

class device_image_layout_transformable {
	friend class cmd_pipeline_barrier;

protected:
	mutable device_image_layout image_layout;

protected:
	template <typename... Args>
	device_image_layout_transformable(Args&&... args)
		: image_layout(std::forward<Args>(args)...)
	{}

	device_image_layout_transformable(device_image_layout_transformable&&) = default;
	device_image_layout_transformable &operator=(device_image_layout_transformable&&) = default;

public:
	auto layout() const { return image_layout.layout.load(std::memory_order_acquire); }
};

}
}
