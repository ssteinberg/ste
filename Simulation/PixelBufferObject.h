// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.h"

#include <vector>

namespace StE {
namespace LLR {

template <typename Type, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class PixelBufferObject : public buffer_object<Type, U> {
private:
	using Base = buffer_object<Type, U>;

private:
	using buffer_object<Type, U>::bind;
	using buffer_object<Type, U>::unbind;

	void bind_pack() const override { Binder::bind(id, GL_PIXEL_PACK_BUFFER); }
	void unbind_pack() const override { Binder::unbind(GL_PIXEL_PACK_BUFFER); }
	void bind_unpack() const override { Binder::bind(id, GL_PIXEL_UNPACK_BUFFER); }
	void unbind_unpack() const override { Binder::unbind(GL_PIXEL_UNPACK_BUFFER); }

public:
	PixelBufferObject(PixelBufferObject &&m) = default;
	PixelBufferObject& operator=(PixelBufferObject &&m) = default;

	using Base::Base;

	template <llr_resource_type type>
	void unpack_to(texture_pixel_transferable<type> &texture, int level, int layer, int offset, std::size_t size) const {
		bind_unpack();
		texture.upload_level(offset, level, layer, size * sizeof(T));
		unbind_unpack();
	}
	template <llr_resource_type type>
	void unpack_to(texture_pixel_transferable<type> &texture, int level = 0, int layer = 0) const { unpack_to(texture, level, layer, 0, buffer_size); }

	template <llr_resource_type type>
	void pack_from(const texture_pixel_transferable<type> &texture, int level, int layer, int offset, std::size_t size) {
		bind_pack();
		texture.download_level(offset, level, layer, size * sizeof(T));
		unbind_pack();
	}
	template <llr_resource_type type>
	void pack_from(const texture_pixel_transferable<type> &texture, int level = 0, int layer = 0) { pack_from(texture, level, layer, 0, buffer_size); }

	void operator<<(const texture_pixel_transferable<type> &texture) { pack_from(texture); }
	void operator>>(texture_pixel_transferable<type> &texture) const { pack_to(texture); }
};

}
}
