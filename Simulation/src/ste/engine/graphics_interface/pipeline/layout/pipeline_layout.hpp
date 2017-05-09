//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <pipeline_layout_exceptions.hpp>

#include <ste_shader_program_stage.hpp>
#include <device_pipeline_shader_stage.hpp>

#include <ste_shader_stage_variable.hpp>
#include <pipeline_external_binding_set_collection.hpp>
#include <pipeline_binding_layout_collection.hpp>

#include <pipeline_attachment_layout.hpp>
#include <pipeline_attachment_layout_collection.hpp>

#include <pipeline_push_constants_layout.hpp>
#include <pipeline_layout_set_index.hpp>
#include <pipeline_binding_set_layout.hpp>
#include <vk_pipeline_layout.hpp>

#include <signal.hpp>

#include <allow_type_decay.hpp>
#include <string>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

namespace ste {
namespace gl {

/**
*	@brief	The pipeline layout descriptor.
*			Fully defines the pipeline shader stages, resource binding layouts and output attachment layouts.
*/
class pipeline_layout : public allow_type_decay<pipeline_layout, vk::vk_pipeline_layout> {
	friend class device_pipeline;

public:
	using shader_stage_t = device_pipeline_shader_stage*;
	using shader_stages_list_t = std::vector<shader_stage_t>;

	using set_layout_modified_signal_t = signal<const std::vector<pipeline_layout_set_index> &>;

private:
	using stages_map_t = boost::container::flat_map<ste_shader_program_stage, shader_stage_t>;

	using variable_map_t = pipeline_binding_layout_collection;
	using variable_ref_map_t = boost::container::flat_map<std::string, pipeline_binding_layout*>;

	using attachment_map_t = pipeline_attachment_layout_collection;

	using spec_map_t = std::unordered_map<ste_shader_program_stage, vk::vk_shader::spec_map>;

	using binding_sets_layout_map_t = boost::container::flat_map<pipeline_layout_set_index, pipeline_binding_set_layout>;

	using spec_to_dependant_array_variables_map_t =
		boost::container::flat_map<const ste_shader_stage_variable*, std::vector<const pipeline_binding_layout*>>;

private:
	std::reference_wrapper<const ste_context> ctx;

	// Attached pipeline stages and their variables and attachment maps
	stages_map_t stages;
	variable_map_t variables_map;
	attachment_map_t attachments_map;

	// Push and specialization constants maps, as well as specializations.
	std::unique_ptr<pipeline_push_constants_layout> push_constants_layout;
	variable_ref_map_t spec_variables_map;
	spec_map_t specializations;

	// Layouts of the descriptor sets
	binding_sets_layout_map_t bindings_set_layouts;
	std::unique_ptr<vk::vk_pipeline_layout> layout;

	// External binding sets
	const pipeline_external_binding_set_collection *external_binding_sets{ nullptr };

	// Layout can be modified (by respecializing constants, which define array length of binding variables).
	// In which case the affected sets need to be recreated.
	boost::container::flat_set<pipeline_layout_set_index> set_layouts_modified_queue;
	// Map of specialization variables to dependant array variables
	spec_to_dependant_array_variables_map_t spec_to_dependant_array_variables_map;
	// Set layout modified signal
	mutable set_layout_modified_signal_t set_layout_modified_signal;

	// If for any reason pipeline layout has changed, mark it and let device_pipeline recreate the pipeline when applicable.
	bool layout_invalidated_flag{ false };

private:
	static void update_variable(variable_map_t &map,
								std::vector<pipeline_binding_layout> &push_constant_bindings,
								const std::string &name,
								const pipeline_binding_layout &binding,
								ste_shader_program_stage stage) {
		// Push constants are handled differently, don't add to variables map
		if (binding.binding_type() == ste_shader_stage_binding_type::push_constant) {
			pipeline_binding_layout push_binding = binding;
			push_binding.stages.insert(stage);
			push_constant_bindings.push_back(std::move(push_binding));
			return;
		}

		// for uniforms/storage blocks and specialization constants, try add to variable map with new name
		auto ret = map.try_emplace(name, binding);
		auto &b = ret.first->second;
		if (!ret.second) {
			// If name exists, verify it is the same variable
			if (*b.binding->variable != *binding.binding->variable) {
				throw pipeline_layout_duplicate_variable_name_exception("Variable's name was already used in layout");
			}
		}

		// Append stage to variable stage list
		b.stages.insert(stage);
	}

