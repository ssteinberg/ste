
#include "stdafx.hpp"
#include "ModelFactory.hpp"

#include "Material.hpp"
#include "Object.hpp"
#include "mesh.hpp"

#include "SurfaceFactory.hpp"

#include "normal_map_from_height_map.hpp"
#include "bme_brdf_representation.hpp"

#include <algorithm>

#include <gli/gli.hpp>

using namespace StE::Resource;
using namespace StE::Graphics;
using StE::Core::Texture2D;

std::future<void> ModelFactory::process_model_mesh(optional<task_scheduler*> sched,
												  Graphics::material_storage *matstorage,
												  const tinyobj::shape_t &shape,
												  Graphics::ObjectGroup *object_group,
												  materials_type &materials,
												  texture_map_type &textures,
												  brdf_map_type &brdfs) {
	std::vector<ObjectVertexData> vbo_data;
	std::vector<std::uint32_t> vbo_indices;

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

		vbo_data.push_back(v);
	}

	if (std::is_same<std::uint32_t, decltype(shape.mesh.indices[0])>::value) {
		vbo_indices = shape.mesh.indices;
	}
	else {
		for (auto ind : shape.mesh.indices)
			vbo_indices.push_back(static_cast<std::uint32_t>(ind));
	}

	if (tc_stride && normals_stride) {
		for (unsigned i = 0; i < shape.mesh.indices.size() - 2; i += 3) {
			auto i0 = shape.mesh.indices[i];
			auto i1 = shape.mesh.indices[i + 1];
			auto i2 = shape.mesh.indices[i + 2];

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

			v0.t += sdir;
			v1.t += sdir;
			v2.t += sdir;
		}

		for (auto &v : vbo_data) {
			glm::vec3 t = v.t;

			v.t = glm::normalize(t - v.n * glm::dot(v.n, t));
		}
	}

	int mat_idx = shape.mesh.material_ids[0];
	std::shared_ptr<Core::Texture2D> &diff = textures[materials[mat_idx].diffuse_texname];
	std::shared_ptr<Core::Texture2D> &opacity = textures[materials[mat_idx].alpha_texname];
	std::shared_ptr<Core::Texture2D> &specular = textures[materials[mat_idx].specular_texname];
	std::shared_ptr<Core::Texture2D> normalmap = materials[mat_idx].bump_texname.length() ? textures[materials[mat_idx].bump_texname + "nm"] : nullptr;

	std::string brdf_name = materials[mat_idx].unknown_parameter["brdf"];
	std::shared_ptr<BRDF> brdf = brdf_name.length() ? brdfs[brdf_name] : nullptr;

	return sched->schedule_now_on_main_thread([=, vbo_data = std::move(vbo_data), vbo_indices = std::move(vbo_indices)](optional<task_scheduler*> sched) {
		auto mat = std::make_shared<Material>();
		if (diff != nullptr) mat->set_diffuse(diff);
		if (specular != nullptr) mat->set_specular(specular);
		if (normalmap != nullptr) mat->set_normalmap(normalmap);
		if (opacity != nullptr) mat->set_alphamap(opacity);
		mat->set_brdf(brdf);
		auto matid = matstorage->add_material(mat);

		std::unique_ptr<StE::Graphics::mesh<StE::Graphics::mesh_subdivion_mode::Triangles>> m = std::make_unique<StE::Graphics::mesh<StE::Graphics::mesh_subdivion_mode::Triangles>>();
		m->set_indices(std::move(vbo_indices));
		m->set_vertices(std::move(vbo_data));

		std::shared_ptr<StE::Graphics::Object> obj = std::make_shared<StE::Graphics::Object>(std::move(m));
		obj->set_material_id(matid);

		object_group->add_object(obj);
	});
}

StE::task<void> ModelFactory::load_texture(const std::string &name,
										  bool srgb,
										  texture_map_type *texmap,
										  bool bumpmap,
										  const boost::filesystem::path &dir,
										  float normal_map_bias) {
	return StE::task<std::unique_ptr<gli::texture2d>>([=](optional<task_scheduler*> sched) {
		std::string normalized_name = name;
		std::replace(normalized_name.begin(), normalized_name.end(), '\\', boost::filesystem::path::preferred_separator);
		boost::filesystem::path full_path = dir / normalized_name;

		auto tex_task = SurfaceFactory::load_surface_2d_task(full_path, srgb);
		gli::texture2d tex = tex_task(sched);
		if (tex.empty())
			ste_log_warn() << "Couldn't load texture " << full_path.string() << std::endl;

		return std::make_unique<gli::texture2d>(std::move(tex));
	}).then_on_main_thread([=](optional<task_scheduler*> sched, std::unique_ptr<gli::texture2d> &&tex) {
		if (!tex->empty() && bumpmap) {
			auto nm = normal_map_from_height_map<gli::FORMAT_R8_UNORM_PACK8>()(*tex, normal_map_bias);
			auto nm_tex = std::make_shared<Core::Texture2D>(std::move(nm), true);
			(*texmap)[name + "nm"] = std::move(nm_tex);
		}

		(*texmap)[name] = std::make_shared<Core::Texture2D>(*tex, true);
	});
}

