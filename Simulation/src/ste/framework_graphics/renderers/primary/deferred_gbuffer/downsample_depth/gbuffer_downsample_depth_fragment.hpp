//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <deferred_gbuffer.hpp>
#include <fragment_compute.hpp>

#include <task.hpp>
#include <cmd_dispatch.hpp>

#include <lib/vector.hpp>
#include <connection.hpp>

namespace ste {
namespace graphics {

class gbuffer_downsample_depth_fragment : public gl::fragment_compute<gbuffer_downsample_depth_fragment> {
    using Base = gl::fragment_compute<gbuffer_downsample_depth_fragment>;

private:
    const deferred_gbuffer *gbuffer;

    gl::task<gl::cmd_dispatch> dispatch_task;
    connection<> gbuffer_depth_target_connection;

    lib::vector<gl::image_view<gl::image_type::image_2d>> downsampled_depth_levels;

private:
    void attach_handles(const ste_context &ctx) {
        auto& downsampled = gbuffer->get_downsampled_depth_target().get_image();
        auto levels = downsampled.get_mips();

        // Create view of individual levels
        lib::vector<gl::image_view<gl::image_type::image_2d>> level_views;
        level_views.reserve(static_cast<std::size_t>(levels));
        for (auto l = 0_mip; l < levels; ++l)
            level_views.emplace_back(downsampled,
                                     downsampled.get_format(),
                                     0_layer, l, 1_mip);
        downsampled_depth_levels = std::move(level_views);

        // Attach to pipeline
        lib::vector<gl::pipeline::image> output_images;
        output_images.reserve(static_cast<std::size_t>(levels));
        for (auto l = 0_mip; l < levels; ++l)
            output_images.emplace_back(downsampled_depth_levels[static_cast<std::size_t>(l)],
                                       gl::image_layout::general);

        pipeline()["levels"] = static_cast<std::size_t>(levels);
        pipeline()["output_images"] = gl::bind(0, output_images);
    }

public:
    gbuffer_downsample_depth_fragment(const gl::rendering_system &rs,
                                      const deferred_gbuffer *gbuffer)
        : Base(rs,
               "gbuffer_downsample_depth.comp"),
        gbuffer(gbuffer)
    {
        dispatch_task.attach_pipeline(pipeline());
        attach_handles(rs.get_creating_context());

        gbuffer_depth_target_connection = make_connection(gbuffer->get_gbuffer_modified_signal(), [this, &rs]() {
            attach_handles(rs.get_creating_context());
        });
    }
    ~gbuffer_downsample_depth_fragment() noexcept {}

    gbuffer_downsample_depth_fragment(gbuffer_downsample_depth_fragment&&) = default;

    static lib::string name() { return "gbuffer_downsample_depth"; }

    void record(gl::command_recorder &recorder) override final {
        constexpr int jobs = 32;
        auto levels = gbuffer->get_downsampled_depth_target().get_image().get_mips() + 1_mip;

        auto extent = static_cast<glm::ivec2>(gbuffer->get_depth_target().get_image().get_extent()) >> static_cast<std::int32_t>(levels);
        auto workgroup = (extent + glm::ivec2(jobs - 1)) / jobs;

        recorder << dispatch_task(workgroup.x, workgroup.y, 1);
    }
};

}
}
