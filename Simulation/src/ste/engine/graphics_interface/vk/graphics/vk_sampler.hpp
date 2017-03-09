//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>

#include <vk_sampler_parameters.hpp>

#include <optional.hpp>

#include <functional>

namespace StE {
namespace GL {

class vk_sampler {
private:
	optional<VkSampler> sampler;
	const vk_logical_device &device;

private:
	template <typename ... Params>
	struct params_chain_extracter {};
	template <typename Param, typename ... Tail>
	struct params_chain_extracter<Param, Tail...> {
		void operator()(VkSamplerCreateInfo &create_info, Param &&param, Tail&& ... params_tail) const {
			params_chain_extracter<Param>()(create_info, std::forward<Param>(param));
			params_chain_extracter<Tail...>()(create_info, std::forward<Tail>(params_tail)...);
		}
	};
	template <typename Param>
	struct params_chain_extracter<Param> {
		static_assert(std::is_base_of<vk_sampler_parameters, Param>::value,
					  "Param is not a vk_sampler parameter (must inherit from vk_sampler_parameters)");
		void operator()(VkSamplerCreateInfo &create_info, Param &&param) const {
			param(create_info);
		}
	};

private:
	static auto default_create_info() {
		VkSamplerCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.unnormalizedCoordinates = false;

		vk_sampler_filtering()(create_info);
		vk_sampler_address_mode()(create_info);
		vk_sampler_anisotropy()(create_info);
		vk_sampler_depth_compare()(create_info);
		vk_sampler_mipmap_lod()(create_info);

		return create_info;
	}

public:
	/**
	*	@brief	Sampler ctor. 
	*			Takes as input variadic number of vk_sampler_parameters structure specifing sampler parameters.
	*
	*	@param device		Creating device
	*	@param params		Sampler parameters
	*/
	template <typename ... Params>
	vk_sampler(const vk_logical_device &device,
			   Params&& ... params) : device(device) {
		VkSamplerCreateInfo create_info = default_create_info();

		// Read passed params structures
		params_chain_extracter<Params...>()(create_info, std::forward<Params>(params)...);

		VkSampler sampler;
		vk_result res = vkCreateSampler(device, &create_info, nullptr, &sampler);
		if (!res) {
			throw vk_exception(res);
		}

		this->sampler = sampler;
	}
	/**
	*	@brief	Unnormalized sampler ctor.
	*
	*	@param device		Creating device
	*	@param unnormalized	Unnomralized sampler parameters
	*/
	vk_sampler(const vk_logical_device &device,
			   const vk_sampler_unnormalized_parameters &unnormalized) : device(device) {
		VkSamplerCreateInfo create_info = default_create_info();
		unnormalized(create_info);

		VkSampler sampler;
		vk_result res = vkCreateSampler(device, &create_info, nullptr, &sampler);
		if (!res) {
			throw vk_exception(res);
		}

		this->sampler = sampler;
	}
	~vk_sampler() noexcept {
		destroy_sampler();
	}

	vk_sampler(vk_sampler &&) = default;
	vk_sampler &operator=(vk_sampler &&) = default;
	vk_sampler(const vk_sampler &) = delete;
	vk_sampler &operator=(const vk_sampler &) = delete;

	void destroy_sampler() {
		if (sampler) {
			vkDestroySampler(device, *this, nullptr);
			sampler = none;
		}
	}

	auto& get_sampler() const { return sampler.get(); }

	operator VkSampler() const { return get_sampler(); }
};

}
}
