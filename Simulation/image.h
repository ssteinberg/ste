// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "bindable_resource.h"
#include "layout_binding.h"
#include "llr_resource_type.h"
#include "texture_enums.h"

#include "RenderTarget.h"

namespace StE {
namespace LLR {

class image_layout_binding_type {};
using image_layout_binding = layout_binding<image_layout_binding_type>;
image_layout_binding inline operator "" _image_idx(unsigned long long int i) { return image_layout_binding(i); }

class ImageBinder {
public:
	static void bind(unsigned int id, const image_layout_binding &unit, int level, bool layered, int layer, ImageAccessMode access, gli::format format) {
		glBindImageTexture(unit, id, level, layered, layer, static_cast<GLenum>(access), opengl::gl_translate_format(format).Internal);
	}
	static void unbind(const image_layout_binding &unit, int level, bool layered, int layer, ImageAccessMode access, gli::format format) {
		glBindImageTexture(unit, 0, 0, 0, 0, 0, 0);
	}
};

template <llr_resource_type type>
class image_layout_bindable : protected bindable_resource<llr_resource_stub_allocator, ImageBinder, image_layout_binding, int, bool, int, ImageAccessMode, gli::format>, virtual public shader_layout_bindable_resource<image_layout_binding_type> {
public:
	using size_type = glm::tvec2<std::size_t>;

private:
	using Base = bindable_resource<llr_resource_stub_allocator, ImageBinder, image_layout_binding, int, bool, int, ImageAccessMode, gli::format>;

protected:
	mutable std::uint64_t image_handle{ 0 };
	size_type size;
	gli::format format;
	ImageAccessMode access;
	llr_resource_type texture_type;
	int level, layers, layer;

	image_layout_bindable(int tex_id, const size_type &size, gli::format format, ImageAccessMode access, int level, int layers, int layer) : size(size), format(format), access(access), level(level), layers(layers), layer(layer) {
		id = tex_id;
	}

public:
	image_layout_bindable(image_layout_bindable &&m) = default;
	image_layout_bindable(const image_layout_bindable &c) = delete;
	image_layout_bindable& operator=(image_layout_bindable &&m) = delete;
	image_layout_bindable& operator=(const image_layout_bindable &c) = delete;

	virtual ~image_layout_bindable() noexcept {}

	using Base::get_resource_id;

	void bind(const LayoutLocationType &binding) const override final { Binder::bind(id, binding, level, layers>1?true:false, layer, access, format); }
	void unbind(const LayoutLocationType &binding) const override final { Binder::unbind(binding, 0, false, 0, access, format); }

	auto get_image_handle() const {
		return image_handle ? image_handle : (image_handle = glGetImageHandleARB(id, level, layers > 1 ? true : false, layer, format));
	}
	void make_resident() const { glMakeImageHandleResidentARB(get_image_handle()); }
	void make_nonresident() const { glMakeImageHandleNonResidentARB(get_image_handle()); }
	bool is_resident() const { return glIsImageHandleResidentARB(get_image_handle()); }

	void set_access(ImageAccessMode access) { this->access = access; }
	int get_layers() const { return layers; }
	int get_level() const { return level; }
	virtual gli::format get_format() const { return format; }
	virtual size_type get_image_size() const { return size; }
	llr_resource_type get_texture_type() const { return texture_type; }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRImageObject; }
};

template <llr_resource_type type>
class image : public image_layout_bindable<type>, virtual public RenderTargetGeneric {
private:
	friend class image_container<type>;

	using Base = image_layout_bindable;

public:
	image(int tex_id, const size_type &size, gli::format format, ImageAccessMode access, int level, int layer) : Base(tex_id, size, format, access, level, 1, layer) {}

	image(image &&m) = default;
	image(const image &c) = delete;
	image& operator=(image &&m) = delete;
	image& operator=(const image &c) = delete;

	int get_layer() const { return layer; }

	gli::format get_format() const override final { return format; }
	size_type get_image_size() const final override { return size; }

	image<type> with_format(gli::format format) const { return image(id, size, format, access, level, layer); }
	image<type> with_access(ImageAccessMode access) const { return image(id, size, format, access, level, layer); }
};

template <llr_resource_type type>
class image_container : public image_layout_bindable<type> {
private:
	using Base = image_layout_bindable;

public:
	image_container(int tex_id, const size_type &size, gli::format format, ImageAccessMode access, int level, int layers) : Base(tex_id, size, format, access, level, layers, 0) {}

	image_container(image_container &&m) = default;
	image_container(const image_container &c) = delete;
	image_container& operator=(image_container &&m) = delete;
	image_container& operator=(const image_container &c) = delete;

	const image<type> operator[](int layer) const { return image<type>(id, size, format, access, level, layer); }

	image_container<type> with_format(gli::format format) const { return image_container(id, size, format, access, level, layers); }
	image_container<type> with_access(ImageAccessMode access) const { return image_container(id, size, format, access, level, layers); }
};

}
}
