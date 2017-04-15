//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <vk_descriptor_set_layout.hpp>

#include <allow_type_decay.hpp>

namespace StE {
namespace GL {

/**
*	@brief	Describes the descriptor set layout
*/
class pipeline_binding_set_layout : public allow_type_decay<pipeline_binding_set_layout, vk_descriptor_set_layout> {
public:
	using bindings_vec_t = std::vector<const pipeline_binding_set_layout_binding*>;

private:
	bindings_vec_t bindings;

	std::vector<vk_descriptor_set_layout_binding> vk_bindings;
	vk_descriptor_set_layout descriptor_set_layout;

private:
	auto generate_vk_layout(const ste_context &ctx) {
		for (auto &b : bindings)
			vk_bindings.push_back(*b);

		return vk_descriptor_set_layout(ctx.device(), vk_bindings);
	}

public:
	pipeline_binding_set_layout(const ste_context &ctx,
								bindings_vec_t &&bindings)
		: bindings(std::move(bindings)),
		descriptor_set_layout(generate_vk_layout(ctx))
	{}
	~pipeline_binding_set_layout() noexcept {}

	pipeline_binding_set_layout(pipeline_binding_set_layout&&) = default;
	pipeline_binding_set_layout &operator=(pipeline_binding_set_layout&&) = default;

	auto& get() const { return descriptor_set_layout; }

	/**
	*	@brief	Returns the bindings that participate in this descriptor set
	*/
	auto& get_bindings() const { return bindings; }

	/**
	*	@brief	Returns the generated Vulkan binding descriptors
	*/
	auto& get_vk_bindings() const { return vk_bindings; }
};

}
}
