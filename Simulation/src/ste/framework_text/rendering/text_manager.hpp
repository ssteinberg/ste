// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "ste_engine_control.hpp"

#include "font.hpp"
#include "glyph_point.hpp"
#include "glyph_manager.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "attributed_string.hpp"

#include "vertex_array_object.hpp"
#include "vertex_buffer_object.hpp"
#include "glsl_program.hpp"

#include "text_renderable.hpp"

#include <memory>
#include <string>
#include <vector>

namespace StE {
namespace Text {

class text_manager {
	friend class Resource::resource_loading_task<text_manager>;
	friend class Resource::resource_instance<text_manager>;

private:
	using ResizeSignalConnectionType = ste_engine_control::framebuffer_resize_signal_type::connection_type;

private:
	friend class text_renderable;

private:
	const ste_engine_control &context;

	glyph_manager gm;
	font default_font;
	int default_size;

	Resource::resource_instance<Resource::glsl_program> text_distance_mapping;

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

private:
	static void adjust_line(std::vector<glyph_point> &, const attributed_wstring &, unsigned, float , float , const glm::vec2 &);
	std::vector<glyph_point> create_points(glm::vec2, const attributed_wstring &);

private:
	text_manager(const ste_engine_control &context,
				const font &default_font,
				int default_size = 28);

public:
	~text_manager() noexcept {}

	std::unique_ptr<text_renderable> create_renderer() {
		return std::make_unique<text_renderable>(this);
	}
};

}

namespace Resource {

template <>
class resource_loading_task<Text::text_manager> {
	using R = Text::text_manager;

public:
	auto loader(const ste_engine_control &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->text_distance_mapping.wait();
		}).then_on_main_thread([object, &ctx]() {
			auto size = ctx.get_backbuffer_size();
			object->text_distance_mapping.get().set_uniform("proj", glm::ortho<float>(0, size.x, 0, size.y, -1, 1));
			object->text_distance_mapping.get().set_uniform("fb_size", glm::vec2(size));
		});
	}
};

}
}
