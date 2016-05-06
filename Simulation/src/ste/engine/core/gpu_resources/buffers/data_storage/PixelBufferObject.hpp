// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "buffer_object.hpp"

#include "image.hpp"
#include "texture_base.hpp"
#include "FramebufferObject.hpp"

#include <vector>
#include <functional>

namespace StE {
namespace Core {

template <typename Type, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class PixelBufferObject : public buffer_object<Type, U> {
private:
	using Base = buffer_object<Type, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

private:
	using buffer_object<Type, U>::bind;
	using buffer_object<Type, U>::unbind;

	void bind_pack() const { Base::Binder::bind(Base::get_resource_id(), GL_PIXEL_PACK_BUFFER); }
	void unbind_pack() const { Base::Binder::unbind(GL_PIXEL_PACK_BUFFER); }
	void bind_unpack() const { Base::Binder::bind(Base::get_resource_id(), GL_PIXEL_UNPACK_BUFFER); }
	void unbind_unpack() const { Base::Binder::unbind(GL_PIXEL_UNPACK_BUFFER); }

	void unpacker(const std::function<void(void)> &unpacker) const {
		bind_unpack();
		unpacker();
		unbind_unpack();
	}
	void packer(const std::function<void(void)> &packer) {
		bind_pack();
		packer();
		unbind_pack();
	}

public:
	PixelBufferObject(PixelBufferObject &&m) = default;
	PixelBufferObject& operator=(PixelBufferObject &&m) = default;

	using Base::Base;

	template <core_resource_type type>
	void unpack_to(texture_pixel_transferable<type> &texture, int level, int layer, int offset, std::size_t size) const {
		unpacker([&]() { texture.upload_level(reinterpret_cast<void*>(offset), level, layer, CubeMapFace::CubeMapFaceNone, size * sizeof(Type)); });
	}
	template <core_resource_type type>
	void unpack_to(texture_pixel_transferable<type> &texture, int level = 0, int layer = 0) const {
		unpack_to(texture, level, layer, 0, Base::buffer_size);
	}

	template <core_resource_type type>
	void pack_from(const texture_pixel_transferable<type> &texture, int level, int layer, int offset, std::size_t size, gli::format format, bool compressed = false) {
		packer([&]() { texture.download_level(reinterpret_cast<void*>(offset), size, level, layer, format, compressed); });
	}
	template <core_resource_type type>
	void pack_from(const texture_pixel_transferable<type> &texture, int level, int layer, int offset, std::size_t size) {
		packer([&]() { texture.download_level(reinterpret_cast<void*>(offset), size, level, layer); });
	}
	template <core_resource_type type>
	void pack_from(const texture_pixel_transferable<type> &texture, int level = 0, int layer = 0) {
		pack_from(texture, level, layer, 0, Base::buffer_size);
	}

	template <core_resource_type type>
	void operator<<(const texture_pixel_transferable<type> &texture) { pack_from(texture); }
	template <core_resource_type type>
	void operator>>(texture_pixel_transferable<type> &texture) const { unpack_to(texture); }


	template<typename A, GLenum t>
	void pack_from(const fbo_color_attachment_point<A, t> &fbo_attachment, int offset, const glm::uvec2 &rect_size, const glm::uvec2 &origin = { 0, 0 }) {
		packer([&]() { fbo_attachment.read_pixels(reinterpret_cast<void*>(offset), Base::buffer_size * sizeof(Type), rect_size, origin); });
	}
	template<typename A, GLenum t>
	void pack_from(const fbo_color_attachment_point<A, t> &fbo_attachment, int offset) {
		packer([&]() { fbo_attachment.read_pixels(reinterpret_cast<void*>(offset), Base::buffer_size * sizeof(Type)); });
	}
	template<typename A, GLenum t>
	void pack_from(const fbo_color_attachment_point<A, t> &fbo_attachment) {
		pack_from(fbo_attachment, 0);
	}

	template<typename A, GLenum t>
	void unpack_to(fbo_color_attachment_point<A, t> &fbo_attachment, int offset, const glm::uvec2 &rect_size, const glm::uvec2 &origin = { 0, 0 }) const {
		unpacker([&]() { fbo_attachment.write_pixels(reinterpret_cast<void*>(offset), rect_size, origin); });
	}
	template<typename A, GLenum t>
	void unpack_to(fbo_color_attachment_point<A, t> &fbo_attachment, int offset) const {
		unpacker([&]() { fbo_attachment.write_pixels(reinterpret_cast<void*>(offset)); });
	}
	template<typename A, GLenum t>
	void unpack_to(fbo_color_attachment_point<A, t> &fbo_attachment) const {
		unpack_to(fbo_attachment, 0);
	}

	template<typename A, GLenum t>
	void operator<<(const fbo_color_attachment_point<A, t> &fbo_attachment) { pack_from(fbo_attachment); }
	template<typename A, GLenum t>
	void operator>>(fbo_color_attachment_point<A, t > &fbo_attachment) const { pack_to(fbo_attachment); }

	core_resource_type resource_type() const override { return core_resource_type::PixelBufferObject; }
};

}
}
