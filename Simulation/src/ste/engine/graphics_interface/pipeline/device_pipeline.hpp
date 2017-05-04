//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <pipeline_layout.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_bind_point.hpp>
#include <pipeline_push_constant_bind_point.hpp>
#include <pipeline_specialization_constant_bind_point.hpp>
#include <pipeline_resource_bind_point.hpp>
#include <device_pipeline_exceptions.hpp>

#include <push_constant_path.hpp>
#include <pipeline_layout_set_index.hpp>
#include <pipeline_binding_set_collection.hpp>
#include <pipeline_binding_set_pool.hpp>

#include <command_buffer.hpp>
#include <command_recorder.hpp>
#include <cmd_bind_descriptor_sets.hpp>

#include <optional.hpp>

namespace ste {
namespace gl {

class device_pipeline {
private:
	// Bind command
	class device_pipeline_cmd_bind : public command {
		const device_pipeline *pipeline;

	public:
		device_pipeline_cmd_bind(const device_pipeline *pipeline)
			: pipeline(pipeline)
		{}
		virtual ~device_pipeline_cmd_bind() noexcept {}

	private:
		void operator()(const command_buffer &buffer, command_recorder &recorder) const override final {
			// Bind external binding sets
			if (pipeline->external_binding_sets != nullptr)
				recorder << pipeline->external_binding_sets->cmd_bind(pipeline->get_pipeline_vk_bind_point(),
																	  &pipeline->layout.get());
			// Bind pipeline's binding sets
			recorder << pipeline->binding_sets.cmd_bind(pipeline->get_pipeline_vk_bind_point());

			// Bind pipeline
			pipeline->bind_pipeline(buffer, recorder);

			// Push constants
			recorder << pipeline->layout.cmd_push_constants();
		}
	};

	// Unbind command
	class device_pipeline_cmd_unbind : public command {
		const device_pipeline *pipeline;

	public:
		device_pipeline_cmd_unbind(const device_pipeline *pipeline)
			: pipeline(pipeline)
		{}
		virtual ~device_pipeline_cmd_unbind() noexcept {}

	private:
		void operator()(const command_buffer &buffer, command_recorder &recorder) const override final {
			pipeline->unbind_pipeline(buffer, recorder);
		}
	};

protected:
	const ste_context &ctx;

	pipeline_layout layout;
	pipeline_resource_binding_queue binding_queue;

	pipeline_binding_set_collection binding_sets;

	// External binding sets
	const pipeline_external_binding_set_collection *external_binding_sets;

private:
	void prebind_update() {
		// Update sets, as needed
		layout.recreate_invalidated_set_layouts();

		// Recreate pipeline if pipeline layout was invalidated for any reason
		if (layout.read_and_reset_invalid_layout_flag()) {
			recreate_pipeline();
		}

		// Write resource descriptors to binding sets from the binding queue and clear it
		if (!binding_queue.empty()) {
			binding_sets.write(binding_queue);
			binding_queue.clear();
		}

		// Call overloadable update
		update();
	}

protected:
	/**
	 *	@brief	Update is called just before binding the pipeline.
	 *			Any kind of lazy (re)instantiation should be done here.
	 */
	virtual void update() {}
	/**
	*	@brief	Overrides should specify the pipeline Vulkna bind point name here.
	*/
	virtual VkPipelineBindPoint get_pipeline_vk_bind_point() const = 0;
	/**
	*	@brief	Overrides should bind the pipeline and other resources here.
	*/
	virtual void bind_pipeline(const command_buffer &, command_recorder &) const = 0;
	/**
	*	@brief	Overrides should clean up and unbind the pipeline and other resources here.
	*/
	virtual void unbind_pipeline(const command_buffer &, command_recorder &) const {}
	/**
	*	@brief	Recreates the pipeline.
	*/
	virtual void recreate_pipeline() = 0;

	device_pipeline(const ste_context &ctx,
					pipeline_binding_set_pool &pool,
					pipeline_layout &&layout,
					optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets)
		: ctx(ctx),
		layout(std::move(layout)),
		binding_sets(this->layout,
					 pool),
		external_binding_sets(external_binding_sets ? &external_binding_sets.get().get() : nullptr)
	{}

public:
	device_pipeline(device_pipeline&&) = default;
	device_pipeline &operator=(device_pipeline&&) = default;

	virtual ~device_pipeline() noexcept {}

	/**
	 *	@brief	Creates a resource binder for a given variable name
	 */
	pipeline_bind_point operator[](const std::string &resource_name) {
		const pipeline_binding_layout *bind;

		// Check first if resource_name refers to a push constant path
		push_constant_path push_path(resource_name);
		auto optional_push_constant = (*layout.push_constants_layout)[push_path];
		if (optional_push_constant) {
			// Found push constant
			auto bp = std::make_unique<pipeline_push_constant_bind_point>(layout.push_constants_layout.get(),
																		  optional_push_constant.get());
			return pipeline_bind_point(std::move(bp));
		}

		// Not a push constant, look at other variables
		auto var_it = layout.variables_map.find(resource_name);
		if (var_it != layout.variables_map.end()) {
			// Name references a variable
			bind = &var_it->second;
		}
		else {
			throw device_pipeline_unrecognized_variable_name_exception("Resource with provided name doesn't exist in pipeline layout");
		}

		// Create the binder
		if (bind->binding->binding_type == ste_shader_stage_binding_type::spec_constant) {
			auto bp = std::make_unique<pipeline_specialization_constant_bind_point>(bind->binding->variable.get(),
																					&layout,
																					resource_name);
			return pipeline_bind_point(std::move(bp));
		}
		if (bind->binding->binding_type == ste_shader_stage_binding_type::storage ||
			bind->binding->binding_type == ste_shader_stage_binding_type::uniform) {
			auto bp = std::make_unique<pipeline_resource_bind_point>(&binding_queue,
																	 &layout,
																	 bind);
			return pipeline_bind_point(std::move(bp));
		}

		// Can't be...
		assert(false);
		throw device_pipeline_unrecognized_variable_name_exception("Resource with provided name doesn't exist in pipeline layout");
	}

	/**
	*	@brief	Creates a bind command that binds the pipeline.
	*			Should be called before issuing any commands that use the pipeline.
	*
	*			Each call to bind should be followed by a call to unbind.
	*/
	auto cmd_bind() {
		prebind_update();
		return device_pipeline_cmd_bind(this);
	}

	/**
	*	@brief	Creates a bind command that unbinds the pipeline.
	*/
	auto cmd_unbind() {
		return device_pipeline_cmd_unbind(this);
	}

	const auto& get_layout() const { return layout; }
};

}
}
