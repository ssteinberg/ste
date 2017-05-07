// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <rendering_system.hpp>

#include <gpu_dispatchable.hpp>
#include <gpu_task.hpp>

#include <fb_clear_dispatch.hpp>

#include <memory>

namespace ste {
namespace graphics {

class basic_renderer : public rendering_system {
	using Base = rendering_system;

private:
	using FbClearTask = ste::graphics::fb_clear_dispatch<>;

private:
	const ste_engine_control &ctx;

	gpu_task::TaskCollection tasks;

	FbClearTask fb_clearer;
	std::shared_ptr<const gpu_task> fb_clearer_task;

public:
	basic_renderer(const ste_engine_control &ctx) : ctx(ctx) {
		fb_clearer_task = make_gpu_task("fb_clearer", &fb_clearer, &ctx.gl()->defaut_framebuffer());
		q.add_task(fb_clearer_task);
	}
	virtual ~basic_renderer() noexcept {}

	void add_task(const gpu_task::TaskPtr &t) {
		q.add_task_dependency(t, fb_clearer_task);
		q.add_task(t);

		tasks.insert(t);
	}

	void remove_task(const gpu_task::TaskPtr &t) {
		q.remove_task(t);
		q.remove_task_dependency(t, fb_clearer_task);

		tasks.erase(t);
	}

	virtual void render_queue() override {
		q.dispatch();
	}

	virtual std::string rendering_system_name() const override { return "basic_renderer"; };
};

}
}
