//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <framebuffer_attachment.hpp>
#include <pipeline_layout_attachment_location.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

template <
	typename Pipeline, 
	void (Pipeline::*attach_func)(pipeline_layout_attachment_location, framebuffer_attachment&&)
>
class pipeline_framebuffer_bind_point {
private:
	pipeline_layout_attachment_location location;
	Pipeline *owner;

	optional<std::reference_wrapper<const framebuffer_attachment>> prev_attachment;
	optional<framebuffer_attachment> new_attachment;

public:
	pipeline_framebuffer_bind_point(pipeline_layout_attachment_location location,
									optional<std::reference_wrapper<const framebuffer_attachment>> &&attachment,
									Pipeline *owner)
		: location(location), owner(owner), prev_attachment(std::move(attachment))
	{
		if (prev_attachment)
			new_attachment = prev_attachment.get().get();
	}
	~pipeline_framebuffer_bind_point() {
		if (new_attachment) {
			auto &a = new_attachment.get();

			if (!prev_attachment ||
				prev_attachment.get().get() != a)
				(owner->*attach_func)(location,
									  std::move(a));

		}
	}

	auto& operator=(framebuffer_attachment &&attach) {
		new_attachment = std::move(attach);
		return *this;
	}
	auto* operator->() { return &new_attachment.get(); }
};

}
}
