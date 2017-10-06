//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <camera_projection_model.hpp>

#include <reversed_perspective.hpp>

namespace ste {
namespace graphics {

/**
*	@brief	Reversed-infinite-perspective projection model
*/
template <typename T>
class camera_projection_reversed_infinite_perspective : public camera_projection_model<T> {
private:
	T fovy;
	T aspect;
	metre near_clip_plane;

	T tan_half_fovy{};
	glm::tvec4<T> proj_xywz;

public:
	/*
	*	@param	fovy		Field-of-view in y axis, in radians.
	*	@param	aspect		Aspect ratio of x to y
	*	@param	near_clip_plane	Distance to the near clip plane
	*/
	camera_projection_reversed_infinite_perspective(T fovy, 
													T aspect, 
													metre near_clip_plane)
		: fovy(fovy), aspect(aspect), near_clip_plane(near_clip_plane)
	{
		assert(aspect > 0);
		assert(fovy > 0);
		assert(near_clip_plane > 0_m);

		tan_half_fovy = glm::tan(fovy / static_cast<T>(2));
		const auto one_over_tan_half_fovy = static_cast<T>(1) / tan_half_fovy;
		const auto one_over_aspect_tan_half_fovy = static_cast<T>(1) / (aspect * tan_half_fovy);

		proj_xywz = { one_over_aspect_tan_half_fovy, -one_over_tan_half_fovy, near_clip_plane, static_cast<T>(-1) };
	}
	~camera_projection_reversed_infinite_perspective() noexcept {}

	camera_projection_reversed_infinite_perspective(camera_projection_reversed_infinite_perspective&&) = default;
	camera_projection_reversed_infinite_perspective &operator=(camera_projection_reversed_infinite_perspective&&) = default;
	camera_projection_reversed_infinite_perspective(const camera_projection_reversed_infinite_perspective&) = default;
	camera_projection_reversed_infinite_perspective &operator=(const camera_projection_reversed_infinite_perspective&) = default;

	/*
	*	@brief	Projects a eye space position to homogeneous clip coordinates.
	*/
	glm::tvec4<T> project(metre_vec4 v) const final override {
		return glm::tvec4<T>{ static_cast<T>(v.x), static_cast<T>(v.y), static_cast<T>(v.w), static_cast<T>(v.z) } * proj_xywz;
	}

	/*
	*	@brief	Projects a z value to a depth value.
	*/
	metre project_depth(metre z) const final override {
		return -near_clip_plane / static_cast<T>(z);
	}

	/*
	*	@brief	Unprojects a depth value to the z value
	*/
	metre unproject_depth(metre d) const final override {
		return -near_clip_plane / static_cast<T>(d);
	}

	/*
	*	@brief	Unprojects a screen position, given with depth value and normalized screen coordinates, into eye space.
	*/
	metre_vec3 unproject_screen_position(metre depth, const glm::tvec2<T> &norm_frag_coords) const final override {
		auto z = unproject_depth(depth);
		return unproject_screen_position_with_z(z, norm_frag_coords);
	}

	/*
	*	@brief	Unprojects a screen position, given with a eye space z value and normalized screen coordinates, into eye space.
	*/
	metre_vec3 unproject_screen_position_with_z(metre z, const glm::tvec2<T> &norm_frag_coords) const final override {
		auto t = norm_frag_coords * static_cast<T>(2);
		t -= glm::tvec2<T>(1);

		glm::tvec2<T> xy = (t * static_cast<T>(z)) / glm::tvec2<T>(proj_xywz.x, proj_xywz.y);

		return { metre(-xy.x), metre(-xy.y), z };
	}

	/*
	*	@brief	Creates a projection matrix
	*/
	glm::tmat4x4<T> projection_matrix() const final override {
		return reversed_infinite_perspective<T>(fovy,
												aspect,
												static_cast<float>(near_clip_plane));
	}

	auto get_fovy() const { return fovy; }
	auto get_aspect() const { return aspect; }
	auto get_near_clip_plane() const { return near_clip_plane; }

	/*
	 *	@brief	Returns a pre-computed tan(fovy/2)
	 */
	auto tan_fovy_over_two() const { return tan_half_fovy; }

	/*
	*	@brief	Returns a pre-computed xywz vector used for fast reversed-infinite-persepective projection.
	*			xywz = ( 1 / (aspect * tan_fovy_over_two), -1 / tan_fovy_over_two, near, -1 )
	*			
	*			Projecting a vector v=(x,y,z,1) is simply: proj(v) = v.xywz * xywz
	*/
	auto projection_xywz() const { return proj_xywz; }
};

}
}
