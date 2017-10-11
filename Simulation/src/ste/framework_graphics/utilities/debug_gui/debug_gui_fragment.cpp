
#include <stdafx.hpp>

// Compile ImGUI
#include <glfw.hpp>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif
#include <imgui/imgui.cpp>
#include <imgui/imgui_draw.cpp>

#include <debug_gui_fragment.hpp>
#include <imgui_timeline.hpp>

#include <surface.hpp>
#include <surface_factory.hpp>

#include <cmd_draw_indexed.hpp>
#include <cmd_bind_index_buffer.hpp>
#include <cmd_bind_vertex_buffers.hpp>
#include <cmd_set_scissor.hpp>
#include <cmd_pipeline_barrier.hpp>

#include <glm_print.hpp>
#include <lib/vector.hpp>
#include <lib/string.hpp>
#include <sstream>
#include <functional>

using namespace ste;
using namespace ste::graphics;

namespace ste::graphics::_detail {

static const char* imgui_get_clipboard_text(void* user_data) {
	return glfwGetClipboardString(reinterpret_cast<const ste_window*>(user_data)->get_window_handle());
}

static void imgui_set_clipboard_text(void* user_data, const char* text) {
	glfwSetClipboardString(reinterpret_cast<const ste_window*>(user_data)->get_window_handle(),
						   text);
}

}

gl::vk::vk_descriptor_pool<> debug_gui_fragment::create_imgui_desciptor_pool(const gl::vk::vk_logical_device<> &device) {
	lib::vector<gl::vk::vk_descriptor_set_layout_binding> set_layout_bindings;
	set_layout_bindings.push_back(gl::vk::vk_descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
																		   VK_SHADER_STAGE_ALL,
																		   0, 1000));

	return gl::vk::vk_descriptor_pool<>(device,
										1000,
										set_layout_bindings,
										true);
}

lib::unique_ptr<gl::texture<gl::image_type::image_2d>> debug_gui_fragment::create_imgui_fonts_texture(const ste_context &ctx) {
	// Read ImGUI font texture
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	// Create a surface
	resource::surface_2d<gl::format::r8g8b8a8_unorm> fonts_surface(glm::u32vec2(static_cast<std::uint32_t>(width),
																				static_cast<std::uint32_t>(height)));
	memcpy(fonts_surface.data(), pixels, width*height*4);

	// Create a texture out of the surface
	return lib::allocate_unique<gl::texture<gl::image_type::image_2d>>(resource::surface_factory::image_from_surface_2d<gl::format::r8g8b8a8_unorm>(ctx,
																																					std::move(fonts_surface),
																																					gl::image_usage::sampled,
																																					gl::image_layout::shader_read_only_optimal,
																																					"ImGUI fonts texture",
																																					false));
}

