
#include "stdafx.h"
#include "ModelLoader.h"

#include "Material.h"
#include "Object.h"

#include "SurfaceIO.h"

using namespace StE::Resource;
using namespace StE::Graphics;
using StE::LLR::Texture2D;

void ModelLoader::process_model_mesh(optional<task_scheduler*> sched, const tinyobj::shape_t &shape, const std::vector<tinyobj::material_t> &materials, const std::string &dir, Graphics::Scene *scene) {
	std::vector<ObjectVertexData> vbo_data;

	int vertices = shape.mesh.positions.size() / 3;
	int tc_stride = shape.mesh.texcoords.size() / vertices;
	int normals_stride = shape.mesh.normals.size() / vertices;
	for (size_t i = 0; i < vertices; ++i) {
		ObjectVertexData v;
		v.p = { shape.mesh.positions[3 * i + 0], shape.mesh.positions[3 * i + 1], shape.mesh.positions[3 * i + 2] };
		if (tc_stride) v.uv = { shape.mesh.texcoords[tc_stride * i + 0], shape.mesh.texcoords[tc_stride * i + 1] };
		if (normals_stride) v.n = { shape.mesh.normals[normals_stride * i + 0], shape.mesh.normals[normals_stride * i + 1], shape.mesh.normals[normals_stride * i + 2] };

		vbo_data.push_back(v);
	}

	int mat_idx = shape.mesh.material_ids[0];

	auto texture_loader = [&](const std::string &texture_name, bool srgb) -> std::unique_ptr<LLR::Texture2D> {
		if (!texture_name.length()) return nullptr;
		std::string full_path = dir + texture_name;

		auto tex_task = SurfaceIO::load_texture_2d_task(full_path, srgb);
		std::unique_ptr<LLR::Texture2D> tex = tex_task(sched);
		if (tex == nullptr)
			ste_log_warn() << "Couldn't load texture " << full_path;
		return tex;
	};

	auto diff = texture_loader(materials[mat_idx].diffuse_texname, true);
	auto normal = texture_loader(materials[mat_idx].bump_texname, false);
	auto opacity = texture_loader(materials[mat_idx].alpha_texname, false);

	StE::task<void> f = [&](optional<task_scheduler*> sched) {
		Material mat;
		if (diff != nullptr) mat.make_diffuse(std::move(*diff));
		if (normal != nullptr) mat.make_heightmap(std::move(*normal));
		if (opacity != nullptr) mat.make_alphamap(std::move(*opacity));

		auto obj = scene->create_object(vbo_data, shape.mesh.indices, std::move(mat));
	};

	if (!sched)
		f();
	else
		sched->schedule_now_on_main_thread(f).wait();
}

StE::task<bool> ModelLoader::load_model_task(const std::string &file_path, Scene *scene) {
	return [=](optional<task_scheduler*> sched) -> bool {
		ste_log() << "Loading OBJ model " << file_path;

		std::string dir = { file_path.begin(), std::find_if(file_path.rbegin(), file_path.rend(), [](char c) { return c == '/' || c == '\\'; }).base() };

		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err = tinyobj::LoadObj(shapes, materials, file_path.c_str(), dir.c_str());

		if (!err.empty()) {
			ste_log_error() << "Couldn't load model " << file_path << ": " << err;
			return false;
		}

		std::vector<std::future<void>> futures;

		for (auto &shape : shapes) {
			StE::task<void> shape_task = [&](optional<task_scheduler*> sched) {
				process_model_mesh(sched, shape, materials, dir, scene);
			};

			if (!sched)
				shape_task();
			else
				futures.push_back(sched->schedule_now(shape_task));
		}

		for (auto &f : futures)
			f.wait();

		return true;
	};
}
