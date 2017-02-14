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

#include <boost_filesystem.hpp>

#include <memory>
#include <unordered_map>
#include <string>

#include <vector>
#include <future>

#pragma warning push
#pragma warning(disable:663)
#include <tiny_obj_loader.h>
#pragma warning pop

namespace StE {
namespace Resource {

class model_factory {
private:
	using texture_map_type = std::unordered_map<std::string, std::shared_ptr<Core::texture_2d>>;
	using shapes_type = std::vector<tinyobj::shape_t>;
	using materials_type = std::vector<tinyobj::material_t>;

	constexpr static char roughness_map_key[] = "map_roughness";
	constexpr static char metallic_map_key[] = "map_metallic";
	constexpr static char anisotropy_map_key[] = "map_anisotropy";
	constexpr static char thickness_map_key[] = "map_thickness";

private:
	static tinyobj::material_t empty_mat;

private:
	~model_factory() {}

	static StE::task_future<void> load_texture(task_scheduler *sched,
											   const std::string &name,
											   bool srgb,
											   bool displacement,
											   texture_map_type *texmap,
											   const boost::filesystem::path &dir,
											   float normal_map_bias);
	static std::vector<StE::task_future<void>> load_textures(task_scheduler* sched,
															 shapes_type &shapes,
															 materials_type &materials,
															 texture_map_type &tex_map,
															 const boost::filesystem::path &dir,
															 float normal_map_bias);
	static StE::task_future<void> process_model_mesh(task_scheduler* sched,
													 Graphics::scene_properties *,
													 const tinyobj::shape_t &,
													 Graphics::object_group *,
													 materials_type &,
													 texture_map_type &,
									 				 std::vector<std::unique_ptr<Graphics::material>> &loaded_materials,
													 std::vector<std::unique_ptr<Graphics::material_layer>> &loaded_material_layers,
									 				 std::vector<std::shared_ptr<Graphics::object>> *loaded_objects);

public:
	static StE::task_future<void> load_model_async(const ste_engine_control &context,
												   const boost::filesystem::path &file_path,
												   Graphics::object_group *object_group,
												   Graphics::scene_properties *scene_properties,
												   float normal_map_bias,
												   std::vector<std::unique_ptr<Graphics::material>> &loaded_materials,
												   std::vector<std::unique_ptr<Graphics::material_layer>> &loaded_material_layers,
												   std::vector<std::shared_ptr<Graphics::object>> *loaded_objects = nullptr);
};

}
}
