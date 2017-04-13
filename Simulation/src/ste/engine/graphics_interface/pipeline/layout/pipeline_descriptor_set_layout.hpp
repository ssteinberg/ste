//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <vk_descriptor_set_layout.hpp>

#include <allow_type_decay.hpp>

namespace StE {
namespace GL {

class pipeline_descriptor_set_layout : public allow_type_decay<pipeline_descriptor_set_layout, vk_descriptor_set_layout> {
private:
	vk_descriptor_set_layout descriptor_set_layout;

public:
	pipeline_descriptor_set_layout(const ste_context &ctx,
								   const std::vector<vk_descriptor_set_layout_binding> &bindings)
		: descriptor_set_layout(ctx.device(), bindings) 
	{}
	~pipeline_descriptor_set_layout() noexcept {}

	pipeline_descriptor_set_layout(pipeline_descriptor_set_layout&&) = default;
	pipeline_descriptor_set_layout &operator=(pipeline_descriptor_set_layout&&) = default;

	auto& get() const { return descriptor_set_layout; }
};

}
}
