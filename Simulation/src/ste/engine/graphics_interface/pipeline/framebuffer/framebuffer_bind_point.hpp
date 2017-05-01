//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_layout_attachment_location.hpp>
#include <framebuffer_attachment.hpp>

#include <optional.hpp>

namespace ste {
namespace gl {

template <
	typename Framebuffer, 
	void (Framebuffer::*attach_func)(pipeline_layout_attachment_location, framebuffer_attachment&&)
>
class framebuffer_bind_point {
	friend Framebuffer;

private:
	pipeline_layout_attachment_location location;
	Framebuffer *owner;
	optional<framebuffer_attachment> new_attachment;

	framebuffer_bind_point(pipeline_layout_attachment_location location,
						   Framebuffer *owner)
		: location(location), owner(owner)
	{}

public:
	framebuffer_bind_point(framebuffer_bind_point&&) = default;
	framebuffer_bind_point &operator=(framebuffer_bind_point&&) = default;

	~framebuffer_bind_point() {
		if (new_attachment) {
			auto &a = new_attachment.get();
			(owner->*attach_func)(location,
								  std::move(a));

		}
	}

	auto& operator=(framebuffer_attachment &&attach) {
		new_attachment = std::move(attach);
		return *this;
	}
};

}
}
