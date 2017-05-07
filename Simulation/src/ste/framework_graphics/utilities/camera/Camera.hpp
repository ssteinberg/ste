// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>
#include <dual_quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/dual_quaternion.hpp>

namespace ste {
namespace graphics {

/**
 *	@brief	Simple camera object
 */
template <typename T>
class camera {
private:
	using vec3 = glm::tvec3<T>;

private:
	vec3 camera_position;
	vec3 camera_look_at;
	vec3 camera_direction;
	vec3 camera_up;

	float camera_speed;

	float camera_pitch_limit;

public:
	camera()
		: camera_position({ 0,0,0 }),
		camera_look_at({ 0,0,-1 }),
		camera_direction({ 0,0,-1 }),
		camera_up({ 0,1,0 }),
		camera_speed(1.f),
		camera_pitch_limit(glm::half_pi<float>() - .05f)
	{}
	camera(const vec3 &position,
		   const vec3 &direction,
		   const vec3 &up) : camera() 
	{
		camera_position = position;
		camera_up = up;
		lookat(camera_position + direction);
	}
	~camera() noexcept {}

	camera(camera&&) = default;
	camera(const camera&) = default;
	camera &operator=(camera&&) = default;
	camera &operator=(const camera&) = default;

	/**
	 *	@brief	Sets camera speed
	 */
	void set_speed(float s) { camera_speed = s; }
	/**
	*	@brief	Sets the pitch limit, in radians, should be positive and <pi/2
	*/
	void set_pitch_limit(float rad) {
		assert(rad < glm::half_pi<float>());
		assert(rad > .0f);

		camera_pitch_limit = rad;
	}

	/**
	*	@brief	Takes a step forward, in the current viewing direction of the camera.
	*/
	void step_forward(float scale = 1.f) {
		auto delta = camera_direction * camera_speed * scale;
		camera_position += delta;
		camera_look_at += delta;
	}

	/**
	*	@brief	Takes a step backward, opposite to the current viewing direction of the camera.
	*/
	void step_backward(float scale = 1.f) {
		auto delta = -camera_direction * camera_speed * scale;
		camera_position += delta;
		camera_look_at += delta;
	}

	/**
	*	@brief	Takes a step rightwards, in tangent to the current viewing direction of the camera.
	*/
	void step_right(float scale = 1.f) {
		auto delta = glm::cross(camera_direction, camera_up) * camera_speed * scale;
		camera_position += delta;
		camera_look_at += delta;
	}

	/**
	*	@brief	Takes a step leftwards, in tangent to the current viewing direction of the camera.
	*/
	void step_left(float scale = 1.f) {
		auto delta = -glm::cross(camera_direction, camera_up) * camera_speed * scale;
		camera_position += delta;
		camera_look_at += delta;
	}

	/**
	*	@brief	Takes a step upwards, in tangent to the current viewing direction of the camera.
	*/
	void step_up(float scale = 1.f) {
		auto delta = camera_up * camera_speed * scale;
		camera_position += delta;
		camera_look_at += delta;
	}

	/**
	*	@brief	Takes a step downwards, in tangent to the current viewing direction of the camera.
	*/
	void step_down(float scale = 1.f) {
		auto delta = -camera_up * camera_speed * scale;
		camera_position += delta;
		camera_look_at += delta;
	}

	/**
	*	@brief	Transforms the camera to new position, mainting previous viewing direction.
	*/
	void set_position(const glm::vec3 &pos) {
		camera_position = pos;
		camera_direction = glm::normalize(camera_look_at - camera_position);
		camera_look_at = camera_position + camera_direction;
	}

	/**
	*	@brief	Rotates the camera to look at a point. Camera position remains unchanged.
	*/
	void lookat(const glm::vec3 &pos) {
		camera_direction = glm::normalize(pos - camera_position);
		camera_look_at = camera_position + camera_direction;
	}

	/**
	*	@brief	Rotates the camera by pitch and yaw angles, in radians. Camera position remains unchanged.
	*/
	void pitch_and_yaw(float pitch, float yaw) {
		glm::vec3 tangent = glm::cross(camera_direction, camera_up);

		// Calculate current pitch
		float camera_pitch = asinf(camera_direction.y);
		// Limit to preset limits
		pitch = glm::clamp(pitch, -(camera_pitch + camera_pitch_limit), camera_pitch_limit - camera_pitch);

		// Build
		glm::quat pitch_quat = glm::normalize(glm::angleAxis(pitch, tangent));
		glm::quat heading_quat = glm::normalize(glm::angleAxis(-yaw, /*camera_up*/glm::vec3(0, 1, 0)));
		//auto q = glm::normalize(glm::cross(pitch_quat, heading_quat));

		camera_direction = pitch_quat * camera_direction * heading_quat;
		camera_up = pitch_quat * camera_up * heading_quat;
		camera_look_at = camera_position + camera_direction;
	}

	const auto &get_position() const { return camera_position; }

	/**
	*	@brief	Returns a 4x4 transform matrix that transforms a non-homogeneous coordinate to the camera's space.
	*/
	auto view_matrix() const { return glm::lookAt(camera_position, camera_look_at, camera_up); }

	/**
	*	@brief	Returns a dual quaternion that transforms a non-homogeneous coordinate to the camera's space.
	*/
	auto view_transform_dquat() const {
		auto vmnt = view_matrix();
		glm::mat3 r = glm::mat3(vmnt);
		glm::vec3 t = -camera_position;

		return dualquat_translate_rotate(r, t);
	}
};

}
}
