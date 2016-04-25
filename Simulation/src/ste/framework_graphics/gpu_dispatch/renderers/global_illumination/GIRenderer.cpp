
#include "stdafx.hpp"
#include "GIRenderer.hpp"

#include "Quad.hpp"
#include "Sampler.hpp"

#include "gl_current_context.hpp"

using namespace StE::Graphics;


void GIRenderer::deferred_composition::set_context_state() const {
	using namespace Core;

	dr->gbuffer.bind_gbuffer();
	0_storage_idx = dr->scene->scene_properties().materials_storage().buffer();
	dr->scene->scene_properties().lights_storage().bind_buffers(2);

	Core::GL::gl_current_context::get()->enable_state(StE::Core::GL::BasicStateName::TEXTURE_CUBE_MAP_SEAMLESS);
	8_tex_unit = *dr->shadows_storage.get_cubemaps();
	8_sampler_idx = *Sampler::SamplerAnisotropicLinearClamp();

	ScreenFillingQuad.vao()->bind();

	program->bind();
}

void GIRenderer::deferred_composition::dispatch() const {
	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Core::GL::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
}


GIRenderer::GIRenderer(const StEngineControl &ctx,
					   const std::shared_ptr<Scene> &scene/*,
					   std::size_t voxel_grid_size,
					   float voxel_grid_ratio*/)
					   : gbuffer(ctx.get_backbuffer_size()),
						 scene(scene),
						 ctx(ctx),
						 //voxel_space(ctx, voxel_grid_size, voxel_grid_ratio),
						 hdr(ctx, &gbuffer),
						 shadows_projector(ctx, &scene->object_group(), &scene->scene_properties().lights_storage(), &shadows_storage),
						 prepopulate_depth_dispatch(ctx, scene.get()),
						 gbuffer_sorter(ctx, &gbuffer),
						 composer(ctx, this),
						 gbuffer_clearer(&gbuffer) {
	resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
		this->gbuffer.resize(size);
		rebuild_task_queue();
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);

	// composer.program->set_uniform("inv_projection", glm::inverse(ctx.projection_matrix()));

	composer_task = make_gpu_task("deferred_composition", &composer, hdr.get_input_fbo());
	fb_clearer_task = make_gpu_task("fb_clearer", &fb_clearer, get_fbo());
	gbuffer_clearer_task = make_gpu_task("gbuffer_clearer", &gbuffer_clearer, nullptr);
	gbuffer_sort_task = make_gpu_task("gbuffer_sorter", &gbuffer_sorter, nullptr);
	shadow_projector_task = make_gpu_task("shadow_projector", &shadows_projector, shadows_storage.get_fbo());
	prepopulate_depth_task = make_gpu_task("scene_prepopulate_depth", &prepopulate_depth_dispatch, get_fbo());

	fb_clearer_task->add_dependency(gbuffer_clearer_task);
	composer_task->add_dependency(fb_clearer_task);
	composer_task->add_dependency(gbuffer_sort_task);
	composer_task->add_dependency(shadow_projector_task);
	gbuffer_sort_task->add_dependency(fb_clearer_task);
	hdr.get_task()->add_dependency(composer_task);
	prepopulate_depth_task->add_dependency(fb_clearer_task);
	scene->add_dependency(prepopulate_depth_task);

	add_task(fb_clearer_task);
	rebuild_task_queue();
}

void GIRenderer::rebuild_task_queue() {
	if (!use_deferred_rendering) {
		for (auto &task_ptr : gui_tasks)
			q.remove_task_dependency(task_ptr, hdr.get_task());

		// q.remove_task(voxel_space.voxelizer(*scene));
		q.remove_task(hdr.get_task());
		q.remove_task(composer_task);
	}
	else {
		// q.add_task(voxel_space.voxelizer(*scene), nullptr);
		q.add_task(composer_task);
		q.add_task(hdr.get_task());
	}

	for (auto &task_ptr : added_tasks)
		mutate_gpu_task(task_ptr, get_fbo());
	mutate_gpu_task(fb_clearer_task, get_fbo());

	if (use_deferred_rendering)
		for (auto &task_ptr : gui_tasks)
			q.add_task_dependency(task_ptr, hdr.get_task());
}

void GIRenderer::set_deferred_rendering_enabled(bool enabled) {
	use_deferred_rendering = enabled;
	rebuild_task_queue();
}

void GIRenderer::render_queue() {
	q.dispatch();
}

void GIRenderer::add_task(const gpu_task::TaskPtr &t) {
	mutate_gpu_task(t, get_fbo());
	q.add_task(t);

	q.add_task_dependency(gbuffer_sort_task, t);
	if (t != fb_clearer_task)
		q.add_task_dependency(t, fb_clearer_task);

	added_tasks.insert(t);
}

void GIRenderer::remove_task(const gpu_task::TaskPtr &t) {
	q.remove_task(t);

	q.remove_task_dependency(gbuffer_sort_task, t);
	q.remove_task_dependency(t, fb_clearer_task);

	added_tasks.erase(t);
}

void GIRenderer::add_gui_task(const gpu_task::TaskPtr &t) {
	mutate_gpu_task(t, &ctx.gl()->defaut_framebuffer());
	q.add_task(t);

	if (use_deferred_rendering)
		q.add_task_dependency(t, hdr.get_task());
	q.add_task_dependency(t, fb_clearer_task);

	gui_tasks.insert(t);
}

void GIRenderer::remove_gui_task(const gpu_task::TaskPtr &t) {
	q.remove_task(t);

	q.remove_task_dependency(t, hdr.get_task());
	q.remove_task_dependency(t, fb_clearer_task);

	gui_tasks.erase(t);
}
