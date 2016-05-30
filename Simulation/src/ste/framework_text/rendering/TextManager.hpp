// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "Font.hpp"
#include "glyph_point.hpp"
#include "glyph_manager.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "AttributedString.hpp"

#include "VertexArrayObject.hpp"
#include "VertexBufferObject.hpp"
#include "glsl_program.hpp"

#include "text_renderable.hpp"

#include <memory>
#include <string>
#include <vector>

namespace StE {
namespace Text {

class TextManager {
	friend class Resource::resource_loading_task<TextManager>;
	friend class Resource::resource_instance<TextManager>;

private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

private:
	friend class text_renderable;

private:
	const StEngineControl &context;

	glyph_manager gm;
	Font default_font;
	int default_size;

	Resource::resource_instance<Resource::glsl_program> text_distance_mapping;

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

private:
	void adjust_line(std::vector<glyph_point> &, const AttributedWString &, unsigned, float , float , const glm::vec2 &);
	std::vector<glyph_point> create_points(glm::vec2, const AttributedWString &);

private:
	TextManager(const StEngineControl &context,
				const Font &default_font,
				int default_size = 28);

public:
	~TextManager() noexcept {}

	std::unique_ptr<text_renderable> create_renderer() {
		return std::make_unique<text_renderable>(this);
	}
};

}

namespace Resource {

template <>
class resource_loading_task<Text::TextManager> {
	using R = Text::TextManager;

public:
	auto loader(const StEngineControl &ctx, R* object) {
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
