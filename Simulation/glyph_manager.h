// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "glyph.h"
#include "glyph_factory.h"
#include "Font.h"

#include "StEngineControl.h"
#include "task.h"
#include "optional.h"

#include "ShaderStorageBuffer.h"

#include "Texture2D.h"

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
		LLR::texture_handle handle;
	};

	using buffer_type = LLR::ShaderStorageBuffer<buffer_glyph_descriptor, static_cast<LLR::BufferUsage::buffer_usage>(LLR::BufferUsage::BufferUsageDynamic | LLR::BufferUsage::BufferUsageSparse)>;

public:
	struct glyph_descriptor {
		std::unique_ptr<LLR::Texture2D> texture;
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
	std::unique_ptr<buffer_type> buffer;
	int buffer_offset{ 0 };

	LLR::Sampler text_glyph_sampler;

private:
	task<const glyph_descriptor*> glyph_loader_task(const Font &font, wchar_t codepoint) {
		return task<glyph>([=](optional<task_scheduler*> sched) -> glyph {
			std::string cache_key = std::string("ttfdf") + font.get_path().string() + std::to_string(static_cast<std::uint32_t>(codepoint));

			optional<glyph> og = none;
			try {
				og = context.cache().get<glyph>(cache_key)();
			}
			catch (std::exception ex) {
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
			gd.texture = std::make_unique<LLR::Texture2D>(g.glyph_distance_field);
			gd.metrics = g.metrics;
			gd.buffer_index = buffer_offset;

			buffer_glyph_descriptor bgd;
			bgd.metrics = g.metrics;
			bgd.handle = gd.texture->get_texture_handle(this->text_glyph_sampler);
			bgd.handle.make_resident();

			buffer->commit_range(buffer_offset, 1);
			buffer->upload(buffer_offset++, 1, &bgd);

			auto &gd_ref = this->fonts[font].glyphs[codepoint];
			gd_ref = std::move(gd);
			return &gd_ref;
		});
	}

public:
	glyph_manager(const StEngineControl &context) : context(context) {
		int page_size = std::max(65536, buffer_type::page_size());
		buffer = std::make_unique<buffer_type>(8 * page_size);

		text_glyph_sampler.set_min_filter(LLR::TextureFiltering::Linear);
		text_glyph_sampler.set_mag_filter(LLR::TextureFiltering::Linear);
		text_glyph_sampler.set_wrap_s(LLR::TextureWrapMode::ClampToBorder);
		text_glyph_sampler.set_wrap_t(LLR::TextureWrapMode::ClampToBorder);
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

	int spacing(const Font &font, wchar_t left, wchar_t right, int pixel_size) {
		return factory.spacing(font, left, right, pixel_size);
	}

	task<void> preload_glyphs_task(const Font &font, std::vector<wchar_t> codepoints) {
		return [=](optional<task_scheduler*> sched) {
			std::vector<std::future<const glyph_descriptor*>> futures;
			for (wchar_t codepoint : codepoints) {
				auto codepoint_task = this->glyph_loader_task(font, codepoint);
				sched ? 
					futures.push_back(sched->schedule_now(std::move(codepoint_task))) : 
					codepoint_task();
			}

			for (auto &f : futures)
				f.wait();
		};
	}

	buffer_type &ssbo() { return *buffer; }
};

}
}
