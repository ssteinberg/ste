// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "core_resource_type.hpp"
#include "query_buffer_object.hpp"

namespace StE {
namespace Core {

enum class query_result_type {
	QueryResultAvailable = GL_QUERY_RESULT_AVAILABLE,
	QueryResult = GL_QUERY_RESULT,
	QueryResultNoWait = GL_QUERY_RESULT_NO_WAIT,
};

template <core_resource_type ResType>
class query_object_allocator : public generic_resource_allocator {
public:
	generic_resource::type allocate() override final {
		generic_resource::type o;
		glCreateQueries(static_cast<GLenum>(ResType), 1, &o);
		return o;
	}
	static void deallocate(generic_resource::type &id) {
		if (id) {
			glDeleteQueries(1, &id);
			id = 0;
		}
	}
};

template <core_resource_type ResType>
class query_object : public resource<query_object_allocator<ResType>> {
	using Base = resource<query_object_allocator<ResType>>;

public:
	void begin_query() const { glBeginQuery(ResType, Base::get_resource_id()); }
	void end_query() const { glEndQuery(ResType, Base::get_resource_id()); }

	std::int32_t get_query_result_32(query_result_type qt) const {
		std::int32_t r;
		glGetQueryObjectiv(Base::get_resource_id(), static_cast<GLenum>(qt), &r);
		return r;
	}
	std::int64_t get_query_result_64(query_result_type qt) const {
		std::int64_t r;
		glGetQueryObjecti64v(Base::get_resource_id(), static_cast<GLenum>(qt), &r);
		return r;
	}
	std::uint32_t get_query_result_u32(query_result_type qt) const {
		std::uint32_t r;
		glGetQueryObjectuiv(Base::get_resource_id(), static_cast<GLenum>(qt), &r);
		return r;
	}
	std::uint64_t get_query_result_u64(query_result_type qt) const {
		std::uint64_t r;
		glGetQueryObjectui64v(Base::get_resource_id(), static_cast<GLenum>(qt), &r);
		return r;
	}

	template <typename Type, BufferUsage::buffer_usage U>
	void get_query_result_32(query_result_type qt, const query_buffer_object<Type, U> &qbo, std::size_t offset = 0) const {
		qbo.bind();
		glGetQueryObjectiv(Base::get_resource_id(), static_cast<GLenum>(qt), reinterpret_cast<void*>(offset));
	}
	template <typename Type, BufferUsage::buffer_usage U>
	void get_query_result_64(query_result_type qt, const query_buffer_object<Type, U> &qbo, std::size_t offset = 0) const {
		qbo.bind();
		glGetQueryObjecti64v(Base::get_resource_id(), static_cast<GLenum>(qt), reinterpret_cast<void*>(offset));
	}
	template <typename Type, BufferUsage::buffer_usage U>
	void get_query_result_u32(query_result_type qt, const query_buffer_object<Type, U> &qbo, std::size_t offset = 0) const {
		qbo.bind();
		glGetQueryObjectuiv(Base::get_resource_id(), static_cast<GLenum>(qt), reinterpret_cast<void*>(offset));
	}
	template <typename Type, BufferUsage::buffer_usage U>
	void get_query_result_u64(query_result_type qt, const query_buffer_object<Type, U> &qbo, std::size_t offset = 0) const {
		qbo.bind();
		glGetQueryObjectui64v(Base::get_resource_id(), static_cast<GLenum>(qt), reinterpret_cast<void*>(offset));
	}

	core_resource_type resource_type() const override { return ResType; }
};

}
}
