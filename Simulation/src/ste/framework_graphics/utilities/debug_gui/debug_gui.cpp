
#include "stdafx.hpp"
#include "debug_gui.hpp"

#include "gl_current_context.hpp"

#include <imgui/imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include "imgui_timeline.hpp"

#include <vector>
#include <algorithm>
#include <functional>

using namespace StE::Graphics;

debug_gui::debug_gui(const StEngineControl &ctx, profiler *prof) : ctx(ctx), prof(prof) {
	assert(prof);

	auto window = ctx.gl()->get_window();
	ImGui_ImplGlfwGL3_Init(window, false);

	hid_pointer_button_signal = std::make_shared<hid_pointer_button_signal_connection_type>([window](HID::pointer::B b, HID::Status action, HID::ModifierBits mod) {
		ImGui_ImplGlfwGL3_MouseButtonCallback(window, static_cast<int>(b), static_cast<int>(action), static_cast<int>(mod));
	});
	hid_scroll_signal = std::make_shared<hid_scroll_signal_connection_type>([window](const glm::dvec2 &v) {
		ImGui_ImplGlfwGL3_ScrollCallback(window, v.x, v.y);
	});
	hid_keyboard_signal = std::make_shared<hid_keyboard_signal_connection_type>([window](HID::keyboard::K k, int scancode, HID::Status action, HID::ModifierBits mod) {
		ImGui_ImplGlfwGL3_KeyCallback(window, static_cast<int>(k), scancode, static_cast<int>(action), static_cast<int>(mod));
	});
	ctx.hid_signal_pointer_button().connect(hid_pointer_button_signal);
	ctx.hid_signal_scroll().connect(hid_scroll_signal);
	ctx.hid_signal_keyboard().connect(hid_keyboard_signal);

	glfwSetCharCallback(window, ImGui_ImplGlfwGL3_CharCallback);
}

debug_gui::~debug_gui() {
	ImGui_ImplGlfwGL3_Shutdown();
}

void debug_gui::dispatch() const {
	auto &entries = prof->get_entries();
	std::vector<std::pair<std::string, float>> times;

	std::string last_name = entries.size() ? entries.back().name : std::string();
	for (auto it = entries.rbegin(); it != entries.rend();) {
		float t = static_cast<float>(it->end - it->start) / 1000.f;

		std::array<float, 10> new_arr;
		new_arr.fill(10.f);
		auto last_samples_it = prof_tasks_last_samples.emplace(std::make_pair(it->name, new_arr)).first;
		std::copy(last_samples_it->second.begin() + 1, last_samples_it->second.end(), last_samples_it->second.begin());
		last_samples_it->second.back() = t;

		float time = .0f;
		for (auto & st : last_samples_it->second)
			time += st;
		time /= 10.f;

		times.emplace(times.begin(), it->name, time);

		++it;
		if (it->name.compare(last_name) == 0)
			break;
	}

	auto bbsize = ctx.get_backbuffer_size();

	ImGui_ImplGlfwGL3_NewFrame();

	ImGui::SetNextWindowPos(ImVec2(20,20));
	ImGui::SetNextWindowSize(ImVec2(bbsize.x - 40,125));
	ImGui::Begin("StE debug", nullptr);

	auto &fts = prof->get_last_times_per_frame();
	ImGui::PlotLines("Frame Times", &fts[0], fts.size(), 0, "ms", 0.f, .1f);
	ImGui::PlotTimeline(times);

	ImGui::End();

	ImGui::Render();
}
