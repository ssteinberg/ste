// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

#include <functional>

#include <glm/glm.hpp>

#include "gl_current_context.hpp"

#include "is_flat_container.hpp"

#include "bindable_resource.hpp"
#include "glsl_shader_object.hpp"

#include "texture_handle.hpp"
#include "image_handle.hpp"

#include "Log.hpp"

namespace StE {
namespace Core {

class glsl_program_allocator : public generic_resource_allocator {
public:
	generic_resource::type allocate() override final {
		generic_resource::type res = glCreateProgram();
		return res;
	}
	static void deallocate(generic_resource::type &id) {
		if (id) {
			glDeleteProgram(id);
			id = 0;
		}
	}
};

class glsl_program_binder {
public:
	static void bind(generic_resource::type id) { GL::gl_current_context::get()->bind_shader_program(id); }
	static void unbind() { GL::gl_current_context::get()->bind_shader_program(0); }
};

class glsl_program_object : public bindable_resource<glsl_program_allocator, glsl_program_binder> {
	using Base = bindable_resource<glsl_program_allocator, glsl_program_binder>;

private:
	bool linked;
	std::vector<std::unique_ptr<glsl_shader_object_generic>> shaders;
	mutable std::unordered_map<std::string, int> uniform_map;

protected:
	int get_uniform_location(const std::string &name) const {
		auto it = uniform_map.find(name);
		if (it != uniform_map.end())
			return it->second;

		auto ret = (uniform_map[name] = glGetUniformLocation(get_resource_id(), name.c_str()));
#ifdef _DEBUG
		if (ret == -1) {
			ste_log_warn() << "Uniform \"" << name << "\" not found\n";
		}
#endif

		return ret;
	}

	int get_uniform_block_index(const std::string &name) const {
		auto it = uniform_map.find(name);
		if (it != uniform_map.end())
			return it->second;

		auto ret = (uniform_map[name] = glGetUniformBlockIndex(get_resource_id(), name.c_str()));
#ifdef _DEBUG
		if (ret == -1) {
			ste_log_warn() << "Uniform block \"" << name << "\" not found\n";
		}
#endif

		return ret;
	}

public:
	glsl_program_object(glsl_program_object &&m) = default;
	glsl_program_object(const glsl_program_object &c) = delete;
	glsl_program_object& operator=(glsl_program_object &&m) = default;
	glsl_program_object& operator=(const glsl_program_object &c) = delete;

	glsl_program_object() : linked(false) {}
	~glsl_program_object() noexcept {}

	void add_shader(std::unique_ptr<glsl_shader_object_generic> shader) {
		if (shader == nullptr || !shader->is_valid()) {
			ste_log_error() << "Error: invalid shader." << std::endl;
			assert(false);
			return;
		}
		shaders.push_back(std::move(shader));
		linked = false;
	}

	bool link();
	bool is_linked() const { return linked; }

	bool link_from_binary(generic_resource::type format, const std::string &data);
	std::string get_binary_representation(generic_resource::type *format) const;

	std::string generate_name() const;

	void bind_attrib_location(GLuint location, const std::string &name) const { glBindAttribLocation(get_resource_id(), location, name.c_str()); }
	void bind_frag_data_location(GLuint location, const std::string &name) const { glBindFragDataLocation(get_resource_id(), location, name.c_str()); }

	void set_uniform(const std::string &name, const glm::vec2 & v) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform2f(get_resource_id(), loc, v.x, v.y);
	}

	void set_uniform(const std::string &name, const glm::i32vec2 & v) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform2i(get_resource_id(), loc, v.x, v.y);
	}

