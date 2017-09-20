// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <glyph.hpp>
#include <ste_context.hpp>
#include <glyph_factory.hpp>
#include <font.hpp>
#include <text_glyph_key.hpp>

#include <command_recorder.hpp>

#include <surface_factory.hpp>
#include <device_buffer.hpp>
#include <device_image.hpp>
#include <stable_vector.hpp>
#include <sampler.hpp>
#include <std430.hpp>
#include <format.hpp>

#include <exception>

#include <optional.hpp>
#include <lib/string.hpp>
#include <lib/vector.hpp>
#include <lib/flat_map.hpp>
#include <alias.hpp>
#include <utility>
#include <mutex>

namespace ste {
namespace text {

/**
 *	@brief	Handles glyph storage, loading and generating the glyph buffer and glyph textures.
 */
class glyph_manager {
	friend class text_manager;

private:
	struct buffer_glyph_descriptor : gl::std430<std::uint32_t, std::uint32_t, std::int32_t, std::int32_t, std::uint32_t> {
		const glyph::glyph_metrics &metrics() const {
			glyph::glyph_metrics m;
			m.width = get<0>();
			m.height = get<1>();
			m.start_y = get<2>();
			m.start_x = get<3>();

			return m;
		}

		const auto &glyph_index() const { return get<4>(); }

		buffer_glyph_descriptor() = default;

		buffer_glyph_descriptor(const glyph::glyph_metrics &metrics,
								std::uint32_t glyph_index) {
			get<0>() = metrics.width;
			get<1>() = metrics.height;
			get<2>() = metrics.start_y;
			get<3>() = metrics.start_x;

			get<4>() = glyph_index;
		}
	};

public:
	using glyph_texture = gl::texture<gl::image_type::image_2d>;

	struct glyph_properties {
		glyph::glyph_metrics metrics{};
		int buffer_index;
	};

	struct glyph_loader_data {
		glyph_properties properties{};
		optional<glyph_texture> texture;
		_internal::glyph_key key;
	};

	using glyphs_t = lib::flat_map<_internal::glyph_key, glyph_properties>;
	using glyph_futures_t = lib::flat_map<_internal::glyph_key, std::future<glyph_loader_data>>;

private:
	alias<const ste_context> context;
	glyph_factory factory;

	gl::stable_vector<buffer_glyph_descriptor> buffer;
	gl::sampler text_glyph_sampler;

	mutable std::mutex mutex;
	glyphs_t glyphs;
	glyph_futures_t glyph_futures;
	lib::vector<buffer_glyph_descriptor> pending_glyphs;
	lib::vector<glyph_texture> glyph_textures;

private:
	/**
	 *	@brief	Thread-safe glyph loader. Loaded glyph will be inserted into pending queue 'loaded_glyphs_queue'.
	 */
	auto glyph_loader(const font &font, wchar_t codepoint) {
		// Cache key
		const lib::string cache_key = lib::string("ttfdf") + lib::to_string(font.get_path().string()) + lib::to_string(static_cast<std::uint32_t>(codepoint));

		// Check cache
		optional<glyph> og;
		try {
			og = context.get().engine().cache().get<glyph>(cache_key);
		}
		catch (const std::exception &) {
			og = none;
		}

		if (!og) {
			// If can't load from cache, rasterize glyph from factory
			glyph g = factory.create_glyph(font, codepoint);
			if (g.empty())
				return glyph_loader_data{};

			// And store in cache
			glyph copy_g = g;
			context.get().engine().cache().insert<glyph>(cache_key, std::move(copy_g));

			og = std::move(g);
		}

		if (og.get().glyph_distance_field == nullptr) {
			// Font doesn't have glyph for requested codepoint
			assert(false);
			return glyph_loader_data{};
		}

		// Create image
		const lib::string name = lib::string("glyph '") + lib::to_string(codepoint) + "' image";
		auto image = resource::surface_factory::image_from_surface_2d<gl::format::r32_sfloat>(context.get(),
																							  std::move(*og.get().glyph_distance_field),
																							  gl::image_usage::sampled,
																							  gl::image_layout::shader_read_only_optimal,
																							  name,
																							  false);

		// Create texture and descriptor
		glyph_loader_data result;
		result.key = _internal::glyph_key(font, codepoint);
		result.texture.emplace(std::move(image));
		result.properties.metrics = og.get().metrics;

		return result;
	}

