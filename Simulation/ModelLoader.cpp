
#include "stdafx.h"
#include "ModelLoader.h"

#include "Material.h"
#include "Object.h"

#include "SurfaceIO.h"

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
	for (size_t i = 0; i < vertices; ++i) {
		ObjectVertexData v;
		v.p = { shape.mesh.positions[3 * i + 0], shape.mesh.positions[3 * i + 1], shape.mesh.positions[3 * i + 2] };
		if (tc_stride) v.uv = { shape.mesh.texcoords[tc_stride * i + 0], shape.mesh.texcoords[tc_stride * i + 1] };
		if (normals_stride) v.n = { shape.mesh.normals[normals_stride * i + 0], shape.mesh.normals[normals_stride * i + 1], shape.mesh.normals[normals_stride * i + 2] };

		vbo_data.push_back(v);
	}

	int mat_idx = shape.mesh.material_ids[0];
	std::shared_ptr<LLR::Texture2D> &diff = textures[materials[mat_idx].diffuse_texname];
	std::shared_ptr<LLR::Texture2D> &normal = textures[materials[mat_idx].bump_texname];
	std::shared_ptr<LLR::Texture2D> &opacity = textures[materials[mat_idx].alpha_texname];

	return sched->schedule_now_on_main_thread([=](optional<task_scheduler*> sched) {
		Material mat;
		if (diff != nullptr) mat.set_diffuse(diff);
		if (normal != nullptr) mat.set_heightmap(normal);
		if (opacity != nullptr) mat.set_alphamap(opacity);

		scene->create_object(vbo_data, shape.mesh.indices, std::move(mat));
	});
}

StE::task<void> ModelLoader::load_texture(const std::string &name, bool srgb, texture_map_type *texmap, const std::string &dir) {
	return StE::task<std::unique_ptr<gli::texture2D>>([=](optional<task_scheduler*> sched) {
		std::string full_path = dir + name;

		auto tex_task = SurfaceIO::load_surface_2d_task(full_path, srgb);
		gli::texture2D tex = tex_task(sched);
		if (tex.empty())
			ste_log_warn() << "Couldn't load texture " << full_path;
		return std::make_unique<gli::texture2D>(std::move(tex));
	}).then_on_main_thread([=](optional<task_scheduler*> sched, std::unique_ptr<gli::texture2D> &&tex) {
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
						futures.push_back(sched->schedule_now(load_texture(str, true, &textures, dir)));
					}
				for (auto &str : { materials[mat_idx].bump_texname, materials[mat_idx].alpha_texname })
					if (str.length() && textures.find(str) == textures.end()) {
						textures.emplace(std::make_pair(str, std::shared_ptr<LLR::Texture2D>(nullptr)));
						futures.push_back(sched->schedule_now(load_texture(str, false, &textures, dir)));
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
