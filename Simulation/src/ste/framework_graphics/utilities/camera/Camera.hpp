// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "dual_quaternion.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace StE {
namespace Graphics {

class camera {
private:
	glm::vec3 camera_position;
	glm::vec3 camera_look_at;
	glm::vec3 camera_direction;
	glm::vec3 camera_up;

	float camera_speed;
	float camera_pitch;

	float camera_pitch_limit;

public:
	camera();
	virtual ~camera() {}

	void set_speed(float s) { camera_speed = s; }
	void set_pitch_limit(float rad) { camera_pitch_limit = rad; }

	void step_forward(float scale = 1.0f);
	void step_backward(float scale = 1.0f);
	void step_left(float scale = 1.0f);
	void step_right(float scale = 1.0f);
	void step_up(float scale = 1.0f);
	void step_down(float scale = 1.0f);

	void set_position(const glm::vec3 &pos);
	void pitch_and_yaw(float pitch, float yaw);
	void lookat(const glm::vec3 &pos);

	const glm::vec3 &get_position() const { return camera_position; }

	glm::mat4 view_matrix() const { return glm::lookAt(camera_position, camera_look_at, camera_up); }

	glm::dualquat view_transform_dquat() const {
		auto vmnt = view_matrix();
		glm::mat3 r = glm::mat3(vmnt);
		glm::vec3 t = -camera_position;

		return dualquat_translate_rotate(r, t);
	}
};

}
}
