//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_binding_set_pool.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_layout_set_index.hpp>
#include <pipeline_layout_exceptions.hpp>

#include <pipeline_external_binding_layout.hpp>
#include <pipeline_external_binding_set_impl.hpp>

#include <pipeline_external_binding_set_cmd_bind.hpp>

#include <vk_shader.hpp>

#include <lib/flat_map.hpp>
#include <lib/unique_ptr.hpp>
#include <alias.hpp>
#include <signal.hpp>

namespace ste {
namespace gl {

class pipeline_external_binding_set {
	friend class pipeline_external_resource_bind_point;
	friend class pipeline_layout;

private:
	using layout_t = pipeline_external_binding_set_layout;

	using name_binding_map_t = lib::flat_map<lib::string, const pipeline_external_binding_layout*>;

	using spec_to_dependant_array_variables_t = lib::flat_set<const ste_shader_stage_variable*>;
	using spec_map_t = lib::flat_map<ste_shader_program_stage, vk::vk_shader<>::spec_map>;

	using cmd_bind_t = pipeline_external_binding_set_cmd_bind;

public:
	using signal_specialization_change_t = signal<const pipeline_external_binding_set*, const pipeline_binding_stages_collection&>;
	using signal_set_invalidated_t = signal<const pipeline_external_binding_set*>;

private:
	alias<pipeline_binding_set_pool> pool;

	pipeline_layout_set_index set_index;
	lib::unique_ptr<_internal::pipeline_external_binding_set_impl> set;
	layout_t layout;

	name_binding_map_t name_map;

	name_binding_map_t spec_variables_map;
	spec_to_dependant_array_variables_t spec_dependant_array_variables;
	spec_map_t specializations;
	mutable signal_specialization_change_t signal_specialization_change;
	mutable signal_set_invalidated_t signal_set_invalidated;

	pipeline_resource_binding_queue binding_queue;

private:
	/**
	*	@brief	Updates the binding set with resource bindings
	*/
	void write_binding_queue() {
		if (binding_queue.empty())
			return;

		for (auto &b : binding_queue) {
			auto &writes = b.second;
			set->write(writes);
		}

		binding_queue.clear();
	}

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

		auto &b = it->second->get_binding();
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
		for (auto &s : it->second->stage_collection()) {
			auto& map = specializations[s];
			if (data)
				map[b.bind_idx] = data.get();
			else
				map.erase(b.bind_idx);
		}

		// Signal specialization constant change
		signal_specialization_change.emit(this, it->second->stage_collection());

		// If some array variable depends on this specialization constant, we need to update the relevant set descriptor layouts
		if (spec_dependant_array_variables.find(var) != spec_dependant_array_variables.end()) {
			// Signal set invalidation, the set should be recreated.
			signal_set_invalidated.emit(this);
		}
	}

	/**
	*	@brief	Specializes constant at a binding point to a value
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
	*	@brief	Remove a specialization of constant at a binding point
	*
	*	@throws	pipeline_layout_variable_not_found_exception	If specialization constant not found
	*/
	void remove_specialization(const lib::string &name) {
		specialize_constant_impl<unsigned>(name);
	}

public:
	/**
	 *	@brief	External binding set collection constructor.
	 */
	pipeline_external_binding_set(layout_t &&input_layout,
								  pipeline_binding_set_pool &pool)
		: pool(pool),
		set_index(input_layout.get_set_index()),
		layout(std::move(input_layout))
	{
		for (auto &b : layout) {
			if (b.get_binding().binding_type == ste_shader_stage_binding_type::push_constant) {
				throw pipeline_layout_push_constant_in_external_set_exception("External binding sets can not containt push constants");
			}

			// Check for duplicate names
			auto ret = name_map.try_emplace(b.name(), &b);
			if (!ret.second) {
				// Name already exists
				throw pipeline_layout_duplicate_variable_name_exception("Variable's name already exists in collection");
			}

			// Individually map specialization constants
			if (b.get_binding().binding_type == ste_shader_stage_binding_type::spec_constant) {
				spec_variables_map[b.name()] = &b;
			}

			// Map array variables whose length depends on specialization constants
			auto var_arr = dynamic_cast<const ste_shader_stage_variable_array*>(b.get_binding().variable.get());
			if (var_arr && var_arr->length_spec_constant()) {
				spec_dependant_array_variables.insert(var_arr->get_length_spec_constant_var());
			}
		}

		// Allocate
		const lib::vector<const layout_t*> layout_ptrs = { &layout };
		auto allocated_sets = pool.allocate_binding_sets<layout_t>(layout_ptrs);

		set = lib::allocate_unique<_internal::pipeline_external_binding_set_impl>(std::move(allocated_sets[0]));
	}
	~pipeline_external_binding_set() noexcept {}


	pipeline_external_binding_set(pipeline_external_binding_set&&) = default;
	pipeline_external_binding_set &operator=(pipeline_external_binding_set&&) = default;

	/**
	*	@brief	Rebuilds the set and layout
	 *
	 *	@param	device			Creating device
	 *	@param	old_layouts		If non-null, old set layout will be pushed into this vector.
	*
	*	@return	Returns the old set
	*/
	auto recreate_set(const vk::vk_logical_device<> &device,
					  lib::vector<vk::vk_descriptor_set_layout<>> *old_layouts = nullptr) {
		lib::vector<const layout_t*> layout_ptrs;

		// Recreate layout
		auto old_layout = layout.recreate(device);

		// Store old
		auto old_set = std::move(*set);
		if (old_layouts)
			old_layouts->push_back(std::move(old_layout));

		layout_ptrs.push_back(&layout);

		// Allocate the new set
		*set = std::move(pool.get().allocate_binding_sets<layout_t>(layout_ptrs)[0]);

		// Copy bindings from old set
		set->copy(old_set);


		return old_set;
	}

	/**
	*	@brief	Creates a resource binder for a given variable name
	*/
	pipeline_external_resource_bind_point operator[](const lib::string &resource_name);

	auto set_idx() const { return set_index; }
	auto& get_set() const { return *set; }
	auto& get_layout() const { return layout; }

	/**
	 *	@brief	Updates resource writes
	 */
	void update() {
		write_binding_queue();
	}

	/**
	 *	@brief	Returns the specialization constant modified signal
	 */
	auto& get_signal_specialization_change() const { return signal_specialization_change; }

	/**
	 *	@brief	Returns the set invalidated signal
	 */
	auto& get_signal_set_invalidated() const { return signal_set_invalidated; }

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

#include <pipeline_external_resource_bind_point.hpp>
