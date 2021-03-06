//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <ste_engine_exceptions.hpp>

#include <ste_window_exceptions.hpp>
#include <ste_window_signals.hpp>

#include <anchored.hpp>
#include <lib/unique_ptr.hpp>

namespace ste {

class ste_window : anchored {
private:
	using window_type = GLFWwindow*;

private:
	window_type window;
	lib::unique_ptr<ste_window_signals> signals;

public:
	ste_window(lib::string window_title, glm::i32vec2 window_size, bool fullscreen = false) {
		const auto fs_monitor_handle = fullscreen ? glfwGetPrimaryMonitor() : nullptr;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(window_size.x,
								  window_size.y,
								  window_title.c_str(),
								  fs_monitor_handle, 
								  nullptr);
		if (window == nullptr) {
			throw ste_window_creation_exception("Window creation failed");
		}

		signals = lib::allocate_unique<ste_window_signals>(this);

	}
	~ste_window() noexcept {
		signals = nullptr;
		glfwDestroyWindow(window);
	}

	/**
	*	@brief	Polls windowing system events.
	*			Note this method polls events for ALL application windows.
	*			
	*	@throws ste_engine_glfw_exception	On windowing system error
	*/
	static void poll_events() {
		glfwPollEvents();
	}

	/**
	*	@brief	Sets window title
	*	
	*	@param title	nul-termianted new window title
	*/
	void set_title(const char * title) { glfwSetWindowTitle(window, title); }

	/**
	*	@brief	Returns window position, in screen coordinates.
	*	
	*	@return Distance to the upper-left corner of the client area
	*/
	glm::i32vec2 get_window_position() const {
		glm::i32vec2 ret;
		glfwGetWindowPos(window, &ret.x, &ret.y);
		return ret;
	}

	/**
	*	@brief	Set window position, in screen coordinates.
	*	
	*	@param p	Point to move window's upper-left corner of client area to
	*/
	void set_position(const glm::i32vec2 &p) {
		glfwSetWindowPos(window, p.x, p.y);
	}

	/**
	*	@brief	Returns the client area size of the window, in screen coordinates
	*/
	glm::i32vec2 get_window_client_area_size() const {
		glm::i32vec2 ret;
		glfwGetWindowSize(window, &ret.x, &ret.y);
		return ret;
	}

	/**
	*	@brief	Set window client are size
	*	
	*	@param size	Size of the client area in screen coordinates
	*/
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
