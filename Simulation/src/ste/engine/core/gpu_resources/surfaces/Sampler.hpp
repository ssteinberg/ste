// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gl_current_context.hpp"

#include "bindable_resource.hpp"
#include "layout_binding.hpp"

#include "texture_enums.hpp"
#include "texture_traits.hpp"

#include "gl_type_traits.hpp"

#include <memory>

namespace StE {
namespace Core {

class sampler_layout_binding_type {};
using sampler_layout_binding = layout_binding<sampler_layout_binding_type>;
sampler_layout_binding inline operator "" _sampler_idx(unsigned long long int i) { return sampler_layout_binding(i); }

class sampler_allocator : public generic_resource_allocator {
public:
	generic_resource::type allocate() override final {
		GLuint id;
		glCreateSamplers(1, &id);
		return id;
	}
	static void deallocate(generic_resource::type &id) {
		if (id) {
			glDeleteSamplers(1, &id);
			id = 0;
		}
	}
};

class sampler_binder {
public:
	static void bind(generic_resource::type id, const sampler_layout_binding &sampler) {
		GL::gl_current_context::get()->bind_sampler(sampler.binding_index(), id);
	}
	static void unbind(const sampler_layout_binding &sampler) {
		GL::gl_current_context::get()->bind_sampler(sampler.binding_index(), 0);
	}
};


struct sampler_descriptor {
	texture_filtering mag_filter{ texture_filtering::Linear };
	texture_filtering min_filter{ texture_filtering::Nearest };
	texture_filtering mipmap_filter{ texture_filtering::None };
	texture_wrap_mode wrap_s{ texture_wrap_mode::Wrap };
	texture_wrap_mode wrap_t{ texture_wrap_mode::Wrap };
	texture_wrap_mode wrap_r{ texture_wrap_mode::Wrap };
	texture_compare_mode compare_mode{ texture_compare_mode::None };
	texture_compare_func compare_func;
	float anisotropy{ 1.f };

	GLenum min_mipmapping_filter() {
		if (mipmap_filter == texture_filtering::None) return static_cast<GLenum>(min_filter);
		if (mipmap_filter == texture_filtering::Nearest && min_filter == texture_filtering::Nearest)	return GL_NEAREST_MIPMAP_NEAREST;
		if (mipmap_filter == texture_filtering::Linear && min_filter == texture_filtering::Nearest)	return GL_NEAREST_MIPMAP_LINEAR;
		if (mipmap_filter == texture_filtering::Nearest && min_filter == texture_filtering::Linear)	return GL_LINEAR_MIPMAP_NEAREST;
		return GL_LINEAR_MIPMAP_LINEAR;
	}

