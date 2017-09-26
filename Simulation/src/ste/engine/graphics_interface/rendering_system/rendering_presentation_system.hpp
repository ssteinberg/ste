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

	ste_device::queues_and_surface_recreate_signal_type::connection_type surface_recreate_signal_connection;

protected:
	/**
	*	@brief	Returns a framebuffer with a swap-chain image (of the selected index) bound to color attachment at location 0.
	*/
	auto& swap_chain_image(std::uint32_t swap_chain_index) { return device().get_surface().get_swap_chain_images()[swap_chain_index]; }

protected:
	/**
	*	@brief	Creates a vector of swap-chain framebuffers of deisred layout and binds a swap-chain image to color attachment at location 0 of each framebuffer.
	*/
	auto create_swap_chain_framebuffers(const gl::framebuffer_layout &fb_layout) {
		auto surface_extent = device().get_surface().extent();

		lib::vector<framebuffer> v;
		for (auto &swap_image : device().get_surface().get_swap_chain_images()) {
			framebuffer fb(get_creating_context(),
						   "swapchain framebuffer",
						   gl::framebuffer_layout(fb_layout),
						   surface_extent,
						   depth);
			fb[0] = framebuffer_attachment(swap_image.view, glm::vec4(.0f));

			v.push_back(std::move(fb));
		}

		return v;
	}

public:
	rendering_presentation_system(const ste_context &ctx,
								  const depth_range &depth = depth_range::zero_to_one())
		: rendering_system(ctx),
		depth(depth)
	{
		// Connect signal to get notifications of presentation surface rebuild
		surface_recreate_signal_connection = make_connection(device().get_queues_and_surface_recreate_signal(), [this](const ste_device*) {
			swap_chain_resized();
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

	/*
	 *	@brief	Notifies implementation of swap-chain resize
	 */
	virtual void swap_chain_resized() {}
};

}
}
