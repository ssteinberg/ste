//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <profiler.hpp>

#include <sstream>
#include <iomanip>
#include <lib/vector.hpp>
#include <lib/string.hpp>

#include <imgui/imgui.h>

namespace ImGui {

inline void PlotTimeline(const ste::lib::vector<std::pair<ste::lib::string, float>> &times) {
	float total_time = 0.f;
	for (auto &pair : times)
		total_time += pair.second;

	ImDrawList *draw_list = ImGui::GetWindowDrawList();

	const ImVec2 canvasPosition = ImGui::GetCursorScreenPos();
	const ImVec2 canvasSize = ImGui::GetContentRegionAvail();

	draw_list->PushClipRect(ImVec2(canvasPosition.x, canvasPosition.y), ImVec2(canvasPosition.x + canvasSize.x, canvasPosition.y + canvasSize.y), true);

	ImGui::InvisibleButton("canvas", canvasSize);

	auto offset = canvasPosition.x;
	for (auto &pair : times) {
		const auto t = pair.second;
		const auto p = t / total_time;
		const auto poffset = std::max<int>(0, static_cast<int>(p * canvasSize.x + .5f) - 1);

		std::ostringstream out;
		out << std::setprecision(5) << t;
		auto t_str = out.str() + "ms";

		static constexpr int y0 = 10;
		static constexpr int y1 = 30;
		static constexpr int bar_h = 35;

		draw_list->AddRectFilled(ImVec2(offset, canvasPosition.y + y1),
								 ImVec2(offset + poffset, canvasPosition.y + y1 + bar_h),
								 ImColor(static_cast<int>(255.f * p), 0, 128, 255), 2.f);

		auto clip0 = ImVec4(offset, canvasPosition.y + y1, offset + poffset, canvasPosition.y + y1 + bar_h);
		auto clip1 = ImVec4(offset, canvasPosition.y + y0, offset + poffset, canvasPosition.y + y1);
		draw_list->AddText(ImGui::GetFont(),
						   ImGui::GetFontSize()*.8f,
						   ImVec2(offset, canvasPosition.y + y1),
						   ImColor(255, 128, 0, 255),
						   pair.first.data(), nullptr, static_cast<float>(poffset),
						   &clip0);
		draw_list->AddText(ImGui::GetFont(),
						   ImGui::GetFontSize()*1.f,
						   ImVec2(offset + 3, canvasPosition.y + y0),
						   ImColor(255, 0, 255, 255),
						   t_str.data(), nullptr, 0.0f,
						   &clip1);

		offset += poffset + 1;
	}

	draw_list->PopClipRect();
}

}
