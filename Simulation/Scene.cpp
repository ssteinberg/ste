
#include "stdafx.h"
#include "Log.h"

#include "GLSLProgramLoader.h"

#include "Scene.h"

#include <vector>

using namespace StE::Graphics;

Scene::Scene(const StEngineControl &ctx) : renderable(StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "scene.vert", "scene.frag" })()) {
	request_state({ GL_CULL_FACE, true });
	request_state({ GL_DEPTH_TEST, true });

	linear_sampler.set_wrap_s(LLR::TextureWrapMode::Wrap);
	linear_sampler.set_wrap_t(LLR::TextureWrapMode::Wrap);
	linear_sampler.set_wrap_r(LLR::TextureWrapMode::Wrap);
	linear_sampler.set_min_filter(LLR::TextureFiltering::Linear);
	linear_sampler.set_mag_filter(LLR::TextureFiltering::Linear);
	linear_sampler.set_mipmap_filter(LLR::TextureFiltering::Linear);
	linear_sampler.set_anisotropic_filter(16);

 	int page_size = std::max(65536, vbo_type::page_size());
 	int virtual_size = 256 * page_size;
 
  	vbo = std::make_unique<vbo_type>(virtual_size);
  	indices = std::make_unique<elements_type>(virtual_size);
  
  	idb = std::make_unique<indirect_draw_buffer_type>(4 * page_size);
  	matbo = std::make_unique<material_data_buffer_type>(8 * page_size);
  
  	vao = std::make_unique<LLR::VertexArrayObject>();
  	(*vao)[0] = (*vbo)[0];
  	(*vao)[1] = (*vbo)[1];
	(*vao)[2] = (*vbo)[2];
	(*vao)[3] = (*vbo)[3];

	mesh_data_bo = std::make_unique<mesh_data_buffer_type>(10);
	mesh_data_bo_ptr = mesh_data_bo->map_write(10);

	get_program()->set_uniform("projection", ctx.projection_matrix());
	get_program()->set_uniform("far", ctx.get_far_clip());
	get_program()->set_uniform("near", ctx.get_near_clip());
	projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &proj, float, float fnear, float ffar) {
		this->get_program()->set_uniform("projection", proj);
		this->get_program()->set_uniform("far", ffar);
		this->get_program()->set_uniform("near", fnear);
	});
	ctx.signal_projection_change().connect(projection_change_connection);
}

void Scene::resize_mesh_data_bo() {
	auto newptr = std::make_unique<mesh_data_buffer_type>(mesh_data_bo->size() << 1);

	range<> lock_range{ 0, mesh_data_bo->size() * sizeof(mesh_data_buffer_type::T) };
	mesh_data_bo_ptr.wait(lock_range);
	mesh_data_bo_ptr.invalidate();

	(*newptr) << *mesh_data_bo;

	mesh_data_bo_ptr = newptr->map_write(newptr->size());
	mesh_data_bo = std::move(newptr);
}

void Scene::add_object(const std::shared_ptr<Object> &obj) {
	auto &ind = obj->get_mesh().get_indices();
	auto &vertices = obj->get_mesh().get_vertices();

 	LLR::IndirectMultiDrawElementsCommand idc;
 	idc.count = ind.size();
 	idc.instance_count = 1;
 	idc.first_index = total_indices;
 	idc.base_vertex = total_vertices;
 	idc.base_instance = 0;
 
  	vbo->commit_range(total_vertices, vertices.size());
  	indices->commit_range(total_indices, ind.size());
 	idb->commit_range(object_count, 1);
 
 	vbo->upload(total_vertices, vertices.size(), &vertices[0]);
 	indices->upload(total_indices, ind.size(), &ind[0]);
 	idb->upload(object_count, 1, &idc);
 
 	total_vertices += vertices.size();
 	total_indices += ind.size();
 
	auto &mat = obj->get_material();
 	objects.insert(std::make_pair(object_count, obj));

	auto diffuse = obj->get_material().get_diffuse();
	auto specular = obj->get_material().get_specular();
	auto normalmap = obj->get_material().get_normalmap();
	auto alphamap = obj->get_material().get_alphamap();
	auto brdf = obj->get_material().get_brdf();
 	material_descriptor md;
	if (diffuse != nullptr) {
		md.diffuse.tex_handler = diffuse->get_texture_handle(linear_sampler);
		md.diffuse.tex_handler.make_resident();
	}
	if (specular != nullptr) {
		md.specular.tex_handler = specular->get_texture_handle(linear_sampler);
		md.specular.tex_handler.make_resident();
	}
	if (normalmap != nullptr) {
		md.normalmap.tex_handler = normalmap->get_texture_handle(linear_sampler);
		md.normalmap.tex_handler.make_resident();
	}
	if (alphamap != nullptr) {
		md.alphamap.tex_handler = alphamap->get_texture_handle(linear_sampler);
		md.alphamap.tex_handler.make_resident();
	}
	if (brdf != nullptr) {
		md.brdf = brdf->descriptor();
		md.brdf.tex_handler.make_resident();
	}
	md.emission = obj->get_material().get_emission();

 	matbo->commit_range(object_count, 1);
 	matbo->upload(object_count, 1, &md);

	if (object_count >= mesh_data_bo->size())
		resize_mesh_data_bo();
	mesh_data_bo_ptr.get()[object_count].model = obj->get_model_transform_clear_dirty_flag();
	mesh_data_bo_ptr.get()[object_count].transpose_inverse_model = glm::transpose(glm::inverse(obj->get_model_transform_clear_dirty_flag()));

	++object_count;
}

void Scene::prepare() const {
 	using namespace LLR;

	for (auto &p : objects) {
		if (p.second->is_model_dirty()) {
			auto &mat = p.second->get_model_transform_clear_dirty_flag();
			range<> lock_range{ p.first * sizeof(mesh_data_buffer_type::T), sizeof(mesh_data_buffer_type::T) };

			mesh_data_bo_ptr.wait(lock_range);
			mesh_data_bo_ptr.get()[p.first].model = mat;
			mesh_data_bo_ptr.get()[p.first].transpose_inverse_model = glm::transpose(glm::inverse(mat));

			ranges_to_lock.push_back(lock_range);
		}
	}
 
	renderable::prepare();
 	vao->bind();
 	indices->bind();
 	idb->bind();
	0_storage_idx = *matbo;
	1_storage_idx = *mesh_data_bo;
}

void Scene::render() const {
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, objects.size(), 0);
}

void Scene::finalize() const {
	for (auto &r : ranges_to_lock)
		mesh_data_bo_ptr.lock(r);
	ranges_to_lock.clear();
}

