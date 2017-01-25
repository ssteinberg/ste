// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.hpp"
#include "buffer_usage.hpp"

namespace StE {
namespace Core {

class query_buffer_object_generic {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;

	virtual ~query_buffer_object_generic() noexcept {}
};

template <typename Type, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class query_buffer_object : public buffer_object<Type, U>, public query_buffer_object_generic {
private:
	using Base = buffer_object<Type, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	query_buffer_object(query_buffer_object &&m) = default;
	query_buffer_object& operator=(query_buffer_object &&m) = default;

	using Base::Base;

	void bind() const final override { Base::Binder::bind(Base::get_resource_id(), GL_QUERY_BUFFER); };
	void unbind() const final override { Base::Binder::unbind(GL_QUERY_BUFFER); };
	void bind_range(const typename Base::LayoutLocationType &sampler, int offset, std::size_t size) const { Base::bind_range(sampler, GL_QUERY_BUFFER, offset, size); }

	core_resource_type resource_type() const override { return core_resource_type::QueryBufferObject; }
};

}
}
