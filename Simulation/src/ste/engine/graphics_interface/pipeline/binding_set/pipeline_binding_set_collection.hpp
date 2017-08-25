//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_layout.hpp>
#include <pipeline_binding_set_pool.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_layout_set_index.hpp>

#include <pipeline_binding_set.hpp>
#include <pipeline_binding_set_collection_cmd_bind.hpp>

#include <lib/flat_map.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

class pipeline_binding_set_collection {
private:
	using layout_t = pipeline_binding_set_layout;
	using collection_t = lib::flat_map<pipeline_layout_set_index, pipeline_binding_set>;
	using cmd_bind_t = pipeline_binding_set_collection_cmd_bind<pipeline_binding_set_collection>;

private:
	collection_t sets;
	alias<pipeline_layout> layout;
	alias<pipeline_binding_set_pool> pool;

public:
	pipeline_binding_set_collection(pipeline_layout& layout,
									pipeline_binding_set_pool &pool)
		: layout(layout),
		pool(pool)
	{
		const auto& binding_sets_layouts_map = layout.set_layouts();
		if (!binding_sets_layouts_map.size())
			return;

		lib::vector<pipeline_layout_set_index> indices;
		lib::vector<const layout_t*> layouts;
		indices.reserve(binding_sets_layouts_map.size());
		layouts.reserve(binding_sets_layouts_map.size());

		for (auto &s : binding_sets_layouts_map) {
			indices.push_back(s.first);
			layouts.push_back(&s.second);
		}

		// Allocate
		auto allocated_sets = pool.allocate_binding_sets<layout_t>(layouts);

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
	pipeline_binding_set_collection(pipeline_layout& layout,
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

	pipeline_binding_set_collection(pipeline_binding_set_collection&&) = default;
	pipeline_binding_set_collection &operator=(pipeline_binding_set_collection&&) = default;

	/**
	 *	@brief	Rebuilds the required sets
	 *	
	 *	@param	device			Creating device
	 *	@param	set_indices		Indices of sets to recreate
	 *	@param	old_layouts		If non-null, old set layouts will be moved to this vector.
	 *	
	 *	@return	Returns the old sets
	 */
	auto recreate_sets(const vk::vk_logical_device<> &device,
					   const lib::flat_set<pipeline_layout_set_index> &set_indices,
					   lib::vector<vk::vk_descriptor_set_layout<>> *old_layouts = nullptr) {
		lib::vector<pipeline_binding_set> ret_old_sets;

		lib::vector<const layout_t*> layouts;
		layouts.reserve(set_indices.size());
		for (auto &set_idx : set_indices) {
			auto l_it = layout.get().set_layouts().find(set_idx);
			if (l_it == layout.get().set_layouts().end()) {
				// Layout not found.
				assert(false);
				return ret_old_sets;
			}

			// Recreate layout
			auto old_layout = layout.get().recreate_set_layout(set_idx);
			// Store old
			if (old_layouts)
				old_layouts->push_back(std::move(old_layout));

			layouts.push_back(&l_it->second);
		}

		// Allocate the new sets
		lib::vector<pipeline_binding_set> new_sets = pool.get().allocate_binding_sets<layout_t>(layouts);
		ret_old_sets.reserve(new_sets.size());
		for (std::size_t i = 0; i<new_sets.size(); ++i) {
			pipeline_layout_set_index set_idx = *(set_indices.begin() + i);
			auto &new_set = new_sets[i];

			// Find old set
			auto it = sets.find(set_idx);
			if (it == sets.end()) {
				// Set not found.
				assert(false);
				continue;
			}
			auto &old_set = it->second;

			// Copy bindings from old set
			new_set.copy(old_set);

			// Save old and store new inplace, in sets collection
			ret_old_sets.push_back(std::move(old_set));
			it->second = std::move(new_set);
		}

		return ret_old_sets;
	}

	auto &get_sets() const { return sets; }

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

	/**
	*	@brief	Binds the binding set collection
	*/
	auto cmd_bind(VkPipelineBindPoint bind_point) const {
		return cmd_bind_t(this,
						  bind_point,
						  &layout.get().get());
	}
};

}
}
