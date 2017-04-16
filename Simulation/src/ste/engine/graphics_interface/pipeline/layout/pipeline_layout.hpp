//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <pipeline_layout_exceptions.hpp>

#include <ste_shader_stage.hpp>
#include <device_pipeline_shader_stage.hpp>

#include <ste_shader_stage_binding_variable.hpp>
#include <pipeline_binding_layout_collection.hpp>

#include <pipeline_layout_set_index.hpp>
#include <pipeline_binding_set_layout.hpp>
#include <vk_pipeline_layout.hpp>

#include <signal.hpp>

#include <allow_type_decay.hpp>
#include <string>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

namespace StE {
namespace GL {

/**
*	@brief	Describes the pipeline shader stages and resource binding points
*/
class pipeline_layout : public allow_type_decay<pipeline_layout, vk_pipeline_layout> {
	friend class device_pipeline;

public:
	using shader_stage_t = device_pipeline_shader_stage*;
	using shader_stages_list_t = std::vector<shader_stage_t>;

	using set_layout_modified_signal_t = signal<const std::vector<pipeline_layout_set_index> &>;

private:
	using stages_map_t = boost::container::flat_map<ste_shader_stage, shader_stage_t>;

	using variable_map_t = pipeline_binding_layout_collection;
	using variable_ref_map_t = boost::container::flat_map<std::string, pipeline_binding_set_layout_binding*>;

	using spec_map_t = std::unordered_map<ste_shader_stage, vk_shader::spec_map>;

	using binding_sets_layout_map_t = boost::container::flat_map<pipeline_layout_set_index, pipeline_binding_set_layout>;

	using spec_to_dependant_array_variables_map_t = 
		boost::container::flat_map<const ste_shader_stage_binding_variable*, std::vector<const pipeline_binding_set_layout_binding*>>;

private:
	const ste_context &ctx;

	// Attached pipeline stages and their variables map
	stages_map_t stages;
	variable_map_t variables_map;

	// Push and specialization constants maps, as well as specializations.
	variable_ref_map_t push_variables_map;
	variable_ref_map_t spec_variables_map;
	spec_map_t specializations;

	// Layouts of the descriptor sets
	binding_sets_layout_map_t bindings_set_layouts;
	std::unique_ptr<vk_pipeline_layout> layout;

	// Layout can be modified (by respecializing constants, which define array length of binding variables).
	// In which case the affected sets need to be recreated.
	boost::container::flat_set<pipeline_layout_set_index> set_layouts_modified_queue;
	// Map of specialization variables to dependant array variables
	spec_to_dependant_array_variables_map_t spec_to_dependant_array_variables_map;
	// Set layout modified signal
	set_layout_modified_signal_t set_layout_modified_signal;

	// If for any reason pipeline layout has changed, mark it and let device_pipeline recreate the pipeline when applicable.
	bool layout_invalidated_flag{ false };

private:
	auto& update_variable(const std::string &name,
						  const pipeline_binding_set_layout_binding &binding,
						  ste_shader_stage stage) {
		// Try add new name
		auto ret = variables_map.try_emplace(name, binding);
		auto &b = ret.first->second;
		if (!ret.second) {
			// If name exists, verify it is the same variable
			if (*b.binding->variable != *binding.binding->variable) {
				throw pipeline_layout_duplicate_variable_name_exception("Variable's name was already used in layout");
			}
		}

		// Append stage to variable stage list
		b.stages.insert(stage);

		return b;
	}

	void create_set_layouts() {
		// Sort variables into sets
		boost::container::flat_map<pipeline_layout_set_index, pipeline_binding_set_layout::bindings_vec_t> sets;
		for (auto &v : variables_map) {
			auto& b = v.second;
			if (b.binding->binding_type == ste_shader_stage_binding_type::uniform ||
				b.binding->binding_type == ste_shader_stage_binding_type::storage)
				sets[b.binding->set_idx].push_back(&b);
		}

		// Create descriptor set layouts
		for (auto &s : sets) {
			auto &v = s.second;
			auto set_layout = pipeline_binding_set_layout(ctx, std::move(v));

			bindings_set_layouts.emplace(std::make_pair(s.first, std::move(set_layout)));
		}
	}

