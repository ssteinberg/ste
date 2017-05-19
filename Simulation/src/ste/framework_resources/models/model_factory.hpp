// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <task_future.hpp>

#include <object.hpp>
#include <object_group.hpp>
#include <scene_properties.hpp>
#include <texture_2d.hpp>
#include <material.hpp>
#include <material_layer.hpp>

#include <ste_engine_control.hpp>

#include <filesystem>

#include <lib/unique_ptr.hpp>
#include <lib/unordered_map.hpp>
#include <lib/string.hpp>

#include <lib/vector.hpp>
#include <future>

#pragma warning push
#pragma warning(disable:663)
#include <tiny_obj_loader.h>
#pragma warning pop

namespace ste {
namespace resource {

class model_factory {
private:
	using texture_map_type = lib::unordered_map<lib::string, lib::shared_ptr<Core::texture_2d>>;
	using shapes_type = lib::vector<tinyobj::shape_t>;
	using materials_type = lib::vector<tinyobj::material_t>;

	constexpr static char roughness_map_key[] = "map_roughness";
	constexpr static char metallic_map_key[] = "map_metallic";
	constexpr static char anisotropy_map_key[] = "map_anisotropy";
	constexpr static char thickness_map_key[] = "map_thickness";

private:
	static tinyobj::material_t empty_mat;

private:
	~model_factory() {}

	static ste::task_future<void> load_texture(task_scheduler *sched,
											   const lib::string &name,
											   bool srgb,
											   bool displacement,
											   texture_map_type *texmap,
											   const std::experimental::filesystem::path &dir,
											   float normal_map_bias);
	static lib::vector<ste::task_future<void>> load_textures(task_scheduler* sched,
															 shapes_type &shapes,
															 materials_type &materials,
															 texture_map_type &tex_map,
															 const std::experimental::filesystem::path &dir,
															 float normal_map_bias);
	static ste::task_future<void> process_model_mesh(task_scheduler* sched,
													 graphics::scene_properties *,
													 const tinyobj::shape_t &,
													 graphics::object_group *,
													 materials_type &,
													 texture_map_type &,
									 				 lib::vector<lib::unique_ptr<graphics::material>> &loaded_materials,
													 lib::vector<lib::unique_ptr<graphics::material_layer>> &loaded_material_layers,
									 				 lib::vector<lib::shared_ptr<graphics::object>> *loaded_objects);

public:
	static ste::task_future<void> load_model_async(const ste_engine_control &context,
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
