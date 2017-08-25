//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_pipeline_layout.hpp>
#include <vk_push_constant_layout.hpp>
#include <pipeline_binding_stages_collection.hpp>
#include <push_constant_descriptor.hpp>

#include <command_recorder.hpp>
#include <cmd_push_constants.hpp>

#include <range.hpp>
#include <lib/vector.hpp>
#include <algorithm>
#include <lib/flat_map.hpp>

namespace ste {
namespace gl {

class pipeline_push_constants_layout {
	friend class pipeline_push_constant_bind_point;

private:
	struct push_variable {
		const ste_shader_stage_variable *variable;
		lib::string push_path;

		std::uint32_t offset;
		pipeline_binding_stages_collection stages;
	};

	struct stage_range {
		stage_flag stage;
		range<> r;
	};

	// Update push constants command
	class pipeline_push_constants_layout_cmd_push_constants : public command {
		const pipeline_push_constants_layout *p;
		const vk::vk_pipeline_layout<> *pipeline_layout;

	public:
		pipeline_push_constants_layout_cmd_push_constants(const pipeline_push_constants_layout *p,
														  const vk::vk_pipeline_layout<> *pipeline_layout)
			: p(p), pipeline_layout(pipeline_layout)
		{}
		virtual ~pipeline_push_constants_layout_cmd_push_constants() noexcept {}

	private:
		void operator()(const command_buffer &, command_recorder &recorder) const override final {
			for (auto &r : p->push_ranges) {
				auto push_layout = r.get_layout();
				auto offset = push_layout.offset;
				auto size = push_layout.size;
				auto range_data = lib::string(p->data.begin() + offset,
											  p->data.begin() + offset + size);

				recorder << cmd_push_constants(pipeline_layout,
											   static_cast<stage_flag>(push_layout.stageFlags),
											   offset,
											   range_data);
			}
		}
	};

private:
	push_constant_descriptor root;
	lib::vector<vk::vk_push_constant_layout> push_ranges;
	lib::string data;

private:
	static void populate_push_variables(const ste_shader_stage_variable *variable,
										std::uint32_t parent_offset,
										const pipeline_binding_stages_collection &stages,
										const lib::string &path,
										lib::vector<push_variable> &variables,
										push_constant_descriptor &node) {
		auto offset = parent_offset + variable->offset();
		auto push_path = path + "." + variable->name();

		// Add a child to current node
		// No checking is done here. Stricter and more comprehensive checking is done per variable later when inserting into 
		// 'variables' array.
		auto& child_node_it = node.emplace_child(variable, parent_offset);

		const auto* struct_variable = dynamic_cast<const ste_shader_stage_variable_struct*>(variable);
		if (struct_variable != nullptr) {
			// A struct variable, populate members
			for (auto &element : *struct_variable) {
				populate_push_variables(element.get(), 
										offset, 
										stages, 
										push_path,
										variables,
										child_node_it);
			}
		}
		else {
			// Non-struct variable, insert into variables in correct position
			auto it = std::lower_bound(variables.begin(), variables.end(), offset, [](const auto &v, std::uint32_t o) {
				return v.offset < o;
			});

			if (it != variables.end() &&
				it->offset == offset) {
				// If a variable exists at this offset, make sure it is the same variable with identical alias
				if (!it->variable->compatible(*variable) ||
					it->push_path != push_path) {
					throw pipeline_layout_duplicate_incompatible_overlapping_push_constants_exception("Overlapping push constants are incompatible");
				}

				it->stages.insert(stages);
			}
			else {
				// No variable exactly at this offset, verify 
				push_variable t = { variable, push_path, offset, stages };
				variables.insert(it, std::move(t));
			}
		}
	}

