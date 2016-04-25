
#include "stdafx.hpp"
#include "ssss_generator.hpp"

#include "ssss_bilateral_blur_x.hpp"
#include "ssss_bilateral_blur_y.hpp"
#include "ssss_write_penumbras.hpp"

using namespace StE::Graphics;

ssss_generator::ssss_generator(const StEngineControl &ctx,
							   const Scene *scene,
							   const shadowmap_storage *shadows_storage,
							   const ssss_storage *ssss,
							   const deferred_gbuffer *gbuffer) : shadows_storage(shadows_storage),
							   									  ssss(ssss),
																  scene(scene),
																  gbuffer(gbuffer) {
	bilateral_blur_x = std::make_unique<ssss_bilateral_blur_x>(this, ctx);
	bilateral_blur_y = std::make_unique<ssss_bilateral_blur_y>(this, ctx);
	write_penumbras = std::make_unique<ssss_write_penumbras>(this, ctx);

	auto blur_x_task = make_gpu_task("ssss_blur_x", bilateral_blur_x.get(), nullptr);
	auto write_task = make_gpu_task("ssss_write_penumbras", write_penumbras.get(), nullptr);
	blur_x_task->add_dependency(write_task);

	task = make_gpu_task("ssss_blur_y", bilateral_blur_y.get(), nullptr, { blur_x_task, write_task });
}

ssss_generator::~ssss_generator() {}

void ssss_generator::set_model_matrix(const glm::mat4 &m) const {
	write_penumbras->set_model_matrix(m);
}
