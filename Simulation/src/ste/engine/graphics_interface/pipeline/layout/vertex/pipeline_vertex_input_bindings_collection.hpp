//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vertex_input_binding_descriptor.hpp>
#include <vertex_attributes.hpp>

#include <boost/container/flat_map.hpp>
#include <algorithm>

namespace ste {
namespace gl {

class pipeline_vertex_input_bindings_collection {
public:
	struct pipeline_vertex_input_bindings_descriptor {
		std::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptors;
		std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptors;
	};

private:
	using idx_t = std::uint32_t;
	using map_t = boost::container::flat_map<idx_t, vertex_input_binding_descriptor>;

private:
	map_t collection;

public:
	pipeline_vertex_input_bindings_collection() = default;

	template <typename B, VkVertexInputRate i>
	void insert(const idx_t &binding_index,
				const vertex_attributes<B, i> &attrib) {
		collection.insert(std::make_pair(binding_index,
										 vertex_input_binding_descriptor(binding_index, attrib)));
	}
	void erase(const idx_t &binding_index) { collection.erase(binding_index); }

	auto get_vk_descriptors() const {
		pipeline_vertex_input_bindings_descriptor desc;

		std::size_t a = 0;
		for (auto &v : collection)
			a += v.second.get_attribute_descriptors().size();

		desc.vertex_input_binding_descriptors.reserve(collection.size());
		desc.vertex_input_attribute_descriptors.reserve(a);

		for (auto &v : collection) {
			desc.vertex_input_binding_descriptors.push_back(v.second.get_binding_descriptor());
			std::copy(v.second.get_attribute_descriptors().begin(),
					  v.second.get_attribute_descriptors().end(),
					  std::back_inserter(desc.vertex_input_attribute_descriptors));
		}

		return desc;
	}
};

}
}