	/*
	 *	@brief	Converts a glyph loader result into a glyph. Critical section.
	 */
	auto finalize_glyph_load(glyph_loader_data &&data) {
		// Insert into pending glyph buffer
		const auto index = static_cast<std::uint32_t>(glyph_textures.size());

		// Add to glyph textures
		if (data.texture)
			glyph_textures.push_back(std::move(data.texture.get()));

		// Add to glyphs map
		data.properties.buffer_index = index;
		auto ret = glyphs.emplace(data.key, data.properties);
		assert(ret.second && "Glyph already loaded?!");

		// Add to pending glyphs
		const buffer_glyph_descriptor bgd(data.properties.metrics, index);
		pending_glyphs.push_back(bgd);

		// Returns pointer to the newly inserted glyph
		return &ret.first->second;
	}

public:
	glyph_manager(const ste_context &context)
		: context(context),
		  buffer(context,
				 gl::buffer_usage::storage_buffer,
				 "glyph_manager buffer"),
		  text_glyph_sampler(context.device(),
							 "glyph_manager sampler",
							 gl::sampler_parameter::filtering(gl::sampler_filter::linear,
															  gl::sampler_filter::linear),
							 gl::sampler_parameter::address_mode(gl::sampler_address_mode::clamp_to_border,
																 gl::sampler_address_mode::clamp_to_border),
							 gl::sampler_parameter::anisotropy(16.f)) {}

	/**
	*	@brief	Returns the glyph descriptor for a specific font and codepoint. If not available, will load asynchronously and return none.
	*/
	optional<const glyph_properties*> glyph_for_font(const font &font, wchar_t codepoint) {
		auto k = _internal::glyph_key(font, codepoint);

		std::unique_lock<std::mutex> l(mutex);

		// Find glyph in storage
		// If not found, enqueue glyph creation and return none.
		auto glyphit = glyphs.find(k);
		if (glyphit == glyphs.end()) {
			// Glyph not found, check if already loading or need to start loader.
			const auto future_it = glyph_futures.lower_bound(k);
			if (future_it != glyph_futures.end() && future_it->first == k) {
				// Already loading
				// Are we done?
				if (future_it->second.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready) {
					auto loader_results = future_it->second.get();
					glyph_futures.erase(future_it);

					return finalize_glyph_load(std::move(loader_results));
				}

				return none;
			}

			// task_scheduler uses low-priority workers. 
			// Use regular priority async tasks here.
			auto future = std::async(std::launch::async,
									 [=]() {
									 // Load glyph
									 return this->glyph_loader(font, codepoint);
								 });
			glyph_futures.emplace_hint(future_it,
									   std::piecewise_construct,
									   std::forward_as_tuple(k),
									   std::forward_as_tuple(std::move(future)));

			return none;
		}

		const auto *ptr = &glyphit->second;
		return ptr;
	}

	/**
	*	@brief	Returns spacing between a couple of glyphs.
	*/
	int spacing(const font &font, const std::pair<wchar_t, wchar_t> &chars, int pixel_size) {
		return factory.read_kerning(font, chars, pixel_size);
	}

	/**
	*	@brief	Should be called from a glyph renderer. Records update commands to upload newly loaded glyphs into device glyph buffer.
	*/
	range<std::uint64_t> update_pending_glyphs(gl::command_recorder &recorder) {
		std::unique_lock<std::mutex> l(mutex);

		if (!pending_glyphs.size()) {
			// Nothing to update
			return { 0,0 };
		}

		range<std::uint64_t> ret;
		ret.start = buffer.size();
		ret.length = pending_glyphs.size();

		// Update
		recorder << buffer.push_back_cmd(context.get(), pending_glyphs);
		pending_glyphs.clear();

		return ret;
	}

	/**
	*	@brief	Used to preload glyphs. Enqueues load tasks for all provided codepoints and returns a future to the task.
	*/
	auto enqueue_glyphs_load(const font &font, lib::vector<wchar_t> codepoints) {
		lib::vector<const std::future<glyph_loader_data>*> glyphs_to_load;
		glyphs_to_load.reserve(codepoints.size());

		// Enumarate all the glyphs that needs to be loaded
		{
			std::unique_lock<std::mutex> l(mutex);
			for (wchar_t codepoint : codepoints) {
				const auto k = _internal::glyph_key(font, codepoint);

				const auto glyph_it = glyphs.find(k);
				if (glyph_it != glyphs.end()) {
					// Already loaded
					continue;
				}

				auto future_it = glyph_futures.lower_bound(k);
				if (future_it != glyph_futures.end() && future_it->first != k) {
					//  Already loading
					continue;
				}

				auto future = context.get().engine().task_scheduler().schedule_now([=]() {
					// Load glyph
					return this->glyph_loader(font, codepoint);
				}).get_future();
				future_it = glyph_futures.emplace_hint(future_it,
													   std::piecewise_construct,
													   std::forward_as_tuple(k),
													   std::forward_as_tuple(std::move(future)));

				glyphs_to_load.emplace_back(&future_it->second);
			}
		}

		return context.get().engine().task_scheduler().schedule_now([=, glyphs_to_load = std::move(glyphs_to_load)]() {
			// Wait for glyphs to load
			for (auto &p : glyphs_to_load)
				p->wait();
		});
	}

	auto &sampler() const { return text_glyph_sampler; }
	auto &ssbo() const { return buffer; }
};

}
}
