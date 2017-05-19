//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_handle.hpp>

#include <vk_pipeline.hpp>
#include <vk_logical_device.hpp>
#include <vk_shader.hpp>
#include <vk_pipeline_layout.hpp>
#include <vk_pipeline_cache.hpp>
#include <vk_render_pass.hpp>
#include <vk_shader_stage_descriptor.hpp>

#include <vk_depth_op_descriptor.hpp>
#include <vk_blend_op_descriptor.hpp>
#include <vk_rasterizer_op_descriptor.hpp>

#include <optional.hpp>

#include <lib/vector.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_pipeline_graphics : public vk_pipeline {
public:
	vk_pipeline_graphics(const vk_logical_device &device,
						 const lib::vector<vk_shader_stage_descriptor> &shader_modules,
						 const vk_pipeline_layout &layout,
						 const vk_render_pass &render_pass,
						 std::uint32_t subpass,
						 const VkViewport &viewport,
						 const VkRect2D &scissor,
						 lib::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptors,
						 lib::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptors,
						 VkPrimitiveTopology topology,
						 const vk_rasterizer_op_descriptor &rasterizer_op,
						 const vk_depth_op_descriptor &depth_op,
						 const lib::vector<vk_blend_op_descriptor> &attachment_blend_op,
						 const glm::vec4 blend_constants,
						 lib::vector<VkDynamicState> dynamic_states,
						 const vk_pipeline_cache *cache = nullptr) : vk_pipeline(device) {
		// Shader modules stages
		lib::vector<vk_shader::shader_stage_info_t> stages_info(shader_modules.size());
		lib::vector<VkPipelineShaderStageCreateInfo> stages(shader_modules.size());
		for (std::size_t i = 0; i < shader_modules.size(); ++i) {
			auto &sd = shader_modules[i];
			assert((sd.stage & VK_SHADER_STAGE_COMPUTE_BIT) == 0 && "shader_stage_descriptor must not be a compute shader");

			sd.shader->shader_stage_create_info(sd.stage,
												stages_info[i],
												*sd.specializations);
			stages[i] = stages_info[i].stage_info;
		}

		// Vertex input
		VkPipelineVertexInputStateCreateInfo vertex_input_state_create = {};
		vertex_input_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state_create.pNext = nullptr;
		vertex_input_state_create.flags = 0;
		vertex_input_state_create.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertex_input_attribute_descriptors.size());
		vertex_input_state_create.pVertexAttributeDescriptions = vertex_input_attribute_descriptors.data();
		vertex_input_state_create.vertexBindingDescriptionCount = static_cast<std::uint32_t>(vertex_input_binding_descriptors.size());
		vertex_input_state_create.pVertexBindingDescriptions = vertex_input_binding_descriptors.data();

		// Assembly
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create = {};
		input_assembly_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_create.pNext = nullptr;
		input_assembly_state_create.flags = 0;
		input_assembly_state_create.primitiveRestartEnable = false;
		input_assembly_state_create.topology = topology;

		// Rasterization
		VkPipelineRasterizationStateCreateInfo rasterization_state_create = {};
		rasterization_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_create.pNext = nullptr;
		rasterization_state_create.flags = 0;
		rasterization_state_create.cullMode = rasterizer_op.cull_mode;
		rasterization_state_create.frontFace = rasterizer_op.front_face;
		rasterization_state_create.lineWidth = 1.f;
		rasterization_state_create.depthBiasEnable = rasterizer_op.depth_bias_enable;
		rasterization_state_create.depthBiasConstantFactor = rasterizer_op.depth_bias_const_factor;
		rasterization_state_create.depthBiasSlopeFactor = rasterizer_op.depth_bias_slope_factor;
		rasterization_state_create.depthClampEnable = false;
		rasterization_state_create.rasterizerDiscardEnable = rasterizer_op.discard_enable;

		// Multisample
		VkPipelineMultisampleStateCreateInfo multisample_state_create = {};
		multisample_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_create.sampleShadingEnable = false;
		multisample_state_create.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_create.minSampleShading = 1.0f;
		multisample_state_create.pSampleMask = nullptr;
		multisample_state_create.alphaToCoverageEnable = false;
		multisample_state_create.alphaToOneEnable = false;

		// Viewport
		VkPipelineViewportStateCreateInfo viewport_state_create = {};
		viewport_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_create.pNext = nullptr;
		viewport_state_create.flags = 0;
		viewport_state_create.viewportCount = 1;
		viewport_state_create.pViewports = &viewport;
		viewport_state_create.scissorCount = 1;
		viewport_state_create.pScissors = &scissor;

		// Depth
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create = {};
		depth_stencil_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_create.pNext = nullptr;
		depth_stencil_state_create.flags = 0;
		depth_stencil_state_create.depthBoundsTestEnable = true;
		depth_stencil_state_create.minDepthBounds = viewport.minDepth;
		depth_stencil_state_create.maxDepthBounds = viewport.maxDepth;
		depth_stencil_state_create.depthCompareOp = depth_op.depth_compare_op;
		depth_stencil_state_create.depthTestEnable = depth_op.depth_test_enable;
		depth_stencil_state_create.depthWriteEnable = depth_op.depth_write_enable;

		// Blend
		lib::vector<VkPipelineColorBlendAttachmentState> blend_attachment_states(attachment_blend_op.size());
		for (std::size_t i = 0; i < attachment_blend_op.size(); ++i)
			blend_attachment_states[i] = *(attachment_blend_op.begin() + i);

		VkPipelineColorBlendStateCreateInfo blend_state_create = {};
		blend_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blend_state_create.pNext = nullptr;
		blend_state_create.flags = 0;
		blend_state_create.logicOpEnable = false;
		blend_state_create.attachmentCount = static_cast<std::uint32_t>(blend_attachment_states.size());
		blend_state_create.pAttachments = blend_attachment_states.data();
		blend_state_create.blendConstants[0] = blend_constants.r;
		blend_state_create.blendConstants[1] = blend_constants.g;
		blend_state_create.blendConstants[2] = blend_constants.b;
		blend_state_create.blendConstants[3] = blend_constants.a;

		// Dynamic states
		VkPipelineDynamicStateCreateInfo dynamic_state_create = {};
		dynamic_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_create.pNext = nullptr;
		dynamic_state_create.flags = 0;
		dynamic_state_create.dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size());
		dynamic_state_create.pDynamicStates = dynamic_states.data();

		VkGraphicsPipelineCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.layout = layout;
		create_info.basePipelineHandle = vk::vk_null_handle;
		create_info.basePipelineIndex = 0;
		create_info.renderPass = render_pass;
		create_info.subpass = subpass;
		create_info.stageCount = static_cast<std::uint32_t>(stages.size());
		create_info.pStages = stages.data();
		create_info.pVertexInputState = &vertex_input_state_create;
		create_info.pInputAssemblyState = &input_assembly_state_create;
		create_info.pTessellationState = nullptr;
		create_info.pMultisampleState = &multisample_state_create;
		create_info.pRasterizationState = &rasterization_state_create;
		create_info.pViewportState = &viewport_state_create;
		create_info.pColorBlendState = &blend_state_create;
		create_info.pDepthStencilState = &depth_stencil_state_create;
		create_info.pDynamicState = &dynamic_state_create;

		VkPipeline pipeline;
		vk_result res = vkCreateGraphicsPipelines(device,
												  cache != nullptr ? static_cast<VkPipelineCache>(*cache) : vk::vk_null_handle,
												  1,
												  &create_info,
												  nullptr,
												  &pipeline);
		if (!res) {
			throw vk_exception(res);
		}

		this->pipeline = pipeline;
	}
	~vk_pipeline_graphics() noexcept {}

	vk_pipeline_graphics(vk_pipeline_graphics &&) = default;
	vk_pipeline_graphics &operator=(vk_pipeline_graphics &&) = default;
	vk_pipeline_graphics(const vk_pipeline_graphics &) = delete;
	vk_pipeline_graphics &operator=(const vk_pipeline_graphics &) = delete;
};


}

}
}
