// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "rendering_system.h"

namespace StE {
namespace Graphics {

class BasicRenderer : public rendering_system {
public:
	virtual ~BasicRenderer() noexcept {}

	virtual void finalize_queue(const StEngineControl &ctx) override {}
	virtual void render_queue(const StEngineControl &ctx) override {
		queue().render(&ctx.gl()->defaut_framebuffer());
	}
};

}
}
