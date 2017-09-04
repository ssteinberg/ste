//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <pipeline_layout_exceptions.hpp>

#include <ste_shader_program_stage.hpp>
#include <device_pipeline_shader_stage.hpp>

#include <ste_shader_stage_variable.hpp>
#include <pipeline_external_binding_set.hpp>
#include <pipeline_binding_layout_collection.hpp>

#include <pipeline_attachment_layout.hpp>
#include <pipeline_attachment_layout_collection.hpp>

#include <pipeline_push_constants_layout.hpp>
#include <pipeline_layout_set_index.hpp>
#include <pipeline_binding_set_layout.hpp>
#include <vk_pipeline_layout.hpp>

#include <allow_type_decay.hpp>
#include <lib/string.hpp>
#include <lib/flat_map.hpp>
#include <lib/flat_set.hpp>

#include <alias.hpp>
#include <anchored.hpp>
#include <connection.hpp>
#include <atomic_pod.hpp>

namespace ste {
namespace gl {

/**
*	@brief	The pipeline layout descriptor.
*			Fully defines the pipeline shader stages, resource binding layouts and output attachment layouts.
*/
class pipeline_layout : public allow_type_decay<pipeline_layout, vk::vk_pipeline_layout<>>, anchored {
	friend class device_pipeline;

public:
	using shader_stage_t = device_pipeline_shader_stage*;
	using shader_stages_list_t = lib::vector<shader_stage_t>;

	using set_layout_modified_signal_t = signal<const lib::vector<pipeline_layout_set_index> &>;

private:
	using stages_map_t = lib::flat_map<ste_shader_program_stage, shader_stage_t>;

	using variable_map_t = pipeline_binding_layout_collection;
	using variable_ref_map_t = lib::flat_map<lib::string, pipeline_binding_layout*>;

	using attachment_map_t = pipeline_attachment_layout_collection;

	using spec_map_t = lib::flat_map<ste_shader_program_stage, vk::vk_shader<>::spec_map>;

	using binding_sets_layout_t = lib::vector<pipeline_binding_set_layout>;

	using spec_to_dependant_array_variables_map_t =
		lib::flat_map<const ste_shader_stage_variable*, lib::vector<const pipeline_binding_layout*>>;

private:
	alias<const ste_context> ctx;

	// Attached pipeline stages and their variables and attachment maps
	stages_map_t stages;
	variable_map_t variables_map;
	attachment_map_t attachments_map;

	// Push and specialization constants maps, as well as specializations.
	lib::unique_ptr<pipeline_push_constants_layout> push_constants_layout;
	variable_ref_map_t spec_variables_map;
	spec_map_t specializations;

	// Layouts of the descriptor sets
	binding_sets_layout_t bindings_set_layouts;
	lib::unique_ptr<vk::vk_pipeline_layout<>> layout;

	// External binding set
	const pipeline_external_binding_set *external_binding_set{ nullptr };
	// And connections
	pipeline_external_binding_set::signal_set_invalidated_t::connection_type external_binding_set_invalidated_connection;
	pipeline_external_binding_set::signal_specialization_change_t::connection_type external_binding_set_constant_specialized_connection;

	// Layout can be modified (by respecializing constants, which define array length of binding variables).
	// In which case the affected sets need to be recreated.
	lib::flat_set<pipeline_layout_set_index> set_layouts_modified_queue;

	// Map of specialization variables to dependant array variables
	spec_to_dependant_array_variables_map_t spec_to_dependant_array_variables_map;

