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
	virtual glm::tvec4<T> project(metre_vec4 v) const = 0;

	/*
	*	@brief	Projects a z value to a depth value. 
	*/
	virtual metre project_depth(metre z) const = 0;

	/*
	*	@brief	Unprojects a depth value to the z value
	*/
	virtual metre unproject_depth(metre d) const = 0;

	/*
	*	@brief	Unprojects a screen position, given with depth value and normalized screen coordinates, into eye space.
	*/
	virtual metre_vec3 unproject_screen_position(metre depth, const glm::tvec2<T> &norm_frag_coords) const = 0;

	/*
	*	@brief	Unprojects a screen position, given with a eye space z value and normalized screen coordinates, into eye space.
	*/
	virtual metre_vec3 unproject_screen_position_with_z(metre z, const glm::tvec2<T> &norm_frag_coords) const = 0;

	/*
	 *	@brief	Creates a projection matrix
	 */
	virtual glm::tmat4x4<T> projection_matrix() const = 0;
};

}
}
