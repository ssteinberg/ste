//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_layout.hpp>
#include <pipeline_binding_set_pool.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_layout_set_index.hpp>

#include <pipeline_binding_set.hpp>

#include <boost/container/flat_map.hpp>

namespace StE {
namespace GL {

class pipeline_binding_set_collection {
private:
	using collection_t = boost::container::flat_map<pipeline_layout_set_index, pipeline_binding_set>;

private:
	collection_t sets;
	std::reference_wrapper<const pipeline_layout> layout;

public:
	pipeline_binding_set_collection(const pipeline_layout& layout,
									pipeline_binding_set_pool &pool)
		: layout(layout)
	{
		const auto& binding_sets_layouts_map = layout.set_layouts();

		std::vector<pipeline_layout_set_index> indices;
		std::vector<const pipeline_binding_set_layout*> layouts;
		indices.reserve(binding_sets_layouts_map.size());
		indices.reserve(layouts.size());

		for (auto &s : binding_sets_layouts_map) {
			indices.push_back(s.first);
			layouts.push_back(&s.second);
		}

		// Allocate
		auto allocated_sets = pool.allocate_binding_sets(layouts);

		// Sort
		for (std::size_t i = 0; i<allocated_sets.size(); ++i) {
			auto idx = indices[i];
			sets.emplace(idx, std::move(allocated_sets[i]));
		}
	}
	~pipeline_binding_set_collection() noexcept {}

	/**
	*	@brief	Copies bindings from another set and writes them to the descriptor.
	*			Assumes the layout are similar, with only binding count (array length) changes.
	*
	*	@throws	device_pipeline_incompatible_binding_sets_exception		If the above assumption is violated
	*/
	pipeline_binding_set_collection(const pipeline_layout& layout,
									pipeline_binding_set_pool &pool,
									const pipeline_binding_set_collection &o)
		: pipeline_binding_set_collection(layout,
										  pool)
	{
		for (auto &s : o.sets) {
			auto &idx = s.first;

			auto it = sets.find(idx);
			if (it == sets.end())
				continue;

			pipeline_binding_set &dst = it->second;
			const pipeline_binding_set &src = s.second;

			dst.copy(src);
		}
	}

	/**
	*	@brief	Updates the binding set with resource bindings
	*/
	void write(const pipeline_resource_binding_queue &q) {
		for (auto &b : q) {
			auto &idx = b.first;
			auto &writes = b.second;

			auto it = sets.find(idx);
			if (it == sets.end()) {
				// Set not found?!
				assert(false);
				continue;
			}

			auto &set = it->second;

			set.write(writes);
		}
	}

	pipeline_binding_set_collection(pipeline_binding_set_collection&&) = default;
	pipeline_binding_set_collection &operator=(pipeline_binding_set_collection&&) = default;
};

}
}
