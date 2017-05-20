//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

template <typename BufferAllocator, typename BufferT>
class ste_device_queue_secondary_command_buffer : public allow_type_decay<ste_device_queue_secondary_command_buffer<BufferAllocator, BufferT>, BufferT> {
private:
	const BufferAllocator *parent;
	BufferT buffer;

public:
	ste_device_queue_secondary_command_buffer(const BufferAllocator *parent, BufferT &&buffer)
		: parent(parent), buffer(std::move(buffer))
	{}

	ste_device_queue_secondary_command_buffer(ste_device_queue_secondary_command_buffer&& o) noexcept : parent(o.parent), buffer(std::move(o.buffer)) {
		o.parent = nullptr;
	}
	ste_device_queue_secondary_command_buffer &operator=(ste_device_queue_secondary_command_buffer&& o) noexcept {
		if (parent)
			parent->dealloc(std::move(*this));

		parent = o.parent;
		buffer = std::move(o.buffer);
		o.parent = nullptr;

		return *this;
	}

	~ste_device_queue_secondary_command_buffer() noexcept {
		if (parent)
			parent->dealloc(std::move(*this));
	}

	const auto& get() const { return buffer; }

	auto record(const secondary_command_buffer_inheritance &inheritance = {}) {
		return buffer.record(inheritance);
	}
	auto get_type() const { return get().get_type(); }
	auto& get_queue_descriptor() const { return buffer.get().get_queue_descriptor(); }
};

}
}
