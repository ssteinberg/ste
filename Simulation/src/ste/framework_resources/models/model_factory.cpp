
#include <stdafx.hpp>
#include <model_factory.hpp>

#include <ste_device_queue.hpp>

#include <material.hpp>
#include <mesh.hpp>

#include <material_storage.hpp>
#include <material_layer_storage.hpp>

#include <surface.hpp>
#include <surface_type_traits.hpp>
#include <surface_convert.hpp>
#include <surface_factory.hpp>

#include <normal_map_from_height_map.hpp>

#include <lib/string.hpp>
#include <algorithm>

using namespace ste;
using namespace ste::resource;

tinyobj::material_t model_factory::empty_mat;

namespace ste::resource::_detail {

template <gl::format format, typename Surface, typename Map>
void store_texture(const ste_context &ctx,
				   const std::string &name,
				   Surface &&surface,
				   graphics::scene_properties *scene_properties,
				   Map *texmap) {
	static_assert(resource::is_surface_v<Surface> || resource::is_opaque_surface_v<Surface>);

	auto t = surface_factory::image_from_surface_2d<format>(ctx,
															surface_convert::convert_2d<format>(std::forward<Surface>(surface)),
															gl::image_usage::sampled,
															gl::image_layout::shader_read_only_optimal);
	(*texmap)[name] = scene_properties->material_textures_storage().allocate_texture(std::move(t));
}

}

void model_factory::add_object_to_object_group(const ste_context &ctx, 
											   graphics::object_group *object_group,
											   const lib::shared_ptr<graphics::object> &obj) {
	// Enqueue object add command on a transfer queue
	ctx.device().submit_onetime_batch(gl::ste_queue_selector<gl::ste_queue_selector_policy_flexible>(gl::ste_queue_type::data_transfer_sparse_queue),
									  [&](gl::command_recorder &recorder) {
		object_group->add_object(recorder,
								 obj);
	});
}