	sampler_descriptor() {}
	sampler_descriptor(texture_filtering mag_filter, texture_filtering min_filter) : mag_filter(mag_filter), min_filter(min_filter) {}
	sampler_descriptor(texture_filtering mag_filter, texture_filtering min_filter, float anisotropic_filtering_max) : mag_filter(mag_filter), min_filter(min_filter), anisotropy(anisotropic_filtering_max) {}
	sampler_descriptor(texture_wrap_mode wrap_s, texture_wrap_mode wrap_t) : wrap_s(wrap_s), wrap_t(wrap_t) {}
	sampler_descriptor(texture_wrap_mode wrap_s, texture_wrap_mode wrap_t, texture_wrap_mode wrap_r) : wrap_s(wrap_s), wrap_t(wrap_r), wrap_r(wrap_r) {}
	sampler_descriptor(texture_filtering mag_filter, texture_filtering min_filter, texture_wrap_mode wrap_s, texture_wrap_mode wrap_t) : mag_filter(mag_filter), min_filter(min_filter), wrap_s(wrap_s), wrap_t(wrap_r) {}
};

class sampler : public bindable_resource<sampler_allocator, sampler_binder, sampler_layout_binding>,
				public shader_layout_bindable_resource<sampler_layout_binding_type> {
private:
	using Base = bindable_resource<sampler_allocator, sampler_binder, sampler_layout_binding>;

protected:
	sampler_descriptor descriptor;

public:
	sampler(sampler &&m) = default;
	sampler& operator=(sampler &&m) = default;
	sampler(const sampler &m) = delete;
	sampler& operator=(const sampler &m) = delete;

	sampler() {}
	sampler(texture_filtering mag_filter, texture_filtering min_filter) {
		set_mag_filter(mag_filter);
		set_min_filter(min_filter);
	}
	sampler(texture_filtering mag_filter, texture_filtering min_filter, float anisotropic_filtering_max) {
		set_mag_filter(mag_filter);
		set_min_filter(min_filter);
		set_anisotropic_filter(anisotropic_filtering_max);
	}
	sampler(texture_wrap_mode wrap_s, texture_wrap_mode wrap_t) {
		set_wrap_s(wrap_s);
		set_wrap_t(wrap_t);
	}
	sampler(texture_wrap_mode wrap_s, texture_wrap_mode wrap_t, texture_wrap_mode wrap_r) {
		set_wrap_s(wrap_s);
		set_wrap_t(wrap_t);
		set_wrap_r(wrap_r);
	}
	sampler(texture_filtering mag_filter, texture_filtering min_filter, texture_wrap_mode wrap_s, texture_wrap_mode wrap_t) {
		set_mag_filter(mag_filter);
		set_min_filter(min_filter);
		set_wrap_s(wrap_s);
		set_wrap_t(wrap_t);
	}
	sampler(texture_filtering mag_filter, texture_filtering min_filter, texture_wrap_mode wrap_s, texture_wrap_mode wrap_t, float anisotropic_filtering_max) {
		set_mag_filter(mag_filter);
		set_min_filter(min_filter);
		set_wrap_s(wrap_s);
		set_wrap_t(wrap_t);
		set_anisotropic_filter(anisotropic_filtering_max);
	}

	void bind() const { bind(LayoutLocationType(0)); }
	void unbind() const { unbind(LayoutLocationType(0)); }
	void bind(const LayoutLocationType &sampler) const final override { Base::bind(sampler); };
	void unbind(const LayoutLocationType &sampler) const final override { Base::unbind(sampler); };

	virtual void set_mag_filter(texture_filtering filter) {
		descriptor.mag_filter = filter;
		if (filter!= texture_filtering::None)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(filter));
	}
	virtual void set_min_filter(texture_filtering filter) {
		descriptor.min_filter = filter;
		if (filter != texture_filtering::None)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(filter));
	}
	virtual void set_anisotropic_filter(float anis) { descriptor.anisotropy = std::max(1.0f,std::min(16.0f,anis)); glSamplerParameterf(get_resource_id(), GL_TEXTURE_MAX_ANISOTROPY_EXT, descriptor.anisotropy); }
	auto get_mag_filter() const { return descriptor.mag_filter; }
	auto get_min_filter() const { return descriptor.min_filter; }
	float get_anisotropic_filter_max() const { return descriptor.anisotropy; }

	void set_wrap_s(texture_wrap_mode wrap_mode) {
		descriptor.wrap_s = wrap_mode;
		if (wrap_mode != texture_wrap_mode::None)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrap_mode));
	}
	void set_wrap_t(texture_wrap_mode wrap_mode) {
		descriptor.wrap_t = wrap_mode;
		if (wrap_mode != texture_wrap_mode::None)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrap_mode));
	}
	void set_wrap_r(texture_wrap_mode wrap_mode) {
		descriptor.wrap_r = wrap_mode;
		if (wrap_mode != texture_wrap_mode::None)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_WRAP_R, static_cast<GLenum>(wrap_mode));
	}
	auto get_wrap_s() const { return descriptor.wrap_s; }
	auto get_wrap_t() const { return descriptor.wrap_t; }
	auto get_wrap_r() const { return descriptor.wrap_r; }

	void set_compare_mode(texture_compare_mode mode) { descriptor.compare_mode = mode; glSamplerParameterf(get_resource_id(), GL_TEXTURE_COMPARE_MODE, static_cast<GLenum>(mode)); }
	void set_compare_func(texture_compare_func func) { descriptor.compare_func = func; glSamplerParameterf(get_resource_id(), GL_TEXTURE_COMPARE_FUNC, static_cast<GLenum>(func)); }
	auto get_compare_mode() const { return descriptor.compare_mode; }
	auto get_compare_func() const { return descriptor.compare_func; }

	void set_border_color(const glm::vec4 &c) const {
		glSamplerParameterfv(get_resource_id(), GL_TEXTURE_BORDER_COLOR, &c[0]);
	}
	void set_border_color(const glm::ivec4 &c) const {
		glSamplerParameterIiv(get_resource_id(), GL_TEXTURE_BORDER_COLOR, &c[0]);
	}
	void set_border_color(const glm::uvec4 &c) const {
		glSamplerParameterIuiv(get_resource_id(), GL_TEXTURE_BORDER_COLOR, &c[0]);
	}
	void set_border_color(float c) const {
		glSamplerParameterf(get_resource_id(), GL_TEXTURE_BORDER_COLOR, c);
	}
	void set_border_color(int c) const {
		glSamplerParameteri(get_resource_id(), GL_TEXTURE_BORDER_COLOR, c);
	}

	core_resource_type resource_type() const override { return core_resource_type::SamplingDescriptor; }


