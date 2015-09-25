// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <functional>
#include <memory>
#include <atomic>

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderContext.h"
#include "hid.h"
#include "signal.h"

namespace StE {

class StEngineControl {
private:
	std::unique_ptr<LLR::RenderContext> context{ nullptr };

	mutable glm::mat4 projection;
	float field_of_view;
	float near_clip;
	float far_clip;

	float fps{ 0 };
	std::chrono::duration<float> tpf{ 0 };

	bool projection_dirty{ true };
	void set_projection_dirty() { projection_dirty = true; }

public:
	using framebuffer_resize_signal_type = signal<glm::i32vec2>;
	using hid_pointer_button_signal_type = signal<HID::pointer::B, HID::Status, HID::ModifierBits>;
	using hid_pointer_movement_signal_type = signal<glm::dvec2>;
	using hid_scroll_signal_type = signal<glm::dvec2>;
	using hid_keyboard_signal_type = signal<HID::keyboard::K, int, HID::Status, HID::ModifierBits>;

private:
	framebuffer_resize_signal_type framebuffer_resize_signal;

	hid_pointer_movement_signal_type hid_pointer_movement_signal;
	hid_pointer_button_signal_type hid_pointer_button_signal;
	hid_scroll_signal_type hid_scroll_signal;
	hid_keyboard_signal_type hid_keyboard_signal;

public:
	StEngineControl(StEngineControl &&m) = delete;
	StEngineControl(const StEngineControl &c) = delete;
	StEngineControl& operator=(StEngineControl &&m) = delete;
	StEngineControl& operator=(const StEngineControl &c) = delete;

	StEngineControl() : projection_dirty(true), field_of_view(M_PI_4), near_clip(.1), far_clip(1000) {}
	~StEngineControl() {}

	bool init_render_context(const char *title, const glm::i32vec2 &size, bool fs = false, bool vsync = true, gli::format format = gli::FORMAT_RGBA8_UNORM, int samples = 0, gli::format depth_format = gli::FORMAT_D24_UNORM);
	const LLR::RenderContext &render_context() const { return *context; }

	void set_windows_title(const char * title) { glfwSetWindowTitle(context->window.get(), title); }
	glm::i32vec2 get_window_position() const {
		glm::i32vec2 ret;
		glfwGetWindowPos(context->window.get(), &ret.x, &ret.y);
		return ret;
	}
	void set_window_position(const glm::i32vec2 &p) {
		glfwSetWindowPos(context->window.get(), p.x, p.y);
	}
	glm::i32vec2 get_window_size() const {
		glm::i32vec2 ret;
		glfwGetWindowSize(context->window.get(), &ret.x, &ret.y);
		return ret;
	}
	void set_window_size(const glm::i32vec2 &size) {
		glfwSetWindowSize(context->window.get(), size.x, size.y);
	}

	void run_loop(std::function<bool()> process);
	auto frames_per_second() const { return fps; }
	auto time_per_frame() const { return tpf; }

	const decltype(framebuffer_resize_signal) &signal_framebuffer_resize() const { return framebuffer_resize_signal; }
	const decltype(hid_pointer_movement_signal) &hid_signal_pointer_movement() const { return hid_pointer_movement_signal; }
	const decltype(hid_pointer_button_signal) &hid_signal_pointer_button() const { return hid_pointer_button_signal; }
	const decltype(hid_scroll_signal) &hid_signal_scroll() const { return hid_scroll_signal; }
	const decltype(hid_keyboard_signal) &hid_signal_keyboard() const { return hid_keyboard_signal; }

	bool window_active() const { return !!glfwGetWindowAttrib(context->window.get(), GLFW_FOCUSED); }
	glm::i32vec2 get_viewport_size() const { return context->framebuffer_size(); }
	void set_fov(float rad) { field_of_view = rad; set_projection_dirty(); }
	void set_clipping_planes(float near_clip_distance, float far_clip_distance) { near_clip = near_clip_distance; far_clip = far_clip_distance; set_projection_dirty(); }
	glm::mat4 projection_matrix() const {
		if (projection_dirty) {
			auto vs = get_viewport_size();
			float aspect = vs.x / vs.y;
			projection = glm::perspective(field_of_view, aspect, near_clip, far_clip);
		}
		return projection;
	}

public:
	glm::i32vec2 get_pointer_position() {
		glm::dvec2 pos;
		glfwGetCursorPos(context->window.get(), &pos.x, &pos.y);
		pos.y = get_window_size().y - pos.y;
		return pos;
	}
	void set_pointer_position(const glm::dvec2 &pos) {
		glfwSetCursorPos(context->window.get(), pos.x, get_window_size().y - pos.y);
	}
	void set_pointer_hidden(bool hidden) { glfwSetInputMode(context->window.get(), GLFW_CURSOR, hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL); }
	HID::Status get_key_status(HID::keyboard::K k) { return HID::keyboard::key_status(context->window.get(), k); }
	HID::Status get_pointer_button_status(HID::pointer::B b) { return HID::pointer::button_status(context->window.get(), b); }
};

}
