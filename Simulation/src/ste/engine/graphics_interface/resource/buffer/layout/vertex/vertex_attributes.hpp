// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <format.hpp>
#include <vertex_input_layout.hpp>

#include <format_type_traits.hpp>
#include <generate_array.hpp>

#include <vector>

namespace ste {
namespace gl {

namespace _internal {

template<typename B, VkVertexInputRate input_rate, bool enable>
class vertex_attributes_descriptor_impl {
	static_assert(enable, "B must be a vertex_input_layout");
};
template<typename V, VkVertexInputRate input_rate>
class vertex_attributes_descriptor_impl<V, input_rate, true> {
private:
	using size_type = std::uint32_t;
	static constexpr auto count = V::count;

	template<int index>
	struct sizes_populator {
		static constexpr size_type value = sizeof(typename V::template type_at<index>);
	};
	template<int index>
	struct elements_format_populator {
		using ValT = typename V::template type_at<index>;
		static constexpr format value = format_for_type_v<ValT>;
		static_assert(value != format::undefined, "ValT is NOT a valid type");
	};

	using sizes_arr = typename generate_array<count, sizes_populator>::result;
	using element_type_names_arr = typename generate_array<count, elements_format_populator>::result;

public:
	size_type attrib_count() const noexcept { return static_cast<size_type>(count); }
	size_type stride() const noexcept { return static_cast<size_type>(sizeof(V)); }

	size_type attrib_size(int attrib) const noexcept { return sizes_arr::data[attrib]; }
	size_type offset_to_attrib(int attrib) const noexcept { return static_cast<size_type>(vertex_input_offset_of<V>(attrib)); }
	format attrib_format(int attrib) const noexcept { return element_type_names_arr::data[attrib]; }

	auto vk_vertex_binding(std::uint32_t binding_index = 0) const noexcept {
		VkVertexInputBindingDescription desc = {};
		desc.binding = binding_index;
		desc.inputRate = input_rate;
		desc.stride = static_cast<std::uint32_t>(stride());

		return desc;
	}

	auto vk_vertex_attributes(std::uint32_t binding_index = 0) const {
		std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptors;
		vertex_input_attribute_descriptors.reserve(attrib_count());
		for (size_type j = 0; j < attrib_count(); ++j) {
			VkVertexInputAttributeDescription desc = {};
			desc.location = j;
			desc.binding = binding_index;
			desc.format = static_cast<VkFormat>(attrib_format(j));
			desc.offset = offset_to_attrib(j);

			vertex_input_attribute_descriptors.push_back(desc);
		}

		return vertex_input_attribute_descriptors;
	}
};

}

template <typename V, VkVertexInputRate input_rate = VK_VERTEX_INPUT_RATE_VERTEX>
using vertex_attributes = _internal::vertex_attributes_descriptor_impl<V, input_rate, is_vertex_input_layout_v<V>>;

}
}