std::vector<std::future<void>> ModelFactory::load_textures(task_scheduler* sched,
														  shapes_type &shapes,
														  materials_type &materials,
														  texture_map_type &tex_map,
														  const boost::filesystem::path &dir,
														  float normal_map_bias) {
	tex_map.emplace(std::make_pair(std::string(""), std::shared_ptr<Core::Texture2D>(nullptr)));

	std::vector<std::future<void>> futures;
	for (auto &shape : shapes) {
		int mat_idx = shape.mesh.material_ids[0];

		for (auto &str : { materials[mat_idx].diffuse_texname })
			if (str.length() && tex_map.find(str) == tex_map.end()) {
				tex_map.emplace(std::make_pair(str, std::shared_ptr<Core::Texture2D>(nullptr)));
				futures.push_back(sched->schedule_now(load_texture(str,
																   true,
																   &tex_map,
																   false,
																   dir,
																   normal_map_bias)));
			}

		for (auto &str : { materials[mat_idx].bump_texname, materials[mat_idx].alpha_texname, materials[mat_idx].specular_texname })
			if (str.length() && tex_map.find(str) == tex_map.end()) {
				tex_map.emplace(std::make_pair(str, std::shared_ptr<Core::Texture2D>(nullptr)));
				futures.push_back(sched->schedule_now(load_texture(str,
																   false,
																   &tex_map,
																   str == materials[mat_idx].bump_texname,
																   dir,
																   normal_map_bias)));
			}
	}

	return futures;
}

std::vector<std::future<void>> ModelFactory::load_brdfs(const StEngineControl *ctx,
													   shapes_type &shapes,
													   materials_type &materials,
													   brdf_map_type &brdf_map,
													   const boost::filesystem::path &dir) {
	std::vector<std::future<void>> futures;

	for (auto &shape : shapes) {
		int mat_idx = shape.mesh.material_ids[0];

		std::string brdf_name = materials[mat_idx].unknown_parameter["brdf"];
		if (brdf_name.length() && brdf_map.find(brdf_name) == brdf_map.end()) {
			std::string normalized_name = brdf_name;
			std::replace(normalized_name.begin(), normalized_name.end(), '\\', boost::filesystem::path::preferred_separator);

			brdf_map.emplace(std::make_pair(brdf_name, std::shared_ptr<Graphics::BRDF>(nullptr)));

			futures.push_back(ctx->scheduler().schedule_now([brdfs = &brdf_map, normalized_name = normalized_name, brdf_name = brdf_name, ctx = ctx, dir = dir](optional<task_scheduler*> sched) {
				std::unique_ptr<BRDF> ptr = Graphics::bme_brdf_representation::BRDF_from_bme_representation_task(*ctx, dir / normalized_name)(&*sched);
				std::shared_ptr<Graphics::BRDF> brdf = std::make_shared<Graphics::BRDF>(std::move(*ptr));
				(*brdfs)[brdf_name] = brdf;
			}));
		}
	}

	return futures;
}

StE::task<bool> ModelFactory::load_model_task(const StEngineControl &context,
											  const boost::filesystem::path &file_path,
											  ObjectGroup *object_group,
											  Graphics::SceneProperties *scene_properties,
											  float normal_map_bias) {
	const StEngineControl *ctx = &context;

	struct _model_loader_task_block {
		bool ret;
		std::unique_ptr<texture_map_type> textures;
		std::unique_ptr<brdf_map_type> brdfs;
	};

	return task<_model_loader_task_block>([=](optional<task_scheduler*> sched) {
		assert(sched);

		_model_loader_task_block block;
		block.textures = std::make_unique<texture_map_type>();
		block.brdfs = std::make_unique<brdf_map_type>();

		auto path_string = file_path.string();
		ste_log() << "Loading OBJ model " << path_string << std::endl;

		std::string dir = { path_string.begin(), std::find_if(path_string.rbegin(), path_string.rend(), [](char c) { return c == '/' || c == '\\'; }).base() };

		shapes_type shapes;
		materials_type materials;

		std::string err;
		if (!tinyobj::LoadObj(shapes, materials, err, path_string.c_str(), dir.c_str(), true)) {
			ste_log_error() << "Couldn't load model " << path_string << ": " << err;
			block.ret = false;
			return block;
		}

		{
			for (auto &f : load_textures(&*sched, shapes, materials, *block.textures, dir, normal_map_bias))
				f.wait();
			for (auto &f : load_brdfs(ctx, shapes, materials, *block.brdfs, boost::filesystem::path("Data/bxdf/")))
				f.wait();
		}

		{
			std::vector<std::future<void>> futures;
			for (auto &shape : shapes)
				futures.push_back(process_model_mesh(sched, &scene_properties->materials_storage(), shape, object_group, materials, *block.textures, *block.brdfs));

			for (auto &f : futures)
				f.wait();
		}

		block.ret = true;
		return block;
	}).then_on_main_thread([](optional<task_scheduler*> sched, _model_loader_task_block &&block) -> bool {
		block.textures = nullptr;
		block.brdfs = nullptr;

		return block.ret;
	});
}