debug_gui_fragment::debug_gui_fragment(gl::rendering_system &rs,
									   const ste_window &window,
									   const ste::text::font &default_font) 

	: Base(rs,
		   gl::device_pipeline_graphics_configurations{},
		   create_fb_layout(rs.get_creating_context()),
		   "imgui_vulkan.vert", "imgui_vulkan.frag"),
	  window(window),
	  imgui_descriptor_pool(create_imgui_desciptor_pool(rs.get_creating_context().device())),
	  vertex_buffer(rs.get_creating_context(),
					gl::buffer_usage::vertex_buffer | gl::buffer_usage::transfer_dst,
					"ImGUI vertex buffer"),
	  index_buffer(rs.get_creating_context(),
				   gl::buffer_usage::index_buffer | gl::buffer_usage::transfer_dst,
				   "ImGUI index buffer")
{
	// Init ImGUI
	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ImGuiKey_Tab] = static_cast<int>(hid::key::KeyTAB);
	io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(hid::key::KeyLEFT);
	io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(hid::key::KeyRIGHT);
	io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(hid::key::KeyUP);
	io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(hid::key::KeyDOWN);
	io.KeyMap[ImGuiKey_PageUp] = static_cast<int>(hid::key::KeyPAGE_UP);
	io.KeyMap[ImGuiKey_PageDown] = static_cast<int>(hid::key::KeyPAGE_DOWN);
	io.KeyMap[ImGuiKey_Home] = static_cast<int>(hid::key::KeyHOME);
	io.KeyMap[ImGuiKey_End] = static_cast<int>(hid::key::KeyEND);
	io.KeyMap[ImGuiKey_Delete] = static_cast<int>(hid::key::KeyDELETE);
	io.KeyMap[ImGuiKey_Backspace] = static_cast<int>(hid::key::KeyBACKSPACE);
	io.KeyMap[ImGuiKey_Enter] = static_cast<int>(hid::key::KeyENTER);
	io.KeyMap[ImGuiKey_Escape] = static_cast<int>(hid::key::KeyESCAPE);
	io.KeyMap[ImGuiKey_A] = static_cast<int>(hid::key::KeyA);
	io.KeyMap[ImGuiKey_C] = static_cast<int>(hid::key::KeyC);
	io.KeyMap[ImGuiKey_V] = static_cast<int>(hid::key::KeyV);
	io.KeyMap[ImGuiKey_X] = static_cast<int>(hid::key::KeyX);
	io.KeyMap[ImGuiKey_Y] = static_cast<int>(hid::key::KeyY);
	io.KeyMap[ImGuiKey_Z] = static_cast<int>(hid::key::KeyZ);

	io.SetClipboardTextFn = _detail::imgui_set_clipboard_text;
	io.GetClipboardTextFn = _detail::imgui_get_clipboard_text;
	io.ClipboardUserData = const_cast<ste_window*>(&window);

	io.ImeWindowHandle = glfwGetWin32Window(window.get_window_handle());

	// Load font
	io.Fonts->AddFontFromFileTTF(default_font.get_path().string().data(), 18);
	imgui_fonts_texture = create_imgui_fonts_texture(rs.get_creating_context());
	pipeline()["fonts"] = gl::bind(gl::pipeline::combined_image_sampler(*imgui_fonts_texture,
																		rs.get_creating_context().device().common_samplers_collection().linear_sampler()));

	// Attach HID callbacks and pass data to ImGUI IO
	hid_pointer_movement_connection = make_connection(window.get_signals().signal_pointer_movement(), [](auto pos) {
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(static_cast<float>(pos.x), static_cast<float>(pos.y));
	});
	hid_pointer_button_connection = make_connection(window.get_signals().signal_pointer_button(), [this](auto b, auto action, auto mod) {
		if (action == ste::hid::status::down && 
			static_cast<int>(b) >= 0 && static_cast<int>(b) < 3)
			pointer_buttons_pressed_signals[static_cast<int>(b)].clear();
	});
	hid_scroll_connection = make_connection(window.get_signals().signal_scroll(), [this](glm::dvec2 v) {
		float last, new_v;
		do {
			last = pointer_wheel_signal_accumulator;
			new_v = last + static_cast<float>(v.y) * pointer_wheel_scale;
		} while (!pointer_wheel_signal_accumulator.compare_exchange_weak(last, new_v));
	});
	hid_keyboard_connection = make_connection(window.get_signals().signal_keyboard(), [](auto key, auto scancode, auto action, auto mod) {
		ImGuiIO& io = ImGui::GetIO();

		if (action == ste::hid::status::down)
			io.KeysDown[static_cast<int>(key)] = true;
		if (action == ste::hid::status::up)
			io.KeysDown[static_cast<int>(key)] = false;

		io.KeyCtrl = io.KeysDown[static_cast<int>(hid::key::KeyLEFT_CONTROL)] || io.KeysDown[static_cast<int>(hid::key::KeyRIGHT_CONTROL)];
		io.KeyShift = io.KeysDown[static_cast<int>(hid::key::KeyLEFT_SHIFT)] || io.KeysDown[static_cast<int>(hid::key::KeyRIGHT_SHIFT)];
		io.KeyAlt = io.KeysDown[static_cast<int>(hid::key::KeyLEFT_ALT)] || io.KeysDown[static_cast<int>(hid::key::KeyRIGHT_ALT)];
		io.KeySuper = io.KeysDown[static_cast<int>(hid::key::KeyLEFT_SUPER)] || io.KeysDown[static_cast<int>(hid::key::KeyRIGHT_SUPER)];
	});
	hid_text_input_connection = make_connection(window.get_signals().signal_text_input(), [](auto c) {
		ImGuiIO& io = ImGui::GetIO();
		if (c > 0 && c < 0x10000)
			io.AddInputCharacter(static_cast<unsigned short>(c));
	});

	for (auto &t : pointer_buttons_pressed_signals)
		t.test_and_set();
}

