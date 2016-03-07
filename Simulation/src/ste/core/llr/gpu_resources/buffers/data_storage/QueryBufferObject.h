// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.h"
#include "buffer_usage.h"

namespace StE {
namespace LLR {

class QueryBufferObjectGeneric {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;

	virtual ~QueryBufferObjectGeneric() noexcept {}
};

template <typename Type, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class QueryBufferObject : public buffer_object<Type, U>, public QueryBufferObjectGeneric {
private:
	using Base = buffer_object<Type, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	QueryBufferObject(QueryBufferObject &&m) = default;
	QueryBufferObject& operator=(QueryBufferObject &&m) = default;

	using Base::Base;

	void bind() const final override { Binder::bind(get_resource_id(), GL_QUERY_BUFFER); };
	void unbind() const final override { Binder::unbind(GL_QUERY_BUFFER); };
	void bind_range(const LayoutLocationType &sampler, int offset, std::size_t size) const { Base::bind_range(sampler, GL_QUERY_BUFFER, offset, size); }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRQueryBufferObject; }
};

}
}
