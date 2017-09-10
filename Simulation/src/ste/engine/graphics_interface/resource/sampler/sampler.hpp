//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_logical_device.hpp>

#include <vk_sampler.hpp>
#include <sampler_parameter.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename ... Params>
struct params_chain_extracter {};
template <typename Param, typename ... Tail>
struct params_chain_extracter<Param, Tail...> {
	void operator()(vk::vk_sampler_info &sampler_info, Param &&param, Tail&& ... params_tail) const {
		params_chain_extracter<Param>()(sampler_info, std::forward<Param>(param));
		params_chain_extracter<Tail...>()(sampler_info, std::forward<Tail>(params_tail)...);
	}
};
template <typename Param>
struct params_chain_extracter<Param> {
	static_assert(std::is_base_of<sampler_parameter::parameter, Param>::value,
				  "Param is not a sampler parameter (must inherit from sampler_parameter::parameter)");
	void operator()(vk::vk_sampler_info &sampler_info, Param &&param) const {
		param(sampler_info);
	}
};

}

class sampler : public allow_type_decay<sampler, vk::vk_sampler<>> {
private:
	vk::vk_sampler<> s;

private:
	static auto default_sampler_info() {
		vk::vk_sampler_info sampler_info = {};

		sampler_info.unnormalized_coordinates = false;
		sampler_parameter::filtering()(sampler_info);
		sampler_parameter::address_mode()(sampler_info);
		sampler_parameter::anisotropy()(sampler_info);
		sampler_parameter::depth_compare()(sampler_info);
		sampler_parameter::mipmap_lod()(sampler_info);

		return sampler_info;
	}

	template <typename ... Params>
	static auto sampler_info(Params&&... params) {
		auto sampler_info = default_sampler_info();
		_internal::params_chain_extracter<Params...>()(sampler_info, std::forward<Params>(params)...);
		return sampler_info;
	}

	static auto sampler_info_unnormalized(const sampler_parameter::unnormalized &unnormalized) {
		auto sampler_info = default_sampler_info();
		unnormalized(sampler_info);
		return sampler_info;
	}

public:
	/**
	*	@brief	Sampler ctor.
	*			Takes as input variadic number of gl::sampler_parameter::* structures specifing sampler parameters.
	*
	*	@param device		Creating device
	*	@param params		Sampler parameters
	*/
	template <typename... Params>
	sampler(const vk::vk_logical_device<> &device,
			const char *name,
			Params&& ... params)
		: s(device,
			sampler_info(std::forward<Params>(params)...),
			name)
	{}
	/**
	*	@brief	Unnormalized sampler ctor.
	*
	*	@param device		Creating device
	*	@param unnormalized	Unnomralized sampler parameters
	*/
	sampler(const vk::vk_logical_device<> &device,
			const sampler_parameter::unnormalized &unnormalized,
			const char *name)
		: s(device,
			sampler_info_unnormalized(unnormalized),
			name)
	{}

	sampler(sampler&&) = default;
	sampler &operator=(sampler&&) = default;

	auto& get() const { return s; }
};

}
}
