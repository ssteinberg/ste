//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <pipeline_layout_exceptions.hpp>

#include <ste_shader_stage.hpp>
#include <device_pipeline_shader_stage.hpp>

#include <ste_shader_stage_binding_variable.hpp>
#include <pipeline_binding_layout_set.hpp>

#include <pipeline_descriptor_set_layout.hpp>
#include <vk_pipeline_layout.hpp>

#include <string>
#include <boost/container/flat_map.hpp>

namespace StE {
namespace GL {

using set_index_t = std::uint32_t;

/**
*	@brief	Describes the pipeline shader stages and resource binding points
*/
class pipeline_layout {
	friend class device_pipeline;

public:
	using shader_stage_t = device_pipeline_shader_stage*;
	using shader_stages_list_t = std::vector<shader_stage_t>;

private:
	using stages_map_t = boost::container::flat_map<ste_shader_stage, shader_stage_t>;

	using variable_map_t = pipeline_binding_layout_set;
	using variable_ref_map_t = boost::container::flat_map<std::string, pipeline_binding_set_layout_binding*>;

	using spec_map_t = std::unordered_map<ste_shader_stage, vk_shader::spec_map>;

	using set_descriptor_set_layout_map_t = boost::container::flat_map<set_index_t, pipeline_descriptor_set_layout>;

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
	set_descriptor_set_layout_map_t descriptor_set_layout;
	std::unique_ptr<vk_pipeline_layout> layout;

	bool layout_modified{ true };

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

	void create_layouts() {
		// Sort variables into sets
		boost::container::flat_map<set_index_t, std::vector<vk_descriptor_set_layout_binding>> sets;
		for (auto &v : variables_map) {
			auto& b = *v.second.binding;
			if (b.binding_type == ste_shader_stage_binding_type::uniform ||
				b.binding_type == ste_shader_stage_binding_type::storage)
				sets[b.set_idx].push_back(v.second);
		}

		// Create descriptor set layouts
		std::vector<VkDescriptorSetLayout> set_layouts;
		for (auto &s : sets) {
			auto set_layout = pipeline_descriptor_set_layout(ctx, s.second);
			set_layouts.push_back(set_layout.get());
			descriptor_set_layout.emplace(std::make_pair(s.first, std::move(set_layout)));
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
													  set_layouts,
													  push_constant_layouts);
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

				pipeline_binding_set_layout_binding &inserted = update_variable(name,
																				new_bind,
																				stage);

				if (b.binding_type == ste_shader_stage_binding_type::spec_constant) {
					spec_variables_map[name] = &inserted;
				}
				else if (b.binding_type == ste_shader_stage_binding_type::push_constant) {
					push_variables_map[name] = &inserted;
				}
			}

			stages[stage] = s;
		}

		// Generate the descriptor set layout for each set and the pipeline layout
		create_layouts();
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

		// Specialize the binding scalar variable
		// Used for dynamic array lengths
		auto it = spec_variables_map.find(name);
		if (it == spec_variables_map.end()) {
			throw pipeline_layout_variable_not_found_exception("Specialization constant with provided name not found");
		}

		auto &b = *it->second->binding;
		auto ptr = dynamic_cast<ste_shader_stage_binding_variable_scalar *>(b.variable.get());
		assert(ptr);
		if (ptr) {
			// Do nothing if value unchanged
			if (ptr->read_specialized_value<T>() == value)
				return;

			ptr->specialize_bin(data);
		}

		for (auto &s : it->second->stages) {
			auto& map = specializations[s];
			map[b.bind_idx] = data;
			stages[s]->set_specializations(map);
		}

		layout_modified = true;
	}
	/**
	*	@brief	Remove a specialization of constant
	*	
	*	@throws	pipeline_layout_variable_not_found_exception	If specialization constant not found
	*/
	void remove_specialization(const std::string &name) {
		auto it = spec_variables_map.find(name);
		if (it == spec_variables_map.end()) {
			throw pipeline_layout_variable_not_found_exception("Specialization constant with provided name not found");
		}

		auto &b = *it->second->binding;
		auto ptr = dynamic_cast<ste_shader_stage_binding_variable_scalar *>(b.variable.get());
		assert(ptr);
		if (ptr) {
			// Do nothing if value unchanged
			if (!ptr->has_non_default_specialization())
				return;

			ptr->reset_specialization();
		}

		for (auto &s : it->second->stages) {
			auto& map = specializations[s];
			map.erase(b.bind_idx);
			stages[s]->set_specializations(map);
		}

		layout_modified = true;
	}
};

}
}
