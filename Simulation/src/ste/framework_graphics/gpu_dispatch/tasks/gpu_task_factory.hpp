// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "gpu_task.hpp"

#include "FramebufferObject.hpp"
#include "GLSLProgram.hpp"

#include <functional>
#include <memory>

#include <typeinfo>

namespace StE {
namespace Graphics {

class gpu_task_factory {
public:
	static std::shared_ptr<const gpu_task> make_gpu_task(const std::string &name,
														 const gpu_dispatchable *dispatchable,
														 const Core::GenericFramebufferObject *fbo,
														 std::vector<std::shared_ptr<const gpu_task>> &&st) {
		return std::make_shared<const gpu_task>(gpu_task::AccessToken(), name, dispatchable, fbo, std::move(st));
	}

	static void set_task_fbo(const std::shared_ptr<const gpu_task> &task, const Core::GenericFramebufferObject *fbo) {
		task->set_fbo(fbo);
	}
};

inline auto make_gpu_task(const gpu_dispatchable *dispatchable) {
	return gpu_task_factory::make_gpu_task(typeid(*dispatchable).name(), dispatchable, nullptr, {});
}

inline auto make_gpu_task(const std::string &name,
						  const gpu_dispatchable *dispatchable,
						  const Core::GenericFramebufferObject *fbo) {
	return gpu_task_factory::make_gpu_task(name, dispatchable, fbo, {});
}

inline auto make_gpu_task(const std::string &name,
						  const gpu_dispatchable *dispatchable,
						  const Core::GenericFramebufferObject *fbo,
						  std::vector<std::shared_ptr<const gpu_task>> &&st) {
	return gpu_task_factory::make_gpu_task(name, dispatchable, fbo, std::move(st));
}

inline void mutate_gpu_task(const std::shared_ptr<const gpu_task> &task, const Core::GenericFramebufferObject *fbo) {
	gpu_task_factory::set_task_fbo(task, fbo);
}

}
}