	static void add_attachment(attachment_map_t &map,
							   const std::string &name,
							   const pipeline_attachment_layout &attachment) {
		auto ret = map.try_emplace(name, attachment);
		const auto &a = ret.first->second;
		if (!ret.second) {
			// Name exists, verify it is the same attachment
			if (*a.attachment->variable != *attachment.attachment->variable) {
				throw pipeline_layout_duplicate_variable_name_exception("Attachment name was already used in layout");
			}
		}
	}

	void erase_variables_provided_by_external_binding_sets(variable_map_t &map) {
		auto &external_sets = external_binding_sets->get_sets();

		for (auto it = map.begin(); it != map.end();) {
			auto& b = it->second;
			auto set_idx = b.set_idx();
			auto bind_idx = b.bind_idx();

			// If variable exists in external_binding_sets, validate compatibility and ignore the binding,
			// it is handled externally
			auto external_it = external_sets.find(set_idx);
			if (external_it != external_sets.end()) {
				const pipeline_external_binding_set_layout &external_set_layout = external_it->second.get_layout();

				// Find the binding and verify compatibility
				for (auto &external_binding : external_set_layout) {
					if (external_binding.bind_idx() == bind_idx) {
						external_binding.validate_layout(b.binding);
						break;
					}
				}

				// Set is handled externally
				auto next_it = it;
				++next_it;
				map.erase(it);
				it = next_it;
			}
			else {
				++it;
			}
		}
	}

