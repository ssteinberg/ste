// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <task_future.hpp>

#include <object.hpp>
#include <object_group.hpp>
#include <scene_properties.hpp>
#include <material.hpp>
#include <material_layer.hpp>
#include <material_texture.hpp>

#include <filesystem>

#include <lib/unique_ptr.hpp>
#include <lib/unordered_map.hpp>

#include <lib/vector.hpp>
#include <future>

#include <tiny_obj_loader.h>

namespace ste {
namespace resource {

class model_factory {
private:
	using texture_t = graphics::material_texture;
	using texture_map_type = lib::unordered_map<std::string, texture_t>;
	using shapes_type = std::vector<tinyobj::shape_t>;
	using materials_type = std::vector<tinyobj::material_t>;

	constexpr static char roughness_map_key[] = "map_roughness";
	constexpr static char metallic_map_key[] = "map_metallic";
	constexpr static char anisotropy_map_key[] = "map_anisotropy";
	constexpr static char thickness_map_key[] = "map_thickness";

private:
	static tinyobj::material_t empty_mat;

private:
	~model_factory() noexcept {}

	static void add_object_to_object_group(const ste_context &ctx,
										   graphics::object_group *object_group,
										   const lib::shared_ptr<graphics::object> &obj);
	static ste::task_future<void> load_texture(const ste_context &,
											   const std::string &,
											   bool srgb,
											   bool is_displacement_map,
											   graphics::scene_properties *,
											   texture_map_type *texmap,
											   const std::experimental::filesystem::path &dir,
											   float normal_map_bias);
	static lib::vector<ste::task_future<void>> load_textures(const ste_context &ctx,
															 shapes_type &shapes,
															 materials_type &materials,
															 graphics::scene_properties *,
															 texture_map_type &tex_map,
															 const std::experimental::filesystem::path &dir,
															 float normal_map_bias);
	static ste::task_future<void> process_model_mesh(const ste_context &ctx,
													 graphics::scene_properties *,
													 const tinyobj::shape_t &,
													 graphics::object_group *,
													 materials_type &,
													 texture_map_type &,
									 				 lib::vector<lib::unique_ptr<graphics::material>> &loaded_materials,
													 lib::vector<lib::unique_ptr<graphics::material_layer>> &loaded_material_layers,
									 				 lib::vector<lib::shared_ptr<graphics::object>> *loaded_objects);

public:
	static ste::task_future<void> load_model_async(const ste_context &ctx,
												   const std::experimental::filesystem::path &file_path,
												   graphics::object_group *object_group,
												   graphics::scene_properties *scene_properties,
												   float normal_map_bias,
												   lib::vector<lib::unique_ptr<graphics::material>> &loaded_materials,
												   lib::vector<lib::unique_ptr<graphics::material_layer>> &loaded_material_layers,
												   lib::vector<lib::shared_ptr<graphics::object>> *loaded_objects = nullptr);
};

}
}
