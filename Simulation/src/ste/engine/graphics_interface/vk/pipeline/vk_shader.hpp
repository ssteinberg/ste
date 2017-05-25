//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>

#include <optional.hpp>

#include <lib/string.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_shader : public allow_type_decay<vk_shader, VkShaderModule> {
private:
	optional<VkShaderModule> module;
	alias<const vk_logical_device> device;

public:
	struct shader_stage_info_t {
		VkPipelineShaderStageCreateInfo stage_info;
		VkSpecializationInfo specialization_info;
		lib::string all_data;
		lib::vector<VkSpecializationMapEntry> entries;

		shader_stage_info_t() : stage_info{} {}

		shader_stage_info_t(shader_stage_info_t&&) = delete;
		shader_stage_info_t &operator=(shader_stage_info_t&&) = delete;
		shader_stage_info_t(const shader_stage_info_t&) = delete;
		shader_stage_info_t &operator=(const shader_stage_info_t&) = delete;
	};

	using spec_map = lib::unordered_map<std::uint32_t, lib::string>;

public:
	vk_shader(const vk_logical_device &device, const lib::string &code) : device(device) {
		VkShaderModuleCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.codeSize = code.length();
		create_info.pCode = reinterpret_cast<const std::uint32_t*>(code.data());

		VkShaderModule shader_module;
		vk_result res = vkCreateShaderModule(device, &create_info, nullptr, &shader_module);
		if (!res) {
			throw vk_exception(res);
		}

		this->module = shader_module;
	}
	~vk_shader() noexcept {
		destroy_shader_module();
	}

	vk_shader(vk_shader &&) = default;
	vk_shader &operator=(vk_shader &&o) noexcept {
		destroy_shader_module();

		module = std::move(o.module);
		device = std::move(o.device);

		return *this;
	}
	vk_shader(const vk_shader &) = delete;
	vk_shader &operator=(const vk_shader &) = delete;

	void destroy_shader_module() {
		if (module) {
			vkDestroyShaderModule(device.get(), *this, nullptr);
			module = none;
		}
	}

	void shader_stage_create_info(const VkShaderStageFlagBits &stage,
								  shader_stage_info_t &stage_info,
								  const spec_map &specializations = {}) const {
		stage_info.stage_info = {};
		stage_info.stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_info.stage_info.pNext = nullptr;
		stage_info.stage_info.flags = 0;
		stage_info.stage_info.module = *this;
		stage_info.stage_info.pName = "main";
		stage_info.stage_info.stage = stage;
		stage_info.stage_info.pSpecializationInfo = nullptr;

		stage_info.specialization_info = {};
		if (specializations.size()) {
			stage_info.all_data = "";
			stage_info.entries.reserve(specializations.size());

			for (auto &s : specializations) {
				auto entry = VkSpecializationMapEntry{ 
					s.first, 
					static_cast<std::uint32_t>(stage_info.all_data.size()),
					s.second.size()
				};
				stage_info.entries.push_back(entry);
				stage_info.all_data += s.second;
			}

			stage_info.specialization_info.mapEntryCount = static_cast<std::uint32_t>(stage_info.entries.size());
			stage_info.specialization_info.pMapEntries = stage_info.entries.data();
			stage_info.specialization_info.dataSize = stage_info.all_data.size();
			stage_info.specialization_info.pData = stage_info.all_data.data();

			stage_info.stage_info.pSpecializationInfo = &stage_info.specialization_info;
		}
	}

	auto& get() const { return module.get(); }
};

}

}
}
