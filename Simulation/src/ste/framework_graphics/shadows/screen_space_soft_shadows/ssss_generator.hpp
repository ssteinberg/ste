// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_task.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "shadowmap_storage.hpp"

#include "ssss_storage.hpp"
#include "Scene.hpp"
#include "deferred_gbuffer.hpp"

#include "Quad.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class ssss_bilateral_blur_x;
class ssss_bilateral_blur_y;
class ssss_write_penumbras;

class ssss_generator {
	friend class ssss_bilateral_blur_x;
	friend class ssss_bilateral_blur_y;
	friend class ssss_write_penumbras;

private:
	const shadowmap_storage *shadows_storage;
	const ssss_storage *ssss;
	const Scene *scene;
	const deferred_gbuffer *gbuffer;

	std::unique_ptr<ssss_bilateral_blur_x> bilateral_blur_x;
	std::unique_ptr<ssss_bilateral_blur_y> bilateral_blur_y;
	std::unique_ptr<ssss_write_penumbras> write_penumbras;

	std::shared_ptr<const gpu_task> task;

public:
	ssss_generator(const StEngineControl &ctx,
				   const Scene *scene,
				   const shadowmap_storage *shadows_storage,
				   const ssss_storage *ssss,
				   const deferred_gbuffer *gbuffer);
	~ssss_generator() noexcept;

	void set_model_matrix(const glm::mat4 &m) const;

	auto get_task() const { return task; }
};

}
}
