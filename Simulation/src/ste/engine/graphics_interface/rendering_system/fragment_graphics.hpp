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

#include <string>
#include <vector>

namespace ste {
namespace gl {

/**
*	@brief	A rendering system fragment with a graphics pipeline
*/
template <typename CRTP>
class fragment_graphics : public fragment {
private:
	std::vector<device_pipeline_shader_stage> shader_stages;

protected:
	device_pipeline_graphics pipeline;

private:
	template <typename RenderingSystem, typename... Names>
	static auto create_graphics_pipeline(const RenderingSystem &rs,
										 device_pipeline_graphics_configurations &&pipeline_graphics_configurations,
										 const pipeline_external_binding_set_collection* external_binding_sets_collection,
										 std::vector<device_pipeline_shader_stage> &shader_stages,
										 Names&&... shader_stages_names) {
		const ste_context &ctx = rs.get_creating_context();

		pipeline_auditor_graphics auditor(std::move(pipeline_graphics_configurations));

		// Expand names and create shader stages, feeding them into the auditor
		shader_stages.reserve(sizeof...(Names));
		_internal::fragment_expand_shader_stages<Names...>()(ctx,
															 shader_stages,
															 auditor,
															 std::forward<Names>(shader_stages_names)...);

		// Configure the auditor
		CRTP::setup_graphics_pipeline(rs, auditor);

		// Create pipeline
		return external_binding_sets_collection ?
			auditor.pipeline(ctx,
							 *external_binding_sets_collection) :
			auditor.pipeline(ctx);
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
		: pipeline(create_graphics_pipeline(rs,
											std::move(pipeline_graphics_configurations),
											rs.external_binding_sets(),
											this->shader_stages,
											std::forward<Names>(shader_stages_names)...))
	{
		static_assert(sizeof...(Names) > 0, "Expected shader stages");
	}

public:
	virtual ~fragment_graphics() noexcept {}

	// Subclasses are expected to declare:
	//static const std::string& name();
};

}
}
