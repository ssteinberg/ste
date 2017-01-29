//	StE
// © Shlomi Steinberg 2015-2016

/**	@file	ste_engine_control.hpp
 *	@brief	Engine Control class
 *
 *	ste_engine_control is used as the engine context.
 *	It holds a rendering context and global helper objects:
 *	Task scheduler, cache and GLSL programs pool.
 *
 *	ste_engine_control should initialized with a GL context
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

namespace StE {

struct ste_engine_control_impl;

/**
 *	@brief	StE Engine Control class
 */
class ste_engine_control {
private:
	std::unique_ptr<ste_engine_control_impl> pimpl;

	std::chrono::duration<float> tpf{ 0 };

	void set_projection_dirty();

	std::unique_ptr<Core::GL::gl_context> context;
	mutable task_scheduler global_scheduler;
	mutable lru_cache<std::string> global_cache;
	mutable Graphics::rendering_system* global_renderer{ nullptr };

public:
	using framebuffer_resize_signal_type = signal<glm::i32vec2>;
	using projection_change_signal_type = signal<float, float, float>;
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
	ste_engine_control(ste_engine_control &&m) = delete;
	ste_engine_control(const ste_engine_control &c) = delete;
	ste_engine_control& operator=(ste_engine_control &&m) = delete;
	ste_engine_control& operator=(const ste_engine_control &c) = delete;

	/**
	*	@brief	Create a control class with a given GL context
	*
	*  @param context	R-value reference to a unique_ptr to a GL context
	*/
	ste_engine_control(std::unique_ptr<Core::GL::gl_context> &&context);
	~ste_engine_control() noexcept;

	/**
	*	@brief	Sets the current system renderer.
	*	e.g. the renderer will be used when calling run_loop().
	*
	*  @param r	Renderer to use
	*/
	void set_renderer(Graphics::rendering_system *r) { global_renderer = r; }

	/**
	*	@brief	Get reference to GL drawing context
	*/
	auto &gl() const { return context; }
	/**
	*	@brief	Get global task schduler
	*/
	auto &scheduler() const { return global_scheduler; }
	/**
	*	@brief	Get reference to engine lru_cache
	*/
	auto &cache() const { return global_cache; }
	/**
	*	@brief	Get reference to engine renderer
	*/
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

	/**
	*	@brief	Capture framebuffer and saves it to Screenshots/ directory
	*/
	void capture_screenshot() const;

	const decltype(framebuffer_resize_signal) &signal_framebuffer_resize() const { return framebuffer_resize_signal; }
	const decltype(projection_change_signal) &signal_projection_change() const { return projection_change_signal; }
	const decltype(hid_pointer_movement_signal) &hid_signal_pointer_movement() const { return hid_pointer_movement_signal; }
	const decltype(hid_pointer_button_signal) &hid_signal_pointer_button() const { return hid_pointer_button_signal; }
	const decltype(hid_scroll_signal) &hid_signal_scroll() const { return hid_scroll_signal; }
	const decltype(hid_keyboard_signal) &hid_signal_keyboard() const { return hid_keyboard_signal; }

	/**
	*	@brief	Returns true if the application window is focused, false otherwise
	*/
	bool window_active() const { return !!glfwGetWindowAttrib(context->window.get(), GLFW_FOCUSED); }
	/**
	*	@brief	Returns backbuffer size in pixels. Ignores multisampling.
	*/
	glm::i32vec2 get_backbuffer_size() const { return context->framebuffer_size(); }

	/**
	*	@brief	Set perspective projection vertical field-of-view
	*/
	void set_fov(float rad);
	/**
	*	@brief	Set perspective projection clipping planes
	*/
	void set_clipping_planes(float near_clip_distance);

	float get_fov() const;
	float get_near_clip() const;
	float get_projection_aspect() const;

public:
	glm::i32vec2 get_pointer_position() const {
		glm::dvec2 pos;
		glfwGetCursorPos(context->window.get(), &pos.x, &pos.y);
		pos.y = get_window_size().y - pos.y;
		return pos;
	}
	void set_pointer_position(const glm::dvec2 &pos) {
		glfwSetCursorPos(context->window.get(), pos.x, get_window_size().y - pos.y);
	}
	void set_pointer_hidden(bool hidden) { glfwSetInputMode(context->window.get(), GLFW_CURSOR, hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL); }
	HID::Status get_key_status(HID::keyboard::K k) const { return HID::keyboard::key_status(context->window.get(), k); }
	HID::Status get_pointer_button_status(HID::pointer::B b) const { return HID::pointer::button_status(context->window.get(), b); }
};

}
