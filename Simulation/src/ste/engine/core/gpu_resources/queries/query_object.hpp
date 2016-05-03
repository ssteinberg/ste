// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "core_resource_type.hpp"
#include "QueryBufferObject.hpp"

namespace StE {
namespace Core {

enum class QueryResultType {
	QueryResultAvailable = GL_QUERY_RESULT_AVAILABLE,
	QueryResult = GL_QUERY_RESULT,
	QueryResultNoWait = GL_QUERY_RESULT_NO_WAIT,
};

template <core_resource_type type>
class QueryObjectAllocator : public generic_resource_allocator {
public:
	GenericResource::type allocate() override final {
		GenericResource::type o;
		glCreateQueries(static_cast<GLenum>(type), 1, &o);
		return o;
	}
	static void deallocate(GenericResource::type &id) {
		if (id) {
			glDeleteQueries(1, &id);
			id = 0;
		}
	}
};

template <core_resource_type type>
class query_object : public resource<QueryObjectAllocator<type>> {
	using Base = resource<QueryObjectAllocator<type>>;

public:
	void begin_query() const { glBeginQuery(type, Base::get_resource_id()); }
	void end_query() const { glEndQuery(type, Base::get_resource_id()); }

	std::int32_t get_query_result_32(QueryResultType qt) const {
		std::int32_t r;
		glGetQueryObjectiv(Base::get_resource_id(), static_cast<GLenum>(qt), &r);
		return r;
	}
	std::int64_t get_query_result_64(QueryResultType qt) const {
		std::int64_t r;
		glGetQueryObjecti64v(Base::get_resource_id(), static_cast<GLenum>(qt), &r);
		return r;
	}
	std::uint32_t get_query_result_u32(QueryResultType qt) const {
		std::uint32_t r;
		glGetQueryObjectuiv(Base::get_resource_id(), static_cast<GLenum>(qt), &r);
		return r;
	}
	std::uint64_t get_query_result_u64(QueryResultType qt) const {
		std::uint64_t r;
		glGetQueryObjectui64v(Base::get_resource_id(), static_cast<GLenum>(qt), &r);
		return r;
	}

	template <typename Type, BufferUsage::buffer_usage U>
	void get_query_result_32(QueryResultType qt, const QueryBufferObject<Type, U> &qbo, std::size_t offset = 0) const {
		qbo.bind();
		glGetQueryObjectiv(Base::get_resource_id(), static_cast<GLenum>(qt), reinterpret_cast<void*>(offset));
	}
	template <typename Type, BufferUsage::buffer_usage U>
	void get_query_result_64(QueryResultType qt, const QueryBufferObject<Type, U> &qbo, std::size_t offset = 0) const {
		qbo.bind();
		glGetQueryObjecti64v(Base::get_resource_id(), static_cast<GLenum>(qt), reinterpret_cast<void*>(offset));
	}
	template <typename Type, BufferUsage::buffer_usage U>
	void get_query_result_u32(QueryResultType qt, const QueryBufferObject<Type, U> &qbo, std::size_t offset = 0) const {
		qbo.bind();
		glGetQueryObjectuiv(Base::get_resource_id(), static_cast<GLenum>(qt), reinterpret_cast<void*>(offset));
	}
	template <typename Type, BufferUsage::buffer_usage U>
	void get_query_result_u64(QueryResultType qt, const QueryBufferObject<Type, U> &qbo, std::size_t offset = 0) const {
		qbo.bind();
		glGetQueryObjectui64v(Base::get_resource_id(), static_cast<GLenum>(qt), reinterpret_cast<void*>(offset));
	}

	core_resource_type resource_type() const override { return type; }
};

}
}
