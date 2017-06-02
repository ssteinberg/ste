//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <resource_storage.hpp>
#include <command.hpp>

#include <vector.hpp>

#include <lib/vector.hpp>
#include <lib/unique_ptr.hpp>
#include <algorithm>

namespace ste {
namespace gl {

template <typename Descriptor>
class resource_storage_dynamic : public resource_storage<resource_storage_dynamic<Descriptor>, Descriptor, vector<Descriptor>> {
	using Base = resource_storage<resource_storage_dynamic<Descriptor>, Descriptor, vector<Descriptor>>;

private:
	std::size_t resource_counter{ 0 };
	lib::vector<std::size_t> identifiers_order;

private:
	int index_of_with_identifier(std::size_t resource_storage_identifier) const override final {
		auto it = std::lower_bound(identifiers_order.begin(), identifiers_order.end(), resource_storage_identifier);
		if (it != identifiers_order.end() && *it == resource_storage_identifier)
			return static_cast<int>(it - identifiers_order.begin());
		return -1;
	}
	lib::unique_ptr<command> allocate_identifier(const typename Base::descriptor_type &descriptor,
												 std::size_t &identifier_out) override final {
		identifier_out = resource_counter++;
		identifiers_order.push_back(identifier_out);

		auto cmd = Base::store.push_back_cmd(descriptor);
		return lib::allocate_unique<decltype(cmd)>(std::move(cmd));
	}
	lib::unique_ptr<command> deallocate_identifier(std::size_t resource_storage_identifier) override final {
		int idx = index_of_with_identifier(resource_storage_identifier);
		if (idx >= 0) {
			identifiers_order.erase(identifiers_order.begin() + idx);

			auto cmd = Base::store.erase_and_shift_cmd(idx);
			return lib::allocate_unique<decltype(cmd)>(std::move(cmd));
		}

		return nullptr;
	}

public:
	resource_storage_dynamic(const ste_context &ctx,
							 const buffer_usage &usage)
		: Base(ctx, usage)
	{}
	virtual ~resource_storage_dynamic() {}
};

}
}
