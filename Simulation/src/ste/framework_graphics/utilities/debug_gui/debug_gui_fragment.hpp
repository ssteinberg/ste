//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <rendering_system.hpp>
#include <fragment_graphics.hpp>
#include <ste_window.hpp>

#include <vk_descriptor_pool.hpp>
#include <texture.hpp>
#include <vector.hpp>

#include <imgui/imgui.h>
#include <imgui_vertex_data.hpp>
#include <profiler.hpp>

#include <connection.hpp>
#include <font.hpp>

#include <lib/string.hpp>
#include <lib/deque.hpp>
#include <lib/unique_ptr.hpp>
#include <functional>
#include <alias.hpp>
#include <optional.hpp>
#include <atomic>
#include <array>
#include <lib/aligned_padded_ptr.hpp>
#include <mutex>

namespace ste {
namespace graphics {

/*
 *	@brief	Simple GUI for debug purposes. Uses ImGUI.
 */
class debug_gui_fragment : public gl::fragment_graphics<debug_gui_fragment> {
	using Base = gl::fragment_graphics<debug_gui_fragment>;

private:
	static constexpr std::size_t frame_times_limit = 500;
	static constexpr auto pointer_wheel_scale = .2f;

private:
	using hid_pointer_movement_connection_type = ste_window_signals::hid_pointer_movement_signal_type::connection_type;
	using hid_pointer_button_connection_type = ste_window_signals::hid_pointer_button_signal_type::connection_type;
	using hid_scroll_connection_type = ste_window_signals::hid_scroll_signal_type::connection_type;
	using hid_keyboard_connection_type = ste_window_signals::hid_keyboard_signal_type::connection_type;
	using hid_text_input_connection_type = ste_window_signals::hid_text_input_signal_type::connection_type;

	using index_t = ImDrawIdx;

private:
	alias<const ste_window> window;
	lib::aligned_padded_ptr<std::mutex> mutex;

	std::function<void(const glm::ivec2 &)> user_gui_lambda;

	metre_vec3 camera_position;
	lib::vector<float> frame_times;

	std::array<std::atomic_flag, 3> pointer_buttons_pressed_signals{ 0, 0, 0 };
	std::atomic<float> pointer_wheel_signal_accumulator{ .0f };

	glm::u32vec2 fb_extent{ 0 };
	gl::vk::vk_descriptor_pool<> imgui_descriptor_pool;
	lib::unique_ptr<gl::texture<gl::image_type::image_2d>> imgui_fonts_texture;
	gl::vector<imgui_vertex_data> vertex_buffer;
	gl::vector<index_t> index_buffer;

private:
	hid_pointer_movement_connection_type hid_pointer_movement_connection;
	hid_pointer_button_connection_type hid_pointer_button_connection;
	hid_scroll_connection_type hid_scroll_connection;
	hid_keyboard_connection_type hid_keyboard_connection;
	hid_text_input_connection_type hid_text_input_connection;

private:
	static gl::vk::vk_descriptor_pool<> create_imgui_desciptor_pool(const gl::vk::vk_logical_device<> &device);
	static lib::unique_ptr<gl::texture<gl::image_type::image_2d>> create_imgui_fonts_texture(const ste_context &ctx);

public:
	static auto create_fb_layout(const ste_context &ctx) {
		gl::framebuffer_layout fb_layout;
		fb_layout[0] = gl::load_store(ctx.device().get_surface().surface_format(),
									  gl::image_layout::color_attachment_optimal,
									  gl::image_layout::color_attachment_optimal,
									  gl::blend_operation(gl::blend_factor::src_alpha,
														  gl::blend_factor::one_minus_src_alpha,
														  gl::blend_op::add));
		return fb_layout;
	}

public:
	debug_gui_fragment(gl::rendering_system &rs,
					   const ste_window &window,
					   const ste::text::font &default_font);
	~debug_gui_fragment() noexcept {}

	static lib::string name() { return "debug_gui_fragment"; }

	static void setup_graphics_pipeline(const gl::rendering_system &rs,
										gl::pipeline_auditor_graphics &auditor) {
		static_assert(sizeof(ImDrawVert) == sizeof(imgui_vertex_data));

		gl::device_pipeline_graphics_configurations config;
		config.rasterizer_op = gl::rasterizer_operation(gl::cull_mode::none,
														gl::front_face::ccw);
		auditor.set_pipeline_settings(std::move(config));
		auditor.set_framebuffer_layout(create_fb_layout(rs.get_creating_context()));
		auditor.set_vertex_attributes(0, gl::vertex_attributes<imgui_vertex_data>());
	}

	void attach_framebuffer(gl::framebuffer &fb) {
		fb_extent = fb.extent();
		pipeline().attach_framebuffer(fb);

		pipeline()["push_t.scale"] = glm::vec2{ 2.f / static_cast<float>(fb_extent.x), 2.f / static_cast<float>(fb_extent.y) };
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
	static bool is_gui_active() {
		return ImGui::IsAnyItemActive();
	}

	/*
	*	@brief	Appends a frame time to the frame times timeline and optionally profiler's segment results 
	*			to display on the debug view.
	*			Should be called before recording the fragment.
	*/
	void append_frame(float frame_time_ms,
					  optional<gl::profiler::profiler::segment_results_t> profiler_segment_results = none);

	/*
	 *	@brief	Updates current camera position for debug display
	 */
	void set_camera_position(metre_vec3 p) {
		camera_position = p;
	}
};

}
}
