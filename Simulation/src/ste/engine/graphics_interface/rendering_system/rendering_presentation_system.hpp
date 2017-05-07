//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device.hpp>
#include <rendering_system.hpp>
#include <presentation_engine.hpp>

#include <framebuffer.hpp>
#include <framebuffer_layout.hpp>

#include <vector>
#include <signal.hpp>
#include <optional.hpp>

namespace ste {
namespace gl {

class rendering_presentation_system : public rendering_system {
private:
	optional<framebuffer_layout> fb_layout;
	std::vector<framebuffer> swap_chain_framebuffers;

	ste_device::queues_and_surface_recreate_signal_type::connection_type surface_recreate_signal_connection;

protected:
	/**
	*	@brief	Rendering system's presentation engine
	*/
	presentation_engine presentation;

private:
	void create_swap_chain_framebuffers() {
		auto surface_extent = device().get_surface().extent();

		fb_layout = framebuffer_layout(surface_extent);
		fb_layout.get()[0] = clear_store(device().get_surface().surface_format(),
										 image_layout::present_src_khr);

		std::vector<framebuffer> v;
		for (auto &swap_image : device().get_surface().get_swap_chain_images()) {
			framebuffer fb(get_creating_context(), fb_layout);
			fb[0] = framebuffer_attachment(swap_image.view, glm::vec4(.0f));
			v.push_back(std::move(fb));
		}

		swap_chain_framebuffers = std::move(v);
	}

protected:
	const auto& device() const { return get_creating_context().device(); }
	const auto& presentation_framebuffer_layout() const { return fb_layout; }
	/**
	*	@brief	Returns a framebuffer with a swap-chain image (of the selected index) bound to color attachment location 0.
	*/
	const auto& swap_chain_framebuffer(std::uint32_t swap_chain_index) const { return swap_chain_framebuffers[swap_chain_index]; }

public:
	rendering_presentation_system(const ste_context &ctx,
								  optional<std::uint32_t> max_frame_lag = none)
		: rendering_system(ctx),
		presentation(ctx.device(), max_frame_lag)
	{
		create_swap_chain_framebuffers();

		// Connect signal to get notifications of presentation surface rebuild
		surface_recreate_signal_connection = make_connection(device().get_queues_and_surface_recreate_signal(), [this](const ste_device*) {
			create_swap_chain_framebuffers();
		});
	}
	virtual ~rendering_presentation_system() noexcept {}

	/**
	*	@brief	Shortcut method for rendering and presenting.
	*/
	void render_and_present() const {
		render();
		present();
	}

	/**
	 *	@brief	Implementations should perform presentation here.
	 */
	virtual void present() const = 0;
};

}
}