	/**
	*	@brief	Flags the set as modified
	*/
	void invalidate_set_layout(const pipeline_layout_set_index &index) {
		set_layouts_modified_queue.insert(index);
	}

	/**
	*	@brief	Flags the pipeline layout invalid
	*/
	void invalidate_layout() {
		layout_invalidated_flag = true;
	}

	template <typename T>
	void specialize_constant_impl(const std::string &name, 
								  const optional<std::string> &data = none,
								  const T *value = nullptr) {
		// Specialize the binding scalar variable
		// Used for dynamic array lengths
		auto it = spec_variables_map.find(name);
		if (it == spec_variables_map.end()) {
			throw pipeline_layout_variable_not_found_exception("Specialization constant with provided name not found");
		}

		auto &b = *it->second->binding;
		auto *var = b.variable.get();
		auto *ptr = dynamic_cast<ste_shader_stage_binding_variable_scalar*>(var);
		assert(ptr);
		if (ptr) {
			// Do nothing if value unchanged
			if (data) {
				if (ptr->read_specialized_value<T>() == *value)
					return;
				ptr->specialize_bin(data.get());
			}
			else {
				if (!ptr->has_non_default_specialization())
					return;
				ptr->reset_specialization();
			}
		}

		// Update shader stage specializations
		for (auto &s : it->second->stages) {
			auto& map = specializations[s];
			if (data)
				map[b.bind_idx] = data.get();
			else
				map.erase(b.bind_idx);
			stages[s]->set_specializations(map);
		}

		// If some array variable depends on this specialization constant, we need to update the relevant set descriptor layouts
		auto dependant_arrays_it = spec_to_dependant_array_variables_map.find(var);
		if (dependant_arrays_it != spec_to_dependant_array_variables_map.end()) {
			for (auto &arr : dependant_arrays_it->second) {
				auto set_idx = arr->binding->set_idx;

				// Flag the descriptor set layout as invalid and invalidate pipeline layout
				invalidate_set_layout(set_idx);
				invalidate_layout();
			}
		}
	}

public:
	pipeline_layout(const ste_context &ctx,
					const shader_stages_list_t &pipeline_shader_stages): ctx(ctx) {
		for (auto &s : pipeline_shader_stages) {
			auto stage = s->get_stage();
			auto& bindings = s->get_stage_bindings();
			for (auto &b : bindings) {
				auto new_bind = pipeline_binding_set_layout_binding{ &b };
				auto& name = b.variable->name();

				// Add new variable, or update exciting ones with new stage
				pipeline_binding_set_layout_binding &inserted = update_variable(name,
																				new_bind,
																				stage);

				// Also individually map push and specialization constants
				if (b.binding_type == ste_shader_stage_binding_type::spec_constant) {
					spec_variables_map[name] = &inserted;
				}
				else if (b.binding_type == ste_shader_stage_binding_type::push_constant) {
					push_variables_map[name] = &inserted;
				}

				// Map array variables whose length depends on specialization constants
				auto var_arr = dynamic_cast<const ste_shader_stage_binding_variable_array*>(b.variable.get());
				if (var_arr && var_arr->length_spec_constant()) {
					spec_to_dependant_array_variables_map[var_arr->get_length_spec_constant_var()].push_back(&inserted);
				}
			}

			stages[stage] = s;
		}

		// Generate the descriptor set layout for each set and the pipeline layout
		create_set_layouts();
		recreate_layout();
	}
	~pipeline_layout() noexcept {}

	pipeline_layout(pipeline_layout&&) = default;
	pipeline_layout &operator=(pipeline_layout&&) = default;