	// Populates variable map, specialization constants map and push constants layout
	void create_variable_layouts(variable_map_t &&map,
								 const std::vector<pipeline_binding_layout> &push_constant_bindings) {
		for (auto &b : map) {
			auto &name = b.second.name();
			auto &val = b.second;

			// Push constants should have already been extracted, sanity check
			if (b.second.binding->binding_type == ste_shader_stage_binding_type::push_constant) {
				assert(false);
				continue;
			}

			// Also individually populate and map push and specialization constants
			if (b.second.binding->binding_type == ste_shader_stage_binding_type::spec_constant) {
				spec_variables_map[name] = &val;
			}

			// Map array variables whose length depends on specialization constants
			// This allows the pipeline layout to react to respecializations
			auto var_arr = dynamic_cast<const ste_shader_stage_variable_array*>(b.second.binding->variable.get());
			if (var_arr && var_arr->length_spec_constant()) {
				spec_to_dependant_array_variables_map[var_arr->get_length_spec_constant_var()].push_back(&val);
			}
		}

		// Store name->variable layout map
		variables_map = std::move(map);

		// Create push constants layout
		push_constants_layout = std::make_unique<pipeline_push_constants_layout>(push_constant_bindings);
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
			auto set_layout = pipeline_binding_set_layout(ctx.get(), std::move(v));

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
		auto *ptr = dynamic_cast<ste_shader_stage_variable_scalar*>(var);
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
	/**
	 *	@brief	Pipeline layout descriptor
	 *
	 *	@param	ctx						Context
	 *	@param	pipeline_shader_stages	Shader stages that define the layout
	 *	@param	external_binding_sets	External binding sets used in this layout
	 *
	 *	@throws	ste_shader_variable_layout_verification_exception		On different validation failures of a shader stage binding with
	 *																external_binding_sets
	 */
	pipeline_layout(const ste_context &ctx,
					const shader_stages_list_t &pipeline_shader_stages,
					optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets)
		: ctx(ctx)
	{
		{
			// Sort the variables from all the shader stages
			// Those data structures will be used later to generate the variables layouts
			variable_map_t all_variables;
			std::vector<pipeline_binding_layout> push_constant_bindings;

			for (auto &s : pipeline_shader_stages) {
				auto stage = s->get_stage();
				const auto& bindings = s->get_stage_bindings();
				const auto& attachments = s->get_stage_attachments();

				for (auto &b : bindings) {
					const auto& name = b.variable->name();
					pipeline_binding_layout new_bind;
					new_bind.binding = &b;

					// Add new variable, or update exciting ones with new stage
					update_variable(all_variables,
									push_constant_bindings,
									name,
									new_bind,
									stage);
				}

				for (auto &a : attachments) {
					// Create new attachment layout descriptor and add to map
					const auto& name = a.variable->name();
					pipeline_attachment_layout attachment = { &a };

					add_attachment(attachments_map,
								   name,
								   attachment);
				}

				// Store all stages
				stages[stage] = s;
			}

			// Verify and erase variables that are handled externally
			if (external_binding_sets) {
				this->external_binding_sets = &external_binding_sets.get().get();
				erase_variables_provided_by_external_binding_sets(all_variables);
			}

			// Then create variables' layouts
			// This creates the name->binding maps and specialization/push constants layout
			create_variable_layouts(std::move(all_variables),
									push_constant_bindings);
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
	*	@param	value	Value to specialize to. Must be a POD.
	*
	*	@throws	pipeline_layout_variable_not_found_exception	If specialization constant not found
	*/
	template <typename T>
	void specialize_constant(const std::string &name, const T &value) {
		using S = std::remove_cv_t<std::remove_reference_t<T>>;
		static_assert(std::is_pod_v<S>, "T must be a POD");

		std::string data;
		data.resize(sizeof(S));
		memcpy(data.data(), &value, sizeof(S));

		specialize_constant_impl<S>(name,
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
		std::vector<const vk::vk_descriptor_set_layout*> set_layout_ptrs;
		for (auto &s : bindings_set_layouts) {
			set_layout_ptrs.push_back(&s.second.get());
		}

		if (external_binding_sets != nullptr) {
			// Add external set layouts to pipeline layout
			for (auto &es : external_binding_sets->get_layouts()) {
				set_layout_ptrs.push_back(&es.get());
			}
		}

		// Create push contant layout descriptors
		auto& push_constant_layouts = push_constants_layout->vk_push_constant_layout_descriptors();

		// Create pipeline layout and raise layout invalidated flag
		layout = std::make_unique<vk::vk_pipeline_layout>(ctx.get().device(),
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
			auto set_layout = pipeline_binding_set_layout(ctx.get(), 
														  std::move(bindings));

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
		std::vector<vk::vk_shader_stage_descriptor> descriptors;
		for (auto &s : stages)
			descriptors.push_back(s.second->pipeline_stage_descriptor());

		return descriptors;
	}

	/**
	 *	@brief	Creates a push constants command to push constants onto the command buffer
	 */
	auto cmd_push_constants() const {
		assert(layout.get() && "Called before creating layout");
		return push_constants_layout->cmd_push(layout.get());
	}

	auto& get() const { return *layout; }

	bool is_pipeline_layout_invalidated() const { return layout_invalidated_flag; }

	auto& set_layouts() const { return bindings_set_layouts; }
	auto& modified_set_layouts() const { return set_layouts_modified_queue; }

	auto& variables() const { return variables_map; }
	auto& push_variables() const { return *push_constants_layout; }
	auto& spec_variables() const { return spec_variables_map; }
	auto& attachments() const { return attachments_map; }

	auto& get_set_layout_modified_signal() const { return set_layout_modified_signal; }
};

}
}
