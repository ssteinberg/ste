
#include "stdafx.h"
#include "Log.h"

#include "Scene.h"

using namespace StE::Graphics;

Scene::Scene() {
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
}

Object *Scene::create_object(const std::vector<ObjectVertexData> &vertices, const std::vector<unsigned> &ind, Material &&mat) {
 	LLR::IndirectMultiDrawElementsCommand idc;
 	idc.count = ind.size();
 	idc.instance_count = 1;
 	idc.first_index = total_indices;
 	idc.base_vertex = total_vertices;
 	idc.base_instance = 0;
 
  	vbo->commit_range(total_vertices, vertices.size());
  	indices->commit_range(total_indices, ind.size());
 	idb->commit_range(objects.size(), 1);
 
 	vbo->upload(total_vertices, vertices.size(), &vertices[0]);
 	indices->upload(total_indices, ind.size(), &ind[0]);
 	idb->upload(objects.size(), 1, &idc);
 
 	total_vertices += vertices.size();
 	total_indices += ind.size();
 
 	Object *obj = new Object(vertices.size(), ind.size());
 	obj->set_material(std::move(mat));
 	objects.push_back(std::unique_ptr<Object>(obj));
 
 	auto diffuse = obj->get_material().get_diffuse();
 	auto specular = obj->get_material().get_specular();
 	auto heightmap = obj->get_material().get_heightmap();
 	auto alphamap = obj->get_material().get_alphamap();
 	auto tex0 = obj->get_material().get_tex0();
 	auto tex1 = obj->get_material().get_tex1();
 	auto tex2 = obj->get_material().get_tex2();
 	auto tex3 = obj->get_material().get_tex3();
 	material_descriptor md;
 	if (diffuse != nullptr) {
 		md.diffuse.tex_handler = diffuse->get_texture_handle(linear_sampler);
		md.diffuse.tex_handler.make_resident();
 	}
 	if (specular != nullptr) {
 		md.specular.tex_handler = specular->get_texture_handle(linear_sampler);
		md.specular.tex_handler.make_resident();
 	}
 	if (heightmap != nullptr) {
 		md.heightmap.tex_handler = heightmap->get_texture_handle(linear_sampler);
		md.heightmap.tex_handler.make_resident();
 	}
 	if (alphamap != nullptr) {
 		md.alphamap.tex_handler = alphamap->get_texture_handle(linear_sampler);
		md.alphamap.tex_handler.make_resident();
 	}
 	if (tex0 != nullptr) {
 		md.tex0.tex_handler = tex0->get_texture_handle();
 	}
 	if (tex1 != nullptr) {
 		md.tex1.tex_handler = tex1->get_texture_handle();
 	}
 	if (tex2 != nullptr) {
 		md.tex2.tex_handler = tex2->get_texture_handle();
 	}
 	if (tex3 != nullptr) {
 		md.tex3.tex_handler = tex3->get_texture_handle();
 	}
 	matbo->commit_range(objects.size() - 1, 1);
 	matbo->upload(objects.size() - 1, 1, &md);
 
 	return obj;
}

void Scene::render() const {
 	using namespace LLR;
 
 	vao->bind();
 	indices->bind();
 	idb->bind();
 	0_storage_idx = *matbo;
 
 	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, objects.size(), 0);
}
