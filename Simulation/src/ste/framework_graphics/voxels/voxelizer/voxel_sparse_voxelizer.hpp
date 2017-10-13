// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment.hpp>

#include <voxelizer_generate_voxel_list.hpp>

#include <scene.hpp>

#include <voxel_storage.hpp>

namespace ste {
namespace graphics {

class voxel_sparse_voxelizer : public gl::fragment {
	using Base = gl::fragment;

private:
	voxelizer_generate_voxel_list generate_voxel_list;

public:
	voxel_sparse_voxelizer(const gl::rendering_system &rs,
						   voxel_storage *voxels,
						   const scene *s)
		: Base(rs.get_creating_context()),
		generate_voxel_list(rs,
							voxels,
							s)
	{}
	~voxel_sparse_voxelizer() noexcept {}

	voxel_sparse_voxelizer(voxel_sparse_voxelizer &&) = default;

	void record(gl::command_recorder &recorder) override final {
		// Create voxel list
		recorder << generate_voxel_list;
	}
};

}
}
