//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace graphics {

/**
*	@brief	Describes a camera's projection model
*/
template <typename T>
class camera_projection_model {
public:
	virtual ~camera_projection_model() noexcept {}

	/*
	*	@brief	Projects a eye space position to homogeneous clip coordinates.
	*/
	virtual glm::tvec4<T> project(const glm::tvec4<T> &v) const = 0;

	/*
	*	@brief	Projects a z value to a depth value. 
	*/
	virtual T project_depth(const T &z) const = 0;

	/*
	*	@brief	Unprojects a depth value to the z value
	*/
	virtual T unproject_depth(const T &d) const = 0;

	/*
	*	@brief	Unprojects a screen position, given with depth value and normalized screen coordinates, into eye space.
	*/
	virtual glm::tvec3<T> unproject_screen_position(const T &depth, const glm::tvec2<T> &norm_frag_coords) const = 0;

	/*
	*	@brief	Unprojects a screen position, given with a eye space z value and normalized screen coordinates, into eye space.
	*/
	virtual glm::tvec3<T> unproject_screen_position_with_z(const T &z, const glm::tvec2<T> &norm_frag_coords) const = 0;

	/*
	 *	@brief	Creates a projection matrix
	 */
	virtual glm::tmat4x4<T> projection_matrix() const = 0;
};

}
}
