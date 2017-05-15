//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <rendering_system.hpp>
#include <fragment.hpp>
#include <fragment_utility.hpp>

#include <device_pipeline_shader_stage.hpp>
#include <pipeline_auditor_graphics.hpp>
#include <device_pipeline_graphics.hpp>
#include <framebuffer_layout.hpp>

#include <string>
#include <vector>
#include <optional.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A rendering system fragment with a graphics pipeline
*/
template <typename CRTP>
class fragment_graphics : public fragment {
private:
	std::vector<device_pipeline_shader_stage> shader_stages;

	struct ctor {};

protected:
	device_pipeline_graphics pipeline;

private:
	template <typename RenderingSystem, typename... Names>
	static auto create_graphics_pipeline(const RenderingSystem &rs,
										 device_pipeline_graphics_configurations &&pipeline_graphics_configurations,
										 const pipeline_external_binding_set_collection* external_binding_sets_collection,
										 optional<framebuffer_layout> &&fb_layout,
										 std::vector<device_pipeline_shader_stage> &out_shader_stages,
										 Names&&... shader_stages_names) {
		const ste_context &ctx = rs.get_creating_context();

		pipeline_auditor_graphics auditor(std::move(pipeline_graphics_configurations));

		// Expand names and create shader stages, feeding them into the auditor
		out_shader_stages.reserve(sizeof...(Names));
		_internal::fragment_expand_shader_stages<Names...>()(ctx,
															 out_shader_stages,
															 auditor,
															 std::forward<Names>(shader_stages_names)...);

		if (fb_layout)
			auditor.set_framebuffer_layout(std::move(fb_layout.get()));

		// Configure the auditor
		CRTP::setup_graphics_pipeline(rs, auditor);

		// Create pipeline
		return external_binding_sets_collection ?
			auditor.pipeline(ctx,
							 *external_binding_sets_collection) :
			auditor.pipeline(ctx);
	}

	template <typename RenderingSystem, typename... Names>
	fragment_graphics(ctor,
					  const RenderingSystem &rs,
					  device_pipeline_graphics_configurations &&pipeline_graphics_configurations,
					  optional<framebuffer_layout> &&fb_layout,
					  Names&&... shader_stages_names)
		: pipeline(create_graphics_pipeline(rs,
											std::move(pipeline_graphics_configurations),
											rs.external_binding_sets(),
											std::move(fb_layout),
											this->shader_stages,
											std::forward<Names>(shader_stages_names)...))
	{
		static_assert(sizeof...(Names) > 0, "Expected shader stages");
	}

protected:
	/**
	 *	@brief	Subclasses can override this declaration to fine tune pipeline_auditor_graphics parameters
	 */
	static void setup_graphics_pipeline(const rendering_system &rs, pipeline_auditor_graphics &auditor) {}

	template <typename RenderingSystem, typename... Names>
	fragment_graphics(const RenderingSystem &rs,
					  device_pipeline_graphics_configurations &&pipeline_graphics_configurations,
					  Names&&... shader_stages_names)
		: fragment_graphics(ctor(),
							rs,
							std::move(pipeline_graphics_configurations),
							none,
							std::forward<Names>(shader_stages_names)...)
	{}
	template <typename RenderingSystem, typename... Names>
	fragment_graphics(const RenderingSystem &rs,
					  device_pipeline_graphics_configurations &&pipeline_graphics_configurations,
					  framebuffer_layout &&fb_layout,
					  Names&&... shader_stages_names)
		: fragment_graphics(ctor(),
							rs,
							std::move(pipeline_graphics_configurations),
							std::move(fb_layout),
							std::forward<Names>(shader_stages_names)...)
	{}

public:
	virtual ~fragment_graphics() noexcept {}

	// Subclasses are expected to declare:
	//static const std::string& name();
};

}
}
