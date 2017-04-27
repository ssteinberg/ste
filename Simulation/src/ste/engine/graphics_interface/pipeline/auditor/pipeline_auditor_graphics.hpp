//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <pipeline_auditor.hpp>
#include <pipeline_layout.hpp>
#include <pipeline_external_binding_set_collection.hpp>

#include <device_pipeline_shader_stage.hpp>
#include <pipeline_layout_exceptions.hpp>

#include <pipeline_vertex_input_bindings_collection.hpp>

#include <device_pipeline_graphics_configurations.hpp>
#include <device_pipeline_graphics.hpp>

#include <memory>
#include <vector>

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

	device_pipeline_graphics_configurations pipeline_settings;

private:
	void set_vertex_stage(const shader_stage_t &stage) { vertex_shader_stage = stage; }
	void set_tess_control_stage(const shader_stage_t &stage) { tess_control_shader_stage = stage; }
	void set_tess_eval_stage(const shader_stage_t &stage) { tess_eval_shader_stage = stage; }
	void set_geometry_stage(const shader_stage_t &stage) { geometry_shader_stage = stage; }
	void set_fragment_stage(const shader_stage_t &stage) { fragment_shader_stage = stage; }

public:
	pipeline_auditor_graphics(device_pipeline_graphics_configurations &&settings) {
		set_pipeline_settings(std::move(settings));
	}
	pipeline_auditor_graphics(device_pipeline_graphics_configurations &&settings,
							  const std::vector<shader_stage_t> &stages)
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
		case ste_shader_stage::vertex_program:
			set_vertex_stage(&stage);
			break;
		case ste_shader_stage::tesselation_control_program:
			set_tess_control_stage(&stage);
			break;
		case ste_shader_stage::tesselation_evaluation_program:
			set_tess_eval_stage(&stage);
			break;
		case ste_shader_stage::geometry_program:
			set_geometry_stage(&stage);
			break;
		case ste_shader_stage::fragment_program:
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

	void set_pipeline_settings(device_pipeline_graphics_configurations &&settings) { pipeline_settings = std::move(settings); }

	/**
	*	@brief	Generates a pipeline from the specifications recorded to the auditor.
	*
	*	@param	ctx				Context
	*	@param	pool			Binding set pool. Used to allocate the binding sets used by the pipeline.
	*	@param	external_binding_sets	A list of binding set layouts that are assumed to be created and bound by an external system.
	*									The pipeline will only check compatibility with the provided shader stages.
	*/
	std::unique_ptr<device_pipeline_graphics> pipeline(const ste_context &ctx,
													   pipeline_binding_set_pool &pool,
													   optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets) const {
		pipeline_layout layout(ctx,
							   stages(),
							   external_binding_sets);
		return std::make_unique<device_pipeline_graphics>(device_pipeline_graphics::ctor(),
														  ctx,
														  pipeline_settings,
														  vertex_input_descriptors.get_vk_descriptors(),
														  pool,
														  std::move(layout),
														  external_binding_sets);
	}
	/**
	*	@brief	See pipeline().
	*/
	std::unique_ptr<device_pipeline_graphics> pipeline(const ste_context &ctx,
													   pipeline_binding_set_pool &pool) const {
		return pipeline(ctx,
						pool,
						none);
	}

private:
	/**
	*	@brief	Generates the list of pipeline stages
	*/
	std::vector<shader_stage_t> stages() const override final {
		std::vector<shader_stage_t> stages;

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