	void set_uniform(const std::string &name, const glm::u32vec2 & v) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform2ui(get_resource_id(), loc, v.x, v.y);
	}

	void set_uniform(const std::string &name, const glm::i64vec2 & v) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform2i64ARB(get_resource_id(), loc, v.x, v.y);
	}

	void set_uniform(const std::string &name, const glm::u64vec2 & v) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform2ui64ARB(get_resource_id(), loc, v.x, v.y);
	}

	void set_uniform(const std::string &name, const glm::vec3 & v) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform3f(get_resource_id(), loc, v.x, v.y, v.z);
	}

	void set_uniform(const std::string &name, const glm::vec4 & v) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform4f(get_resource_id(), loc, v.x, v.y, v.z, v.w);
	}

	void set_uniform(const std::string &name, const glm::mat2 & m, bool transpose = false) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniformMatrix2fv(get_resource_id(), loc, 1, transpose, &m[0][0]);
	}

	void set_uniform(const std::string &name, const glm::mat3 & m, bool transpose = false) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniformMatrix3fv(get_resource_id(), loc, 1, transpose, &m[0][0]);
	}

	void set_uniform(const std::string &name, const glm::mat4 & m, bool transpose = false) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniformMatrix4fv(get_resource_id(), loc, 1, transpose, &m[0][0]);
	}

	void set_uniform(const std::string &name, float val) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1f(get_resource_id(), loc, val);
	}

	void set_uniform(const std::string &name, std::int32_t val) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1i(get_resource_id(), loc, val);
	}

	void set_uniform(const std::string &name, std::int64_t val) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1i64ARB(get_resource_id(), loc, val);
	}

	void set_uniform(const std::string &name, std::uint32_t val) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1ui(get_resource_id(), loc, val);
	}

	void set_uniform(const std::string &name, std::uint64_t val) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1ui64ARB(get_resource_id(), loc, val);
	}

	void set_uniform(const std::string &name, bool val) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1i(get_resource_id(), loc, val);
	}

	void set_uniform(const std::string &name, const texture_handle &handle) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniformHandleui64ARB(get_resource_id(), loc, handle);
	}

	void set_uniform(const std::string &name, const image_handle &handle) const {
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1ui64ARB(get_resource_id(), loc, handle);
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform_subroutine(GLenum shader_type, const V<std::uint32_t, Args...> &ids) const {
		static_assert(is_flat_container<decltype(ids)>::value, "V must be a flat container.");
		glUniformSubroutinesuiv(shader_type, ids.size(), &ids[0]);
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<glm::vec2, Args...> & v) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform2fv(get_resource_id(), loc, v.size(), reinterpret_cast<const float*>(&v[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<glm::i32vec2, Args...> & v) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform2iv(get_resource_id(), loc, v.size(), reinterpret_cast<const std::int32_t*>(&v[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<glm::u32vec2, Args...> & v) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform2uiv(get_resource_id(), loc, v.size(), reinterpret_cast<const std::uint32_t*>(&v[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<glm::i64vec2, Args...> & v) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform2i64vARB(get_resource_id(), loc, v.size(), reinterpret_cast<const std::int64_t*>(&v[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<glm::u64vec2, Args...> & v) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform2ui64vARB(get_resource_id(), loc, v.size(), reinterpret_cast<const std::uint64_t*>(&v[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<glm::vec3, Args...> & v) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform3fv(get_resource_id(), loc, v.size(), reinterpret_cast<const float*>(&v[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<glm::vec4, Args...> & v) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform4fv(get_resource_id(), loc, v.size(), reinterpret_cast<const float*>(&v[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<glm::mat2, Args...> & m, bool transpose = false) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniformMatrix2fv(get_resource_id(), loc, m.size(), transpose, reinterpret_cast<const float*>(&m[0][0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<glm::mat3, Args...> & m, bool transpose = false) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniformMatrix3fv(get_resource_id(), loc, m.size(), transpose, reinterpret_cast<const float*>(&m[0][0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<glm::mat4, Args...> & m, bool transpose = false) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniformMatrix4fv(get_resource_id(), loc, m.size(), transpose, reinterpret_cast<const float*>(&m[0][0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<float, Args...> & val) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1fv(get_resource_id(), loc, val.size(), reinterpret_cast<const float*>(&val[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<std::int32_t, Args...> &val) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1iv(get_resource_id(), loc, val.size(), reinterpret_cast<const std::int32_t*>(&val[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<std::int64_t, Args...> &val) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1i64vARB(get_resource_id(), loc, val.size(), reinterpret_cast<const std::int64_t*>(&val[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<std::uint32_t, Args...> &val) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1uiv(get_resource_id(), loc, val.size(), reinterpret_cast<const std::uint32_t*>(&val[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<std::uint64_t, Args...> &val) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");
		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1ui64vARB(get_resource_id(), loc, val.size(), reinterpret_cast<const std::uint64_t*>(&val[0]));
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<texture_handle, Args...> &handles) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");

		std::vector<decltype(texture_handle().get_handle())> v;
		v.reserve(handles.size());
		for (auto &ref : handles)
			v.push_back(ref.get_handle());

		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniformHandleui64vARB(get_resource_id(), loc, v.size(), &v[0]);
	}

	template <template<typename, typename...> class V, typename ... Args>
	void set_uniform(const std::string &name, const V<image_handle, Args...> &handles) const {
		static_assert(is_flat_container<V<int, Args...>>::value, "V must be a flat container.");

		std::vector<decltype(texture_handle().get_handle())> v;
		v.reserve(handles.size());
		for (auto &ref : handles)
			v.push_back(ref.get_handle());

		int loc = get_uniform_location(name);
		if (loc >= 0)
			glProgramUniform1ui64vARB(get_resource_id(), loc, v.size(), &v[0]);
	}

	template <std::size_t N, typename T, typename ... Params>
	void set_uniform(const std::string &name, const std::array<T, N> &val, Params&&... p) const {
		std::vector<T> v(std::begin(val), std::end(val));
		set_uniform(name, v, std::forward<Params>(p)...);
	}

	template <std::size_t N, typename T, typename ... Params>
	void set_uniform(const std::string &name, const T (&arr)[N], Params&&... p) const {
		std::vector<T> v(std::begin(arr), std::end(arr));
		set_uniform(name, v, std::forward<Params>(p)...);
	}

	void set_uniform_block_binding(const std::string &name, int binding) const {
		auto idx = get_uniform_block_index(name);
		if (idx >= 0)
			glUniformBlockBinding(get_resource_id(), idx, binding);
	}


	core_resource_type resource_type() const override { return core_resource_type::GLSLProgram; }
};

}
}
