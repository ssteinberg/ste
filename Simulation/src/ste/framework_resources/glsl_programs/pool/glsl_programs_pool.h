// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "GLSLProgram.h"

#include "task.h"
#include "concurrent_unordered_map.h"
#include "hash_combine.h"

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <algorithm>

namespace StE {
	
class StEngineControl;
	
namespace Resource {

class glsl_programs_pool {
public:
	class glsl_programs_pool_key {
	private:
		friend class std::hash<glsl_programs_pool_key>;
	
	private:
		std::size_t hash;
		
		std::size_t compute_hash(const std::vector<std::string> &names) {
			if (!names.size())
				return 0;
				
			std::sort(names.begin(), names.end());
				
			auto it = names.begin();
			auto h = std::hash<std::string>()(*it);
			while ((++it) != names.end())
				h = StE::hash_combine(h, std::hash<std::string>()(*it));
				
			return h;
		}
		
	public:
		glsl_programs_pool_key(const std::vector<std::string> &names) {
			hash = compute_hash(names);
		}
		
		glsl_programs_pool_key(const glsl_programs_pool_key &) = default;
		glsl_programs_pool_key(glsl_programs_pool_key &&) = default;
		glsl_programs_pool_key &operator=(const glsl_programs_pool_key &) = default;
		glsl_programs_pool_key &operator=(glsl_programs_pool_key &&) = default;
		
		bool operator==(const glsl_programs_pool_key &k) const {
			return hash == k.hash;
		}
		
		bool operator!=(const glsl_programs_pool_key &k) const {
			return !((*this)==k);
		}
	};
	
private:
	const StEngineControl &context;
	concurrent_unordered_map<glsl_programs_pool_key, std::weak_ptr<LLR::GLSLProgram>> programs; 
	
public:
	glsl_programs_pool(const StEngineControl &context) : context(context) {}
	
	task<std::shared_ptr<LLR::GLSLProgram>> fetch_program_task(const std::vector<std::string> &names);
};

}
}


namespace std {

template <> struct hash<StE::Resource::glsl_programs_pool::glsl_programs_pool_key> {
	size_t inline operator()(const StE::Resource::glsl_programs_pool::glsl_programs_pool_key &x) const {
		return x.hash;
	}
};

}
