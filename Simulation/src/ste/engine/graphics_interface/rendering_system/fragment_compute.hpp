//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <rendering_system.hpp>
#include <fragment.hpp>

#include <device_pipeline_shader_stage.hpp>
#include <pipeline_auditor_compute.hpp>
#include <device_pipeline_compute.hpp>

#include <lib/string.hpp>
#include <lib/unique_ptr.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A rendering system fragment with a compute pipeline
*/
template <typename CRTP>
class fragment_compute : public fragment {
private:
	lib::unique_ptr<device_pipeline_shader_stage> shader_stage;
	lib::unique_ptr<device_pipeline_compute> pipeline_object;

private:
	static auto create_compute_pipeline(const ste_context &ctx,
										const char *name,
										const pipeline_external_binding_set *external_binding_sets_collection,
										device_pipeline_shader_stage &shader_stage) {
		// Compute pipeline auditor
		pipeline_auditor_compute auditor(shader_stage);

		// Create pipeline
		auto obj = external_binding_sets_collection
					   ? auditor.pipeline(ctx,
										  std::ref(*external_binding_sets_collection),
										  name)
					   : auditor.pipeline(ctx,
										  name);

		return lib::allocate_unique<device_pipeline_compute>(std::move(obj));
	}

protected:
	fragment_compute(const rendering_system &rs,
					 const lib::string &shader_stage_name)
		: fragment(rs.get_creating_context()),
		  shader_stage(lib::allocate_unique<device_pipeline_shader_stage>(rs.get_creating_context(), shader_stage_name)),
		  pipeline_object(create_compute_pipeline(rs.get_creating_context(),
												  CRTP::name().data(),
												  rs.external_binding_set(),
												  *this->shader_stage)) {}

public:
	virtual ~fragment_compute() noexcept {}

	fragment_compute(fragment_compute &&) = default;
	fragment_compute &operator=(fragment_compute &&) = default;

	// Subclasses are expected to declare:
	//static lib::string name();

protected:
	auto &pipeline() { return *pipeline_object; }
	auto &pipeline() const { return *pipeline_object; }
};

}
}
