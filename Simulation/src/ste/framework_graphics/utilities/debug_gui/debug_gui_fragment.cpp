
#include <stdafx.hpp>

// Compile ImGUI
#include <glfw.hpp>
#include <imgui/imgui.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/glfw_vulkan_impl/imgui_impl_glfw_vulkan.cpp>

#include <debug_gui_fragment.hpp>
#include <imgui_timeline.hpp>

#include <glm_print.hpp>
#include <lib/vector.hpp>
#include <algorithm>
#include <functional>
#include <sstream>

using namespace ste;
using namespace ste::graphics;

static void imgui_vk_result_checker(VkResult result) {
	const gl::vk::vk_result res = result;
	if (!res) {
		throw gl::vk::vk_exception(res);
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

debug_gui_fragment::debug_gui_fragment(gl::rendering_system &rs,
									   const ste_window &window,
									   gl::framebuffer_layout &&fb_layout,
									   profiler *prof, 
									   const ste::text::font &default_font) 

	: Base(rs,
		   gl::device_pipeline_graphics_configurations{},
		   std::move(fb_layout),
		   "imgui_vulkan.vert", "imgui_vulkan.frag"),
	  prof(prof),
	  imgui_descriptor_pool(create_imgui_desciptor_pool(rs.get_creating_context().device()))
{
	// Init ImGUI
	ImGui_ImplGlfwVulkan_Init_Data imgui_init = {};
	imgui_init.allocator = &gl::vk::vk_host_allocator<>::allocation_callbacks();
	imgui_init.check_vk_result = imgui_vk_result_checker;
	imgui_init.descriptor_pool = imgui_descriptor_pool;
	imgui_init.device = rs.get_creating_context().device();
	imgui_init.gpu = rs.get_creating_context().device().physical_device().device;
	imgui_init.render_pass = this->pipeline().get_renderpass();
	if (!ImGui_ImplGlfwVulkan_Init(window.get_window_handle(),
								   false,		// Do not set GLFW callbacks
								   &imgui_init)) {
		throw std::runtime_error("ImGUI init failed");
	}

	// Load font
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(default_font.get_path().string().data(), 18);

	rs.get_creating_context().device().submit_onetime_batch(gl::ste_queue_selector<gl::ste_queue_selector_policy_flexible>(gl::ste_queue_type::data_transfer_sparse_queue),
															[=](gl::command_recorder &recorder) {
		ImGui_ImplGlfwVulkan_CreateFontsTexture(recorder.get_command_buffer());
	});

	// Attach HID callbacks and pass data to ImGUI IO
	hid_pointer_movement_connection = make_connection(window.get_signals().signal_pointer_movement(), [](auto pos) {
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(static_cast<float>(pos.x), static_cast<float>(pos.y));
	});
	hid_pointer_button_connection = make_connection(window.get_signals().signal_pointer_button(), [handle = window.get_window_handle()](auto b, auto action, auto mod) {
		ImGui_ImplGlfwVulkan_MouseButtonCallback(handle, static_cast<int>(b), static_cast<int>(action), static_cast<int>(mod));
	});
	hid_scroll_connection = make_connection(window.get_signals().signal_scroll(), [handle = window.get_window_handle()](auto v) {
		ImGui_ImplGlfwVulkan_ScrollCallback(handle, v.x, v.y);
	});
	hid_keyboard_connection = make_connection(window.get_signals().signal_keyboard(), [handle = window.get_window_handle()](auto k, auto scancode, auto action, auto mod) {
		ImGui_ImplGlfwVulkan_KeyCallback(handle, static_cast<int>(k), scancode, static_cast<int>(action), static_cast<int>(mod));
	});
	hid_text_input_connection = make_connection(window.get_signals().signal_text_input(), [handle = window.get_window_handle()](auto ch) {
		ImGui_ImplGlfwVulkan_CharCallback(handle, ch);
	});
}

debug_gui_fragment::~debug_gui_fragment() {
	ImGui_ImplGlfwVulkan_Shutdown();
}

bool debug_gui_fragment::is_gui_active() const {
	return ImGui::IsAnyItemActive();
}

void debug_gui_fragment::record(gl::command_recorder &recorder) {
	auto &entries = prof->get_entries();
	lib::vector<std::pair<lib::string, float>> times;

	if (entries.size()) {
		lib::string last_name = entries.back().name;
		for (auto it = entries.rbegin();;) {
			float t = static_cast<float>(it->end - it->start) / 1000.f;

			auto last_samples_it = prof_tasks_last_samples.emplace(std::piecewise_construct,
																   std::forward_as_tuple(it->name),
																   std::forward_as_tuple()).first;
			last_samples_it->second.push_back(t);
			if (last_samples_it->second.size() > 10)
				last_samples_it->second.erase(last_samples_it->second.begin());

			float time = .0f;
			for (auto & st : last_samples_it->second)
				time += st;
			time /= 10.f;

			times.insert(times.begin(), std::make_pair(it->name, time));

			++it;
			if (it == entries.rend() || it->name.compare(last_name) == 0)
				break;
		}
	}

	ImGui_ImplGlfwVulkan_NewFrame();

	ImGui::SetNextWindowPos(ImVec2(20,20), 
							ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(fb_extent.x) - 40.f,
									135.f), 
							 ImGuiSetCond_FirstUseEver);
	if (ImGui::Begin("StE debug", nullptr)) {
		// Plot frame times graph
//		auto &fts = prof->get_last_times_per_frame();
//		ImGui::PlotLines("Frame Times", 
//						 &fts[0], 
//						 static_cast<int>(fts.size()), 
//						 0, 
//						 "ms", 
//						 0.f, 
//						 .1f);
//
//		// Write camera position
//		ImGui::SameLine(0, 75);
//		std::stringstream camera_pos_str;
//		print_vec(camera_pos_str, camera_position);
//		ImGui::Text(camera_pos_str.str().data());
//		
//		// Write device name
//		ImGui::SameLine(0, 75);
//		ImGui::Text(get_creating_context().device().physical_device().properties.deviceName);
//		
//		// Plot profiler timeline
//		ImGui::PlotTimeline(times);
	}
	ImGui::End();

	// Render user provided GUIs
	if (user_gui_lambda)
		user_gui_lambda(fb_extent);

	ImGui_ImplGlfwVulkan_Render(recorder.get_command_buffer());
}
