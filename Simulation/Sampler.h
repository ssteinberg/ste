// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gl_current_context.h"

#include "bindable_resource.h"
#include "layout_binding.h"

#include "texture_enums.h"
#include "texture_traits.h"

namespace StE {
namespace LLR {

class sampler_layout_binding_type {};
using sampler_layout_binding = layout_binding<sampler_layout_binding_type>;
sampler_layout_binding inline operator "" _sampler_idx(unsigned long long int i) { return sampler_layout_binding(i); }

class SamplerAllocator : public generic_resource_allocator {
protected:
	unsigned allocate() override final {
		GLuint id;
		glCreateSamplers(1, &id);
		return id;
	}
	static void deallocate(unsigned &id) {
		if (id)
			glDeleteSamplers(1, &id);
		id = 0;
	}
};

class SamplerBinder {
public:
	static void bind(unsigned int id, const sampler_layout_binding &sampler) {
		gl_current_context::get()->bind_sampler(sampler.binding_index(), id);
	}
	static void unbind(const sampler_layout_binding &sampler) {
		gl_current_context::get()->bind_sampler(sampler.binding_index(), 0);
	}
};

struct sampler_descriptor {
	TextureFiltering mag_filter{ TextureFiltering::Linear };
	TextureFiltering min_filter{ TextureFiltering::Nearest };
	TextureFiltering mipmap_filter{ TextureFiltering::None };
	TextureWrapMode wrap_s{ TextureWrapMode::Wrap };
	TextureWrapMode wrap_t{ TextureWrapMode::Wrap };
	TextureWrapMode wrap_r{ TextureWrapMode::Wrap };
	float anisotropy{ 1.f };

	GLenum min_mipmapping_filter() {
		if (mipmap_filter == TextureFiltering::None) return static_cast<GLenum>(min_filter);
		if (mipmap_filter == TextureFiltering::Nearest && min_filter == TextureFiltering::Nearest)	return GL_NEAREST_MIPMAP_NEAREST;
		if (mipmap_filter == TextureFiltering::Linear && min_filter == TextureFiltering::Nearest)	return GL_NEAREST_MIPMAP_LINEAR;
		if (mipmap_filter == TextureFiltering::Nearest && min_filter == TextureFiltering::Linear)	return GL_LINEAR_MIPMAP_NEAREST;
		return GL_LINEAR_MIPMAP_LINEAR;
	}

