//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_resource.hpp>
#include <task_binding.hpp>
#include <device_pipeline_shader_stage.hpp>

#include <vk_render_pass.hpp>
#include <vk_pipeline_graphics.hpp>
#include <vk_pipeline.hpp>
#include <vk_pipeline_cache.hpp>
#include <vk_pipeline_layout.hpp>
#include <vk_descriptor_set_layout.hpp>

#include <job.hpp>

#include <memory>
#include <vector>

namespace StE {
namespace GL {

class task_auditor {
private:
	using bindings_t = std::vector<task_binding>;

public:
	struct task_auditor_shader_stage {
		ste_resource<device_pipeline_shader_stage> shader;
		bindings_t resource_bindings;

		template <typename... Ts>
		task_auditor_shader_stage(Ts&&... ts) : shader(std::forward<Ts>(ts)...) {}

		task_auditor_shader_stage(task_auditor_shader_stage&&) = default;
		task_auditor_shader_stage &operator=(task_auditor_shader_stage&&) = default;
	};

protected:
	using shader_stage_t = std::unique_ptr<task_auditor_shader_stage>;

protected:
	std::unique_ptr<vk_pipeline> pipeline;
	std::unique_ptr<vk_pipeline_layout> pipeline_layout;

protected:
	task_auditor() = default;

public:
	virtual ~task_auditor() noexcept {}

	task_auditor(task_auditor&&) = default;
	task_auditor &operator=(task_auditor&&) = default;

	virtual std::unique_ptr<job> generate(const vk_pipeline_cache *cache = nullptr) = 0;
};

class task_auditor_graphics : public task_auditor {
	using Base = task_auditor;

public:
	struct graphics_task_pipeline_settings {
		VkViewport viewport;
		VkRect2D scissor;
		std::vector<vk_pipeline_graphics::vertex_input_descriptor> vertex_attributes;
		VkPrimitiveTopology topology;
		vk_rasterizer_op_descriptor rasterizer_op;
		vk_depth_op_descriptor depth_op;
		std::vector<vk_blend_op_descriptor> attachment_blend_op;
		glm::vec4 blend_constants;
	};

private:
	std::unique_ptr<vk_render_pass> renderpass;

private:
	shader_stage_t vertex_shader_stage;
	shader_stage_t tess_control_shader_stage;
	shader_stage_t tess_eval_shader_stage;
	shader_stage_t geometry_shader_stage;
	shader_stage_t fragment_shader_stage;

	graphics_task_pipeline_settings pipeline_settings;

public:
	~task_auditor_graphics() noexcept {}

	task_auditor_graphics(task_auditor_graphics&&) = default;
	task_auditor_graphics &operator=(task_auditor_graphics&&) = default;

	void set_vertex_stage(shader_stage_t &&stage) { vertex_shader_stage = std::move(stage); }
	void set_tess_control(shader_stage_t &&stage) { tess_control_shader_stage = std::move(stage); }
	void set_tess_eval_stage(shader_stage_t &&stage) { tess_eval_shader_stage = std::move(stage); }
	void set_geometry_stage(shader_stage_t &&stage) { geometry_shader_stage = std::move(stage); }
	void set_fragment_stage(shader_stage_t &&stage) { fragment_shader_stage = std::move(stage); }
	void set_pipeline_settings(graphics_task_pipeline_settings &&settings) { pipeline_settings = std::move(settings); }

	std::unique_ptr<job> generate(const vk_pipeline_cache *cache = nullptr) override final;
};

class task_auditor_compute : public task_auditor {
	using Base = task_auditor;

private:
	shader_stage_t compute_shader_stage;

public:
	~task_auditor_compute() noexcept {}

	task_auditor_compute(task_auditor_compute&&) = default;
	task_auditor_compute &operator=(task_auditor_compute&&) = default;

	void set_compute_stage(shader_stage_t &&stage) { compute_shader_stage = std::move(stage); }

	std::unique_ptr<job> generate(const vk_pipeline_cache *cache = nullptr) override final;
};

}
}