	/**
	*	@brief	Specializes a constant
	*
	*	@param	name		Name of specialization constant
	*	@param	value		Value to specialize to. Must be a POD.
	*	
	*	@throws	pipeline_layout_variable_not_found_exception	If specialization constant not found
	*/
	template <typename T>
	void specialize_constant(const std::string &name, const T &value) {
		static_assert(std::is_pod_v<T>, "T must be a POD");

		std::string data;
		data.resize(sizeof(T));
		memcpy(data.data(), &value, sizeof(T));

		specialize_constant_impl<T>(name,
									std::move(data),
									&value);
	}
	/**
	*	@brief	Remove a specialization of constant
	*	
	*	@throws	pipeline_layout_variable_not_found_exception	If specialization constant not found
	*/
	void remove_specialization(const std::string &name) {
		specialize_constant_impl<unsigned>(name);
	}

	/**
	*	@brief	Recreates the pipeline layout and resets flag
	*/
	void recreate_layout() {
		std::vector<const vk_descriptor_set_layout*> set_layout_ptrs;
		for (auto &s : bindings_set_layouts) {
			set_layout_ptrs.push_back(&s.second.get());
		}

		// Create push contant layout descriptors
		std::vector<vk_push_constant_layout> push_constant_layouts;
		for (auto &p : push_variables_map) {
			auto &b = *p.second;
			push_constant_layouts.push_back(vk_push_constant_layout(b.stages,
																	b.binding->variable->size_bytes()));
		}

		// Create pipeline layout
		layout = std::make_unique<vk_pipeline_layout>(ctx.device(),
													  set_layout_ptrs,
													  push_constant_layouts);
		layout_invalidated_flag = false;
	}

	/**
	*	@brief	Recreates invalidated set layout and resets flags
	*	
	*	@return	True if sets were modified, false otherwise.
	*/
	void recreate_invalidated_set_layouts() {
		if (set_layouts_modified_queue.empty())
			return;

		std::vector<pipeline_layout_set_index> modified_set_indices;
		for (auto &set_idx : set_layouts_modified_queue) {
			auto set_layout_it = bindings_set_layouts.find(set_idx);
			if (set_layout_it == bindings_set_layouts.end()) {
				// Set not found, can not be.
				assert(false);
				continue;
			}

			// Recreate set based on same bindings
			// (only possible change is modified array length of a binding)
			auto bindings = set_layout_it->second.get_bindings();
			auto set_layout = pipeline_binding_set_layout(ctx, std::move(bindings));

			// Replace set with new one
			set_layout_it->second = std::move(set_layout);

			modified_set_indices.push_back(set_idx);
		}

		// Erase queue
		set_layouts_modified_queue.clear();

		// Notify
		set_layout_modified_signal.emit(modified_set_indices);
	}

	/**
	*	@brief	Returns the status of the layout invalid flag, and resets it
	*/
	auto read_and_reset_invalid_layout_flag() {
		auto ret = layout_invalidated_flag;
		layout_invalidated_flag = false;
		return ret;
	}

	/**
	*	@brief	Returns the collection of shader stage descriptors, used to create pipeline objects.
	*/
	auto shader_stage_descriptors() const {
		std::vector<vk_shader_stage_descriptor> descriptors;
		for (auto &s : stages)
			descriptors.push_back(s.second->pipeline_stage_descriptor());

		return descriptors;
	}

	auto& get() const { return *layout; }

	bool is_pipeline_layout_invalidated() const { return layout_invalidated_flag; }

	auto& set_layouts() const { return bindings_set_layouts; }
	auto& modified_set_layouts() const { return set_layouts_modified_queue; }
	
	auto& variables() const { return variables_map; }
	auto& push_variables() const { return push_variables_map; }
	auto& spec_variables() const { return spec_variables_map; }

	auto& get_set_layout_modified_signal() const { return set_layout_modified_signal; }
};

}
}
