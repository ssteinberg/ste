//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <resource_storage.hpp>

#include <stable_vector.hpp>

#include <type_traits>

namespace ste {
namespace gl {

template <typename Descriptor>
class resource_storage_stable : public resource_storage<resource_storage_stable<Descriptor>, Descriptor, stable_vector> {
	using Base = resource_storage<resource_storage_stable<Descriptor>, Descriptor, stable_vector>;

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