	// If for any reason pipeline layout has changed, mark it and let device_pipeline recreate the pipeline when applicable.
	atomic_pod<bool> layout_invalidated_flag{ false };

private:
	static void update_variable(variable_map_t &map,
								lib::vector<pipeline_binding_layout> &push_constant_bindings,
								const lib::string &name,
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
							   const lib::string &name,
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

	void erase_sets_provided_by_external_binding_sets(variable_map_t &map) {
		auto &external_set_layout = external_binding_set->get_layout();

		for (auto it = map.begin(); it != map.end();) {
			auto& b = it->second;
			auto set_idx = b.set_idx();
			auto bind_idx = b.bind_idx();

			if (b.binding->binding_type == ste_shader_stage_binding_type::push_constant)
				continue;

			// If variable belongs to a set provided by external_binding_set, validate compatibility and ignore the binding,
			// it is handled externally
			if (external_binding_set->set_idx() == set_idx) {
				// Find the binding
				bool found = false;
				for (auto &external_binding : external_set_layout) {
					if (external_binding.bind_idx() == bind_idx) {
						// Verify
						if (!external_binding.get_binding().compatible(*b.binding)) {
							throw pipeline_layout_variable_incompatible_with_external_set_exception("Variable is incompatible with external binding set");
						}

						found = true;
						break;
					}
				}

				if (!found) {
					// Not found in external binding set
					throw pipeline_layout_variable_not_found_in_external_set_exception("Variable bound to external set but not found in external binding sets");
				}

				// Set is handled externally
				auto next_it = std::next(it);
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
								 const lib::vector<pipeline_binding_layout> &push_constant_bindings) {
		for (auto &b : map) {
			auto &name = b.second.name();
			auto &val = b.second;

			// Push constants should have already been extracted, sanity check
			if (b.second.binding->binding_type == ste_shader_stage_binding_type::push_constant) {
				assert(false);
				continue;
			}

			// Also individually map specialization constants
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
		push_constants_layout = lib::allocate_unique<pipeline_push_constants_layout>(push_constant_bindings);
	}

	void create_set_layouts() {
		if (!variables_map.size())
			return;

		// Find first set index, and count
		pipeline_layout_set_index largest_set_idx = 0;
		for (auto &v : variables_map)
			largest_set_idx = std::max(largest_set_idx, v.second.binding->set_idx);
		
		// Sort variables into sets
		lib::vector<pipeline_binding_set_layout::bindings_vec_t> sets;
		const auto set_count = largest_set_idx + 1;
		sets.resize(set_count);
		for (auto &v : variables_map) {
			auto& b = v.second;
			if (b.binding->binding_type == ste_shader_stage_binding_type::uniform ||
				b.binding->binding_type == ste_shader_stage_binding_type::storage) {
				sets[b.binding->set_idx].push_back(&b);
			}
		}

		// If we have an external binding set: 
		// 1) External set must be bound to a set index greater than any local set
		// 2) Make sure we have consecutive set layouts, including the external set. 
		if (external_binding_set) {
			if (external_binding_set->set_idx() <= largest_set_idx) {
				throw pipeline_layout_exception("External binding set overlaps local binding set");
			}

			largest_set_idx = external_binding_set->set_idx() - 1;
		}

		// Create descriptor set layouts
		for (std::size_t i=0; i<sets.size(); ++i) {
			auto &v = sets[i];

			auto set_layout = pipeline_binding_set_layout(ctx.get().device(), 
														  std::move(v),
														  static_cast<pipeline_layout_set_index>(i));
			bindings_set_layouts.emplace_back(std::move(set_layout));
		}
	}

	/**
	*	@brief	Flags the set as modified
	*/
	void invalidate_set_layout(const pipeline_layout_set_index &index) {
		set_layouts_modified_queue.insert(index);
		std::atomic_thread_fence(std::memory_order_release);
	}

	/**
	*	@brief	Flags the pipeline layout invalid
	*/
	void invalidate_layout() {
		layout_invalidated_flag.get().store(true, std::memory_order_release);
	}

	/**
	 *	@brief	Updates the specialization map of an attached shader stage
	 */
	void update_shader_stage_specialization_map(const ste_shader_program_stage &stage) {
		const auto& map = specializations[stage];
		if (!this->external_binding_set) {
			// No external binding set attached
			stages[stage]->set_specializations(vk::vk_shader<>::spec_map(map));
			return;
		}

		const auto it = this->external_binding_set->specializations.find(stage);
		if (it == this->external_binding_set->specializations.end()) {
			// External binding set does not have any specializations for the stage
			stages[stage]->set_specializations(vk::vk_shader<>::spec_map(map));
			return;
		}

		const auto &external_map = it->second;

		// Combine specialization maps
		vk::vk_shader<>::spec_map spec;
		for (auto &s : map)
			spec.emplace(s);
		for (auto &s : external_map)
			spec.emplace(s);
		
		stages[stage]->set_specializations(std::move(spec));
	}

	/**
	 *	@brief	Specializes a constant
	 */
	template <typename T>
	void specialize_constant_impl(const lib::string &name,
								  const optional<lib::string> &data = none,
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
			update_shader_stage_specialization_map(s);
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

	/**
	 *	@brief	Attaches an external binding set to the layout
	 */
	void attach_external_binding_set_collection(const pipeline_external_binding_set &external_binding_set) {
		this->external_binding_set = &external_binding_set;

		// Update specializations
		for (auto &spec : external_binding_set.specializations) {
			update_shader_stage_specialization_map(spec.first);
		}

		// Connect to external binding set's signals
		external_binding_set_invalidated_connection = make_connection(external_binding_set.get_signal_set_invalidated(), [this](auto) {
			// External binding set was invalidated, add to set recreation queue and invalidate layout.
			set_layouts_modified_queue.insert(this->external_binding_set->set_idx());
			std::atomic_thread_fence(std::memory_order_release);

			invalidate_layout();
		});
		external_binding_set_constant_specialized_connection = make_connection(external_binding_set.get_signal_specialization_change(),
																			   [this](auto, const auto &stages) {
			// External binding set's constant was specialized, update specialization map
			for (auto &s : stages)
				update_shader_stage_specialization_map(s);
		});
	}

public:
	/**
	 *	@brief	Pipeline layout descriptor
	 *
	 *	@param	ctx						Context
	 *	@param	pipeline_shader_stages	Shader stages that define the layout
	 *	@param	external_binding_set	External binding sets used in this layout
	 *
	 *	@throws	ste_shader_variable_layout_verification_exception		On different validation failures of a shader stage binding with
	 *																external_binding_set
	 */
	pipeline_layout(const ste_context &ctx,
					const shader_stages_list_t &pipeline_shader_stages,
					optional<std::reference_wrapper<const pipeline_external_binding_set>> external_binding_set)
		: ctx(ctx)
	{
		{
			// Sort the variables from all the shader stages
			// Those data structures will be used later to generate the variables layouts
			variable_map_t all_variables;
			lib::vector<pipeline_binding_layout> push_constant_bindings;

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
					const pipeline_attachment_layout attachment = { &a };

					add_attachment(attachments_map,
								   name,
								   attachment);
				}

				// Store all stages
				stages[stage] = s;
			}

			// Verify and erase variables that are handled externally
			if (external_binding_set) {
				attach_external_binding_set_collection(external_binding_set.get().get());
				erase_sets_provided_by_external_binding_sets(all_variables);
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

	/**
	*	@brief	Specializes a constant
	*
	*	@param	name		Name of specialization constant
	*	@param	value	Value to specialize to. Must be a POD or arithmetic type.
	*
	*	@throws	pipeline_layout_variable_not_found_exception	If specialization constant not found
	*/
	template <typename T>
	void specialize_constant(const lib::string &name, const T &value) {
		using S = std::remove_cv_t<std::remove_reference_t<T>>;
		static_assert(std::is_pod_v<S> || is_arithmetic_v<S>, "T must be a POD or arithmetic type");

		lib::string data;
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
	void remove_specialization(const lib::string &name) {
		specialize_constant_impl<unsigned>(name);
	}

	/**
	 *	@brief	Recreates a specific set layout.
	 *			Returns the old set.
	 *
	 *	@throws	pipeline_layout_exception	If set index not found
	 */
	auto recreate_set_layout(pipeline_layout_set_index set_idx) {
		if (bindings_set_layouts.size() <= set_idx) {
			// Not found
			throw pipeline_layout_exception("Set does not exist");
		}

		return bindings_set_layouts[set_idx].recreate(ctx.get().device());
	}

	/**
	*	@brief	Recreates the pipeline layout and resets flag
	*
	*	@return	Returns the old layout object
	*/
	lib::unique_ptr<vk::vk_pipeline_layout<>> recreate_layout() {
		auto size = bindings_set_layouts.size();
		if (external_binding_set != nullptr) {
			// The external binding set is the last one, immediately after the local sets. This invariant is ensured in create_set_layouts().
			++size;
			assert(external_binding_set->set_idx() + 1 == size);
		}

		lib::vector<const vk::vk_descriptor_set_layout<>*> set_layout_ptrs;
		set_layout_ptrs.resize(size, nullptr);

		for (std::size_t i=0; i<bindings_set_layouts.size(); ++i)
			set_layout_ptrs[i] = &bindings_set_layouts[i].get();
		if (external_binding_set != nullptr) {
			// Add external set layout to pipeline layouts
			set_layout_ptrs[external_binding_set->set_idx()] = &external_binding_set->get_layout().get();
		}

		// Create push contant layout descriptors
		auto& push_constant_layouts = push_constants_layout->vk_push_constant_layout_descriptors();

		// Save old layout
		auto old_layout = std::move(layout);

		// Create pipeline layout
		layout = lib::allocate_unique<vk::vk_pipeline_layout<>>(ctx.get().device(),
																set_layout_ptrs,
																push_constant_layouts);
		// Erase invalid flag
		layout_invalidated_flag.get().store(false, std::memory_order_relaxed);

		return old_layout;
	}

	/**
	*	@brief	Returns the status of the layout invalid flag
	*/
	auto is_layout_invalidated() const {
		return layout_invalidated_flag.get().load(std::memory_order_acquire);
	}

	/**
	 *	@brief	Returns a copy of the queue of modified set indices, and clears the queue.
	 */
	auto read_and_clear_modified_sets_queue() {
		std::atomic_thread_fence(std::memory_order_acquire);

		auto v = set_layouts_modified_queue;
		set_layouts_modified_queue.clear();

		return v;
	}

	/**
	*	@brief	Returns the collection of shader stage descriptors, used to create pipeline objects.
	*/
	auto shader_stage_descriptors() const {
		lib::vector<vk::vk_shader_stage_descriptor<>> descriptors;
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

	auto& set_layouts() const { return bindings_set_layouts; }

	auto& variables() const { return variables_map; }
	auto& push_variables() const { return *push_constants_layout; }
	auto& spec_variables() const { return spec_variables_map; }
	auto& attachments() const { return attachments_map; }
};

}
}