private:
	static std::unique_ptr<sampler> _sampler_nearest;
	static std::unique_ptr<sampler> _sampler_linear;
	static std::unique_ptr<sampler> _sampler_anisotropic_linear;
	static std::unique_ptr<sampler> _sampler_nearest_clamp;
	static std::unique_ptr<sampler> _sampler_linear_clamp;
	static std::unique_ptr<sampler> _sampler_anisotropic_linear_clamp;

public:
	static sampler *sampler_nearest();
	static sampler *sampler_linear();
	static sampler *sampler_anisotropic_linear();
	static sampler *sampler_nearest_clamp();
	static sampler *sampler_linear_clamp();
	static sampler *sampler_anisotropic_linear_clamp();
};

class sampler_mipmapped : public sampler {
private:
	using Base = sampler;

public:
	using Base::sampler;
	sampler_mipmapped() : Base() { descriptor.mipmap_filter = texture_filtering::Linear; }
	sampler_mipmapped(texture_filtering mag_filter, texture_filtering min_filter, texture_filtering mipmap_filter) : sampler_mipmapped() {
		set_mag_filter(mag_filter);
		set_min_filter(min_filter);
		set_mipmap_filter(mipmap_filter);
	}
	sampler_mipmapped(texture_filtering mag_filter, texture_filtering min_filter, texture_filtering mipmap_filter,
					 texture_wrap_mode wrap_s, texture_wrap_mode wrap_t) : sampler_mipmapped(mag_filter, min_filter, mipmap_filter) {
		set_wrap_s(wrap_s);
		set_wrap_t(wrap_t);
	}

	void set_min_filter(texture_filtering filter) override final {
		descriptor.min_filter = filter;
		auto f = descriptor.min_mipmapping_filter();
		if (f)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_MIN_FILTER, f);
	}
	void set_mipmap_filter(texture_filtering filter) {
		descriptor.mipmap_filter = filter;
		auto f = descriptor.min_mipmapping_filter();
		if (f)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_MIN_FILTER, f);
	}
	texture_filtering get_mipmap_filter() const { return descriptor.mipmap_filter; }


private:
	static std::unique_ptr<sampler_mipmapped> _mipmapped_sampler_nearest;
	static std::unique_ptr<sampler_mipmapped> _mipmapped_sampler_linear;
	static std::unique_ptr<sampler_mipmapped> _mipmapped_sampler_anisotropic_linear;
	static std::unique_ptr<sampler_mipmapped> _mipmapped_sampler_nearest_clamp;
	static std::unique_ptr<sampler_mipmapped> _mipmapped_sampler_linear_clamp;
	static std::unique_ptr<sampler_mipmapped> _mipmapped_sampler_anisotropic_linear_clamp;

public:
	static sampler_mipmapped *mipmapped_sampler_nearest();
	static sampler_mipmapped *mipmapped_sampler_linear();
	static sampler_mipmapped *mipmapped_sampler_anisotropic_linear();
	static sampler_mipmapped *mipmapped_sampler_nearest_clamp();
	static sampler_mipmapped *mipmapped_sampler_linear_clamp();
	static sampler_mipmapped *mipmapped_sampler_anisotropic_linear_clamp();
};

}
}
