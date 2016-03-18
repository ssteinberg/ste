// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "glyph.hpp"
#include "glyph_factory.hpp"
#include "Font.hpp"

#include "StEngineControl.hpp"
#include "task.hpp"
#include "optional.hpp"

#include "ShaderStorageBuffer.hpp"
#include "gstack.hpp"

#include "Texture2D.hpp"

#include <exception>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace StE {
namespace Text {

class glyph_manager {
private:
	struct buffer_glyph_descriptor {
		glyph::glyph_metrics metrics;
		Core::texture_handle handle;
	};

public:
	struct glyph_descriptor {
		std::unique_ptr<Core::Texture2D> texture;
		glyph::glyph_metrics metrics;
		int advance_x;
		int buffer_index;
	};

	struct font_storage {
		std::unordered_map<wchar_t, glyph_descriptor> glyphs;
	};

private:
	const StEngineControl &context;
	glyph_factory factory;

	std::unordered_map<Font, font_storage> fonts;
	Core::gstack<buffer_glyph_descriptor> buffer;

	Core::Sampler text_glyph_sampler;

private:
	task<const glyph_descriptor*> glyph_loader_task(const Font &font, wchar_t codepoint) {
		return task<glyph>([=](optional<task_scheduler*> sched) -> glyph {
			std::string cache_key = std::string("ttfdf") + font.get_path().string() + std::to_string(static_cast<std::uint32_t>(codepoint));

			optional<glyph> og = none;
			try {
				og = context.cache().get<glyph>(cache_key)();
			}
			catch (const std::exception &ex) {
				og = none;
			}
			if (og)
				return std::move(og.get());

			glyph g = factory.create_glyph_task(font, codepoint)();
			if (g.empty())
				return g;

			glyph retg = g;
			context.cache().insert<glyph>(cache_key, std::move(g));

			return std::move(retg);
		}).then_on_main_thread([=](optional<task_scheduler*> sched, glyph &&g) -> const glyph_descriptor* {
			if (g.empty())
				return nullptr;

			glyph_descriptor gd;
			gd.texture = std::make_unique<Core::Texture2D>(*g.glyph_distance_field);
			gd.metrics = g.metrics;
			gd.buffer_index = buffer.size();

			buffer_glyph_descriptor bgd;
			bgd.metrics = g.metrics;
			bgd.handle = gd.texture->get_texture_handle(this->text_glyph_sampler);
			bgd.handle.make_resident();

			buffer.push_back(bgd);

			auto &gd_ref = this->fonts[font].glyphs[codepoint];
			gd_ref = std::move(gd);
			return &gd_ref;
		});
	}

public:
	glyph_manager(const StEngineControl &context) : context(context) {
		text_glyph_sampler.set_min_filter(Core::TextureFiltering::Linear);
		text_glyph_sampler.set_mag_filter(Core::TextureFiltering::Linear);
		text_glyph_sampler.set_wrap_s(Core::TextureWrapMode::ClampToBorder);
		text_glyph_sampler.set_wrap_t(Core::TextureWrapMode::ClampToBorder);
		text_glyph_sampler.set_anisotropic_filter(16);
	}

	const glyph_descriptor* glyph_for_font(const Font &font, wchar_t codepoint) {
		auto it = this->fonts.find(font);
		if (it == this->fonts.end())
			it = this->fonts.emplace(std::make_pair(font, font_storage())).first;

		auto glyphit = it->second.glyphs.find(codepoint);
		if (glyphit == it->second.glyphs.end()) {
			auto *gd = glyph_loader_task(font, codepoint)();
			return gd;
		}

		return &glyphit->second;
	}

	int spacing(const Font &font, const std::pair<wchar_t, wchar_t> &chars, int pixel_size) {
		return factory.read_kerning(font, chars, pixel_size);
	}

	task<void> preload_glyphs_task(const Font &font, std::vector<wchar_t> codepoints) {
		return [=](optional<task_scheduler*> sched) {
			std::vector<std::future<const glyph_descriptor*>> futures;
			for (wchar_t codepoint : codepoints) {
				auto codepoint_task = this->glyph_loader_task(font, codepoint);
				if (sched) 
					futures.push_back(sched->schedule_now(std::move(codepoint_task)));
				else 
					codepoint_task();
			}

			for (auto &f : futures)
				f.wait();
		};
	}

	auto &ssbo() { return buffer.get_buffer(); }
};

}
}
