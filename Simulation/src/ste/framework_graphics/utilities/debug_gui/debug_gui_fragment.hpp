//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <rendering_system.hpp>
#include <fragment_graphics.hpp>
#include <ste_window.hpp>

#include <vk_descriptor_pool.hpp>

#include <imgui/imgui.h>
#include <profiler.hpp>

#include <connection.hpp>
#include <font.hpp>

#include <lib/map.hpp>
#include <lib/vector.hpp>
#include <lib/string.hpp>
#include <alias.hpp>
#include <functional>

namespace ste {
namespace graphics {

/*
 *	@brief	Simple GUI for debug purposes. Uses ImGUI.
 */
class debug_gui_fragment : public gl::fragment_graphics<debug_gui_fragment> {
	using Base = gl::fragment_graphics<debug_gui_fragment>;

private:
	using hid_pointer_movement_connection_type = ste_window_signals::hid_pointer_movement_signal_type::connection_type;
	using hid_pointer_button_connection_type = ste_window_signals::hid_pointer_button_signal_type::connection_type;
	using hid_scroll_connection_type = ste_window_signals::hid_scroll_signal_type::connection_type;
	using hid_keyboard_connection_type = ste_window_signals::hid_keyboard_signal_type::connection_type;
	using hid_text_input_connection_type = ste_window_signals::hid_text_input_signal_type::connection_type;

private:
	profiler *prof;
	glm::vec3 camera_position;
	glm::u32vec2 fb_extent{ 0 };

	gl::vk::vk_descriptor_pool<> imgui_descriptor_pool;

	std::function<void(const glm::ivec2 &)> user_gui_lambda;
	mutable lib::map<lib::string, lib::vector<float>> prof_tasks_last_samples;

private:
	hid_pointer_movement_connection_type hid_pointer_movement_connection;
	hid_pointer_button_connection_type hid_pointer_button_connection;
	hid_scroll_connection_type hid_scroll_connection;
	hid_keyboard_connection_type hid_keyboard_connection;
	hid_text_input_connection_type hid_text_input_connection;

private:
	static gl::vk::vk_descriptor_pool<> create_imgui_desciptor_pool(const gl::vk::vk_logical_device<> &device);

public:
	debug_gui_fragment(gl::rendering_system &rs,
					   const ste_window &window,
					   gl::framebuffer_layout &&fb_layout,
					   profiler *prof, 
					   const ste::text::font &default_font);
	~debug_gui_fragment() noexcept;

	static lib::string name() { return "debug_gui_fragment"; }

	static void setup_graphics_pipeline(const gl::rendering_system &rs,
										gl::pipeline_auditor_graphics &auditor) {}

	void attach_framebuffer(gl::framebuffer &fb) {
		fb_extent = fb.extent();
		pipeline().attach_framebuffer(fb);
	}

	void record(gl::command_recorder &recorder) override final;

	/*
	*	@brief	Sets a lambda that can draw additional UI elements
	*/
	void set_custom_gui(std::function<void(const glm::ivec2 &)> &&f) {
		user_gui_lambda = std::move(f);
	}

	/*
	*	@brief	Returns true if any UI element is in use
	*/
	bool is_gui_active() const;

	/*
	 *	@brief	Updates current camera position for debug display
	 */
	void set_camera_position(const glm::vec3 &p) {
		camera_position = p;
	}
};

}
}
