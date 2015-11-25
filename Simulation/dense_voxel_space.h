// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "dense_voxelizer.h"
#include "texture_sparse.h"
#include "Scene.h"

#include <memory>

namespace StE {
namespace Graphics {

class dense_voxel_space {
private:
	static constexpr gli::format space_format = gli::format::FORMAT_RGBA32_SFLOAT;
	static constexpr float voxel_size_factor = 5.f;
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

	friend class dense_voxelizer;

private:
	const StEngineControl &ctx;
	LLR::texture_sparse_3d::size_type size, tile_size;
	std::uint32_t mipmaps{ 0 };
	std::uint32_t step_size{ 0 };
	std::uint32_t steps{ 0 };
	float voxel_size;

	std::unique_ptr<LLR::texture_sparse_3d> space;

	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

protected:
	std::shared_ptr<LLR::GLSLProgram> voxelizer_program;

protected:
	void create_dense_voxel_space();

public:
	dense_voxel_space(const StEngineControl &ctx) : ctx(ctx) {
		auto ts = LLR::texture_sparse_3d::page_sizes(space_format)[0];

		size = static_cast<decltype(size)>(glm::min(LLR::texture_sparse_3d::max_size(), 1024));
		tile_size = static_cast<decltype(tile_size)>(glm::max(ts.x, glm::max(ts.y, ts.z)));

		voxelizer_program = Resource::GLSLProgramLoader::load_program_task(ctx, { "voxelizer.vert", "voxelizer.frag", "voxelizer.geom" })();

		create_dense_voxel_space();

		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &, float, float, float) {
			create_dense_voxel_space();
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	std::unique_ptr<dense_voxelizer> voxelizer(Scene &scene) const {
		return std::make_unique<dense_voxelizer>(ctx, this, scene);
	}

	void update_shader_voxel_uniforms(LLR::GLSLProgram &prg) const {
		prg.set_uniform("voxels_steps", steps);
		prg.set_uniform("voxels_step_texels", step_size);
		prg.set_uniform("voxels_voxel_texel_size", voxel_size);
		prg.set_uniform("voxels_texture_levels", mipmaps);

		std::vector<decltype((*space)[0].get_image_handle())> handles;
		for (unsigned i = 0; i < mipmaps; ++i) {
			(*space)[i].make_resident();
			handles.push_back((*space)[i].get_image_handle());
		}
		prg.set_uniform("voxel_levels", handles);
	}

	void set_world_center(const glm::vec3 &c) const {
		glm::vec3 translation = -glm::round(c / voxel_size) * voxel_size;
		voxelizer_program->set_uniform("translation", translation);
	}

	auto get_steps() const { return steps; }
	auto get_step_texels() const { return step_size; }
	auto get_voxel_texel_size() const { return voxel_size; }
	auto get_voxel_texture_levels() const { return mipmaps; }
};

}
}
