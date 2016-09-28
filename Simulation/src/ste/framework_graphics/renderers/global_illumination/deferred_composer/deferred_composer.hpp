// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "signal.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "gpu_dispatchable.hpp"

#include "glsl_program.hpp"
#include "Texture2DArray.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class GIRenderer;

class deferred_composer : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<deferred_composer>;
	friend class Resource::resource_instance<deferred_composer>;

private:
	Resource::resource_instance<Resource::glsl_program> program;
	GIRenderer *dr;

	std::shared_ptr<connection<>> vss_storage_connection;
	std::shared_ptr<connection<>> shadows_storage_connection;

	std::unique_ptr<Core::Texture2DArray> microfacet_refraction_ratio_fit_lut;

private:
	void load_microfacet_fit_lut();
	void attach_handles() const;

private:
	deferred_composer(const StEngineControl &ctx, GIRenderer *dr);

public:
	~deferred_composer() noexcept {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::deferred_composer> {
	using R = Graphics::deferred_composer;

public:
	auto loader(const StEngineControl &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->program.wait();
		}).then_on_main_thread([object]() {
			object->attach_handles();
			object->load_microfacet_fit_lut();
		});
	}
};

}
}
