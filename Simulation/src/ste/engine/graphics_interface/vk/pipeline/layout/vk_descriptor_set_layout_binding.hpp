//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_sampler.hpp>

#include <lib/vector.hpp>
#include <functional>

namespace ste {
namespace gl {

namespace vk {

class vk_descriptor_set_layout_binding {
private:
	VkDescriptorType type;
	VkShaderStageFlags stage;
	std::uint32_t binding_index;
	std::uint32_t count;
	lib::vector<VkSampler> immutable_samplers;

public:
	vk_descriptor_set_layout_binding(const VkDescriptorType &type,
									 const VkShaderStageFlags &stage,
									 std::uint32_t binding_index,
									 std::uint32_t count = 1)
		: type(type), stage(stage), binding_index(binding_index), count(count)
	{}
	vk_descriptor_set_layout_binding(const VkShaderStageFlags &stage,
									 std::uint32_t binding_index,
									 const lib::vector<std::reference_wrapper<const vk_sampler<>>> &immutable_samplers,
									 bool combined = false)
		: type(combined ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_SAMPLER),
		stage(stage), binding_index(binding_index),
		count(static_cast<std::uint32_t>(immutable_samplers.size()))
	{
		this->immutable_samplers.resize(immutable_samplers.size());
		for (std::size_t i = 0; i < immutable_samplers.size(); ++i)
			this->immutable_samplers[i] = (immutable_samplers.begin() + i)->get();
	}

	vk_descriptor_set_layout_binding(vk_descriptor_set_layout_binding &&) = default;
	vk_descriptor_set_layout_binding &operator=(vk_descriptor_set_layout_binding &&) = default;
	vk_descriptor_set_layout_binding(const vk_descriptor_set_layout_binding &) = default;
	vk_descriptor_set_layout_binding &operator=(const vk_descriptor_set_layout_binding &) = default;

	auto get_binding() const {
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = binding_index;
		binding.descriptorCount = count;
		binding.descriptorType = type;
		binding.stageFlags = stage;
		binding.pImmutableSamplers = immutable_samplers.data();

		return binding;
	}

	auto get_type() const { return type; }
	auto get_stage() const { return stage; }
	auto get_index() const { return binding_index; }
	auto get_count() const { return count; }

	operator VkDescriptorSetLayoutBinding() const { return get_binding(); }
};

}

}
}
