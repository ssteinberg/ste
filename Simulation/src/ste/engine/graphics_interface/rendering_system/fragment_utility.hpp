//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_pipeline_shader_stage.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename Name, typename... Names>
struct fragment_expand_shader_stages {
	template <typename StagesStorage, typename Auditor>
	void operator()(const ste_context &ctx,
					StagesStorage &storage, 
					Auditor &auditor,
					const Name& name,
					const Names&... names) {
		fragment_expand_shader_stages<Name>()(ctx,
											  storage,
											  auditor,
											  name);

		fragment_expand_shader_stages<Names...>()(ctx,
												  storage,
												  auditor,
												  names...);
	}
};
template <typename Name>
struct fragment_expand_shader_stages<Name> {
	template <typename StagesStorage, typename Auditor>
	void operator()(const ste_context &ctx,
					StagesStorage &storage,
					Auditor &auditor,
					const Name& name) {
		device_pipeline_shader_stage stage(ctx, static_cast<lib::string>(name));
		storage.push_back(std::move(stage));
		auditor.attach_shader_stage(storage.back());
	}
};

}

}
}
