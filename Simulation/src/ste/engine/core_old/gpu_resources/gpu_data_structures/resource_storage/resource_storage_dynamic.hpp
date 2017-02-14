// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>
#include <resource_storage.hpp>

#include <gstack.hpp>

#include <is_base_of.hpp>

#include <vector>
#include <algorithm>

namespace StE {
namespace Core {

template <typename Descriptor>
class resource_storage_dynamic : public resource_storage<resource_storage_dynamic<Descriptor>, Descriptor, gstack> {
	using Base = resource_storage<resource_storage_dynamic<Descriptor>, Descriptor, gstack>;

private:
	std::size_t resource_counter{ 0 };
	std::vector<std::size_t> identifiers_order;

private:
	int index_of_with_identifier(std::size_t resource_storage_identifier) const override final {
		auto it = std::lower_bound(identifiers_order.begin(), identifiers_order.end(), resource_storage_identifier);
		if (*it == resource_storage_identifier)
			return static_cast<int>(it - identifiers_order.begin());
		return -1;
	}
	std::size_t allocate_identifier(const typename Base::descriptor_type &descriptor) override final {
		auto id = resource_counter++;
		identifiers_order.push_back(id);
		Base::stack.push_back(descriptor);

		return id;
	}
	void deallocate_identifier(std::size_t resource_storage_identifier) override final {
		int idx = index_of_with_identifier(resource_storage_identifier);
		if (idx >= 0) {
			identifiers_order.erase(identifiers_order.begin() + idx);
			Base::stack.erase_and_shift(idx);
		}
	}

public:
	virtual ~resource_storage_dynamic() {}
};

}
}
