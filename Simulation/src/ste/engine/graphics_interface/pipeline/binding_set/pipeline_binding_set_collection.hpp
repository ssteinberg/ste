//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_layout.hpp>
#include <pipeline_binding_set_pool.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_layout_set_index.hpp>

#include <pipeline_binding_set.hpp>
#include <pipeline_binding_set_collection_cmd_bind.hpp>

#include <boost/container/flat_map.hpp>
#include <memory>

namespace ste {
namespace gl {

class pipeline_binding_set_collection {
private:
	using layout_t = pipeline_binding_set_layout;
	using collection_t = boost::container::flat_map<pipeline_layout_set_index, pipeline_binding_set>;
	using pipeline_layout_set_modified_connection_t = pipeline_layout::set_layout_modified_signal_t::connection_type;
	using cmd_bind_t = pipeline_binding_set_collection_cmd_bind<pipeline_binding_set_collection>;

private:
	collection_t sets;
	std::reference_wrapper<const pipeline_layout> layout;
	std::reference_wrapper<pipeline_binding_set_pool> pool;

	std::shared_ptr<pipeline_layout_set_modified_connection_t> set_modified_connection;

private:
	void recreate_sets(const std::vector<pipeline_layout_set_index> &set_indices) {
		std::vector<const layout_t*> layouts;
		auto &pipeline_layout_map = layout.get().set_layouts();
		for (auto &set_idx : set_indices) {
			// Define layout for new set
			auto l_it = pipeline_layout_map.find(set_idx);
			if (l_it == pipeline_layout_map.end()) {
				// Layout not found.
				assert(false);
				return;
			}
			auto *layout = &l_it->second;

			layouts.push_back(layout);
		}

		// Allocate the new sets
		std::vector<pipeline_binding_set> new_sets = pool.get().allocate_binding_sets<layout_t>(layouts);
		for (std::size_t i = 0; i<new_sets.size(); ++i) {
			auto &set_idx = set_indices[i];
			auto &new_set = new_sets[i];

			// Find old set
			auto it = sets.find(set_idx);
			if (it == sets.end()) {
				// Set not found.
				assert(false);
				return;
			}
			auto &old_set = it->second;

			// Copy bindings from old set
			new_set.copy(old_set);

			// And store in collection
			it->second = std::move(new_set);
		}
	}

public:
	pipeline_binding_set_collection(const pipeline_layout& layout,
									pipeline_binding_set_pool &pool)
		: layout(layout),
		pool(pool)
	{
		const auto& binding_sets_layouts_map = layout.set_layouts();

		std::vector<pipeline_layout_set_index> indices;
		std::vector<const layout_t*> layouts;
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

		// Connect to 'set modified' signal
		set_modified_connection =
			std::make_shared<pipeline_layout_set_modified_connection_t>([this](const std::vector<pipeline_layout_set_index> &set_indices) {
			this->recreate_sets(set_indices);
		});
		layout.get_set_layout_modified_signal().connect(set_modified_connection);
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

	pipeline_binding_set_collection(pipeline_binding_set_collection&&) = default;
	pipeline_binding_set_collection &operator=(pipeline_binding_set_collection&&) = default;

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
