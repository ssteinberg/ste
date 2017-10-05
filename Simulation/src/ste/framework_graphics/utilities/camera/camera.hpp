//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <dual_quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/dual_quaternion.hpp>

#include <signal.hpp>

namespace ste {
namespace graphics {

/**
 *	@brief	Simple camera object
 */
template <typename T, template<typename> class projection_model>
class camera {
	// Internal compiler error (VS2017).
//	static_assert(is_floating_point_v<T>, "T expected to be a floating point type");

public:
	using vec3 = glm::tvec3<T>;
	using projection_model_t = projection_model<T>;

	using projection_change_signal = signal<const camera<T, projection_model>*, const projection_model_t&>;

private:
	metre_vec3 camera_position;
	metre_vec3 camera_look_at;
	vec3 camera_direction;
	vec3 camera_up;

	T camera_speed;

	T camera_pitch_limit;

	projection_model_t proj;
	mutable projection_change_signal proj_change_signal;

public:
	camera() = default;

	camera(const projection_model_t &proj)
		: camera_position({ 0_m, 0_m, 0_m }),
		  camera_look_at({ 0_m, 0_m, -1_m }),
		  camera_direction({ 0,0,-1 }),
		  camera_up({ 0,1,0 }),
		  camera_speed(1),
		  camera_pitch_limit(glm::half_pi<T>() - static_cast<T>(.05)),
		  proj(std::move(proj)) {}

	camera(const metre_vec3 &position,
		   const vec3 &direction,
		   const vec3 &up)
		: camera(proj) {
		camera_position = position;
		camera_up = up;
		lookat(camera_position + metre_vec3(direction));
	}

	camera(const metre_vec3 &position,
		   const vec3 &direction,
		   const vec3 &up,
		   const projection_model_t &proj)
		: camera(position,
				 direction,
				 up) {
		update_projection_model(proj);
	}

	~camera() noexcept {}

	camera(camera &&) = default;
	camera(const camera &) = default;
	camera &operator=(camera &&) = default;
	camera &operator=(const camera &) = default;

	/*
	*	@brief	Sets a new instance of the projection model associated with the camera
	*/
	void set_projection_model(const projection_model_t &proj) {
		this->proj = proj;
		proj_change_signal.emit(this, this->proj);
	}

	/*
	*	@brief	Returns a reference to the projection model associated with the camera
	*/
	auto &get_projection_model() const { return proj; }

	/**
	 *	@brief	Sets camera speed
	 */
	void set_speed(T s) { camera_speed = s; }

	/**
	*	@brief	Sets the pitch limit, in radians, should be positive and <pi/2
	*/
	void set_pitch_limit(T rad) {
		assert(rad < glm::half_pi<T>());
		assert(rad > static_cast<T>(0));

		camera_pitch_limit = rad;
	}

	/**
	*	@brief	Takes a step forward, in the current viewing direction of the camera.
	*/
	void step_forward(T scale = static_cast<T>(1)) {
		auto delta = camera_direction * camera_speed * scale;
		camera_position += metre_vec3(delta);
		camera_look_at += metre_vec3(delta);
	}

	/**
	*	@brief	Takes a step backward, opposite to the current viewing direction of the camera.
	*/
	void step_backward(T scale = static_cast<T>(1)) {
		auto delta = -camera_direction * camera_speed * scale;
		camera_position += metre_vec3(delta);
		camera_look_at += metre_vec3(delta);
	}

	/**
	*	@brief	Takes a step rightwards, in tangent to the current viewing direction of the camera.
	*/
	void step_right(T scale = static_cast<T>(1)) {
		auto delta = glm::cross(camera_direction, camera_up) * camera_speed * scale;
		camera_position += metre_vec3(delta);
		camera_look_at += metre_vec3(delta);
	}

	/**
	*	@brief	Takes a step leftwards, in tangent to the current viewing direction of the camera.
	*/
	void step_left(T scale = static_cast<T>(1)) {
		auto delta = -glm::cross(camera_direction, camera_up) * camera_speed * scale;
		camera_position += metre_vec3(delta);
		camera_look_at += metre_vec3(delta);
	}

	/**
	*	@brief	Takes a step upwards, in tangent to the current viewing direction of the camera.
	*/
	void step_up(T scale = static_cast<T>(1)) {
		auto delta = camera_up * camera_speed * scale;
		camera_position += metre_vec3(delta);
		camera_look_at += metre_vec3(delta);
	}

	/**
	*	@brief	Takes a step downwards, in tangent to the current viewing direction of the camera.
	*/
	void step_down(T scale = static_cast<T>(1)) {
		auto delta = -camera_up * camera_speed * scale;
		camera_position += metre_vec3(delta);
		camera_look_at += metre_vec3(delta);
	}

	/**
	*	@brief	Transforms the camera to new position, mainting previous viewing direction.
	*/
	void set_position(metre_vec3 pos) {
		camera_position = pos;
		camera_direction = glm::normalize(camera_look_at.v() - camera_position.v());
		camera_look_at = camera_position + metre_vec3(camera_direction);
	}

	/**
	*	@brief	Rotates the camera to look at a point. Camera position remains unchanged.
	*/
	void lookat(const metre_vec3 &pos) {
		camera_direction = glm::normalize((pos - camera_position).v());
		camera_look_at = camera_position + metre_vec3(camera_direction);
	}

	/**
	*	@brief	Rotates the camera by pitch and yaw angles, in radians. Camera position remains unchanged.
	*/
	void pitch_and_yaw(T pitch, T yaw) {
		const glm::vec3 tangent = glm::cross(camera_direction, camera_up);

		// Calculate current pitch
		const T camera_pitch = glm::asin(camera_direction.y);
		// Limit to preset limits
		pitch = glm::clamp(pitch, -(camera_pitch + camera_pitch_limit), camera_pitch_limit - camera_pitch);

		// Build
		glm::quat pitch_quat = glm::normalize(glm::angleAxis(pitch, tangent));
		glm::quat heading_quat = glm::normalize(glm::angleAxis(-yaw, /*camera_up*/glm::vec3(0, 1, 0)));
		//auto q = glm::normalize(glm::cross(pitch_quat, heading_quat));

		camera_direction = pitch_quat * camera_direction * heading_quat;
		camera_up = pitch_quat * camera_up * heading_quat;
		camera_look_at = camera_position + metre_vec3(camera_direction);
	}

	const auto &get_position() const { return camera_position; }

	/**
	*	@brief	Returns a 4x4 transform matrix that transforms a non-homogeneous coordinate to the camera's space.
	*/
	auto view_matrix() const { return glm::lookAt(camera_position.v(), camera_look_at.v(), camera_up); }

	/**
	*	@brief	Returns a dual quaternion that transforms a non-homogeneous coordinate to the camera's space.
	*/
	auto view_transform_dquat() const {
		auto vmnt = view_matrix();
		const auto r = glm::mat3(vmnt);
		const auto t = -camera_position.v();

		return dualquat_translate_rotate(r, t);
	}

	auto &get_projection_change_signal() const { return proj_change_signal; }
};

}
}
