
#include <stdafx.hpp>
// TODO
//#include <debug_gui.hpp>
//
//#include <gl_current_context.hpp>
//
//#include <imgui/imgui.h>
//#include "imgui_glfw_integration/imgui_impl_glfw_gl3.h"
//#include <imgui_timeline.hpp>
//
//#include <gl_utils.hpp>
//#include <glm_print.hpp>
//
//#include <vector>
//#include <algorithm>
//#include <functional>
//#include <cstring>
//#include <sstream>
//
//using namespace StE::Graphics;
//
//debug_gui::debug_gui(const ste_engine_control &ctx, profiler *prof, const StE::Text::font &default_font, const camera *cam) : ctx(ctx), prof(prof), cam(cam) {
//	assert(prof);
//
//	ImGuiIO& io = ImGui::GetIO();
//	io.Fonts->AddFontFromFileTTF(default_font.get_path().string().data(), 18);
//
//	auto window = ctx.gl()->get_window();
//	ImGui_ImplGlfwGL3_Init(window, false);
//
//	hid_pointer_button_signal = std::make_shared<hid_pointer_button_signal_connection_type>([window](HID::pointer::B b, HID::Status action, HID::ModifierBits mod) {
//		ImGui_ImplGlfwGL3_MouseButtonCallback(window, static_cast<int>(b), static_cast<int>(action), static_cast<int>(mod));
//	});
//	hid_scroll_signal = std::make_shared<hid_scroll_signal_connection_type>([window](const glm::dvec2 &v) {
//		ImGui_ImplGlfwGL3_ScrollCallback(window, v.x, v.y);
//	});
//	hid_keyboard_signal = std::make_shared<hid_keyboard_signal_connection_type>([window](HID::keyboard::K k, int scancode, HID::Status action, HID::ModifierBits mod) {
//		ImGui_ImplGlfwGL3_KeyCallback(window, static_cast<int>(k), scancode, static_cast<int>(action), static_cast<int>(mod));
//	});
//	ctx.hid_signal_pointer_button().connect(hid_pointer_button_signal);
//	ctx.hid_signal_scroll().connect(hid_scroll_signal);
//	ctx.hid_signal_keyboard().connect(hid_keyboard_signal);
//
//	glfwSetCharCallback(window, ImGui_ImplGlfwGL3_CharCallback);
//}
//
//debug_gui::~debug_gui() {
//	ImGui_ImplGlfwGL3_Shutdown();
//}
//
//void debug_gui::dispatch() const {
//	auto &entries = prof->get_entries();
//	std::vector<std::pair<std::string, float>> times;
//
//	if (entries.size()) {
//		std::string last_name = entries.back().name;
//		for (auto it = entries.rbegin();;) {
//			float t = static_cast<float>(it->end - it->start) / 1000.f;
//
//			auto last_samples_it = prof_tasks_last_samples.emplace(std::piecewise_construct,
//																   std::forward_as_tuple(it->name),
//																   std::forward_as_tuple()).first;
//			last_samples_it->second.push_back(t);
//			if (last_samples_it->second.size() > 10)
//				last_samples_it->second.erase(last_samples_it->second.begin());
//
//			float time = .0f;
//			for (auto & st : last_samples_it->second)
//				time += st;
//			time /= 10.f;
//
//			times.insert(times.begin(), std::make_pair(it->name, time));
//
//			++it;
//			if (it == entries.rend() || it->name.compare(last_name) == 0)
//				break;
//		}
//	}
//
//	ImGui_ImplGlfwGL3_NewFrame();
//
//	auto bbsize = ctx.get_backbuffer_size();
//	ImGui::SetNextWindowPos(ImVec2(20,20), ImGuiSetCond_FirstUseEver);
//	ImGui::SetNextWindowSize(ImVec2(bbsize.x - 40,135), ImGuiSetCond_FirstUseEver);
//	if (ImGui::Begin("StE debug", nullptr)) {
//		auto &fts = prof->get_last_times_per_frame();
//		ImGui::PlotLines("Frame Times", &fts[0], fts.size(), 0, "ms", 0.f, .1f);
//
//		ImGui::SameLine(0, 75);
//		if (cam) {
//			std::stringstream camera_pos_str;
//			print_vec(camera_pos_str, cam->get_position());
//			ImGui::Text(camera_pos_str.str().data());
//		}
//		ImGui::SameLine(0, 75);
//		ImGui::Text(Core::GL::gl_utils::get_renderer().data());
//		
//		ImGui::PlotTimeline(times);
//	}
//
//	ImGui::End();
//
//	for (auto &f : user_guis)
//		f(bbsize);
//
//	ImGui::Render();
//}
//
//bool debug_gui::is_gui_active() const {
//	return ImGui::IsAnyItemActive();
//}
