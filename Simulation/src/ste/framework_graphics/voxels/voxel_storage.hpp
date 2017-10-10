//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_pipeline.hpp>

#include <voxels_configuration.hpp>

#include <cmd_fill_buffer.hpp>

#include <stable_vector.hpp>
#include <array.hpp>

#include <alias.hpp>

namespace ste {
namespace graphics {

class voxel_storage {
private:
	static constexpr auto max_voxel_tree_size = 512 * 1024 * 1024;

	using voxel_buffer_word_t = gl::std430<std::uint32_t>;

private:
	alias<const ste_context> ctx;
	const voxels_configuration config;

	gl::stable_vector<voxel_buffer_word_t, max_voxel_tree_size> voxels;
	gl::array<gl::std430<std::uint32_t>> voxels_counter;

public:
	voxel_storage(const ste_context &ctx,
				  voxels_configuration config)
		: ctx(ctx),
		  config(config),
		  voxels(ctx,
				 gl::buffer_usage::storage_buffer | gl::buffer_usage::transfer_src,
				 "voxels buffer"),
		  voxels_counter(ctx,
						 1,
						 gl::buffer_usage::storage_buffer | gl::buffer_usage::transfer_src,
						 "voxels counter buffer") {}

	~voxel_storage() noexcept {}

	voxel_storage(voxel_storage &&) = default;

	/**
	 *	@brief	Binds voxel buffers and sets specialization constants
	 */
	void configure_voxel_pipeline(gl::device_pipeline &pipeline) const {
		pipeline["voxel_buffer_binding"] = gl::bind(voxels);
		pipeline["voxel_counter_binding"] = gl::bind(voxels_counter);

		// Configure parameters
		pipeline["voxel_P"] = config.P;
		pipeline["voxel_Pi"] = config.Pi;
//		pipeline["voxel_leaf_level"] = config.leaf_level;
		pipeline["voxel_world"] = config.world;
	}

	/**
	 *	@brief	Resets the voxel buffers to initial state
	 */
	void clear(gl::command_recorder &recorder) {
		const auto temp_size = 512 * 1024 * 1024;

		// Sparse resize
		recorder << voxels.resize_cmd(ctx, temp_size);

		// Reset
		recorder << gl::cmd_fill_buffer(gl::buffer_view(voxels, 0, voxels.size()),
										static_cast<std::uint32_t>(0));
		recorder << gl::cmd_fill_buffer(gl::buffer_view(voxels_counter),
										static_cast<std::uint32_t>(config.voxel_tree_root_size()) >> 2);
	}

	/**
	 *	@brief	Sparse resizes the voxel buffer
	 */
	auto resize_cmd(std::size_t size) {
		return voxels.resize_cmd(ctx.get(), size);
	}

	auto &voxels_buffer() const { return voxels; }
	auto &voxels_counter_buffer() const { return voxels_counter; }
};

}
}
