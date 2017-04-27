//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <vk_sampler.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class sampler : public allow_type_decay<sampler, vk::vk_sampler> {
private:
	vk::vk_sampler s;

public:
	template <typename... Ts>
	sampler(const ste_context &ctx,
			Ts&&... ts) : s(ctx.device(),
							std::forward<Ts>(ts)...) {}

	sampler(sampler&&) = default;
	sampler &operator=(sampler&&) = default;

	auto& get() const { return s; }
};

}
}