void debug_gui_fragment::append_frame(float frame_time_ms,
									  optional<gl::profiler::profiler::segment_results_t> profiler_segment_results) {
	// Append frame time
	frame_times.push_back(frame_time_ms);
	if (frame_times.size() > frame_times_limit)
		frame_times.erase(frame_times.begin());

	lib::vector<std::pair<lib::string, float>> times;
	if (profiler_segment_results) {
		for (auto &a : profiler_segment_results.get().data) {
			float t = a.time_end_ms - a.time_start_ms;
			times.emplace_back(a.name, t);
		}
	}

	// Prepare new ImGUI frame
	std::unique_lock<std::mutex> l(shared_data.m);

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(fb_extent.x), 
							static_cast<float>(fb_extent.y));
	io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

	// Test pointer button press signals
	for (int i = 0; i < 3; i++) {
		io.MouseDown[i] =
			!pointer_buttons_pressed_signals[i].test_and_set() ||
			glfwGetMouseButton(window.get().get_window_handle(), i) != 0;
	}
	// Read accumulated mouse wheel ticks
	{
		float wheel = pointer_wheel_signal_accumulator.load(std::memory_order_relaxed);
		while (!pointer_wheel_signal_accumulator.compare_exchange_weak(wheel, .0f)) {}
		io.MouseWheel = wheel;
	}

	// Start the frame
	ImGui::NewFrame();

	// Create debug UI
	ImGui::SetNextWindowPos(ImVec2(20, 20),
							ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(fb_extent.x) - 40.f,
									120.f),
							 ImGuiSetCond_FirstUseEver);
	if (ImGui::Begin("StE debug", nullptr)) {
		// Plot frame times graph
		{
			std::stringstream stream;
			stream << std::fixed << std::setprecision(3) << frame_time_ms << " ms";
			ImGui::PlotLines("frame times",
							 &frame_times[0],
							 static_cast<int>(frame_times.size()),
							 0,
							 stream.str().c_str());
		}

		// Write camera position
		{
			ImGui::SameLine(0, 75);
			std::stringstream camera_pos_str;
			print_vec(camera_pos_str, camera_position.v());
			ImGui::Text(camera_pos_str.str().data());
		}

		// Write device name
		{
			ImGui::SameLine(0, 75);
			ImGui::Text(get_creating_context().device().physical_device().get_properties().deviceName);
		}

		// Plot profiler timeline
		{
			ImGui::PlotTimeline(times);
		}
	}
	ImGui::End();

	// Create user provided GUIs
	if (user_gui_lambda)
		user_gui_lambda(fb_extent);

	// Render GUI
	ImGui::Render();
}

