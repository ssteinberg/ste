// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace StE {
namespace LLR {

class Camera {
private:
	glm::vec3 camera_position;
	glm::vec3 camera_look_at;
	glm::vec3 camera_direction;
	glm::vec3 camera_up;

	float camera_speed;
	float camera_pitch;

	float camera_pitch_limit;

public:
	Camera();
	virtual ~Camera() {}

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
	glm::mat4 view_matrix_no_translation() const { return glm::lookAt(glm::vec3(0), camera_look_at - camera_position, camera_up); }
};

}
}
