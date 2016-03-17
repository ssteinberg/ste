
#include "stdafx.hpp"
#include "GIRenderer.hpp"

#include "Quad.hpp"

#include "gl_current_context.hpp"

using namespace StE::Graphics;


void GIRenderer::deferred_composition::set_context_state() const {
	using namespace Core;
	
	Base::set_context_state();

	dr->fbo.bind_output_textures();
	0_storage_idx = dr->scene->scene_properties().material_storage().buffer();
	dr->scene->scene_properties().lights_storage().bind_buffers(1);
	ScreenFillingQuad.vao()->bind();
	
	program->bind();
}

void GIRenderer::deferred_composition::dispatch() const {
	dr->scene->scene_properties().pre_draw();
	Core::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
	dr->scene->scene_properties().post_draw();
}


GIRenderer::GIRenderer(const StEngineControl &ctx, 
					   Scene *scene/*,
					   std::size_t voxel_grid_size, 
					   float voxel_grid_ratio*/) 
					   : fbo(ctx.get_backbuffer_size()), 
						 scene(scene), 
						 ctx(ctx),
						 //voxel_space(ctx, voxel_grid_size, voxel_grid_ratio), 
						 hdr(std::make_shared<hdr_dof_postprocess>(ctx, fbo.z_buffer())), 
						 composer(std::make_shared<deferred_composition>(ctx, this)),
						 fb_clearer(std::make_shared<FbClearTask>()) {
	resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
		this->fbo.resize(size);
		hdr->set_z_buffer(fbo.z_buffer());
		
		for (auto &task_ptr : gui_tasks)
			q.update_task_fbo(task_ptr, &this->ctx.gl()->defaut_framebuffer());
		for (auto &task_ptr : added_tasks)
			q.update_task_fbo(task_ptr, get_fbo());
			
		q.update_task_fbo(hdr, &this->ctx.gl()->defaut_framebuffer());
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);

	composer->program->set_uniform("inv_projection", glm::inverse(ctx.projection_matrix()));
	
	composer->add_dependency(fb_clearer);
	hdr->add_dependency(composer);
	
	add_task(fb_clearer);
	rebuild_task_queue();
}

void GIRenderer::rebuild_task_queue() {
	q.remove_task(hdr);
	q.remove_task(composer);

	// q.add_task(voxel_space.voxelizer(*scene));
	
	if (use_deferred_rendering) {
		q.add_task(composer, hdr->get_input_fbo());
		q.add_task(hdr, &ctx.gl()->defaut_framebuffer());
	}
	
	for (auto &task_ptr : added_tasks)
		q.update_task_fbo(task_ptr, get_fbo());
}

void GIRenderer::set_deferred_rendering_enabled(bool enabled) {
	use_deferred_rendering = enabled;
	rebuild_task_queue();
}

void GIRenderer::render_queue(const StEngineControl &ctx) {	
	// queue().render(get_fbo());
	// ppq.render(&ctx.gl()->defaut_framebuffer());
	q.dispatch();
}

void GIRenderer::add_task(const gpu_task::TaskPtr &t) {
	composer->add_dependency(t);
	t->add_dependency(fb_clearer);
	
	q.add_task(t, get_fbo());
	added_tasks.insert(t);
}

void GIRenderer::remove_task(const gpu_task::TaskPtr &t) {
	composer->remove_dependency(t);
	t->remove_dependency(fb_clearer);
	
	q.remove_task(t);
	added_tasks.erase(t);
}

void GIRenderer::add_gui_task(const gpu_task::TaskPtr &t) {
	t->add_after(hdr);
	t->add_dependency(fb_clearer);
	
	q.add_task(t, &ctx.gl()->defaut_framebuffer());
	
	gui_tasks.insert(t);
}

void GIRenderer::remove_gui_task(const gpu_task::TaskPtr &t) {
	t->remove_after(hdr);
	t->remove_dependency(fb_clearer);
	
	q.remove_task(t);
	
	gui_tasks.erase(t);
}
