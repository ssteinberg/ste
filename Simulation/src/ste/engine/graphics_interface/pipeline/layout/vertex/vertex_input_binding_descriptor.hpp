//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vertex_attributes.hpp>

#include <vector>

namespace StE {
namespace GL {

class vertex_input_binding_descriptor {
private:
	VkVertexInputBindingDescription binding_descriptor;
	std::vector<VkVertexInputAttributeDescription> attribute_descriptors;

public:
	template <typename B, VkVertexInputRate i>
	vertex_input_binding_descriptor(std::uint32_t binding_index,
									const vertex_attributes<B, i> &attrib) {
		binding_descriptor = attrib.vk_vertex_binding(binding_index);
		attribute_descriptors = attrib.vk_vertex_attributes(binding_index);
	}

	vertex_input_binding_descriptor(vertex_input_binding_descriptor&&) = default;
	vertex_input_binding_descriptor(const vertex_input_binding_descriptor&) = default;
	vertex_input_binding_descriptor &operator=(vertex_input_binding_descriptor&&) = default;
	vertex_input_binding_descriptor &operator=(const vertex_input_binding_descriptor&) = default;

	auto &get_binding_descriptor() const { return binding_descriptor; }
	auto &get_attribute_descriptors() const { return attribute_descriptors; }
};

}
}
