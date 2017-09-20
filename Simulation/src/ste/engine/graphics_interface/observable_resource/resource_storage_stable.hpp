//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <resource_storage.hpp>
#include <command.hpp>

#include <stable_vector.hpp>
#include <lib/unique_ptr.hpp>

namespace ste {
namespace gl {

template <typename Descriptor>
class resource_storage_stable : public resource_storage<resource_storage_stable<Descriptor>, Descriptor, stable_vector<Descriptor>> {
	using Base = resource_storage<resource_storage_stable<Descriptor>, Descriptor, stable_vector<Descriptor>>;

private:
	int index_of_with_identifier(std::size_t resource_storage_identifier) const override final {
		return static_cast<int>(resource_storage_identifier);
	}
	lib::unique_ptr<command> allocate_identifier(const typename Base::descriptor_type &descriptor,
												 std::size_t &identifier_out) override final {
		std::uint64_t location;
		auto cmd = lib::allocate_unique<Base::storage_type::insert_cmd_t>(Base::store.insert_cmd(Base::ctx, descriptor, location));

		identifier_out = static_cast<std::size_t>(location);

		return cmd;
	}
	lib::unique_ptr<command> deallocate_identifier(std::size_t resource_storage_identifier) override final {
		const int idx = index_of_with_identifier(resource_storage_identifier);
		Base::store.tombstone(idx);

		return nullptr;
	}

public:
	resource_storage_stable(const ste_context &ctx,
							const buffer_usage &usage,
							const char *name)
		: Base(ctx, 
			   usage,
			   name)
	{}
	virtual ~resource_storage_stable() {}
};

}
}