	sampler_descriptor() {}
	sampler_descriptor(TextureFiltering mag_filter, TextureFiltering min_filter) : mag_filter(mag_filter), min_filter(min_filter) {}
	sampler_descriptor(TextureFiltering mag_filter, TextureFiltering min_filter, float anisotropic_filtering_max) : mag_filter(mag_filter), min_filter(min_filter), anisotropy(anisotropic_filtering_max) {}
	sampler_descriptor(TextureWrapMode wrap_s, TextureWrapMode wrap_t) : wrap_s(wrap_s), wrap_t(wrap_t) {}
	sampler_descriptor(TextureWrapMode wrap_s, TextureWrapMode wrap_t, TextureWrapMode wrap_r) : wrap_s(wrap_s), wrap_t(wrap_r), wrap_r(wrap_r) {}
	sampler_descriptor(TextureFiltering mag_filter, TextureFiltering min_filter, TextureWrapMode wrap_s, TextureWrapMode wrap_t) : mag_filter(mag_filter), min_filter(min_filter), wrap_s(wrap_s), wrap_t(wrap_r) {}
};

class Sampler : public bindable_resource<SamplerAllocator, SamplerBinder, sampler_layout_binding>,
				public shader_layout_bindable_resource<sampler_layout_binding_type> {
private:
	using Base = bindable_resource<SamplerAllocator, SamplerBinder, sampler_layout_binding>;

protected:
	sampler_descriptor descriptor;

public:
	Sampler(Sampler &&m) = default;
	Sampler& operator=(Sampler &&m) = default;
	Sampler(const Sampler &m) = delete;
	Sampler& operator=(const Sampler &m) = delete;

	Sampler() {}
	Sampler(TextureFiltering mag_filter, TextureFiltering min_filter) {
		set_mag_filter(mag_filter);
		set_min_filter(min_filter);
	}
	Sampler(TextureFiltering mag_filter, TextureFiltering min_filter, float anisotropic_filtering_max) {
		set_mag_filter(mag_filter);
		set_min_filter(min_filter);
		set_anisotropic_filter(anisotropic_filtering_max);
	}
	Sampler(TextureWrapMode wrap_s, TextureWrapMode wrap_t) {
		set_wrap_s(wrap_s);
		set_wrap_t(wrap_t);
	}
	Sampler(TextureWrapMode wrap_s, TextureWrapMode wrap_t, TextureWrapMode wrap_r) {
		set_wrap_s(wrap_s);
		set_wrap_t(wrap_t);
		set_wrap_r(wrap_r);
	}
	Sampler(TextureFiltering mag_filter, TextureFiltering min_filter, TextureWrapMode wrap_s, TextureWrapMode wrap_t) {
		set_mag_filter(mag_filter);
		set_min_filter(min_filter);
		set_wrap_s(wrap_s);
		set_wrap_t(wrap_t);
	}

	void bind() const { bind(LayoutLocationType(0)); }
	void unbind() const { unbind(LayoutLocationType(0)); }
	void bind(const LayoutLocationType &sampler) const final override { Base::bind(sampler); };
	void unbind(const LayoutLocationType &sampler) const final override { Base::unbind(sampler); };

	virtual void set_mag_filter(TextureFiltering filter) { 
		descriptor.mag_filter = filter; 
		if (filter!= TextureFiltering::None) 
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(filter)); 
	}
	virtual void set_min_filter(TextureFiltering filter) {
		descriptor.min_filter = filter;
		if (filter != TextureFiltering::None)  
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(filter));
	}
	virtual void set_anisotropic_filter(float anis) { descriptor.anisotropy = std::max(1.0f,std::min(16.0f,anis)); glSamplerParameterf(get_resource_id(), GL_TEXTURE_MAX_ANISOTROPY_EXT, descriptor.anisotropy); }
	TextureFiltering get_mag_filter() const { return descriptor.mag_filter; }
	TextureFiltering get_min_filter() const { return descriptor.min_filter; }
	float get_anisotropic_filter_max() const { return descriptor.anisotropy; }

	void set_wrap_s(TextureWrapMode wrap_mode) {
		descriptor.wrap_s = wrap_mode;
		if (wrap_mode != TextureWrapMode::None) 
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrap_mode));
	}
	void set_wrap_t(TextureWrapMode wrap_mode) {
		descriptor.wrap_t = wrap_mode;
		if (wrap_mode != TextureWrapMode::None)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrap_mode));
	}
	void set_wrap_r(TextureWrapMode wrap_mode) {
		descriptor.wrap_r = wrap_mode;
		if (wrap_mode != TextureWrapMode::None)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_WRAP_R, static_cast<GLenum>(wrap_mode));
	}
	TextureWrapMode get_wrap_s() const { return descriptor.wrap_s; }
	TextureWrapMode get_wrap_t() const { return descriptor.wrap_t; }
	TextureWrapMode get_wrap_r() const { return descriptor.wrap_r; }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRSamplingDescriptor; }
};

class SamplerMipmapped : public Sampler {
private:
	using Base = Sampler;

public:
	using Base::Base;
	SamplerMipmapped() : Base() { descriptor.mipmap_filter = TextureFiltering::Linear; }
	SamplerMipmapped(TextureFiltering mag_filter, TextureFiltering min_filter, TextureFiltering mipmap_filter) : SamplerMipmapped() {
		set_mag_filter(mag_filter);
		set_min_filter(min_filter);
		set_mipmap_filter(mipmap_filter);
	}

	void set_min_filter(TextureFiltering filter) override final {
		descriptor.min_filter = filter;
		auto f = descriptor.min_mipmapping_filter();
		if (f)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_MIN_FILTER, f);
	}
	void set_mipmap_filter(TextureFiltering filter) {
		descriptor.mipmap_filter = filter;
		auto f = descriptor.min_mipmapping_filter();
		if (f)
			glSamplerParameteri(get_resource_id(), GL_TEXTURE_MIN_FILTER, f);
	}
	TextureFiltering get_mipmap_filter() const { return descriptor.mipmap_filter; }
};

}
}
