//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device.hpp>
#include <rendering_system.hpp>

#include <framebuffer.hpp>
#include <framebuffer_layout.hpp>
#include <depth_range.hpp>

#include <lib/vector.hpp>
#include <signal.hpp>

namespace ste {
namespace gl {

class rendering_presentation_system : public rendering_system {
private:
	gl::framebuffer_layout fb_layout;
	depth_range depth;

	lib::vector<framebuffer> swap_chain_framebuffers;

	ste_device::queues_and_surface_recreate_signal_type::connection_type surface_recreate_signal_connection;

protected:
	/**
	*	@brief	Returns a framebuffer with a swap-chain image (of the selected index) bound to color attachment at location 0.
	*/
	auto& swap_chain_image(std::uint32_t swap_chain_index) { return device().get_surface().get_swap_chain_images()[swap_chain_index]; }
	/**
	*	@brief	Returns a framebuffer with a swap-chain image (of the selected index) bound to color attachment at location 0.
	*/
	auto& swap_chain_framebuffer(std::uint32_t swap_chain_index) { return swap_chain_framebuffers[swap_chain_index]; }

private:
	void create_swap_chain_framebuffers() {
		auto surface_extent = device().get_surface().extent();

		lib::vector<framebuffer> v;
		for (auto &swap_image : device().get_surface().get_swap_chain_images()) {
			auto l = fb_layout;
			framebuffer fb(get_creating_context(),
						   "swapchain framebuffer",
						   std::move(l),
						   surface_extent,
						   depth);
			fb[0] = framebuffer_attachment(swap_image.view, glm::vec4(.0f));

			v.push_back(std::move(fb));
		}

		swap_chain_framebuffers = std::move(v);
	}

public:
	rendering_presentation_system(const ste_context &ctx,
								  gl::framebuffer_layout &&fb_layout,
								  const depth_range &depth = depth_range::zero_to_one())
		: rendering_system(ctx),
		fb_layout(std::move(fb_layout)),
		depth(depth)
	{
		create_swap_chain_framebuffers();

		// Connect signal to get notifications of presentation surface rebuild
		surface_recreate_signal_connection = make_connection(device().get_queues_and_surface_recreate_signal(), [this](const ste_device*) {
			create_swap_chain_framebuffers();
		});
	}
	virtual ~rendering_presentation_system() noexcept {}

	const auto& presentation_framebuffer_layout() const { return fb_layout; }

	/**
	 *	@brief	Implementations should perform presentation here.
	 */
	virtual void present() = 0;

protected:
	/**
	*	@brief	Implementations should use this to record rendering to a command queue.
	*/
	virtual void render(gl::command_recorder &) override {}
};

}
}
