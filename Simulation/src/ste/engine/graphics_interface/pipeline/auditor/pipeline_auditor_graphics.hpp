//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <pipeline_auditor.hpp>
#include <pipeline_layout.hpp>
#include <pipeline_external_binding_set.hpp>

#include <device_pipeline_shader_stage.hpp>
#include <pipeline_layout_exceptions.hpp>

#include <pipeline_vertex_input_bindings_collection.hpp>
#include <framebuffer_layout.hpp>

#include <device_pipeline_graphics_configurations.hpp>
#include <device_pipeline_graphics.hpp>

#include <lib/unique_ptr.hpp>
#include <lib/vector.hpp>

namespace ste {
namespace gl {

/**
 *	@brief	Describes the graphical pipeline configuration. Specifies the shader stages, attachments and graphical configurations.
 */
class pipeline_auditor_graphics : public pipeline_auditor {
	using Base = pipeline_auditor;

private:
	shader_stage_t vertex_shader_stage{ nullptr };
	shader_stage_t tess_control_shader_stage{ nullptr };
	shader_stage_t tess_eval_shader_stage{ nullptr };
	shader_stage_t geometry_shader_stage{ nullptr };
	shader_stage_t fragment_shader_stage{ nullptr };

	pipeline_vertex_input_bindings_collection vertex_input_descriptors;
	framebuffer_layout fb_layout;

	device_pipeline_graphics_configurations pipeline_settings;

private:
	void set_vertex_stage(const shader_stage_t &stage) { vertex_shader_stage = stage; }
	void set_tess_control_stage(const shader_stage_t &stage) { tess_control_shader_stage = stage; }
	void set_tess_eval_stage(const shader_stage_t &stage) { tess_eval_shader_stage = stage; }
	void set_geometry_stage(const shader_stage_t &stage) { geometry_shader_stage = stage; }
	void set_fragment_stage(const shader_stage_t &stage) { fragment_shader_stage = stage; }

public:
	pipeline_auditor_graphics(device_pipeline_graphics_configurations &&settings)
		: fb_layout()
	{
		set_pipeline_settings(std::move(settings));
	}
	pipeline_auditor_graphics(device_pipeline_graphics_configurations &&settings,
							  const lib::vector<shader_stage_t> &stages)
		: pipeline_auditor_graphics(std::move(settings))
	{
		for (auto &s : stages)
			attach_shader_stage(*s);
	}
	~pipeline_auditor_graphics() noexcept {}

	pipeline_auditor_graphics(pipeline_auditor_graphics&&) = default;
	pipeline_auditor_graphics &operator=(pipeline_auditor_graphics&&) = default;

	/**
	*	@brief	Attaches a shader stage.
	*
	*	@throws	pipeline_layout_incompatible_stage_exception	If provided stage is not a graphical shader stage
	*/
	void attach_shader_stage(device_pipeline_shader_stage &stage) {
		switch (stage.get_stage()) {
		case ste_shader_program_stage::vertex_program:
			set_vertex_stage(&stage);
			break;
		case ste_shader_program_stage::tessellation_control_program:
			set_tess_control_stage(&stage);
			break;
		case ste_shader_program_stage::tessellation_evaluation_program:
			set_tess_eval_stage(&stage);
			break;
		case ste_shader_program_stage::geometry_program:
			set_geometry_stage(&stage);
			break;
		case ste_shader_program_stage::fragment_program:
			set_fragment_stage(&stage);
			break;
		default:
			throw pipeline_layout_incompatible_stage_exception("Excepted a graphics shader stage.");
		}
	}

	/**
	 *	@brief	Defines vertex attributes for a binding
	 */
	template <typename B, VkVertexInputRate i>
	void set_vertex_attributes(std::uint32_t binding_index,
							   const vertex_attributes<B, i> &attrib) {
		vertex_input_descriptors.insert(binding_index, attrib);
	}

	/**
	*	@brief	Defines the framebuffer layout for the pipeline
	*/
	void set_framebuffer_layout(const framebuffer_layout &layout) {
		fb_layout = layout;
	}
	/**
	*	@brief	Defines the framebuffer layout for the pipeline
	*/
	void set_framebuffer_layout(framebuffer_layout &&layout) {
		fb_layout = std::move(layout);
	}

	void set_pipeline_settings(device_pipeline_graphics_configurations &&settings) { pipeline_settings = std::move(settings); }

	/**
	*	@brief	Generates a pipeline from the specifications recorded to the auditor.
	*
	*	@param	ctx				Context
	*	@param	external_binding_set	A list of binding set layouts that are assumed to be created and bound by an external system.
	*									The pipeline will only check compatibility with the provided shader stages.
	*/
	auto pipeline(const ste_context &ctx,
				  optional<std::reference_wrapper<pipeline_external_binding_set>> external_binding_set) const {
		auto layout = lib::allocate_unique<pipeline_layout>(ctx,
															stages(),
															external_binding_set);
		return device_pipeline_graphics(device_pipeline_graphics::ctor(),
										ctx,
										pipeline_settings,
										vertex_input_descriptors.get_vk_descriptors(),
										fb_layout,
										std::move(layout),
										external_binding_set);
	}
	/**
	*	@brief	See pipeline().
	*/
	auto pipeline(const ste_context &ctx) const {
		return pipeline(ctx,
						none);
	}

private:
	/**
	*	@brief	Generates the list of pipeline stages
	*/
	lib::vector<shader_stage_t> stages() const override final {
		lib::vector<shader_stage_t> stages;

		if (vertex_shader_stage != nullptr)
			stages.push_back(vertex_shader_stage);
		if (tess_control_shader_stage != nullptr)
			stages.push_back(tess_control_shader_stage);
		if (tess_eval_shader_stage != nullptr)
			stages.push_back(tess_eval_shader_stage);
		if (geometry_shader_stage != nullptr)
			stages.push_back(geometry_shader_stage);
		if (fragment_shader_stage != nullptr)
			stages.push_back(fragment_shader_stage);

		return stages;
	}
};

}
}
