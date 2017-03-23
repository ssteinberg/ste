//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_shader.hpp>
#include <vk_pipeline_layout.hpp>
#include <vk_pipeline_cache.hpp>
#include <vk_render_pass.hpp>

#include <vk_vertex_input_attributes.hpp>
#include <vk_depth_op_descriptor.hpp>
#include <vk_blend_op_descriptor.hpp>
#include <vk_rasterizer_op_descriptor.hpp>

#include <optional.hpp>

#include <vector>
#include <allow_class_decay.hpp>

namespace StE {
namespace GL {

struct vk_graphics_shader_descriptor {
	const vk_shader *shader;
	VkShaderStageFlagBits stage;
};

class vk_pipeline_graphics : public allow_class_decay<vk_pipeline_graphics, VkPipeline> {
private:
	optional<VkPipeline> pipeline;
	std::reference_wrapper<const vk_logical_device> device;

public:
	struct vertex_input_descriptor {
		std::uint32_t binding_index;
		const vk_vertex_input_attributes& attributes;
//		VkVertexInputRate input_rate;

		vertex_input_descriptor() = delete;
	};

public:
	vk_pipeline_graphics(const vk_logical_device &device,
						 const std::vector<vk_graphics_shader_descriptor> &shader_modules,
						 const vk_pipeline_layout &layout,
						 const vk_render_pass &render_pass,
						 std::uint32_t subpass,
						 const VkViewport &viewport,
						 const VkRect2D &scissor,
						 const std::vector<vertex_input_descriptor> &vertex_attributes,
						 VkPrimitiveTopology topology,
						 const vk_rasterizer_op_descriptor &rasterizer_op,
						 const vk_depth_op_descriptor &depth_op,
						 const std::vector<vk_blend_op_descriptor> &attachment_blend_op,
						 const glm::vec4 blend_constants,
						 const vk_pipeline_cache *cache = nullptr) : device(device) {
		// Shader modules stages
		std::vector<vk_shader::shader_stage_info_t> stages_info(shader_modules.size());
		std::vector<VkPipelineShaderStageCreateInfo> stages(shader_modules.size());
		for (std::size_t i = 0; i < shader_modules.size(); ++i) {
			auto &sd = shader_modules[i];
			sd.shader->shader_stage_create_info(sd.stage, stages_info[i]);
			stages[i] = stages_info[i].stage_info;
		}

		// Vertex input
		std::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptors(vertex_attributes.size());
		int vertex_attributes_count = 0;
		for (std::size_t i = 0; i < vertex_attributes.size(); ++i) {
			const auto &vert = vertex_attributes[i];

			VkVertexInputBindingDescription desc = {};
			desc.binding = vert.binding_index;
			desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;// vert.input_rate;
			desc.stride = vert.attributes.stride();

			vertex_input_binding_descriptors[i] = desc;

			vertex_attributes_count += vert.attributes.attrib_count();
		}
		std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptors;
		vertex_input_attribute_descriptors.reserve(vertex_attributes_count);
		for (std::size_t i = 0; i < vertex_attributes.size(); ++i) {
			const auto &vert = vertex_attributes[i];

			for (std::size_t j = 0; j < vert.attributes.attrib_count(); ++j) {
				VkVertexInputAttributeDescription desc = {};
				desc.location = j;
				desc.binding = vert.binding_index;
				desc.format = vert.attributes.attrib_format(j);
				desc.offset = vert.attributes.offset_to_attrib(j);

				vertex_input_attribute_descriptors.push_back(desc);
			}
		}
		VkPipelineVertexInputStateCreateInfo vertex_input_state_create = {};
		vertex_input_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state_create.pNext = nullptr;
		vertex_input_state_create.flags = 0;
		vertex_input_state_create.vertexAttributeDescriptionCount = vertex_input_attribute_descriptors.size();
		vertex_input_state_create.pVertexAttributeDescriptions = vertex_input_attribute_descriptors.data();
		vertex_input_state_create.vertexBindingDescriptionCount = vertex_input_binding_descriptors.size();
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
		std::vector<VkPipelineColorBlendAttachmentState> blend_attachment_states(attachment_blend_op.size());
		for (std::size_t i = 0; i < attachment_blend_op.size(); ++i)
			blend_attachment_states[i] = *(attachment_blend_op.begin() + i);

		VkPipelineColorBlendStateCreateInfo blend_state_create = {};
		blend_state_create.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blend_state_create.pNext = nullptr;
		blend_state_create.flags = 0;
		blend_state_create.logicOpEnable = false;
		blend_state_create.attachmentCount = blend_attachment_states.size();
		blend_state_create.pAttachments = blend_attachment_states.data();
		blend_state_create.blendConstants[0] = blend_constants.r;
		blend_state_create.blendConstants[1] = blend_constants.g;
		blend_state_create.blendConstants[2] = blend_constants.b;
		blend_state_create.blendConstants[3] = blend_constants.a;

		VkGraphicsPipelineCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.layout = layout;
		create_info.basePipelineHandle = VK_NULL_HANDLE;
		create_info.basePipelineIndex = 0;
		create_info.renderPass = render_pass;
		create_info.subpass = subpass;
		create_info.stageCount = stages.size();
		create_info.pStages = stages.data();
		create_info.pVertexInputState = &vertex_input_state_create;
		create_info.pInputAssemblyState = &input_assembly_state_create;
		create_info.pTessellationState = nullptr;
		create_info.pMultisampleState = &multisample_state_create;
		create_info.pRasterizationState = &rasterization_state_create;
		create_info.pViewportState = &viewport_state_create;
		create_info.pColorBlendState = &blend_state_create;
		create_info.pDepthStencilState = &depth_stencil_state_create;
		create_info.pDynamicState = nullptr;

		VkPipeline pipeline;
		vk_result res = vkCreateGraphicsPipelines(device,
												  cache!=nullptr ? *cache : VK_NULL_HANDLE,
												  1,
												  &create_info,
												  nullptr,
												  &pipeline);
		if (!res) {
			throw vk_exception(res);
		}

		this->pipeline = pipeline;
	}
	~vk_pipeline_graphics() noexcept {
		destroy_pipeline();
	}

	vk_pipeline_graphics(vk_pipeline_graphics &&) = default;
	vk_pipeline_graphics &operator=(vk_pipeline_graphics &&) = default;
	vk_pipeline_graphics(const vk_pipeline_graphics &) = delete;
	vk_pipeline_graphics &operator=(const vk_pipeline_graphics &) = delete;

	void destroy_pipeline() {
		if (pipeline) {
			vkDestroyPipeline(device.get(), *this, nullptr);
			pipeline = none;
		}
	}

	auto& get() const { return pipeline.get(); }
};

}
}
