
#include "stdafx.h"
#include "Model.h"
#include "SurfaceIO.h"
#include "GLSLProgram.h"

#include <tinyobjloader/tiny_obj_loader.h>

#include <filesystem>
#include <stddef.h>
#include <algorithm>
#include <functional>

using namespace StE::Resource;
using StE::LLR::Texture2D;
using StE::LLR::VertexBufferObject;
using StE::LLR::VertexArrayObject;
using StE::LLR::ElementBufferObject;

Model::Model() {
	bLoaded = false;
}

bool Model::load_model(const std::string &file_path) {
	ste_log() << "Loading OBJ model " << file_path;

	 std::string dir = { file_path.begin(), std::find_if(file_path.rbegin(), file_path.rend(), [](char c) { return c == '/' || c == '\\'; }).base() };

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err = tinyobj::LoadObj(shapes, materials, file_path.c_str(), dir.c_str());

	if (!err.empty()) {
		ste_log_error() << "Couldn't load model " << file_path << ": " << err;
		return false;
	}

	std::vector<VertexBufferModel> vbo_data;
	std::vector<unsigned int> ind;

	int offset = 0;
	for (auto &shape : shapes) {
		auto &indices_vector = shape.mesh.indices;

		indices_offset.push_back(offset * sizeof(decltype(ind)::value_type));
		indices_sizes.push_back(indices_vector.size());
		ind.insert(ind.end(), indices_vector.begin(), indices_vector.end());
		auto transform_start = ind.end() - indices_vector.size();
		std::transform(transform_start, ind.end(), transform_start, std::bind2nd(std::plus<typename decltype(ind)::value_type>(), vbo_data.size()));

		material_indices.push_back(shape.mesh.material_ids[0]);

		offset += indices_vector.size();

		int vertices = shape.mesh.positions.size() / 3;
		int tc_stride = shape.mesh.texcoords.size() / vertices;
		int normals_stride = shape.mesh.normals.size() / vertices;
		for (size_t i = 0; i < vertices; ++i) {
			VertexBufferModel v;
			v.p = { shape.mesh.positions[3 * i + 0], shape.mesh.positions[3 * i + 1], shape.mesh.positions[3 * i + 2] };
			if (tc_stride) v.uv = { shape.mesh.texcoords[tc_stride * i + 0], shape.mesh.texcoords[tc_stride * i + 1] };
			if (normals_stride) v.n = { shape.mesh.normals[normals_stride * i + 0], shape.mesh.normals[normals_stride * i + 1], shape.mesh.normals[normals_stride * i + 2] };

			vbo_data.push_back(v);
		}
	}

	auto texture_loader = [&](const std::string &texture_name, texture_storage_type &texture_storage, int tex_storage_map) {
		std::string full_path = dir + texture_name;

		auto tex = SurfaceIO::load_2d(full_path);
		if (tex == nullptr)
			ste_log_warn() << "Couldn't load texture " << file_path;
		texture_storage[tex_storage_map] = std::move(tex);
	};

	for (size_t i = 0; i < materials.size(); i++) {
		auto diff = materials[i].diffuse_texname;
		auto normal = materials[i].bump_texname;
		auto opacity = materials[i].alpha_texname;

		if (diff.length()) texture_loader(diff, textures, i);
		if (normal.length()) texture_loader(normal, normal_maps, i);
		if (opacity.length()) texture_loader(opacity, masks, i);
	}

	this->indices = std::shared_ptr<ElementBufferObject<>>(new ElementBufferObject<>(ind));
	this->vbo = std::shared_ptr<vbo_type>(new vbo_type(vbo_data));

	vao = std::unique_ptr<VertexArrayObject>(new VertexArrayObject);
	(*vao)[0] = (*vbo)[0];
	(*vao)[1] = (*vbo)[1];
	(*vao)[2] = (*vbo)[2];

	return (bLoaded = true);
}

using namespace StE::LLR;

void Model::render(const LLR::GLSLProgram &program) {
	if (!bLoaded)
		return;

	vao->bind();
	VertexArrayObject::enable_vertex_attrib_array(0);
	VertexArrayObject::enable_vertex_attrib_array(1);
	VertexArrayObject::enable_vertex_attrib_array(2);
	indices->bind();

	for (int i = 0; i < indices_sizes.size(); ++i) {
		auto mat_index = material_indices[i];

		auto& tex = textures[mat_index];
		auto& nm = normal_maps[mat_index];
		auto& mask = masks[mat_index];

		if (tex != nullptr && tex->is_valid()) 
			0_sampler_idx = *tex;
		if (nm != nullptr && nm->is_valid()) 
			1_sampler_idx = *nm;
		if (mask != nullptr && mask->is_valid()) 
			2_sampler_idx = *mask;

		program.set_uniform("has_mask", mask != nullptr);
		program.set_uniform("has_normal_map", nm != nullptr);

		glDrawElements(GL_TRIANGLES, indices_sizes[i], GL_UNSIGNED_INT, reinterpret_cast<void*>(indices_offset[i]));
	}
}
