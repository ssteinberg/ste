// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "task_future.hpp"

#include "Object.hpp"
#include "ObjectGroup.hpp"
#include "SceneProperties.hpp"
#include "Texture2D.hpp"

#include "StEngineControl.hpp"
#include "material_storage.hpp"

#include "boost_filesystem.hpp"

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

class ModelFactory {
private:
	using texture_map_type = std::unordered_map<std::string, std::shared_ptr<Core::Texture2D>>;
	using shapes_type = std::vector<tinyobj::shape_t>;
	using materials_type = std::vector<tinyobj::material_t>;

private:
	~ModelFactory() {}

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
													 Graphics::material_storage *,
													 const tinyobj::shape_t &,
													 Graphics::ObjectGroup *,
													 materials_type &,
													 texture_map_type &,
									 				 std::vector<std::unique_ptr<Graphics::Material>> &loaded_materials,
									 				 std::vector<std::shared_ptr<Graphics::Object>> *loaded_objects);

public:
	static StE::task_future<void> load_model_async(const StEngineControl &context,
												   const boost::filesystem::path &file_path,
												   Graphics::ObjectGroup *object_group,
												   Graphics::SceneProperties *scene_properties,
												   float normal_map_bias,
												   std::vector<std::unique_ptr<Graphics::Material>> &loaded_materials,
												   std::vector<std::shared_ptr<Graphics::Object>> *loaded_objects = nullptr);
};

}
}
