// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "gl_utils.hpp"
#include "gl_current_context.hpp"

#include "bindable_resource.hpp"
#include "layout_binding.hpp"
#include "llr_resource_type.hpp"
#include "texture_enums.hpp"
#include "surface_constants.hpp"

#include "image_handle.hpp"

#include "RenderTarget.hpp"

namespace StE {
namespace LLR {
    
template <llr_resource_type type>
class image_container;

class image_layout_binding_type {};
using image_layout_binding = layout_binding<image_layout_binding_type>;
image_layout_binding inline operator "" _image_idx(unsigned long long int i) { return image_layout_binding(i); }

class image_dummy_resource_allocator : public generic_resource_allocator {
public:
	static bool is_valid(GenericResource::type id) { return false; }
	GenericResource::type allocate() override final { return 0; };
	static void deallocate(GenericResource::type &id) { id = 0; }
};

class ImageBinder {
public:
	static void bind(GenericResource::type id, const image_layout_binding &unit, int level, bool layered, int layer, ImageAccessMode access, gli::format format) {
		auto swizzle = swizzles_rgba;
		gl_current_context::get()->bind_image_texture(unit, id, level, layered, layer, static_cast<GLenum>(access), gl_utils::translate_format(format, swizzle).Internal);
	}
	static void unbind(const image_layout_binding &unit, int level, bool layered, int layer, ImageAccessMode access, gli::format format) {
		gl_current_context::get()->bind_image_texture(unit, 0, 0, 0, 0, 0, 0);
	}
};

template <llr_resource_type type>
class image_layout_bindable : protected bindable_resource<image_dummy_resource_allocator, ImageBinder, image_layout_binding, int, bool, int, ImageAccessMode, gli::format>, virtual public shader_layout_bindable_resource<image_layout_binding_type> {
public:
	using size_type = glm::ivec2;

private:
	using Base = bindable_resource<image_dummy_resource_allocator, ImageBinder, image_layout_binding, int, bool, int, ImageAccessMode, gli::format>;

protected:
	size_type size;
	gli::format format;
	ImageAccessMode access;
	llr_resource_type texture_type;
	int level, layers, layer;

	template <class A2>
	image_layout_bindable(const resource<A2> &res, const size_type &size, gli::format format, ImageAccessMode access, int level, int layers, int layer) : Base(res), size(size), format(format), access(access), level(level), layers(layers), layer(layer) {}

public:
	image_layout_bindable(image_layout_bindable &&m) = default;
	image_layout_bindable(const image_layout_bindable &c) = delete;
	image_layout_bindable& operator=(image_layout_bindable &&m) = delete;
	image_layout_bindable& operator=(const image_layout_bindable &c) = delete;

	virtual ~image_layout_bindable() noexcept {}

	using Base::get_resource_id;

	void bind(const LayoutLocationType &binding) const override final { Binder::bind(get_resource_id(), binding, level, layers>1?true:false, layer, access, format); }
	void unbind(const LayoutLocationType &binding) const override final { Binder::unbind(binding, 0, false, 0, access, format); }

	auto get_image_handle() const {
		auto swizzle = swizzles_rgba;
		return image_handle(glGetImageHandleARB(get_resource_id(), level, layers > 1 ? true : false, layer, gl_utils::translate_format(format, swizzle).Internal), access);
	}

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
public:
	using size_type = glm::ivec2;
    
private:
	friend class image_container<type>;

	using Base = image_layout_bindable<type>;

public:
	template <class A2>
	image(const resource<A2> &res, const size_type &size, gli::format format, ImageAccessMode access, int level, int layer) : Base(res, size, format, access, level, 1, layer) {}

	image(image &&m) = default;
	image(const image &c) = delete;
	image& operator=(image &&m) = delete;
	image& operator=(const image &c) = delete;

	int get_layer() const { return Base::layer; }

	gli::format get_format() const override final { return Base::format; }
	size_type get_image_size() const final override { return Base::size; }

	image<type> with_format(gli::format format) const { return image(*this, size, format, Base::access, Base::level, Base::layer); }
	image<type> with_access(ImageAccessMode access) const { return image(*this, size, Base::format, access, Base::level, Base::layer); }
};

template <llr_resource_type type>
class image_container : public image_layout_bindable<type> {
public:
	using size_type = glm::ivec2;
    
private:
	using Base = image_layout_bindable<type>;

public:
	template <class A2>
	image_container(const resource<A2> &res, const size_type &size, gli::format format, ImageAccessMode access, int level, int layers) : Base(res, size, format, access, level, layers, 0) {}

	image_container(image_container &&m) = default;
	image_container(const image_container &c) = delete;
	image_container& operator=(image_container &&m) = delete;
	image_container& operator=(const image_container &c) = delete;

	const image<type> operator[](int layer) const { return image<type>(*this, Base::size, Base::format, Base::access, Base::level, layer); }

	image_container<type> with_format(gli::format format) const { return image_container(*this, Base::size, format, Base::access, Base::level, Base::layers); }
	image_container<type> with_access(ImageAccessMode access) const { return image_container(*this, Base::size, Base::format, access, Base::level, Base::layers); }
};

}
}
