//	StE
// © Shlomi Steinberg 2015-2016

/**	@file	StEngineControl.hpp
 *	@brief	Engine Control class
 *
 *	StEngineControl is used as the engine context.
 *	It holds a rendering context and global helper objects:
 *	Task scheduler, cache and GLSL programs pool.
 *
 *	StEngineControl should initialized with a GL context
 *	Core::gl_context.
 *
 *	@author	Shlomi Steinberg
 */

#pragma once

#include "stdafx.hpp"

#include <functional>
#include <memory>
#include <atomic>

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_context.hpp"
#include "hid.hpp"
#include "signal.hpp"

#include "task_scheduler.hpp"
#include "lru_cache.hpp"
#include "rendering_system.hpp"
#include "glsl_programs_pool.hpp"

namespace StE {

struct ste_engine_control_impl;

/**
 *	@brief	StE Engine Control class
 */
class StEngineControl {
private:
	std::unique_ptr<ste_engine_control_impl> pimpl;

	mutable glm::mat4 projection;

	std::chrono::duration<float> tpf{ 0 };

	void set_projection_dirty();

	std::unique_ptr<Core::GL::gl_context> context;
	mutable task_scheduler global_scheduler;
	mutable lru_cache<std::string> global_cache;
	mutable Resource::glsl_programs_pool glslprogramspool{ *this };
	mutable Graphics::rendering_system* global_renderer{ nullptr };

public:
	using framebuffer_resize_signal_type = signal<glm::i32vec2>;
	using projection_change_signal_type = signal<const glm::mat4&, float, float, float>;
	using hid_pointer_button_signal_type = signal<HID::pointer::B, HID::Status, HID::ModifierBits>;
	using hid_pointer_movement_signal_type = signal<glm::dvec2>;
	using hid_scroll_signal_type = signal<glm::dvec2>;
	using hid_keyboard_signal_type = signal<HID::keyboard::K, int, HID::Status, HID::ModifierBits>;

private:
	framebuffer_resize_signal_type framebuffer_resize_signal;
	projection_change_signal_type projection_change_signal;

	hid_pointer_movement_signal_type hid_pointer_movement_signal;
	hid_pointer_button_signal_type hid_pointer_button_signal;
	hid_scroll_signal_type hid_scroll_signal;
	hid_keyboard_signal_type hid_keyboard_signal;

	void update_tpf();
	void setup_signals();

public:
	StEngineControl(StEngineControl &&m) = delete;
	StEngineControl(const StEngineControl &c) = delete;
	StEngineControl& operator=(StEngineControl &&m) = delete;
	StEngineControl& operator=(const StEngineControl &c) = delete;

	/**
	*	@brief	Create a control class with a given GL context
	*
	*  @param context	R-value reference to a unique_ptr to a GL context
	*/
	StEngineControl(std::unique_ptr<Core::GL::gl_context> &&context);
	~StEngineControl() noexcept;

	/**
	*	@brief	Sets the current system renderer.
	*	e.g. the renderer will be used when calling run_loop().
	*
	*  @param r	Renderer to use
	*/
	void set_renderer(Graphics::rendering_system *r) { global_renderer = r; }

	auto &gl() const { return context; }
	auto &scheduler() const { return global_scheduler; }
	auto &cache() const { return global_cache; }
	auto &glslprograms_pool() const { return glslprogramspool; }
	Graphics::rendering_system *renderer() const { return global_renderer; }

	void set_window_title(const char * title) { glfwSetWindowTitle(context->window.get(), title); }
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

	auto time_per_frame() const { return tpf; }

	/**
	*	@brief	Dispatch rendering tasks, swap buffers, handle input and window system events.
	*/
	bool run_loop();

	void capture_screenshot() const;

	const decltype(framebuffer_resize_signal) &signal_framebuffer_resize() const { return framebuffer_resize_signal; }
	const decltype(projection_change_signal) &signal_projection_change() const { return projection_change_signal; }
	const decltype(hid_pointer_movement_signal) &hid_signal_pointer_movement() const { return hid_pointer_movement_signal; }
	const decltype(hid_pointer_button_signal) &hid_signal_pointer_button() const { return hid_pointer_button_signal; }
	const decltype(hid_scroll_signal) &hid_signal_scroll() const { return hid_scroll_signal; }
	const decltype(hid_keyboard_signal) &hid_signal_keyboard() const { return hid_keyboard_signal; }

	bool window_active() const { return !!glfwGetWindowAttrib(context->window.get(), GLFW_FOCUSED); }
	glm::i32vec2 get_backbuffer_size() const { return context->framebuffer_size(); }
	void set_fov(float rad);
	void set_clipping_planes(float near_clip_distance, float far_clip_distance);

	float get_fov() const;
	float get_near_clip() const;
	float get_far_clip() const;
	glm::mat4 &projection_matrix() const;

	glm::mat4 ortho_projection_matrix() const {
		auto vs = get_backbuffer_size();
		return glm::ortho<float>(0, vs.x, 0, vs.y, -1, 1);
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
