//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <ste_glfw_handle.hpp>

#include <optional.hpp>

#include <ste_window_exceptions.hpp>
#include <ste_window_signals.hpp>

namespace StE {

class ste_window {
private:
	using window_type = GLFWwindow*;

private:
	// Global GLFW handle
	static ste_glfw_handle glfw;

private:
	window_type window;
	std::unique_ptr<ste_window_signals> signals;

public:
	ste_window(std::string window_title, glm::i32vec2 window_size, bool fullscreen = false) {
		auto fs_monitor_handle = fullscreen ? glfwGetPrimaryMonitor() : nullptr;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(window_size.x,
								  window_size.y,
								  window_title.c_str(),
								  fs_monitor_handle, 
								  nullptr);
		if (window == nullptr) {
			throw ste_window_creation_exception("Window creation failed");
		}

		signals = std::make_unique<ste_window_signals>(this);

	}
	~ste_window() noexcept {
		signals = nullptr;
		glfwDestroyWindow(window);
	}

	ste_window(ste_window &&) = default;
	ste_window &operator=(ste_window &&) = default;

	void set_title(const char * title) { glfwSetWindowTitle(window, title); }
	glm::i32vec2 get_window_position() const {
		glm::i32vec2 ret;
		glfwGetWindowPos(window, &ret.x, &ret.y);
		return ret;
	}
	void set_position(const glm::i32vec2 &p) {
		glfwSetWindowPos(window, p.x, p.y);
	}

	glm::i32vec2 get_window_client_area_size() const {
		glm::i32vec2 ret;
		glfwGetWindowSize(window, &ret.x, &ret.y);
		return ret;
	}
	void set_size(const glm::i32vec2 &size) {
		glfwSetWindowSize(window, size.x, size.y);
	}

	/**
	*	@brief	Returns true if the window is focused, false otherwise
	*/
	bool is_window_focused() const { return !!glfwGetWindowAttrib(window, GLFW_FOCUSED); }
	/**
	*	@brief	Returns true if the window is requesting to close
	*/
	bool should_close() const { return !!glfwWindowShouldClose(window); }

	auto get_window_handle() const { return window; }
	auto& get_signals() const { return *signals; }
};

}
