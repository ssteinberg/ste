//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_pipeline.hpp>
#include <voxels_configuration.hpp>

#include <cmd_fill_buffer.hpp>

#include <array.hpp>
#include <alias.hpp>

namespace ste {
namespace graphics {

class voxel_storage {
private:
	static constexpr auto max_voxel_tree_size = 128 * 1024 * 1024;

	using voxel_buffer_word_t = gl::std430<std::uint32_t>;

	using voxel_list_element_t = gl::std430<float, float, float, std::uint32_t, std::uint32_t>;

private:
	alias<const ste_context> ctx;
	const voxels_configuration config;

	gl::array<voxel_buffer_word_t> voxels;
	gl::array<gl::std430<std::uint32_t>> voxels_counter;

	gl::array<voxel_list_element_t> voxel_list;
	gl::array<gl::std430<std::uint32_t>> voxel_list_counter;

public:
	voxel_storage(const ste_context &ctx,
				  voxels_configuration config)
		: ctx(ctx),
		  config(config),
		  voxels(ctx,
				 max_voxel_tree_size,
				 gl::buffer_usage::storage_buffer | gl::buffer_usage::transfer_src,
				 "voxels buffer"),
		  voxels_counter(ctx,
						 1,
						 gl::buffer_usage::storage_buffer | gl::buffer_usage::transfer_src,
						 "voxels counter buffer"),
		  voxel_list(ctx, 16 * 1024 * 1024,
					 gl::buffer_usage::storage_buffer | gl::buffer_usage::transfer_src,
					 "voxel list buffer"),
		  voxel_list_counter(ctx,
							 1,
							 gl::buffer_usage::storage_buffer | gl::buffer_usage::transfer_src,
							 "voxel list counter buffer")
	{}
	~voxel_storage() noexcept {}

	voxel_storage(voxel_storage &&) = default;

	/**
	 *	@brief	Binds voxel buffers and sets specialization constants
	 */
	void configure_voxel_pipeline(gl::device_pipeline &pipeline) const {
		pipeline["voxel_buffer_binding"] = gl::bind(voxels);

		// Configure parameters
		pipeline["voxel_P"] = config.P;
		pipeline["voxel_Pi"] = config.Pi;
		pipeline["voxel_leaf_level"] = config.leaf_level;
		pipeline["voxel_world"] = config.world;
	}

	/**
	 *	@brief	Resets the voxel buffers to initial state
	 */
	void clear(gl::command_recorder &recorder) {
		// Reset
		recorder << gl::cmd_fill_buffer(gl::buffer_view(voxels, 0, static_cast<std::size_t>(config.voxel_tree_root_binary_map_size()) >> 2),
										static_cast<std::uint32_t>(0));
		recorder << gl::cmd_fill_buffer(gl::buffer_view(voxels_counter),
										static_cast<std::uint32_t>(config.voxel_tree_root_size()) >> 2);
		recorder << gl::cmd_fill_buffer(gl::buffer_view(voxel_list_counter),
										static_cast<std::uint32_t>(0));
	}

	auto &voxels_buffer() const { return voxels; }
	auto &voxels_counter_buffer() const { return voxels_counter; }

	auto &voxel_list_buffer() const { return voxel_list; }
	auto &voxel_list_counter_buffer() const { return voxel_list_counter; }
};

}
}