void debug_gui_fragment::record(gl::command_recorder &recorder) {
	struct draw_list_element_t {
		std::uint32_t element_count{ 0 };
		gl::i32rect scissor_rect;
		draw_list_element_t(gl::i32rect r) : scissor_rect(r) {}
	};
	struct draw_list_t {
		std::uint32_t index_buffer_size;
		std::uint32_t vertex_buffer_size;
		lib::vector<draw_list_element_t> draws_element_count;
	};

	// Memory barrier before transfer
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::vertex_input,
															  gl::pipeline_stage::transfer,
															  gl::buffer_memory_barrier(vertex_buffer,
																						gl::access_flags::vertex_attribute_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(index_buffer,
																						gl::access_flags::index_read,
																						gl::access_flags::transfer_write)));

	lib::vector<draw_list_t> draw_lists;
	{
		std::unique_lock<std::mutex> l(shared_data.m);

		// Update draw data
		const auto draw_data = ImGui::GetDrawData();
		assert(draw_data->Valid);

		const std::size_t vertex_count = draw_data->TotalVtxCount;
		std::size_t index_count = draw_data->TotalIdxCount;

		if (!vertex_count || !index_count)
			return;

		// Align index buffer to 4 bytes, reserve an extra 1 element passing for each draw list
		index_count = (index_count + draw_data->CmdListsCount + 2) >> 1 << 1;

		// Sparse resize buffers (if needed, and grow only)
		if (vertex_count > vertex_buffer.size())
			recorder << vertex_buffer.resize_cmd(get_creating_context(), vertex_count);
		if (index_count > index_buffer.size())
			recorder << index_buffer.resize_cmd(get_creating_context(), index_count);

		// Upload new data
		draw_lists.reserve(draw_data->CmdListsCount);
		std::uint32_t offset_vertex = 0;
		std::uint32_t offset_index = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];

			if (cmd_list->VtxBuffer.Size && cmd_list->IdxBuffer.Size) {
				const auto idx_buffer_size = static_cast<std::uint32_t>(cmd_list->IdxBuffer.Size + 1) >> 1 << 1;

				// Store draw list data
				draw_list_t dl;
				dl.vertex_buffer_size = static_cast<std::uint32_t>(cmd_list->VtxBuffer.Size);
				dl.index_buffer_size = idx_buffer_size;

				// Write
				recorder << vertex_buffer.overwrite_cmd(offset_vertex,
														reinterpret_cast<const imgui_vertex_data *>(cmd_list->VtxBuffer.Data),
														dl.vertex_buffer_size);
				recorder << index_buffer.overwrite_cmd(offset_index,
													   reinterpret_cast<const index_t *>(cmd_list->IdxBuffer.Data),
													   idx_buffer_size);

				offset_vertex += dl.vertex_buffer_size;
				offset_index += idx_buffer_size;

				// Save per-draw elements count
				dl.draws_element_count.reserve(cmd_list->CmdBuffer.Size);
				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
					const auto &cmd = cmd_list->CmdBuffer[cmd_i];

					draw_list_element_t e(gl::i32rect{
						{ static_cast<std::int32_t>(cmd.ClipRect.x), static_cast<std::int32_t>(cmd.ClipRect.y) },
						{ static_cast<std::uint32_t>(cmd.ClipRect.z - cmd.ClipRect.x), static_cast<std::uint32_t>(cmd.ClipRect.w - cmd.ClipRect.y + 1) }
					});
					e.element_count = cmd.ElemCount;

					dl.draws_element_count.emplace_back(std::move(e));
				}
				draw_lists.emplace_back(std::move(dl));
			}
		}
	}

	// Memory barrier after transfer
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
															  gl::pipeline_stage::vertex_input,
															  gl::buffer_memory_barrier(vertex_buffer,
																						gl::access_flags::transfer_write,
																						gl::access_flags::vertex_attribute_read),
															  gl::buffer_memory_barrier(index_buffer,
																						gl::access_flags::transfer_write,
																						gl::access_flags::index_read)));

	// Render
	recorder << pipeline().cmd_bind();
	recorder << gl::cmd_bind_vertex_buffers(0, vertex_buffer);
	recorder << gl::cmd_bind_index_buffer(index_buffer);

	std::int32_t vtx_offset = 0;
	std::uint32_t idx_offset = 0;
	for (const auto &dl : draw_lists) {
		std::uint32_t cmd_idx_offset = 0;
		for (const auto &dle : dl.draws_element_count) {
			if (dle.element_count) {
				recorder << gl::cmd_set_scissor(dle.scissor_rect);
				recorder << gl::cmd_draw_indexed(dle.element_count,
												 1, 
												 idx_offset + cmd_idx_offset,
												 vtx_offset);

				cmd_idx_offset += dle.element_count;
			}
		}

		idx_offset += dl.index_buffer_size;
		vtx_offset += static_cast<int32_t>(dl.vertex_buffer_size);
	}

	recorder << pipeline().cmd_unbind();
}
