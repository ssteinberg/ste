// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <glyph.hpp>
#include <ste_context.hpp>
#include <glyph_factory.hpp>
#include <font.hpp>
#include <format.hpp>

#include <surface_factory.hpp>

#include <device_buffer.hpp>
#include <device_image.hpp>
#include <stable_vector.hpp>
#include <sampler.hpp>
#include <std430.hpp>

#include <exception>

#include <optional.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

#include <command_recorder.hpp>

namespace ste {
namespace text {

class glyph_manager {
private:
	struct buffer_glyph_descriptor : gl::std430<std::uint32_t, std::uint32_t, std::int32_t, std::int32_t, std::uint32_t> {
		const glyph::glyph_metrics& metrics() const {
			glyph::glyph_metrics m;
			m.width = get<0>();
			m.height = get<1>();
			m.start_y = get<2>();
			m.start_x = get<3>();

			return m;
		}
		const auto& glyph_index() const { return get<4>(); }

		buffer_glyph_descriptor() = default;
		buffer_glyph_descriptor(const glyph::glyph_metrics& metrics,
								std::uint32_t glyph_index) {
			get<0>() = metrics.width;
			get<1>() = metrics.height;
			get<2>() = metrics.start_y;
			get<3>() = metrics.start_x;

			get<4>() = glyph_index;
		}
	};

public:
	struct glyph_descriptor {
		glyph::glyph_metrics metrics;
		int advance_x;
		int buffer_index;
	};

	struct glyph_texture {
		gl::device_image<2> texture;
		gl::image_view<gl::image_type::image_2d> view;

		glyph_texture() = delete;
		glyph_texture(glyph_texture&&) = default;
		glyph_texture &operator=(glyph_texture&&) = default;
	};

	struct font_storage {
		std::unordered_map<wchar_t, glyph_descriptor> glyphs;
	};

private:
	std::reference_wrapper<const ste_context> context;
	glyph_factory factory;

	std::unordered_map<font, font_storage> fonts;

	std::vector<buffer_glyph_descriptor> pending_glyphs;

	gl::stable_vector<buffer_glyph_descriptor> buffer;
	std::vector<glyph_texture> glyph_textures;
	gl::sampler text_glyph_sampler;

	std::mutex m;

private:
	const glyph_descriptor* glyph_loader(const font &font, wchar_t codepoint) {
		std::string cache_key = std::string("ttfdf") + font.get_path().string() + std::to_string(static_cast<std::uint32_t>(codepoint));

		optional<glyph> og;
		try {
			og = context.get().engine().cache().get<glyph>(cache_key)();
		}
		catch (const std::exception &) {
			og = none;
		}

		if (!og) {
			glyph g = factory.create_glyph(font, codepoint);
			if (g.empty())
				return nullptr;

			glyph copy_g = g;
			context.get().engine().cache().insert<glyph>(cache_key, std::move(copy_g));

			og = std::move(g);
		}

		if (og.get().glyph_distance_field == nullptr) {
			return nullptr;
		}
		auto image = resource::surface_factory::create_image_2d<gl::format::r32_sfloat>(context.get(),
																						std::move(*og.get().glyph_distance_field),
																						gl::image_usage::sampled,
																						gl::image_layout::shader_read_only_optimal,
																						false);
		auto view = gl::image_view<gl::image_type::image_2d>(*image);

		{
			std::unique_lock<std::mutex> l(this->m);

			auto index = glyph_textures.size();
			glyph_textures.push_back(glyph_texture{ std::move(image.get()), std::move(view) });

			glyph_descriptor gd;
			gd.metrics = og.get().metrics;
			gd.buffer_index = index;

			buffer_glyph_descriptor bgd(og.get().metrics, index);

			pending_glyphs.push_back(bgd);

			auto &gd_ref = this->fonts[font].glyphs[codepoint];
			gd_ref = std::move(gd);
			return &gd_ref;
		}
	}

public:
	glyph_manager(const ste_context &context)
		: context(context), 
		buffer(context, gl::buffer_usage::storage_buffer),
		text_glyph_sampler(context.device(),
						   gl::sampler_parameter::filtering(gl::sampler_filter::linear, 
															gl::sampler_filter::linear),
						   gl::sampler_parameter::address_mode(gl::sampler_address_mode::clamp_to_border, 
															   gl::sampler_address_mode::clamp_to_border),
						   gl::sampler_parameter::anisotropy(16.f))
	{}

	optional<const glyph_descriptor*> glyph_for_font(const font &font, wchar_t codepoint) {
		// Find , or create if none, glyph storage for requested font
		auto it = this->fonts.find(font);
		if (it == this->fonts.end())
			it = this->fonts.emplace_hint(it, std::make_pair(font, font_storage()));

		// Find glyph in storage
		// If not found, enqueue glyph creation and return none.
		auto glyphit = it->second.glyphs.find(codepoint);
		if (glyphit == it->second.glyphs.end()) {
			context.get().engine().task_scheduler().schedule_now([=]() {
				glyph_loader(font, codepoint);
			});

			return none;
		}

		return &glyphit->second;
	}

	int spacing(const font &font, const std::pair<wchar_t, wchar_t> &chars, int pixel_size) {
		return factory.read_kerning(font, chars, pixel_size);
	}

	range<> update_pending_glyphs(gl::command_recorder &recorder) {
		if (!pending_glyphs.size()) {
			// Nothing to update
			return { 0,0 };
		}

		range<> ret;
		ret.start = buffer.size();
		ret.length = pending_glyphs.size();

		// Update
		recorder << buffer.push_back_cmd(pending_glyphs);
		pending_glyphs.clear();

		return ret;
	}
	
	auto enqueue_glyphs_load(const font &font, std::vector<wchar_t> codepoints) {
		return context.get().engine().task_scheduler().schedule_now([=]() {
			for (wchar_t codepoint : codepoints) {
				this->glyph_loader(font, codepoint);
			}
		});
	}

	auto &textures() const { return glyph_textures; }
	auto &sampler() const { return text_glyph_sampler; }
	auto &ssbo() const { return buffer; }
};

}
}
