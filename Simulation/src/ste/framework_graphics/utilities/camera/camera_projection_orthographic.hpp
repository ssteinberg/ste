//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <camera_projection_model.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ste {
namespace graphics {

/**
*	@brief	Orthographic projection model
*/
template <typename T>
class camera_projection_orthographic : public camera_projection_model<T> {
private:
	glm::tvec2<T> left_bottom;
	glm::tvec2<T> right_top;
	T near_clip_plane;
	T far_clip_plane;

	glm::tmat4x4<T> proj_mat;

public:
	/*
	*	@param	left_bottom		Coordinates of the left-bottom edge of the projection volume
	*	@param	right_top		Coordinates of the right-top edge of the projection volume
	*	@param	near_clip_plane	Distance to the near clip plane
	*	@param	far_clip_plane	Distance to the far clip plane
	*/
	camera_projection_orthographic(const glm::tvec2<T> &left_bottom,
								   const glm::tvec2<T> &right_top,
								   const T &near_clip_plane,
								   const T &far_clip_plane)
		: left_bottom(left_bottom), right_top(right_top), near_clip_plane(near_clip_plane), far_clip_plane(far_clip_plane),
		proj_mat(glm::orthoLH(left_bottom.x, right_top.x, left_bottom.y, right_top.y, near_clip_plane, far_clip_plane))
	{
		assert(right_top.x > left_bottom.x);
		assert(right_top.y > left_bottom.y);
		assert(near_clip_plane > 0);
		assert(far_clip_plane > 0);
		assert(far_clip_plane > near_clip_plane);
	}
	~camera_projection_orthographic() noexcept {}

	camera_projection_orthographic(camera_projection_orthographic&&) = default;
	camera_projection_orthographic &operator=(camera_projection_orthographic&&) = default;
	camera_projection_orthographic(const camera_projection_orthographic&) = default;
	camera_projection_orthographic &operator=(const camera_projection_orthographic&) = default;

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
		auto a = proj_mat[3][2] - proj_mat[3][3] * d;
		auto b = proj_mat[2][3] * d - proj_mat[2][2];
		return a / b;
	}

	/*
	*	@brief	Unprojects a screen position, given with depth value and normalized screen coordinates, into eye space.
	*/
	glm::tvec3<T> unproject_screen_position(const T &depth, const glm::tvec2<T> &norm_frag_coords) const final override {
		auto z = unproject_depth(depth);
		return unproject_screen_position_with_z(z, norm_frag_coords);
	}

	/*
	*	@brief	Unprojects a screen position, given with a eye space z value and normalized screen coordinates, into eye space.
	*/
	glm::tvec3<T> unproject_screen_position_with_z(const T &z, const glm::tvec2<T> &norm_frag_coords) const final override {
		auto t = norm_frag_coords * static_cast<T>(2);
		t -= glm::tvec2<T>(1);

		const auto &P = proj_mat;

		auto a = (P[1][1] * P[0][0]);
		auto b = P[2][3] * z + P[3][3];
		auto tx = t.x * b;
		auto ty = t.y * b;

		auto x = (P[1][0] * (ty - P[3][1]) / a + (tx - P[3][0]) / P[0][0]) / (static_cast<T>(1) - P[1][0] * P[0][1] / a);
		auto y = (ty - P[0][1] * x - P[3][1]) / P[1][1];

		return { x, y, z };
	}

	/*
	*	@brief	Creates a projection matrix
	*/
	glm::tmat4x4<T> projection_matrix() const final override {
		return proj_mat;
	}
};

}
}
