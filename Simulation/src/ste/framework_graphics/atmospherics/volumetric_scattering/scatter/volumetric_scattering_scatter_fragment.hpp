// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>

#include <volumetric_scattering_storage.hpp>
#include <light_storage.hpp>

#include <cmd_clear_color_image.hpp>

namespace ste {
namespace graphics {

class volumetric_scattering_scatter_fragment : public gl::fragment_compute {
	using Base = gl::fragment_compute;

private:
	gl::task<gl::cmd_dispatch> dispatch_task;

	const volumetric_scattering_storage *vss;
	const light_storage *ls;

public:
	volumetric_scattering_scatter_fragment(const gl::rendering_system &rs,
										   const volumetric_scattering_storage *vss,
										   const light_storage *ls)
		: Base(rs,
			   "volumetric_scattering_scatter.comp"),
		vss(vss),
		ls(ls)
	{
		dispatch_task.attach_pipeline(pipeline);
	}
	~volumetric_scattering_scatter_fragment() noexcept {}

	volumetric_scattering_scatter_fragment(volumetric_scattering_scatter_fragment&&) = default;

	static const lib::string& name() { return "scatter"; }

	void record(gl::command_recorder &recorder) override final {
		static const glm::ivec2 jobs = { 32, 32 };
		static const glm::vec4 clear_data = { .0f, .0f, .0f, .0f };

		const auto size = (glm::ivec2{ vss->get_tiles_extent().x, vss->get_tiles_extent().y } + jobs - glm::ivec2(1)) / jobs;

		recorder << gl::cmd_clear_color_image(vss->get_volume_texture().get_image(), 
											  gl::image_layout::transfer_dst_optimal, 
											  clear_data);
		recorder << dispatch_task(static_cast<std::uint32_t>(size.x), 
								  static_cast<std::uint32_t>(size.y),
								  1u);
	}
};

}
}
