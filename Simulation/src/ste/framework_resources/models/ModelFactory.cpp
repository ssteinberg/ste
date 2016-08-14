
#include "stdafx.hpp"
#include "ModelFactory.hpp"

#include "material.hpp"
#include "mesh.hpp"

#include "material_storage.hpp"
#include "material_layer_storage.hpp"

#include "SurfaceFactory.hpp"

#include "normal_map_from_height_map.hpp"

#include <string>
#include <algorithm>

using namespace StE::Resource;
using namespace StE::Graphics;
using StE::Core::Texture2D;

tinyobj::material_t ModelFactory::empty_mat;

StE::task_future<void> ModelFactory::process_model_mesh(task_scheduler* sched,
														Graphics::SceneProperties *scene_properties,
														const tinyobj::shape_t &shape,
														Graphics::ObjectGroup *object_group,
														materials_type &materials,
														texture_map_type &textures,
									 					std::vector<std::unique_ptr<Graphics::material>> &loaded_materials,
														std::vector<std::unique_ptr<Graphics::material_layer>> &loaded_material_layers,
									 					std::vector<std::shared_ptr<Graphics::Object>> *loaded_objects) {
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

	int mat_idx = shape.mesh.material_ids.size() > 0 ? shape.mesh.material_ids[0] : -1;

	auto &material = mat_idx >= 0 ? materials[mat_idx] : empty_mat;

	auto roughness_it = material.unknown_parameter.find("roughness");
	auto metallic_it = material.unknown_parameter.find("metallic");
	auto ior_it = material.unknown_parameter.find("ior");
	auto anisotropy_it = material.unknown_parameter.find("anisotropy");
	auto sheen_it = material.unknown_parameter.find("sheen");

	return sched->schedule_now_on_main_thread([=, vbo_data = std::move(vbo_data), vbo_indices = std::move(vbo_indices), &loaded_materials, &loaded_material_layers, &textures, &material]() {
		std::shared_ptr<Core::Texture2D> diff_map = mat_idx >= 0 ? textures[material.diffuse_texname] : nullptr;
		std::shared_ptr<Core::Texture2D> opacity_map = mat_idx >= 0 ? textures[material.alpha_texname] : nullptr;
		std::shared_ptr<Core::Texture2D> specular_map = mat_idx >= 0 ? textures[material.specular_texname] : nullptr;
		std::shared_ptr<Core::Texture2D> normalmap = mat_idx >= 0 ? textures[material.bump_texname] : nullptr;
		
		auto layer = scene_properties->material_layers_storage().allocate_layer();
		auto mat = scene_properties->materials_storage().allocate_material(layer.get());

		if (diff_map != nullptr) layer->set_basecolor_map(diff_map);
		if (specular_map != nullptr) mat->set_cavity_map(specular_map);
		if (normalmap != nullptr) mat->set_normal_map(normalmap);
		if (opacity_map != nullptr) mat->set_mask_map(opacity_map);

		if (roughness_it != material.unknown_parameter.end()) layer->set_roughness(std::stof(roughness_it->second));
		if (metallic_it != material.unknown_parameter.end()) layer->set_metallic(std::stof(metallic_it->second));
		if (ior_it != material.unknown_parameter.end()) layer->set_index_of_refraction(std::stof(ior_it->second));
		if (anisotropy_it != material.unknown_parameter.end()) layer->set_anisotropy(std::stof(anisotropy_it->second));
		if (sheen_it != material.unknown_parameter.end()) layer->set_sheen(std::stof(sheen_it->second));

		std::unique_ptr<Graphics::mesh<Graphics::mesh_subdivion_mode::Triangles>> m = std::make_unique<Graphics::mesh<Graphics::mesh_subdivion_mode::Triangles>>();
		m->set_indices(std::move(vbo_indices));
		m->set_vertices(std::move(vbo_data));

		std::shared_ptr<Graphics::Object> obj = std::make_shared<Graphics::Object>(std::move(m));
		obj->set_material(mat.get());

		object_group->add_object(obj);

		loaded_materials.push_back(std::move(mat));
		loaded_material_layers.push_back(std::move(layer));
		if (loaded_objects) loaded_objects->push_back(obj);
	});
}

