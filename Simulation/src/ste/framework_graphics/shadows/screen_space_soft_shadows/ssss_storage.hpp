// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "signal.hpp"

#include "Texture2DArray.hpp"
#include "FramebufferObject.hpp"

namespace StE {
namespace Graphics {

class ssss_storage {
private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

private:
	std::unique_ptr<Core::Texture2DArray> penumbra_layers;

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

public:
	ssss_storage(const StEngineControl &ctx) {
		resize_connection = std::make_shared<ResizeSignalConnectionType>([this](const glm::i32vec2 &size) {
			resize(size, this->layers_count());
		});
		// ctx.signal_framebuffer_resize().connect(resize_connection);

		resize(ctx.get_backbuffer_size(), 2);
	}

	void resize(const glm::uvec2 &size, int layers) {
		penumbra_layers = std::make_unique<Core::Texture2DArray>(gli::format::FORMAT_R16_SFLOAT_PACK16, glm::ivec3{ glm::floorPowerOfTwo(size.x), glm::floorPowerOfTwo(size.y), layers });
	}
	int layers_count() const { return penumbra_layers->get_layers(); }
	glm::uvec2 layers_size() const { return { penumbra_layers->get_size().x, penumbra_layers->get_size().y }; }

	auto* get_penumbra_layers() const { return penumbra_layers.get(); }
};

}
}
