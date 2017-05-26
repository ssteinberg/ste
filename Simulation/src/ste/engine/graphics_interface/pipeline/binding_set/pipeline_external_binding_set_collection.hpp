//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_binding_set_pool.hpp>
#include <pipeline_external_resource_bind_point.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_layout_set_index.hpp>

#include <pipeline_external_binding_layout.hpp>
#include <pipeline_external_binding_set.hpp>
#include <pipeline_binding_set_collection_cmd_bind.hpp>

#include <lib/flat_map.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

class pipeline_external_binding_set_collection {
private:
	using layout_t = pipeline_external_binding_set_layout;
	using set_layouts_t = lib::vector<pipeline_external_binding_set_layout>;
	
	using collection_t = lib::flat_map<pipeline_layout_set_index, pipeline_external_binding_set>;
	using name_binding_map_t = lib::flat_map<lib::string, const pipeline_external_binding_layout*>;

	using cmd_bind_t = pipeline_binding_set_collection_cmd_bind<pipeline_external_binding_set_collection>;

private:
	collection_t sets;
	set_layouts_t layouts;
	alias<pipeline_binding_set_pool> pool;

	name_binding_map_t name_map;

	pipeline_resource_binding_queue binding_queue;

private:
	/**
	*	@brief	Updates the binding set with resource bindings
	*/
	void write_binding_queue() {
		if (binding_queue.empty())
			return;

		for (auto &b : binding_queue) {
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

		binding_queue.clear();
	}

public:
	pipeline_external_binding_set_collection(set_layouts_t &&layouts,
											 pipeline_binding_set_pool &pool)
		: layouts(std::move(layouts)),
		pool(pool)
	{
		lib::vector<pipeline_layout_set_index> indices;
		lib::vector<const layout_t*> layout_ptrs;
		indices.reserve(this->layouts.size());
		layout_ptrs.reserve(this->layouts.size());

		for (auto &l : this->layouts) {
			indices.push_back(l.get_set_index());
			layout_ptrs.push_back(&l);

			for(auto &b : l) {
				name_map[b.name()] = &b;
			}
		}

		// Allocate
		auto allocated_sets = pool.allocate_binding_sets<layout_t>(layout_ptrs);

		// Sort
		for (std::size_t i = 0; i < allocated_sets.size(); ++i) {
			auto idx = indices[i];
			sets.emplace(idx, std::move(allocated_sets[i]));
		}
	}
	~pipeline_external_binding_set_collection() noexcept {}


	pipeline_external_binding_set_collection(pipeline_external_binding_set_collection&&) = default;
	pipeline_external_binding_set_collection &operator=(pipeline_external_binding_set_collection&&) = default;

	/**
	*	@brief	Creates a resource binder for a given variable name
	*/
	auto operator[](const lib::string &resource_name) {
		const pipeline_external_binding_layout *bind = nullptr;

		auto var_it = name_map.find(resource_name);
		if (var_it != name_map.end()) {
			// Name references a variable
			bind = var_it->second;
		}
		else {
			throw device_pipeline_unrecognized_variable_name_exception("Resource with provided name doesn't exist in pipeline layout");
		}

		// Create the binder
		return pipeline_external_resource_bind_point(&binding_queue,
													 resource_name,
													 bind);
	}

	auto& get_sets() const { return sets; }
	auto& get_layouts() const { return layouts; }

	/**
	 *	@brief	Updates resource writes
	 */
	void update() {
		write_binding_queue();
	}

	/**
	*	@brief	Binds the binding set collection
	*/
	auto cmd_bind(VkPipelineBindPoint bind_point,
				  const vk::vk_pipeline_layout<> *layout) const {
		return cmd_bind_t(this,
						  bind_point,
						  layout);
	}
};

}
}
