
#include "stdafx.h"
#include "ModelLoader.h"

#include "Material.h"
#include "Object.h"

#include "SurfaceIO.h"

#include "normal_map_from_height_map.h"

#include <gli/gli.hpp>

using namespace StE::Resource;
using namespace StE::Graphics;
using StE::LLR::Texture2D;

std::future<void> ModelLoader::process_model_mesh(optional<task_scheduler*> sched,
												  const tinyobj::shape_t &shape,
												  Graphics::Scene *scene,
												  std::vector<tinyobj::material_t> &materials,
												  texture_map_type &textures) {
	std::vector<ObjectVertexData> vbo_data;

	unsigned vertices = shape.mesh.positions.size() / 3;
	unsigned tc_stride = shape.mesh.texcoords.size() / vertices;
	unsigned normals_stride = shape.mesh.normals.size() / vertices;
	for (unsigned i = 0; i < vertices; ++i) {
		ObjectVertexData v;
		v.p = { shape.mesh.positions[3 * i + 0], shape.mesh.positions[3 * i + 1], shape.mesh.positions[3 * i + 2] };
		if (tc_stride)
			v.uv = { shape.mesh.texcoords[tc_stride * i + 0], shape.mesh.texcoords[tc_stride * i + 1] };
		else
			v.uv = glm::vec2(0);
		if (normals_stride)
			v.n = { shape.mesh.normals[normals_stride * i + 0], shape.mesh.normals[normals_stride * i + 1], shape.mesh.normals[normals_stride * i + 2] };
		else
			v.n = glm::vec3(0);
		v.t = glm::vec3(0);
		v.b = glm::vec3(0);

		vbo_data.push_back(v);
	}

	if (tc_stride && normals_stride) {
		for (unsigned i = 0; i < shape.mesh.indices.size(); i += 3) {
			unsigned i0 = shape.mesh.indices[i];
			unsigned i1 = shape.mesh.indices[i + 1];
			unsigned i2 = shape.mesh.indices[i + 2];

			ObjectVertexData &v0 = vbo_data[i0];
			ObjectVertexData &v1 = vbo_data[i1];
			ObjectVertexData &v2 = vbo_data[i2];

			float x0 = v1.p.x - v0.p.x;
			float x1 = v2.p.x - v0.p.x;
			float y0 = v1.p.y - v0.p.y;
			float y1 = v2.p.y - v0.p.y;
			float z0 = v1.p.z - v0.p.z;
			float z1 = v2.p.z - v0.p.z;

			float s0 = v1.uv.x - v0.uv.x;
			float s1 = v2.uv.x - v0.uv.x;
			float t0 = v1.uv.y - v0.uv.y;
			float t1 = v2.uv.y - v0.uv.y;

			float r = 1.f / (s0 * t1 - s1 * t0);
			glm::vec3 sdir((t1 * x0 - t0 * x1) * r, (t1 * y0 - t0 * y1) * r, (t1 * z0 - t0 * z1) * r);
			glm::vec3 tdir((s0 * x1 - s1 * x0) * r, (s0 * y1 - s1 * y0) * r, (s0 * z1 - s1 * z0) * r);

			v0.t += sdir;
			v1.t += sdir;
			v2.t += sdir;
			v0.b += tdir;
			v1.b += tdir;
			v2.b += tdir;
		}

		for (auto &v : vbo_data) {
			glm::vec3 t = v.t;

			v.t = glm::normalize(t - v.n * glm::dot(v.n, t));
			v.b = glm::cross(v.n, v.t);
		}
	}

	int mat_idx = shape.mesh.material_ids[0];
	std::shared_ptr<LLR::Texture2D> &diff = textures[materials[mat_idx].diffuse_texname];
	std::shared_ptr<LLR::Texture2D> &heightmap = textures[materials[mat_idx].bump_texname];
	std::shared_ptr<LLR::Texture2D> &opacity = textures[materials[mat_idx].alpha_texname];
	std::shared_ptr<LLR::Texture2D> &specular = textures[materials[mat_idx].specular_texname];
	std::shared_ptr<LLR::Texture2D> normalmap = materials[mat_idx].bump_texname.length() ? textures[materials[mat_idx].bump_texname + "nm"] : nullptr;

	return sched->schedule_now_on_main_thread([=](optional<task_scheduler*> sched) {
		Material mat;
		if (diff != nullptr) mat.set_diffuse(diff);
		if (specular != nullptr) mat.set_specular(specular);
		if (heightmap != nullptr) mat.set_heightmap(heightmap);
		if (normalmap != nullptr) mat.set_normalmap(normalmap);
		if (opacity != nullptr) mat.set_alphamap(opacity);

		scene->create_object(vbo_data, shape.mesh.indices, std::move(mat));
	});
}

