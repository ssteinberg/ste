
#include "stdafx.hpp"
#include "ModelFactory.hpp"

#include "Material.hpp"
#include "Object.hpp"
#include "mesh.hpp"

#include "SurfaceFactory.hpp"

#include "normal_map_from_height_map.hpp"

#include <string>
#include <algorithm>

using namespace StE::Resource;
using namespace StE::Graphics;
using StE::Core::Texture2D;

std::future<void> ModelFactory::process_model_mesh(optional<task_scheduler*> sched,
												  Graphics::material_storage *matstorage,
												  const tinyobj::shape_t &shape,
												  Graphics::ObjectGroup *object_group,
												  materials_type &materials,
												  texture_map_type &textures) {
	std::vector<ObjectVertexData> vbo_data;
	std::vector<std::uint32_t> vbo_indices;
	std::vector<std::pair<glm::vec3, glm::vec3>> nt;

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

		if (normals_stride) {
			glm::vec3 n = { shape.mesh.normals[normals_stride * i + 0], shape.mesh.normals[normals_stride * i + 1], shape.mesh.normals[normals_stride * i + 2] };
			n = glm::normalize(n);
			nt.push_back(std::make_pair(n, glm::vec3(0)));
		}
		else
			nt.push_back(std::make_pair(glm::vec3(0), glm::vec3(0)));

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

			nt[i0].second += sdir;
			nt[i1].second += sdir;
			nt[i2].second += sdir;
		}

		for (unsigned i = 0; i < vbo_data.size(); ++i) {
			auto &v = vbo_data[i];

			// Gram-Schmidt orthogonalization process
			glm::vec3 n = nt[i].first;
			glm::vec3 t = nt[i].second;
			t = glm::normalize(t - n * glm::dot(n, t));
			glm::vec3 b = glm::cross(t,n);

			v.tangent_frame_from_tbn(t,b,n);
		}
	}

	int mat_idx = shape.mesh.material_ids[0];
	auto &material = materials[mat_idx];

	std::shared_ptr<Core::Texture2D> &diff_map = textures[material.diffuse_texname];
	std::shared_ptr<Core::Texture2D> &opacity_map = textures[material.alpha_texname];
	std::shared_ptr<Core::Texture2D> &specular_map = textures[material.specular_texname];
	std::shared_ptr<Core::Texture2D> normalmap = material.bump_texname.length() ? textures[material.bump_texname + "nm"] : nullptr;

	bool has_roughness = material.unknown_parameter.find("roughness") != material.unknown_parameter.end();
	bool has_metallic = material.unknown_parameter.find("metallic") != material.unknown_parameter.end();
	bool has_ior = material.unknown_parameter.find("ior") != material.unknown_parameter.end();
	bool has_anisotropy = material.unknown_parameter.find("anisotropy") != material.unknown_parameter.end();
	bool has_sheen = material.unknown_parameter.find("sheen") != material.unknown_parameter.end();
	float roughness = has_roughness ? std::stof(material.unknown_parameter["roughness"]) : .0f;
	float metallic = has_metallic ? std::stof(material.unknown_parameter["metallic"]) : .0f;
	float ior = has_ior ? std::stof(material.unknown_parameter["ior"]) : .0f;
	float anisotropy = has_anisotropy ? std::stof(material.unknown_parameter["anisotropy"]) : .0f;
	float sheen = has_sheen ? std::stof(material.unknown_parameter["sheen"]) : .0f;

	return sched->schedule_now_on_main_thread([=, vbo_data = std::move(vbo_data), vbo_indices = std::move(vbo_indices)](optional<task_scheduler*> sched) {
		auto mat = std::make_shared<Material>();
		if (diff_map != nullptr) mat->set_basecolor_map(diff_map);
		if (specular_map != nullptr) mat->set_cavity_map(specular_map);
		if (normalmap != nullptr) mat->set_normal_map(normalmap);
		if (opacity_map != nullptr) mat->set_mask_map(opacity_map);
		if (has_roughness) mat->set_roughness(roughness);
		if (has_metallic) mat->set_metallic(metallic);
		if (has_ior) mat->set_index_of_refraction(ior);
		if (has_anisotropy) mat->set_anisotropy(anisotropy);
		if (has_sheen) mat->set_sheen(sheen);

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
		std::replace(normalized_name.begin(), normalized_name.end(), '\\', '/');
		boost::filesystem::path full_path = dir / boost::filesystem::path(normalized_name).make_preferred();

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
		else
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

StE::task<bool> ModelFactory::load_model_task(const StEngineControl &context,
											  const boost::filesystem::path &file_path,
											  ObjectGroup *object_group,
											  Graphics::SceneProperties *scene_properties,
											  float normal_map_bias) {
	struct _model_loader_task_block {
		bool ret;
		std::unique_ptr<texture_map_type> textures;
	};

	return task<_model_loader_task_block>([=](optional<task_scheduler*> sched) {
		assert(sched);

		_model_loader_task_block block;
		block.textures = std::make_unique<texture_map_type>();

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
		}

		{
			std::vector<std::future<void>> futures;
			for (auto &shape : shapes)
				futures.push_back(process_model_mesh(sched, &scene_properties->materials_storage(), shape, object_group, materials, *block.textures));

			for (auto &f : futures)
				f.wait();
		}

		block.ret = true;
		return block;
	}).then_on_main_thread([](optional<task_scheduler*> sched, _model_loader_task_block &&block) -> bool {
		block.textures = nullptr;
		return block.ret;
	});
}
