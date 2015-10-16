// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "glyph.h"
#include "Font.h"

#include "task.h"

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

	task<glyph> create_glyph_task(const Font &font, wchar_t codepoint);
};

}
}
