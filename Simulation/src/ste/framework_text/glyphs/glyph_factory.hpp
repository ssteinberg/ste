// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <glyph.hpp>
#include <font.hpp>

#include <task_scheduler.hpp>

#include <memory>
#include <string>

namespace StE {
namespace Text {

struct glyph_factory_impl;

class glyph_factory {
private:
	std::unique_ptr<glyph_factory_impl> pimpl;

public:
	glyph_factory();
	~glyph_factory();

	task_future<glyph> create_glyph_async(task_scheduler *sched, const font &font, wchar_t codepoint);

	int read_kerning(const font &font, const std::pair<wchar_t, wchar_t> &p, int pixel_size);
};

}
}
