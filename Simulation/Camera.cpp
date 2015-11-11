
#include "stdafx.h"

#include "Camera.h"

#include <glm/gtc/quaternion.hpp>

using namespace StE::LLR;

Camera::Camera() {
	camera_position = { 0, 0, 0 };
	camera_up = { 0, 1, 0 };
	camera_direction = { 0, 0, -1 };
	camera_look_at = { 0, 0, -1 };
	camera_speed = 1;
	camera_pitch_limit = glm::half_pi<float>() - 0.05f;
}

void Camera::step_forward(float scale) {
	auto Δ = camera_direction * camera_speed * scale;
	camera_position += Δ;
	camera_look_at += Δ;
}

void Camera::step_backward(float scale) {
	auto Δ = -camera_direction * camera_speed * scale;
	camera_position += Δ;
	camera_look_at += Δ;
}

void Camera::step_right(float scale) {
	auto Δ = glm::cross(camera_direction, camera_up) * camera_speed * scale;
	camera_position += Δ;
	camera_look_at += Δ;
}

void Camera::step_left(float scale) {
	auto Δ = -glm::cross(camera_direction, camera_up) * camera_speed * scale;
	camera_position += Δ;
	camera_look_at += Δ;
}

void Camera::step_up(float scale) {
	auto Δ = camera_up * camera_speed * scale;
	camera_position += Δ;
	camera_look_at += Δ;
}

void Camera::step_down(float scale) {
	auto Δ = -camera_up * camera_speed * scale;
	camera_position += Δ;
	camera_look_at += Δ;
}

void Camera::set_position(const glm::vec3 &pos)  { 
	camera_position = pos;
	camera_direction = glm::normalize(camera_look_at - camera_position);
	camera_look_at = camera_position + camera_direction;
}

void Camera::lookat(const glm::vec3 &pos)  { 
	camera_look_at = pos;
	camera_direction = glm::normalize(camera_look_at - camera_position);
	camera_look_at = camera_position + camera_direction;
}

void Camera::pitch_and_yaw(float pitch, float yaw) {
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
