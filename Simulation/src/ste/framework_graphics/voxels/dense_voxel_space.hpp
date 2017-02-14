// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>

#include <dense_voxelizer.hpp>
#include <scene.hpp>

#include <glsl_program.hpp>
#include <glsl_program_factory.hpp>

#include <framebuffer_object.hpp>
#include <render_target.hpp>
#include <texture_sparse.hpp>
#include <Sampler.hpp>

#include <memory>
#include <unordered_set>

namespace StE {
namespace Graphics {

class dense_voxel_space {
private:
	static constexpr gli::format space_format_radiance = gli::format::FORMAT_RGBA16_SFLOAT_PACK16;
	static constexpr gli::format space_format_data = gli::format::FORMAT_RGBA16_SFLOAT_PACK16;
	static constexpr int voxel_steps_multiplier = 8;

	using ProjectionSignalConnectionType = ste_engine_control::projection_change_signal_type::connection_type;

	friend class dense_voxelizer;

private:
	const ste_engine_control &ctx;
	Core::texture_sparse_3d::size_type size, tile_size;
	int mipmaps{ 0 };
	glm::ivec3 step_size{ 0 };
	int steps{ 0 };
	int tiles_per_step{ 0 };
	float voxel_size, space_size;

	std::unique_ptr<Core::texture_sparse_3d> space_radiance;
	std::unique_ptr<Core::texture_sparse_3d> space_data;

	Core::sampler_mipmapped sampler;

	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

	Core::framebuffer_object voxelizer_fbo;
	std::unique_ptr<Core::render_target> voxelizer_output;

protected:
	Resource::resource_instance<Resource::glsl_program> voxelizer_program;
	Resource::resource_instance<Resource::glsl_program> voxelizer_upsampler_program;

	mutable std::unordered_set<const Core::glsl_program_object*> consumers;

	std::vector<Core::image_handle> handles_radiance;
	std::vector<Core::image_handle> handles_data;
	Core::texture_handle radiance_texture_handle;
	Core::texture_handle data_texture_handle;

protected:
	void create_dense_voxel_space(float voxel_size_factor);
	void clear_space() const;

	void update_shader_voxel_uniforms(const Core::glsl_program_object &prg) const;
	void update_shader_voxel_world_translation(const glm::vec3 &c, const Core::glsl_program_object &prg) const {
		auto vs = voxel_size * voxel_steps_multiplier;
		glm::vec3 translation = glm::round(c / vs) * vs;
		prg.set_uniform("voxels_world_translation", c - translation);
	}

public:
	dense_voxel_space(const ste_engine_control &ctx, std::size_t max_size = 1024, float voxel_size_factor = 1.f);

	void add_consumer_program(const Core::glsl_program_object *prg) const {
		consumers.insert(prg);
		update_shader_voxel_uniforms(*prg);
	}
	void remove_consumer_program(const Core::glsl_program_object *prg) const {
		consumers.erase(prg);
	}

	std::unique_ptr<dense_voxelizer> voxelizer(scene &scene) const {
		return std::make_unique<dense_voxelizer>(ctx, this, scene);
	}

	void set_model_matrix(const glm::mat4 &m, const glm::vec3 &translate) const {
		auto vs = voxel_size * voxel_steps_multiplier;
		glm::vec3 translation = -glm::round(translate / vs) * vs;
		voxelizer_program.get().set_uniform("translation", translation);

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
