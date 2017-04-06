//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_sampler.hpp>
#include <allow_type_decay.hpp>

namespace StE {
namespace GL {

class sampler : public allow_type_decay<sampler, vk_sampler> {
private:
	vk_sampler s;

public:
	template <typename... Ts>
	sampler(Ts&&... ts) : s(std::forward<Ts>(ts)...) {}

	sampler(sampler&&) = default;
	sampler &operator=(sampler&&) = default;

	auto& get() const { return s; }
};

}
}
