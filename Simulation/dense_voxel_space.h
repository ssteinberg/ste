// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "dense_voxelizer.h"
#include "Scene.h"

#include "GLSLProgram.h"
#include "GLSLProgramLoader.h"

#include "FramebufferObject.h"
#include "RenderTarget.h"
#include "texture_sparse.h"

#include <memory>

namespace StE {
namespace Graphics {

class dense_voxel_space {
private:
	static constexpr gli::format space_format_radiance = gli::format::FORMAT_RGBA16_SFLOAT;
	static constexpr gli::format space_format_data = gli::format::FORMAT_RGBA16_SFLOAT;
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

	friend class dense_voxelizer;

private:
	const StEngineControl &ctx;
	LLR::texture_sparse_3d::size_type size, tile_size;
	std::uint32_t mipmaps{ 0 };
	std::uint32_t step_size{ 0 };
	std::uint32_t steps{ 0 };
	float voxel_size;

	std::unique_ptr<LLR::texture_sparse_3d> space_radiance;
	std::unique_ptr<LLR::texture_sparse_3d> space_data;

	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

	LLR::FramebufferObject voxelizer_fbo;
	std::unique_ptr<LLR::RenderTarget> voxelizer_output;

protected:
	std::shared_ptr<LLR::GLSLProgram> voxelizer_program;

protected:
	void create_dense_voxel_space(float voxel_size_factor);
	void clear_space() const;

public:
	dense_voxel_space(const StEngineControl &ctx, std::size_t max_size = 1024, float voxel_size_factor = 1.f) : ctx(ctx) {
		auto ts = LLR::texture_sparse_3d::page_sizes(space_format_radiance)[0];

		size = static_cast<decltype(size)>(glm::min<int>(LLR::texture_sparse_3d::max_size(), max_size));
		tile_size = static_cast<decltype(tile_size)>(glm::max(ts.x, glm::max(ts.y, ts.z)));

		voxelizer_program = Resource::GLSLProgramLoader::load_program_task(ctx, { "voxelizer.vert", "voxelizer.frag", "voxelizer.geom" })();

		voxelizer_output = std::make_unique<LLR::RenderTarget>(gli::format::FORMAT_R8_UNORM, size.xy);
		voxelizer_fbo[0] = *voxelizer_output;

		create_dense_voxel_space(voxel_size_factor);
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &, float, float, float) {
			create_dense_voxel_space(voxel_size_factor);
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	std::unique_ptr<dense_voxelizer> voxelizer(Scene &scene) const {
		return std::make_unique<dense_voxelizer>(ctx, this, scene);
	}

	void update_shader_voxel_uniforms(LLR::GLSLProgram &prg) const {
		prg.set_uniform("voxels_step_texels", step_size);
		prg.set_uniform("voxels_voxel_texel_size", voxel_size);
		prg.set_uniform("voxels_texture_levels", mipmaps);

		std::vector<std::uint64_t> handles_radiance;
		std::vector<std::uint64_t> handles_data;
		for (unsigned i = 0; i < mipmaps; ++i) {
			handles_radiance.push_back((*space_radiance)[i].get_image_handle());
			handles_data.push_back((*space_data)[i].get_image_handle());
		}
		prg.set_uniform("voxel_radiance_levels_handles", handles_radiance);
		prg.set_uniform("voxel_data_levels_handles", handles_data);
		prg.set_uniform("voxel_space_radiance", space_radiance->get_texture_handle());
		prg.set_uniform("voxel_space_data", space_data->get_texture_handle());
	}

	void set_model_matrix(const glm::mat4 &m, const glm::vec3 &translate) const {
		auto vs = voxel_size * 4;
		glm::vec3 translation = -glm::round(translate / vs) * vs;
		voxelizer_program->set_uniform("translation", translation);
		voxelizer_program->set_uniform("trans_inverse_view_matrix", glm::transpose(glm::inverse(m)));
	}

	auto get_steps() const { return steps; }
	auto get_step_texels() const { return step_size; }
	auto get_voxel_texel_size() const { return voxel_size; }
	auto get_voxel_texture_levels() const { return mipmaps; }
};

}
}
