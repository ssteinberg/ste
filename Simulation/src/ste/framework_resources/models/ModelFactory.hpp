// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "task.hpp"
#include "ObjectGroup.hpp"
#include "SceneProperties.hpp"
#include "Texture2D.hpp"

#include "StEngineControl.hpp"
#include "material_storage.hpp"

#define BOOST_FILESYSTEM_NO_DEPRECATED 
#include <boost/filesystem.hpp>

#include <memory>
#include <unordered_map>
#include <string>

#include <vector>
#include <future>

#include <tiny_obj_loader.h>

namespace StE {
namespace Resource {

class ModelFactory {
private:
	using texture_map_type = std::unordered_map<std::string, std::shared_ptr<Core::Texture2D>>;
	using brdf_map_type = std::unordered_map<std::string, std::shared_ptr<Graphics::BRDF>>;
	using shapes_type = std::vector<tinyobj::shape_t>;
	using materials_type = std::vector<tinyobj::material_t>;

private:
	~ModelFactory() {}

	static StE::task<void> load_texture(const std::string &name, 
										bool srgb, 
										texture_map_type *texmap, 
										bool bumpmap, 
										const boost::filesystem::path &dir, 
										float normal_map_bias);
	static std::vector<std::future<void>> load_textures(task_scheduler* sched, 
														shapes_type &shapes, 
														materials_type &materials, 
														texture_map_type &tex_map, 
														const boost::filesystem::path &dir, 
														float normal_map_bias);
	static std::vector<std::future<void>> load_brdfs(const StEngineControl *context, 
													 shapes_type &shapes, 
													 materials_type &materials, 
													 brdf_map_type &brdf_map, 
													 const boost::filesystem::path &dir);
	static std::future<void> process_model_mesh(optional<task_scheduler*> sched,
												Graphics::material_storage *,
												const tinyobj::shape_t &,
												Graphics::ObjectGroup *,
												materials_type &,
												texture_map_type &,
												brdf_map_type &);

public:
	static task<bool> load_model_task(const StEngineControl &context, 
									  const boost::filesystem::path &file_path, 
									  Graphics::ObjectGroup *object_group, 
									  Graphics::SceneProperties *scene_properties, 
									  float normal_map_bias);
};

}
}
