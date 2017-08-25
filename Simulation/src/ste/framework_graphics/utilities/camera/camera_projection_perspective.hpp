//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <camera_projection_model.hpp>
#include <glm/gtc/matrix_transform.inl>

namespace ste {
namespace graphics {

/**
*	@brief	Perspective projection model
*/
template <typename T>
class camera_projection_perspective : public camera_projection_model<T> {
private:
	T fovy;
	T aspect;
	T near_clip_plane;
	T far_clip_plane;

	glm::tmat4x4<T> proj_mat;

public:
	/*
	 *	@param	fovy		Field-of-view in y axis, in radians.
	 *	@param	aspect		Aspect ratio of x to y
	 *	@param	near_clip_plane	Distance to the near clip plane
	 *	@param	far_clip_plane	Distance to the far clip plane
	 */
	camera_projection_perspective(const T &fovy, 
								  const T &aspect, 
								  const T &near_clip_plane, 
								  const T &far_clip_plane)
		: fovy(fovy), aspect(aspect), near_clip_plane(near_clip_plane), far_clip_plane(far_clip_plane),
		proj_mat(glm::perspectiveRH(fovy, aspect, near_clip_plane, far_clip_plane))
	{
		assert(aspect > 0);
		assert(fovy > 0);
		assert(near_clip_plane > 0);
		assert(far_clip_plane > 0);
		assert(far_clip_plane > near_clip_plane);
	}
	~camera_projection_perspective() noexcept {}

	camera_projection_perspective(camera_projection_perspective&&) = default;
	camera_projection_perspective &operator=(camera_projection_perspective&&) = default;
	camera_projection_perspective(const camera_projection_perspective&) = default;
	camera_projection_perspective &operator=(const camera_projection_perspective&) = default;

	/*
	*	@brief	Projects a eye space position to homogeneous clip coordinates.
	*/
	glm::tvec4<T> project(const glm::tvec4<T> &v) const final override {
		return proj_mat * v;
	}

	/*
	*	@brief	Projects a z value to a depth value.
	*/
	T project_depth(const T &z) const final override {
		auto u = project(glm::tvec4<T>(0, 0, z, 1));
		return u.z / u.w;
	}

	/*
	*	@brief	Unprojects a depth value to the z value
	*/
	T unproject_depth(const T &d) const final override {
		// TODO
		return .0f;
	}

	/*
	*	@brief	Unprojects a screen position, given with depth value and normalized screen coordinates, into eye space.
	*/
	glm::tvec3<T> unproject_screen_position(const T &depth, const glm::tvec2<T> &norm_frag_coords) const final override {
		auto t = norm_frag_coords * static_cast<T>(2);
		t -= glm::tvec2<T>(1);

		float z = unproject_depth(depth);
		// TODO
	}

	/*
	*	@brief	Unprojects a screen position, given with a eye space z value and normalized screen coordinates, into eye space.
	*/
	glm::tvec3<T> unproject_screen_position_with_z(const T &z, const glm::tvec2<T> &norm_frag_coords) const final override {
		auto t = norm_frag_coords * static_cast<T>(2);
		t -= glm::tvec2<T>(1);

		// TODO
	}

	/*
	*	@brief	Creates a projection matrix
	*/
	glm::tmat4x4<T> projection_matrix() const final override {
		return proj_mat;
	}

	auto get_fovy() const { return fovy; }
	auto get_aspect() const { return aspect; }
	auto get_near_clip_plane() const { return near_clip_plane; }
	auto get_far_clip_plane() const { return far_clip_plane; }
};

}
}
