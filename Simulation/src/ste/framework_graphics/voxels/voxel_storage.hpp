//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_pipeline.hpp>

#include <voxels_configuration.hpp>

#include <vector.hpp>
#include <array.hpp>

#include <alias.hpp>

namespace ste {
namespace graphics {

class voxel_storage {
private:
	alias<const ste_context> ctx;
	const voxels_configuration config;

	gl::vector<gl::std430<std::uint32_t>> voxels;
	gl::array<gl::std430<std::uint32_t>> voxels_counter;

public:
	voxel_storage(const ste_context &ctx,
				  voxels_configuration config)
		: ctx(ctx),
		  config(config),
		  voxels(ctx,
				 gl::buffer_usage::storage_buffer,
				 "Voxels buffer"),
		  voxels_counter(ctx,
						 1,
						 gl::buffer_usage::storage_buffer,
						 "voxels counter buffer") {}

	~voxel_storage() noexcept {}

	voxel_storage(voxel_storage &&) = default;

	/*
	 *	@brief	Binds voxel buffers and sets specialization constants
	 */
	void configure_voxel_pipeline(gl::device_pipeline &pipeline) const {
		pipeline["voxel_buffer_binding"] = gl::bind(voxels);
		pipeline["voxel_counter_binding"] = gl::bind(voxels_counter);

		// Configure parameters
		pipeline["voxel_P"] = config.P;
		pipeline["voxel_Pi"] = config.Pi;
		pipeline["voxel_leaf_level"] = config.leaf_level;
		pipeline["voxel_world"] = config.world;
	}

	auto &voxels_buffer() const { return voxels; }
	auto &voxels_counter_buffer() const { return voxels_counter; }
};

}
}