StE::task<void> ModelLoader::load_texture(const std::string &name, bool srgb, texture_map_type *texmap, bool bumpmap, const std::string &dir) {
	return StE::task<std::unique_ptr<gli::texture2D>>([=](optional<task_scheduler*> sched) {
		std::string full_path = dir + name;

		auto tex_task = SurfaceIO::load_surface_2d_task(full_path, srgb);
		gli::texture2D tex = tex_task(sched);
		if (tex.empty())
			ste_log_warn() << "Couldn't load texture " << full_path;

		return std::make_unique<gli::texture2D>(std::move(tex));
	}).then_on_main_thread([=](optional<task_scheduler*> sched, std::unique_ptr<gli::texture2D> &&tex) {
		if (!tex->empty() && bumpmap) {
			auto nm = normal_map_from_height_map<gli::FORMAT_R8_UNORM>()(*tex, .01f);
			auto nm_tex = std::make_shared<LLR::Texture2D>(std::move(nm), true);
			(*texmap)[name + "nm"] = std::move(nm_tex);
		}

		(*texmap)[name] = std::make_shared<LLR::Texture2D>(*tex, true);
	});
}

StE::task<bool> ModelLoader::load_model_task(const std::string &file_path, Scene *scene) {
	return [=](optional<task_scheduler*> sched) -> bool {
		assert(sched);
		ste_log() << "Loading OBJ model " << file_path;

		std::string dir = { file_path.begin(), std::find_if(file_path.rbegin(), file_path.rend(), [](char c) { return c == '/' || c == '\\'; }).base() };

		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err = tinyobj::LoadObj(shapes, materials, file_path.c_str(), dir.c_str());

		if (!err.empty()) {
			ste_log_error() << "Couldn't load model " << file_path << ": " << err;
			return false;
		}

		texture_map_type textures;
		textures.emplace(std::make_pair(std::string(""), std::shared_ptr<LLR::Texture2D>(nullptr)));

		{
			std::vector<std::future<void>> futures;
			for (auto &shape : shapes) {
				int mat_idx = shape.mesh.material_ids[0];

				for (auto &str : { materials[mat_idx].diffuse_texname })
					if (str.length() && textures.find(str) == textures.end()) {
						textures.emplace(std::make_pair(str, std::shared_ptr<LLR::Texture2D>(nullptr)));
						futures.push_back(sched->schedule_now(load_texture(str, true, &textures, false, dir)));
					}
				for (auto &str : { materials[mat_idx].bump_texname, materials[mat_idx].alpha_texname, materials[mat_idx].specular_texname })
					if (str.length() && textures.find(str) == textures.end()) {
						textures.emplace(std::make_pair(str, std::shared_ptr<LLR::Texture2D>(nullptr)));
						futures.push_back(sched->schedule_now(load_texture(str, false, &textures, str == materials[mat_idx].bump_texname, dir)));
					}
			}

			for (auto &f : futures)
				f.wait();
		}

		{
			std::vector<std::future<void>> futures;
			for (auto &shape : shapes)
				futures.push_back(process_model_mesh(sched, shape, scene, materials, textures));

			for (auto &f : futures)
				f.wait();
		}

		return true;
	};
}
