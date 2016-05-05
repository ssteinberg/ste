
#include "stdafx.hpp"
#include "GIRenderer.hpp"

#include "Quad.hpp"
#include "Sampler.hpp"

#include "ShaderStorageBuffer.hpp"

#include "gl_current_context.hpp"

using namespace StE::Graphics;


void GIRenderer::deferred_composition::set_context_state() const {
	using namespace Core;

	auto &ls = dr->scene->scene_properties().lights_storage();

	GL::gl_current_context::get()->enable_state(StE::Core::GL::BasicStateName::TEXTURE_CUBE_MAP_SEAMLESS);

	dr->gbuffer.bind_gbuffer();
	0_storage_idx = dr->scene->scene_properties().materials_storage().buffer();

	ls.bind_lights_buffer(2);
	ls.bind_lights_transform_buffer(3);

	dr->lll_storage.bind_lll_buffer();

	8_tex_unit = *dr->shadows_storage.get_cubemaps();
	8_sampler_idx = dr->shadows_storage.get_shadow_sampler();

	ScreenFillingQuad.vao()->bind();

	program->bind();
}

void GIRenderer::deferred_composition::dispatch() const {
	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Core::GL::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
}


GIRenderer::GIRenderer(const StEngineControl &ctx,
					   const Camera *camera,
					   Scene *scene/*,
					   std::size_t voxel_grid_size,
					   float voxel_grid_ratio*/)
					   : gbuffer(ctx.get_backbuffer_size()),
						 ctx(ctx),
						 camera(camera),
						 scene(scene),
						 //voxel_space(ctx, voxel_grid_size, voxel_grid_ratio),
						 hdr(ctx, &gbuffer),
						 lll_storage(ctx.get_backbuffer_size()),
						 lll_gen_dispatch(ctx, &scene->scene_properties().lights_storage(), &lll_storage),
						 shadows_storage(ctx),
						 shadows_projector(ctx, scene, &scene->scene_properties().lights_storage(), &shadows_storage),
						 prepopulate_depth_dispatch(ctx, scene),
						 scene_frustum_cull(ctx, scene, &scene->scene_properties().lights_storage()),
						 gbuffer_sorter(ctx, &gbuffer),
						 light_preprocess(ctx, &scene->scene_properties().lights_storage(), &hdr),
						 composer(ctx, this),
						 gbuffer_clearer(&gbuffer) {
	resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
		this->gbuffer.resize(size);
		this->lll_storage.resize(size);

		this->lll_gen_dispatch.set_depth_map(gbuffer.get_depth_target());

		rebuild_task_queue();
	});
	projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](const glm::mat4 &proj, float, float n, float f) {
		shadowmap_storage::update_shader_shadow_proj_uniforms(composer.program.get(), n, f);
		update_shader_shadow_proj_uniforms(proj);
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);
	ctx.signal_projection_change().connect(projection_change_connection);

	shadowmap_storage::update_shader_shadow_proj_uniforms(composer.program.get(), ctx.get_near_clip(), ctx.get_far_clip());
	update_shader_shadow_proj_uniforms(ctx.projection_matrix());


	lll_gen_dispatch.set_depth_map(gbuffer.get_depth_target());
	scene->set_target_gbuffer(get_gbuffer());

	composer_task = make_gpu_task("composition", &composer, hdr.get_input_fbo());
	scene_task = make_gpu_task("scene", scene, nullptr);
	fb_clearer_task = make_gpu_task("fb_clearer", &fb_clearer, get_fbo());
	prepopulate_depth_task = make_gpu_task("prepopulate_depth", &prepopulate_depth_dispatch, get_fbo());
	scene_frustum_cull_task = make_gpu_task("frustum_cull", &scene_frustum_cull, nullptr);
	gbuffer_clearer_task = make_gpu_task("gbuffer_clear", &gbuffer_clearer, nullptr);
	gbuffer_sort_task = make_gpu_task("gbuffer_sort", &gbuffer_sorter, nullptr);
	shadow_projector_task = make_gpu_task("shadow_project", &shadows_projector, nullptr);
	lll_gen_task = make_gpu_task("pp_ll_gen", &lll_gen_dispatch, get_fbo());

	fb_clearer_task->add_dependency(gbuffer_clearer_task);
	gbuffer_sort_task->add_dependency(fb_clearer_task);
	hdr.get_task()->add_dependency(composer_task);
	prepopulate_depth_task->add_dependency(fb_clearer_task);
	prepopulate_depth_task->add_dependency(scene_frustum_cull_task);
	scene_task->add_dependency(prepopulate_depth_task);
	scene_task->add_dependency(scene_frustum_cull_task);
	scene_frustum_cull_task->add_dependency(light_preprocess.get_task());
	lll_gen_task->add_dependency(light_preprocess.get_task());
	lll_gen_task->add_dependency(prepopulate_depth_task);
	shadow_projector_task->add_dependency(light_preprocess.get_task());
	composer_task->add_dependency(fb_clearer_task);
	composer_task->add_dependency(gbuffer_sort_task);
	composer_task->add_dependency(lll_gen_task);
	composer_task->add_dependency(light_preprocess.get_task());
	composer_task->add_dependency(shadow_projector_task);

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
	view_matrix_buffer.update_with_camera(*this->camera, ctx.projection_matrix());
	view_matrix_buffer.bind_buffer(view_matrix_buffer_bind_location);

	if (q.get_profiler() != nullptr) {
		auto ft = ctx.time_per_frame().count();
		q.get_profiler()->record_frame(ft);
	}

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

void GIRenderer::update_shader_shadow_proj_uniforms(const glm::mat4 &projection) {
	float proj00 = projection[0][0];
	float proj11 = projection[1][1];
	float proj22 = projection[2][2];
	float proj23 = projection[3][2];

	composer.program->set_uniform("proj00", proj00);
	composer.program->set_uniform("proj11", proj11);
	composer.program->set_uniform("proj22", proj22);
	composer.program->set_uniform("proj23", proj23);
}
