// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <glyph.hpp>
#include <ste_context.hpp>
#include <glyph_factory.hpp>
#include <font.hpp>
#include <format.hpp>

#include <device_buffer.hpp>
#include <device_image.hpp>
#include <vk_sampler.hpp>
#include <stable_vector.hpp>

#include <exception>

#include <optional.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <command_recorder.hpp>

namespace ste {
namespace text {

class glyph_manager {
private:
	struct buffer_glyph_descriptor {
		glyph::glyph_metrics metrics;
		std::uint32_t glyph_index;
	};

public:
	struct glyph_descriptor {
		glyph::glyph_metrics metrics;
		int advance_x;
		int buffer_index;
	};

	struct glyph_texture {
		gl::device_image<2> texture;
		gl::vk::vk_image_view<gl::image_type::image_2d> view;

		glyph_texture() = delete;
		glyph_texture(glyph_texture&&) = default;
		glyph_texture &operator=(glyph_texture&&) = default;
	};

	struct font_storage {
		std::unordered_map<wchar_t, glyph_descriptor> glyphs;
	};

private:
	const ste_context &context;
	glyph_factory factory;

	std::unordered_map<font, font_storage> fonts;

	std::vector<buffer_glyph_descriptor> pending_glyphs;

	gl::stable_vector<buffer_glyph_descriptor> buffer;
	std::vector<glyph_texture> glyph_textures;
	gl::sampler text_glyph_sampler;

private:
	const glyph_descriptor* glyph_loader(const font &font, wchar_t codepoint) {
		std::string cache_key = std::string("ttfdf") + font.get_path().string() + std::to_string(static_cast<std::uint32_t>(codepoint));

		optional<glyph> og;
		try {
			og = context.engine().cache().get<glyph>(cache_key)();
		}
		catch (const std::exception &) {
			og = none;
		}

		if (!og) {
			glyph g = factory.create_glyph(font, codepoint);
			if (g.empty())
				return nullptr;

			glyph copy_g = g;
			context.engine().cache().insert<glyph>(cache_key, std::move(copy_g));

			og = std::move(g);
		}

		auto index = glyph_textures.size();

		if (og.get().glyph_distance_field == nullptr) {
			return nullptr;
		}
		auto image = gl::device_image<2>::create_image_2d<gl::format::r32_sfloat>(context,
																				std::move(*og.get().glyph_distance_field),
																				  gl::image_usage::sampled,
																				false);
		auto view = gl::vk::vk_image_view<gl::image_type::image_2d>(*image, image->get_format());
		glyph_textures.push_back(glyph_texture{ std::move(image), std::move(view) });

		glyph_descriptor gd;
		gd.metrics = og.get().metrics;
		gd.buffer_index = index;

		buffer_glyph_descriptor bgd;
		bgd.metrics = og.get().metrics;
		bgd.glyph_index = index;

		pending_glyphs.push_back(bgd);

		auto &gd_ref = this->fonts[font].glyphs[codepoint];
		gd_ref = std::move(gd);
		return &gd_ref;
	}

public:
	glyph_manager(const ste_context &context)
		: context(context), 
		buffer(context, gl::buffer_usage::storage_buffer),
		text_glyph_sampler(context,
						   gl::sampler_parameter::filtering(gl::sampler_filter::linear, 
															gl::sampler_filter::linear),
						   gl::sampler_parameter::address_mode(gl::sampler_address_mode::clamp_to_border, 
															   gl::sampler_address_mode::clamp_to_border),
						   gl::sampler_parameter::anisotropy(16.f))
	{}

	const glyph_descriptor* glyph_for_font(const font &font, wchar_t codepoint) {
		auto it = this->fonts.find(font);
		if (it == this->fonts.end())
			it = this->fonts.emplace(std::make_pair(font, font_storage())).first;

		auto glyphit = it->second.glyphs.find(codepoint);
		if (glyphit == it->second.glyphs.end()) {
			auto *gd = glyph_loader(font, codepoint);
			return gd;
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

	task_future<void> preload_glyphs_async(task_scheduler *sched, const font &font, std::vector<wchar_t> codepoints) {
		return sched->schedule_now([=]() {
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