StE::task_future<void> ModelFactory::load_texture(task_scheduler* sched,
												  const std::string &name,
												  bool srgb,
												  bool displacement,
												  texture_map_type *texmap,
												  const boost::filesystem::path &dir,
												  float normal_map_bias) {
	return sched->schedule_now([=]() {
		std::string normalized_name = name;
		std::replace(normalized_name.begin(), normalized_name.end(), '\\', '/');
		boost::filesystem::path full_path = dir / boost::filesystem::path(normalized_name).make_preferred();

		gli::texture2d tex = SurfaceFactory::load_surface_2d(full_path, srgb);
		if (tex.empty())
			ste_log_warn() << "Couldn't load texture " << full_path.string() << std::endl;

		return std::make_unique<gli::texture2d>(std::move(tex));
	}).then_on_main_thread([=](std::unique_ptr<gli::texture2d> &&tex) {
		bool is_displacement_map = displacement;

		if (tex->empty()) {
			ste_log_warn() << "Couldn't load model texture: " << name << std::endl;
			return;
		}

		if (is_displacement_map && tex->format() != gli::FORMAT_R8_UNORM_PACK8) {
			if (tex->format() == gli::FORMAT_RGB8_UNORM_PACK8 || tex->format() == gli::FORMAT_RGBA8_UNORM_PACK8) {
				ste_log_warn() << "Texture \"" << name << "\" looks like a normal map and not a displacement map as specified by the model. Assuming a normal map." << std::endl;
				is_displacement_map = false;
			}
			else {
				ste_log_warn() << "Texture \"" << name << "\" doesn't look like a displacement map. Bailing out..." << std::endl;
				return;
			}
		}

		if (is_displacement_map) {
			auto nm = normal_map_from_height_map<gli::FORMAT_R8_UNORM_PACK8, false>()(*tex, normal_map_bias);
			(*texmap)[name] = std::make_shared<Core::Texture2D>(std::move(nm), true);
		}
		else
			(*texmap)[name] = std::make_shared<Core::Texture2D>(*tex, true);
	});
}

std::vector<StE::task_future<void>> ModelFactory::load_textures(task_scheduler* sched,
														 		shapes_type &shapes,
														 		materials_type &materials,
														 		texture_map_type &tex_map,
														 		const boost::filesystem::path &dir,
														 		float normal_map_bias) {
	tex_map.emplace(std::make_pair(std::string(""), std::shared_ptr<Core::Texture2D>(nullptr)));

	std::vector<StE::task_future<void>> futures;
	for (auto &shape : shapes) {
		int mat_idx = shape.mesh.material_ids.size() > 0 ? shape.mesh.material_ids[0] : -1;
		if (mat_idx < 0)
			continue;

		for (auto &str : { materials[mat_idx].diffuse_texname,
						   materials[mat_idx].bump_texname,
						   materials[mat_idx].displacement_texname,
						   materials[mat_idx].alpha_texname,
						   materials[mat_idx].specular_texname })
			if (str.length() && tex_map.find(str) == tex_map.end()) {
				bool srgb = str == materials[mat_idx].diffuse_texname;
				bool displacement = str == materials[mat_idx].displacement_texname;

				tex_map.emplace(std::make_pair(str, std::shared_ptr<Core::Texture2D>(nullptr)));
				futures.push_back(load_texture(sched,
											   str,
											   srgb,
											   displacement,
											   &tex_map,
											   dir,
											   normal_map_bias));
			}


		if (materials[mat_idx].bump_texname.length())
			materials[mat_idx].displacement_texname = "";
		else
			materials[mat_idx].bump_texname = materials[mat_idx].displacement_texname;
	}

	return futures;
}

StE::task_future<void> ModelFactory::load_model_async(const StEngineControl &ctx,
													  const boost::filesystem::path &file_path,
													  ObjectGroup *object_group,
													  Graphics::SceneProperties *scene_properties,
													  float normal_map_bias,
													  std::vector<std::unique_ptr<Graphics::material>> &loaded_materials,
													  std::vector<std::unique_ptr<Graphics::material_layer>> &loaded_material_layers,
													  std::vector<std::shared_ptr<Graphics::Object>> *loaded_objects) {
	return ctx.scheduler().schedule_now([=, &loaded_materials, &loaded_material_layers, &ctx]() {
		std::unique_ptr<texture_map_type> textures = std::make_unique<texture_map_type>();

		auto path_string = file_path.string();
		ste_log() << "Loading OBJ model " << path_string << std::endl;

		std::string dir = { path_string.begin(), std::find_if(path_string.rbegin(), path_string.rend(), [](char c) { return c == '/' || c == '\\'; }).base() };

		shapes_type shapes;
		materials_type materials;

		std::string err;
		if (!tinyobj::LoadObj(shapes, materials, err, path_string.c_str(), dir.c_str(), true)) {
			ste_log_error() << "Couldn't load model " << path_string << ": " << err;
			throw resource_io_error("Could not load/parse model");
		}

		{
			for (auto &f : load_textures(&ctx.scheduler(), shapes, materials, *textures, dir, normal_map_bias))
				f.get();
		}

		{
			std::vector<StE::task_future<void>> futures;
			for (auto &shape : shapes)
				futures.push_back(process_model_mesh(&ctx.scheduler(),
													 scene_properties,
													 shape,
													 object_group,
													 materials,
													 *textures,
													 loaded_materials,
													 loaded_material_layers,
													 loaded_objects));

			for (auto &f : futures)
				f.get();
		}

		return textures;
	}).then_on_main_thread([](std::unique_ptr<texture_map_type> &&textures) {
		textures = nullptr;
	});
}
