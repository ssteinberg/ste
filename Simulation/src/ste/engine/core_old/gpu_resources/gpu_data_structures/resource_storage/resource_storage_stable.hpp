// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>
#include <resource_storage.hpp>

#include <gstack_stable.hpp>

#include <is_base_of.hpp>

#include <type_traits>

namespace StE {
namespace Core {

template <typename Descriptor>
class resource_storage_stable : public resource_storage<resource_storage_stable<Descriptor>, Descriptor, gstack_stable> {
	using Base = resource_storage<resource_storage_stable<Descriptor>, Descriptor, gstack_stable>;

private:
	int index_of_with_identifier(std::size_t resource_storage_identifier) const override final {
		return resource_storage_identifier;
	}
	std::size_t allocate_identifier(const typename Base::descriptor_type &descriptor) override final {
		return Base::stack.insert(descriptor);
	}
	void deallocate_identifier(std::size_t resource_storage_identifier) override final {
		int idx = index_of_with_identifier(resource_storage_identifier);
		Base::stack.mark_tombstone(idx);
	}

public:
	virtual ~resource_storage_stable() {}
};

}
}
