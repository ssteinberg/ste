
#include "stdafx.h"
#include "glsl_programs_pool.h"

#include "StEngineControl.h"
#include "GLSLProgramFactory.h"

using namespace StE::Resource;


StE::task<std::shared_ptr<StE::LLR::GLSLProgram>> glsl_programs_pool::fetch_program_task(const glsl_programs_pool_key &k) {
	return StE::task<std::shared_ptr<LLR::GLSLProgram>>([k = std::move(k), this](optional<task_scheduler*> sched) -> std::shared_ptr<LLR::GLSLProgram> {		
		{
			auto val_guard = programs[k];
			if (val_guard.is_valid()) {
				std::weak_ptr<LLR::GLSLProgram> wprog = *val_guard;
				if (auto prog_ptr = wprog.lock())
					return prog_ptr;
			}
		}
		
		auto uptr_prog = GLSLProgramFactory::load_program_task(this->context, k.names)();
		if (uptr_prog) {
			std::shared_ptr<LLR::GLSLProgram> prog = std::move(uptr_prog);
			programs.emplace(k, prog);
			return prog;
		}
		
		return nullptr;
	});
}

StE::task<std::shared_ptr<StE::LLR::GLSLProgram>> glsl_programs_pool::fetch_program_task(const std::vector<std::string> &names) {
	glsl_programs_pool_key k{ names };
	return fetch_program_task(k);
}
