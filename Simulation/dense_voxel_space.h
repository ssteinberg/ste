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
#include "Sampler.h"

#include <memory>
#include <unordered_set>

namespace StE {
namespace Graphics {

class dense_voxel_space {
private:
	static constexpr gli::format space_format_radiance = gli::format::FORMAT_RGBA16_SFLOAT;
	static constexpr gli::format space_format_data = gli::format::FORMAT_RGBA16_SFLOAT;
	static constexpr int voxel_steps_multiplier = 8;

	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

	friend class dense_voxelizer;

private:
	const StEngineControl &ctx;
	LLR::texture_sparse_3d::size_type size, tile_size;
	std::uint32_t mipmaps{ 0 };
	glm::u32vec3 step_size{ 0 };
	std::uint32_t steps{ 0 };
	std::uint32_t tiles_per_step{ 0 };
	float voxel_size, space_size;

	std::unique_ptr<LLR::texture_sparse_3d> space_radiance;
	std::unique_ptr<LLR::texture_sparse_3d> space_data;

	LLR::SamplerMipmapped sampler;

	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

	LLR::FramebufferObject voxelizer_fbo;
	std::unique_ptr<LLR::RenderTarget> voxelizer_output;

protected:
	std::shared_ptr<LLR::GLSLProgram> voxelizer_program;
	std::shared_ptr<LLR::GLSLProgram> voxelizer_upsampler_program;

	mutable std::unordered_set<const LLR::GLSLProgram*> consumers;

	std::vector<LLR::image_handle> handles_radiance;
	std::vector<LLR::image_handle> handles_data;
	LLR::texture_handle radiance_texture_handle;
	LLR::texture_handle data_texture_handle;

protected:
	void create_dense_voxel_space(float voxel_size_factor);
	void clear_space() const;

	void update_shader_voxel_uniforms(const LLR::GLSLProgram &prg) const;
	void update_shader_voxel_world_translation(const glm::vec3 &c, const LLR::GLSLProgram &prg) const {
		auto vs = voxel_size * voxel_steps_multiplier;
		glm::vec3 translation = glm::round(c / vs) * vs;
		prg.set_uniform("voxels_world_translation", c - translation);
	}

public:
	dense_voxel_space(const StEngineControl &ctx, std::size_t max_size = 1024, float voxel_size_factor = 1.f);

	void add_consumer_program(const LLR::GLSLProgram *prg) const {
		consumers.insert(prg);
		update_shader_voxel_uniforms(*prg);
	}
	void remove_consumer_program(const LLR::GLSLProgram *prg) const {
		consumers.erase(prg);
	}

	std::unique_ptr<dense_voxelizer> voxelizer(Scene &scene) const {
		return std::make_unique<dense_voxelizer>(ctx, this, scene);
	}

	void set_model_matrix(const glm::mat4 &m, const glm::vec3 &translate) const {
		auto vs = voxel_size * voxel_steps_multiplier;
		glm::vec3 translation = -glm::round(translate / vs) * vs;
		voxelizer_program->set_uniform("translation", translation);
		voxelizer_program->set_uniform("trans_inverse_view_matrix", glm::transpose(glm::inverse(m)));

		for (auto *p : consumers)
			update_shader_voxel_world_translation(translate, *p);
	}

	auto get_steps() const { return steps; }
	auto get_step_texels() const { return step_size; }
	auto get_voxel_texel_size() const { return voxel_size; }
	auto get_voxel_texture_levels() const { return mipmaps; }
};

}
}
