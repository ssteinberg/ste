
#include "stdafx.hpp"
#include "GIRenderer.hpp"

#include "Quad.hpp"
#include "Sampler.hpp"

#include "ShaderStorageBuffer.hpp"

#include "gl_current_context.hpp"

using namespace StE::Graphics;

void GIRenderer::setup_tasks() {
	composer_task = make_gpu_task("composition", &composer.get(), hdr.get().get_input_fbo());
	scene_task = make_gpu_task("scene", scene, nullptr);
	fb_clearer_task = make_gpu_task("fb_clearer", &fb_clearer, get_fbo());
	prepopulate_depth_task = make_gpu_task("prepopulate_depth", &prepopulate_depth_dispatch.get(), get_fbo());
	scene_geo_cull_task = make_gpu_task("geo_cull", &scene_geo_cull.get(), nullptr);
	downsample_depth_task = make_gpu_task("downsample_depth", &downsample_depth.get(), nullptr);
	shadow_projector_task = make_gpu_task("shdw_project", &shadows_projector.get(), nullptr);
	volumetric_scattering_scatter_task = make_gpu_task("scatter", &vol_scat_scatter.get(), nullptr);
	volumetric_scattering_gather_task = make_gpu_task("gather", &vol_scat_gather.get(), nullptr);
	lll_gen_task = make_gpu_task("pp_ll_gen", &lll_gen_dispatch.get(), get_fbo());

	hdr.get().get_task()->add_dependency(composer_task);

	prepopulate_depth_task->add_dependency(fb_clearer_task);
	prepopulate_depth_task->add_dependency(scene_geo_cull_task);

	downsample_depth_task->add_dependency(prepopulate_depth_task);

	scene_task->add_dependency(prepopulate_depth_task);
	scene_task->add_dependency(scene_geo_cull_task);
	scene_task->add_dependency(downsample_depth_task);

	scene_geo_cull_task->add_dependency(light_preprocess.get().get_task());

	lll_gen_task->add_dependency(light_preprocess.get().get_task());
	lll_gen_task->add_dependency(prepopulate_depth_task);
	lll_gen_task->add_dependency(downsample_depth_task);

	shadow_projector_task->add_dependency(light_preprocess.get().get_task());

	volumetric_scattering_scatter_task->add_dependency(light_preprocess.get().get_task());
	volumetric_scattering_scatter_task->add_dependency(shadow_projector_task);
	volumetric_scattering_scatter_task->add_dependency(downsample_depth_task);
	volumetric_scattering_scatter_task->add_dependency(lll_gen_task);
	volumetric_scattering_scatter_task->add_dependency(prepopulate_depth_task);
	volumetric_scattering_gather_task->add_dependency(volumetric_scattering_scatter_task);
	volumetric_scattering_gather_task->add_dependency(downsample_depth_task);

	composer_task->add_dependency(fb_clearer_task);
	composer_task->add_dependency(lll_gen_task);
	composer_task->add_dependency(light_preprocess.get().get_task());
	composer_task->add_dependency(shadow_projector_task);
	composer_task->add_dependency(volumetric_scattering_gather_task);
}

GIRenderer::GIRenderer(ctor_token,
					   const StEngineControl &ctx,
					   const Camera *camera,
					   Scene *scene)
					   : ctx(ctx),
					   	 gbuffer(ctx.get_backbuffer_size(), gbuffer_depth_target_levels()),
						 camera(camera),
						 scene(scene),
						 lll_storage(ctx.get_backbuffer_size()),
						 shadows_storage(ctx),
						 vol_scat_storage(ctx.get_backbuffer_size()) {
	hdr.load(ctx, &gbuffer);
	downsample_depth.load(ctx, &gbuffer);
	prepopulate_depth_dispatch.load(ctx, scene);
	scene_geo_cull.load(ctx, scene, &scene->scene_properties().lights_storage());
	composer.load(ctx, this);
	shadows_projector.load(ctx, scene, &scene->scene_properties().lights_storage(), &shadows_storage);
	lll_gen_dispatch.load(ctx, &scene->scene_properties().lights_storage(), &lll_storage);
	light_preprocess.load(ctx, &scene->scene_properties().lights_storage());
	vol_scat_gather.load(ctx, &vol_scat_storage);
	vol_scat_scatter.load(ctx, &vol_scat_storage, &lll_storage, &scene->scene_properties().lights_storage(), &shadows_storage);

	resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
		this->transform_buffers.update_proj_data(this->ctx.get_fov(), this->ctx.get_projection_aspect(), this->ctx.get_near_clip(), size);

		this->gbuffer.resize(size);
		this->lll_storage.resize(size);
		this->vol_scat_storage.resize(size);

		this->lll_gen_dispatch.get().set_depth_map(gbuffer.get_downsampled_depth_target());
		this->vol_scat_storage.set_depth_map(gbuffer.get_downsampled_depth_target());

		this->rebuild_task_queue();
	});
	projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](float aspect, float fovy, float near) {
		this->transform_buffers.update_proj_data(fovy, aspect, near, this->ctx.get_backbuffer_size());
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);
	ctx.signal_projection_change().connect(projection_change_connection);

	this->transform_buffers.update_proj_data(ctx.get_fov(), ctx.get_projection_aspect(), ctx.get_near_clip(), this->ctx.get_backbuffer_size());

	vol_scat_storage.set_depth_map(gbuffer.get_downsampled_depth_target());
	scene->set_target_gbuffer(&gbuffer);
}

void GIRenderer::rebuild_task_queue() {
	q.add_task(composer_task);
	q.add_task(hdr.get().get_task());
	q.add_task(get_scene_task());

	for (auto &task_ptr : added_tasks)
		mutate_gpu_task(task_ptr, get_fbo());
	mutate_gpu_task(fb_clearer_task, get_fbo());

	for (auto &task_ptr : gui_tasks)
		q.add_task_dependency(task_ptr, hdr.get().get_task());
}

void GIRenderer::render_queue() {
	transform_buffers.update_view_data(*this->camera);
	transform_buffers.bind_view_buffer(view_transform_buffer_bind_location);
	transform_buffers.bind_proj_buffer(proj_transform_buffer_bind_location);

	scene->update_scene();

	if (q.get_profiler() != nullptr) {
		auto ft = ctx.time_per_frame().count();
		q.get_profiler()->record_frame(ft);
	}

	q.dispatch();
}

void GIRenderer::add_task(const gpu_task::TaskPtr &t) {
	mutate_gpu_task(t, get_fbo());
	q.add_task(t);

	if (t != fb_clearer_task)
		q.add_task_dependency(t, fb_clearer_task);

	added_tasks.insert(t);
}

void GIRenderer::remove_task(const gpu_task::TaskPtr &t) {
	q.remove_task(t);

	q.remove_task_dependency(t, fb_clearer_task);

	added_tasks.erase(t);
}

void GIRenderer::add_gui_task(const gpu_task::TaskPtr &t) {
	mutate_gpu_task(t, &ctx.gl()->defaut_framebuffer());
	q.add_task(t);

	q.add_task_dependency(t, hdr.get().get_task());
	q.add_task_dependency(t, fb_clearer_task);

	gui_tasks.insert(t);
}

void GIRenderer::remove_gui_task(const gpu_task::TaskPtr &t) {
	q.remove_task(t);

	q.remove_task_dependency(t, hdr.get().get_task());
	q.remove_task_dependency(t, fb_clearer_task);

	gui_tasks.erase(t);
}

int GIRenderer::gbuffer_depth_target_levels() {
	return glm::ceil(glm::log(linked_light_lists::lll_image_res_multiplier)) + 1;
}
