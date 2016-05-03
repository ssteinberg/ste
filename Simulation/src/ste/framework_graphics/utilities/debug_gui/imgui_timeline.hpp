// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "profiler.hpp"

#include <iostream>
#include <iomanip>
#include <map>
#include <string>

#include <imgui/imgui.h>

namespace ImGui {

inline void PlotTimeline(const std::vector<std::pair<std::string, float>> &times) {
	float total_time = 0.f;
	for (auto &pair : times)
		total_time += pair.second;

	ImDrawList *draw_list = ImGui::GetWindowDrawList();

	ImVec2 canvasPosition = ImGui::GetCursorScreenPos();
	ImVec2 canvasSize = ImGui::GetContentRegionAvail();

	draw_list->PushClipRect(ImVec4(canvasPosition.x, canvasPosition.y, canvasPosition.x + canvasSize.x, canvasPosition.y + canvasSize.y));

	ImGui::InvisibleButton("canvas", canvasSize);

	int offset = canvasPosition.x;
	for (auto &pair : times) {
		auto t = pair.second;
		auto p = t / total_time;
		auto poffset = std::max<int>(0, static_cast<int>(p * canvasSize.x + .5f) - 1);

		std::ostringstream out;
		out << std::setprecision(5) << t / 1000.f;
		auto t_str = out.str() + "ms";

		static constexpr int y0 = 18;
		static constexpr int y1 = 38;
		static constexpr int bar_h = 40;

		draw_list->AddRectFilled(ImVec2(offset, canvasPosition.y + y1),
								 ImVec2(offset + poffset, canvasPosition.y + y1 + bar_h),
								 ImColor(static_cast<int>(255.f * p), 0, 128, 255), 2.f);

		auto clip0 = ImVec4(offset, canvasPosition.y + y1, offset + poffset, canvasPosition.y + y1 + bar_h);
		auto clip1 = ImVec4(offset, canvasPosition.y + y0, offset + poffset, canvasPosition.y + y1);
		draw_list->AddText(ImGui::GetFont(),
						   ImGui::GetFontSize()*1.0f,
						   ImVec2(offset, canvasPosition.y + y1),
						   ImColor(255, 128, 0, 255),
						   pair.first.data(), nullptr, static_cast<float>(poffset),
						   &clip0);
		draw_list->AddText(ImGui::GetFont(),
						   ImGui::GetFontSize()*1.0f,
						   ImVec2(offset + poffset / 2, canvasPosition.y + y0),
						   ImColor(255, 0, 255, 255),
						   t_str.data(), nullptr, 0.0f,
						   &clip1);

		offset += poffset + 1;
	}

	draw_list->PopClipRect();
}

}