ste::task_future<void> model_factory::process_model_mesh(const ste_context &ctx,
														 graphics::scene_properties *scene_properties,
                                                         const vertex_attrib_type &attrib,
														 const tinyobj::shape_t &shape,
														 graphics::object_group *object_group,
														 materials_type &materials,
														 texture_map_type &textures,
														 lib::vector<lib::unique_ptr<graphics::material>> &loaded_materials,
														 lib::vector<lib::unique_ptr<graphics::material_layer>> &loaded_material_layers,
														 lib::vector<lib::shared_ptr<graphics::object>> *loaded_objects) {
	lib::vector<model_factory_vertex> vertices;
	lib::vector<std::uint32_t> vbo_indices;

	lib::unordered_map<model_factory_vertex, std::uint32_t> vertex_to_index_map;

    // Reserve memory
	vertices.reserve(shape.mesh.indices.size());
    vbo_indices.reserve(shape.mesh.indices.size());

	// Sanity check
	static constexpr auto face_vertices = 3;
    for (auto &face : shape.mesh.num_face_vertices) {
        assert(face == face_vertices && "Not a triangle!");
    }
    assert(shape.mesh.indices.size() % face_vertices == 0);

	// Load faces
	for (std::size_t i=0; i < shape.mesh.indices.size(); i+=face_vertices) {
		// Load face vertices
		for (int j=0; j < face_vertices; ++j) {
			auto index = shape.mesh.indices[i + j];
			model_factory_vertex v;

			// Position
			assert(index.vertex_index >= 0);
			v.p = glm::vec3{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			// Texture coordinates
			if (index.texcoord_index != -1) {
				v.uv = glm::vec2{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.f - attrib.texcoords[2 * index.texcoord_index + 1]        // Convert origin from bottom-left to top-left
				};
			}

			// Normal
			if (index.normal_index != -1) {
				const glm::vec3 n = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
				v.n = glm::normalize(n);
			}
			// Tangent
			v.t = glm::vec3(.0f);

			// Create vertex index
			auto vertex_index = static_cast<std::uint32_t>(vertices.size());

			// If we have an identical vertex, use it.
			const auto ret = vertex_to_index_map.try_emplace(v, vertex_index);
			if (ret.second) {
				// Store vertex
				vertices.push_back(v);
			}
			else {
				// Reuse index
				vertex_index = ret.first->second;
			}

			// Store index
			vbo_indices.push_back(vertex_index);
		}

		// Align tangents to u's (texture coordinate) direction
		auto &v0 = vertices[*(vbo_indices.end() - 1)];
		auto &v1 = vertices[*(vbo_indices.end() - 2)];
		auto &v2 = vertices[*(vbo_indices.end() - 3)];

		const auto x0 = v1.p.x - v0.p.x;
		const auto x1 = v2.p.x - v0.p.x;
		const auto y0 = v1.p.y - v0.p.y;
		const auto y1 = v2.p.y - v0.p.y;
		const auto z0 = v1.p.z - v0.p.z;
		const auto z1 = v2.p.z - v0.p.z;

		const auto s0 = v1.uv.x - v0.uv.x;
		const auto s1 = v2.uv.x - v0.uv.x;
		const auto t0 = v1.uv.y - v0.uv.y;
		const auto t1 = v2.uv.y - v0.uv.y;

		const auto r = 1.f / (s0 * t1 - s1 * t0);
		const glm::vec3 sdir = { (t1 * x0 - t0 * x1) * r, (t1 * y0 - t0 * y1) * r, (t1 * z0 - t0 * z1) * r };

		v0.t += sdir;
		v1.t += sdir;
		v2.t += sdir;
    }

	// Create vertex objects
	lib::vector<graphics::object_vertex_data> vbo_data;
	vbo_data.reserve(vertices.size());
	for (auto &v : vertices) {
		graphics::object_vertex_data vo;
		vo.p() = v.p;
		vo.uv() = v.uv;

		// Gram-Schmidt orthogonalization process
		const glm::vec3 n = v.n;
		glm::vec3 t = v.t;
		if (glm::dot(t,t) < 1e-2) {
			// Handle singularity
			t = glm::cross(n, glm::vec3{ 1,0,0 });
			if (glm::dot(t,t) < 1e-2)
				t = glm::cross(n, glm::vec3{ 0,1,0 });
		}
		t = glm::normalize(t - n * glm::dot(n, t));
		const glm::vec3 b = glm::cross(t, n);
		// Create tangent frame
		vo.tangent_frame_from_tbn(t, b, n);

		vbo_data.push_back(vo);
	}

	// Create async task that generates object, material and material layer
	const int mat_idx = shape.mesh.material_ids.size() > 0 ? shape.mesh.material_ids[0] : -1;
	auto &material = mat_idx >= 0 ? materials[mat_idx] : empty_mat;

	return ctx.engine().task_scheduler().schedule_now([=, &ctx, vbo_data = std::move(vbo_data), vbo_indices = std::move(vbo_indices), &loaded_materials, &loaded_material_layers, &textures, &material]() {
		// Read loaded textures, if any.
		texture_t diff_map;
		texture_t opacity_map;
		texture_t specular_map;
		texture_t normalmap;
		texture_t roughness_map;
		texture_t metallic_map;
//		texture_t anisotropy_map;
		texture_t thickness_map;
		if (mat_idx >= 0) {
			diff_map = textures[material.diffuse_texname];
			opacity_map = textures[material.alpha_texname];
			specular_map = textures[material.specular_texname];
			normalmap = textures[material.bump_texname];
			roughness_map = textures[material.unknown_parameter[roughness_map_key]];
			metallic_map = textures[material.unknown_parameter[metallic_map_key]];
//			anisotropy_map = textures[material.unknown_parameter[anisotropy_map_key]];
			thickness_map = textures[material.unknown_parameter[thickness_map_key]];
		}

		// Allocate material and head layer
		auto layer = scene_properties->material_layers_storage().allocate_layer();
		auto mat = scene_properties->materials_storage().allocate_material(ctx,
																		   layer.get());

		// Set material textures
		if (diff_map)		mat->set_texture(diff_map);
		if (specular_map)	mat->set_cavity_map(specular_map);
		if (normalmap)		mat->set_normal_map(normalmap);
		if (opacity_map)	mat->set_mask_map(opacity_map);

		if (roughness_map)	layer->set_roughness(roughness_map);
		if (metallic_map)	layer->set_metallic(metallic_map);
//		if (anisotropy_map)	layer->set_anisotropy(anisotropy_map);
		if (thickness_map)	layer->set_layer_thickness(thickness_map);

		// Create mesh from vertices and indices
		lib::unique_ptr<graphics::mesh<graphics::mesh_subdivion_mode::Triangles>> m = lib::allocate_unique<graphics::mesh<graphics::mesh_subdivion_mode::Triangles>>();
		m->set_indices(std::move(vbo_indices));
		m->set_vertices(std::move(vbo_data));

		// Create object from mesh and material
		lib::shared_ptr<graphics::object> obj = lib::allocate_shared<graphics::object>(std::move(m));
		obj->set_material(mat.get());

		// Add object to scene's object group
		add_object_to_object_group(ctx,
								   object_group,
								   obj);

		// Bookeeping
		loaded_materials.push_back(std::move(mat));
		loaded_material_layers.push_back(std::move(layer));
		if (loaded_objects) 
			loaded_objects->push_back(obj);
	});
}

ste::task_future<void> model_factory::load_texture(const ste_context &ctx,
												   const std::string &name,
												   bool srgb,
												   bool is_displacement_map,
												   graphics::scene_properties *scene_properties,
												   texture_map_type *texmap,
												   const std::experimental::filesystem::path &dir,
												   float normal_map_bias) {
	return ctx.engine().task_scheduler().schedule_now([=, &ctx]() {
		// Correct path
		std::string normalized_name = name;
		std::replace(normalized_name.begin(), normalized_name.end(), '\\', '/');
		std::experimental::filesystem::path full_path = dir / std::experimental::filesystem::path(normalized_name).make_preferred();

		// Load surface from file
		optional<opaque_surface<2>> surface;
		try {
			surface = surface_io::load_surface_2d(full_path, srgb);
		}
		catch (...) {}

		if (!surface) {
			ste_log_warn() << "Couldn't load texture " << full_path.string() << std::endl;
			return;
		}

		auto surface_format_traits = gl::format_id(surface.get().surface_format());

		// Enforce correct normal maps and displacement maps. 
		bool displacement = is_displacement_map;
		if (displacement && surface_format_traits.elements != 1) {
			if (surface_format_traits.elements == 3) {
				ste_log_warn() << "Texture \"" << name << "\" looks like a normal map and not a displacement map as specified by the model. Assuming a normal map." << std::endl;
				displacement = false;
			}
			else {
				ste_log_warn() << "Texture \"" << name << "\" doesn't look like a displacement map. Bailing out..." << std::endl;
				return;
			}
		}

		// Create texture.
		texture_t texture;
		if (displacement) {
			// We use normal maps, if a displacement map is provided, use it to generate a normal map.
			auto normal_map = graphics::normal_map_from_height_map<gl::format::r8g8b8a8_unorm>()(surface_convert::convert_2d<gl::format::r8_unorm>(std::move(surface).get()), normal_map_bias);
			_detail::store_texture<gl::format::r8g8b8a8_unorm>(ctx, name, std::move(normal_map), scene_properties, texmap);
		}
		else if (surface_format_traits.elements == 1 && surface_format_traits.is_srgb)
			_detail::store_texture<gl::format::r8_srgb>(ctx, name, std::move(surface).get(), scene_properties, texmap);
		else if (surface_format_traits.elements == 1 && !surface_format_traits.is_srgb)
			_detail::store_texture<gl::format::r8_unorm>(ctx, name, std::move(surface).get(), scene_properties, texmap);
		else if (surface_format_traits.elements >= 3 && surface_format_traits.is_srgb)
			_detail::store_texture<gl::format::r8g8b8a8_srgb>(ctx, name, std::move(surface).get(), scene_properties, texmap);
		else if (surface_format_traits.elements >= 3 && !surface_format_traits.is_srgb)
			_detail::store_texture<gl::format::r8g8b8a8_unorm>(ctx, name, std::move(surface).get(), scene_properties, texmap);
		else {
			assert(false);
		}
	});
}

lib::vector<ste::task_future<void>> model_factory::load_textures(const ste_context &ctx,
																 shapes_type &shapes,
																 materials_type &materials,
																 graphics::scene_properties *scene_properties,
																 texture_map_type &tex_map,
																 const std::experimental::filesystem::path &dir,
																 float normal_map_bias) {
	// Some models will have an empty string for a texture, avoid loading and using such textures by presetting an empty texture.
	tex_map.emplace(std::make_pair(lib::string(""), texture_t()));

	// For each shape, load all textures.
	lib::vector<ste::task_future<void>> futures;
	for (auto &shape : shapes) {
		const int mat_idx = shape.mesh.material_ids.size() > 0 ? shape.mesh.material_ids[0] : -1;
		if (mat_idx < 0)
			continue;

		for (auto &str : { materials[mat_idx].diffuse_texname,
						   materials[mat_idx].bump_texname,
						   materials[mat_idx].displacement_texname,
						   materials[mat_idx].alpha_texname,
						   materials[mat_idx].specular_texname,
						   materials[mat_idx].unknown_parameter[roughness_map_key],
						   materials[mat_idx].unknown_parameter[metallic_map_key],
						   materials[mat_idx].unknown_parameter[anisotropy_map_key],
						   materials[mat_idx].unknown_parameter[thickness_map_key] })
			if (str.length() && tex_map.find(str) == tex_map.end()) {
				const bool srgb = str == materials[mat_idx].diffuse_texname;
				const bool displacement = str == materials[mat_idx].displacement_texname;

				// Create texture loading task.
				tex_map.emplace(std::make_pair(str, texture_t()));
				futures.push_back(load_texture(ctx,
											   str,
											   srgb,
											   displacement,
											   scene_properties,
											   &tex_map,
											   dir,
											   normal_map_bias));
			}


		// If we have a normal map, ignore displacement map.
		if (materials[mat_idx].bump_texname.length())
			materials[mat_idx].displacement_texname = "";
		else
			materials[mat_idx].bump_texname = materials[mat_idx].displacement_texname;
	}

	return futures;
}

ste::task_future<void> model_factory::load_model_async(const ste_context &ctx,
													   const std::experimental::filesystem::path &file_path,
													   graphics::object_group *object_group,
													   graphics::scene_properties *scene_properties,
													   float normal_map_bias,
													   lib::vector<lib::unique_ptr<graphics::material>> &loaded_materials,
													   lib::vector<lib::unique_ptr<graphics::material_layer>> &loaded_material_layers,
													   lib::vector<lib::shared_ptr<graphics::object>> *loaded_objects) {
	// Create async task that handles model loading
	return ctx.engine().task_scheduler().schedule_now([=, &loaded_materials, &loaded_material_layers, &ctx]() {
		// Texture map
		lib::unique_ptr<texture_map_type> textures = lib::allocate_unique<texture_map_type>();

		auto path_string = file_path.string();
		ste_log() << "Loading OBJ model " << path_string << std::endl;

		lib::string dir = { path_string.begin(), std::find_if(path_string.rbegin(), path_string.rend(), [](char c) { return c == '/' || c == '\\'; }).base() };

		shapes_type shapes;
		materials_type materials;
        vertex_attrib_type attribs;

		// Load model
		std::string err;
		if (!tinyobj::LoadObj(&attribs,
                              &shapes,
							  &materials,
							  &err, 
							  path_string.c_str(), 
							  dir.c_str())) {
			ste_log_error() << "Couldn't load model " << path_string << ": " << err;
			throw resource_io_error("Could not load/parse model");
		}

		// Load all textures, in parallel
		{
			for (auto &f : load_textures(ctx, 
										 shapes, 
										 materials,
										 scene_properties,
										 *textures, 
										 dir, 
										 normal_map_bias))
				f.get();
		}

		// Once all textures are loaded, process all shapes, create materials and objects and add them to scene.
		{
			lib::vector<ste::task_future<void>> futures;
			for (auto &shape : shapes)
				futures.push_back(process_model_mesh(ctx,
													 scene_properties,
                                                     attribs,
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
	});
}