	static auto generate_ranges(const lib::vector<push_variable> &variables,
								const pipeline_binding_stages_collection &all_stages,
								std::size_t data_size) {
		lib::vector<stage_range> ranges;
		ranges.reserve(all_stages.size());

		// Create a push range per stage
		for (auto &stage : all_stages) {
			std::uint32_t stage_range_start = static_cast<std::uint32_t>(data_size);
			std::uint32_t stage_range_end = 0;
			for (auto it = variables.begin(); it != variables.end(); ++it) {
				if (!it->stages.exists(stage))
					continue;

				std::uint32_t offset = it->offset;
				std::uint32_t size = it->variable->size_bytes();

				stage_range_start = std::min(stage_range_start, offset);
				stage_range_end =	std::max(stage_range_end,	offset + size);
			}

			// Create range
			auto r = stage_range{ ste_shader_program_stage_to_stage_flag(stage), range<>(stage_range_start, stage_range_end - stage_range_start) };

			// Insert range for stage, sorted.
			auto it = std::upper_bound(ranges.begin(), ranges.end(), r, [](const auto &lhs, const auto &rhs) {
				return lhs.r.start < rhs.r.start;
			});
			ranges.insert(it, std::move(r));
		}

		// Group identical ranges together
		lib::vector<vk::vk_push_constant_layout> layouts;
		layouts.reserve(ranges.size());
		for (std::size_t i=0; i<ranges.size();) {
			stage_flag stages = ranges[i].stage;

			auto j = i + 1;
			while (j<ranges.size() && ranges[i].r == ranges[j].r)
				stages = stages | ranges[j].stage;

			layouts.push_back(vk::vk_push_constant_layout(static_cast<VkShaderStageFlags>(stages),
														  static_cast<std::uint32_t>(ranges[i].r.length),
														  static_cast<std::uint32_t>(ranges[i].r.start)));

			i = j;
		}

		return layouts;
	}

	template <typename T>
	void write_constant(const T &t,
						const push_constant_descriptor *constant) {
		using S = std::remove_cv_t<std::remove_reference_t<T>>;
		static_assert(std::is_pod_v<S> || is_arithmetic_v<S>, "T must be a POD or arithmetic type");

		// Validate type
		constant->validate<S>();

		// And copy to push constants data
		auto offset = constant->offset();
		*reinterpret_cast<S*>(data.data() + offset) = t;
	}

public:
	pipeline_push_constants_layout(lib::vector<pipeline_binding_layout> push_bindings) {
		pipeline_binding_stages_collection all_stages;
		lib::vector<push_variable> variables;
		for (auto &b : push_bindings) {
			// Save a set of all stages that use this push constants layout
			all_stages.insert(b.stages);

			// Populate all push constants from binding
			populate_push_variables(b.binding->variable.get(),
									0,
									b.stages,
									"",
									variables,
									root);
		}

		if (variables.empty()) {
			// No push-constants
			return;
		}

		// Resize data, the push constants storage, to fit all the constants
		data.resize(variables.back().offset + variables.back().variable->size_bytes());

		// Generate the stages' push ranges
		this->push_ranges = generate_ranges(variables,
											all_stages,
											data.size());
	}
	~pipeline_push_constants_layout() noexcept {}

	pipeline_push_constants_layout(pipeline_push_constants_layout&&) = default;
	pipeline_push_constants_layout &operator=(pipeline_push_constants_layout&&) = default;

	optional<const push_constant_descriptor*> operator[](push_constant_path &path) const {
		auto ret = root[path];
		// Root is not a valid constant (it means path was empty, which is an error)
		if (ret && ret.get() == &root)
			return none;
		return ret;
	}

	/**
	 *	@brief	Returns Vulkan push constants layout descriptors
	 */
	auto& vk_push_constant_layout_descriptors() const {
		return push_ranges;
	}

	/**
	 *	@brief	Command to push constants
	 */
	auto cmd_push(const vk::vk_pipeline_layout<> *pipeline_layout) const {
		return pipeline_push_constants_layout_cmd_push_constants(this, pipeline_layout);
	}

	auto size() const { return data.size(); }
};

}
}
