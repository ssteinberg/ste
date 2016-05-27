// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "glyph.hpp"
#include "Font.hpp"

#include "task_scheduler.hpp"

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

	task_future<glyph> create_glyph_task(task_scheduler *sched, const Font &font, wchar_t codepoint);

	int read_kerning(const Font &font, const std::pair<wchar_t, wchar_t> &p, int pixel_size);
};

}
}
