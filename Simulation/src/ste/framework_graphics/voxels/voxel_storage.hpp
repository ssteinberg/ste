//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_pipeline.hpp>
#include <voxels_configuration.hpp>

#include <cmd_fill_buffer.hpp>

#include <array.hpp>
#include <texture.hpp>

#include <alias.hpp>

namespace ste {
namespace graphics {

class voxel_storage {
private:
	static constexpr auto voxel_buffer_line = 32768;
	static constexpr auto max_voxel_tree_lines = 6144;
	static constexpr auto voxel_list_size = 5 * 1024 * 1024;

	using voxel_buffer_word_t = gl::std430<std::uint32_t>;

	using voxel_data_t = gl::std430<glm::uvec4>;
	using voxel_assembly_list_element_t = gl::std430<voxel_data_t, glm::vec3, glm::uint>;
	using voxel_upsample_list_element_t = gl::std430<glm::vec3, glm::uint>;

private:
	alias<const ste_context> ctx;
	const voxels_configuration config;

	// Voxel structure
	gl::texture<gl::image_type::image_2d> voxels;
	// Allcoation counter
	gl::array<gl::std430<std::uint32_t>> voxels_counter;

	// Supporting structures
	gl::array<voxel_assembly_list_element_t> voxel_assembly_list;
	gl::array<gl::std430<std::uint32_t>> voxel_assembly_list_counter;

public:
	voxel_storage(const ste_context &ctx,
				  voxels_configuration config)
		: ctx(ctx),
		  config(config),
		  voxels(resource::surface_factory::image_empty_2d<gl::format::r32_uint>(ctx,
																				 gl::image_usage::storage | gl::image_usage::sampled,
																				 gl::image_layout::shader_read_only_optimal,
																				 "voxels buffer",
																				 glm::u32vec2{ voxel_buffer_line, max_voxel_tree_lines })),
		  voxels_counter(ctx,
						 1,
						 gl::buffer_usage::storage_buffer | gl::buffer_usage::transfer_dst,
						 "voxels counter buffer"),
		  voxel_assembly_list(ctx,
							  voxel_list_size,
							  gl::buffer_usage::storage_buffer,
							  "voxel assembly list buffer"),
		  voxel_assembly_list_counter(ctx,
									  1,
									  gl::buffer_usage::storage_buffer | gl::buffer_usage::transfer_dst,
									  "voxel list counter buffer")
	{}

	~voxel_storage() noexcept {}

	voxel_storage(voxel_storage &&) = default;

	/**
	 *	@brief	Binds voxel buffers and sets specialization constants
	 */
	void configure_voxel_pipeline(gl::device_pipeline &pipeline) const {
		// Configure parameters
		pipeline["voxel_P"] = config.P;
		pipeline["voxel_Pi"] = config.Pi;
		pipeline["voxel_leaf_level"] = config.leaf_level;
		pipeline["voxel_world"] = config.world;
	}

	/**
	 *	@brief	Resets the voxel buffers to initial state
	 */
	void clear(gl::command_recorder &recorder) const {
		// Reset
		recorder << gl::cmd_fill_buffer(gl::buffer_view(voxels_counter),
										static_cast<std::uint32_t>(config.voxel_tree_root_size()) >> 2);
		recorder << gl::cmd_fill_buffer(gl::buffer_view(voxel_assembly_list_counter),
										static_cast<std::uint32_t>(0));
	}

	auto &voxels_buffer_image() const { return voxels; }
	auto &voxels_counter_buffer() const { return voxels_counter; }

	auto &voxel_assembly_list_buffer() const { return voxel_assembly_list; }
	auto &voxel_assembly_list_counter_buffer() const { return voxel_assembly_list_counter; }
};

}
}
