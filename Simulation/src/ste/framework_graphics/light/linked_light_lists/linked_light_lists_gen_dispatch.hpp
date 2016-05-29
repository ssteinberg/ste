// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gpu_dispatchable.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

#include "linked_light_lists.hpp"
#include "light_storage.hpp"

#include "glsl_program.hpp"
#include "Texture2D.hpp"
#include "Sampler.hpp"

namespace StE {
namespace Graphics {

class linked_light_lists_gen_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<linked_light_lists_gen_dispatch>;

	struct ctor_token {};

private:
	light_storage *ls;
	linked_light_lists *lll;
	Resource::resource_instance<Core::glsl_program> program;

	Core::SamplerMipmapped depth_sampler;

	Core::Texture2D *depth_map;

public:
	linked_light_lists_gen_dispatch(ctor_token,
					   				const StEngineControl &ctx,
									light_storage *ls,
									linked_light_lists *lll) : ls(ls), lll(lll),
															   depth_sampler(Core::TextureFiltering::Nearest, Core::TextureFiltering::Nearest, Core::TextureFiltering::Nearest,
															   				 Core::TextureWrapMode::ClampToEdge, Core::TextureWrapMode::ClampToEdge) {
		program.load(ctx, std::vector<std::string>{ "passthrough.vert", "linked_light_lists_gen.frag" });
	}

	void set_depth_map(Core::Texture2D *dm) { depth_map = dm; }

protected:
	virtual void set_context_state() const override;
	virtual void dispatch() const override;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::linked_light_lists_gen_dispatch> {
	using R = Graphics::linked_light_lists_gen_dispatch;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, const Ts&... args) {
		return ctx.scheduler().schedule_now_on_main_thread([=, &ctx]() {
			return std::make_unique<R>(R::ctor_token(), ctx, args...);
		}).then([](std::unique_ptr<R> &&object) {
			object->program.wait();

			return std::move(object);
		});
	}
};

}
}
