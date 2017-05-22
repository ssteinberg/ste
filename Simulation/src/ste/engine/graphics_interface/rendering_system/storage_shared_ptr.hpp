//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <storage.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

class rendering_system;

namespace _internal {

template <typename RS>
struct storage_shared_ptr_base {
	storage_base *storage;
	alias<RS> rs;
	const void* tag;
	std::uint32_t references{ 0 };

	storage_shared_ptr_base() = default;
	storage_shared_ptr_base(storage_base *storage, alias<RS> rs, const void *tag)
		: storage(storage), rs(rs), tag(tag)
	{}

	void add_ref() {
		++references;
	}
	void release() {
		assert(references > 0);
		if (--references == 0) {
			lib::default_alloc<storage_base>::destroy(storage);
			rs->remove_storage(*this);
		}
	}

	bool operator<(const storage_shared_ptr_base<RS> &rhs) const {
		return tag < rhs.tag;
	}
};


}

template <typename Storage>
class storage_shared_ptr {
	using ptr_base = _internal::storage_shared_ptr_base<rendering_system>;

private:
	Storage *storage;
	ptr_base *base;

public:
	storage_shared_ptr(ptr_base &base)
		: storage(reinterpret_cast<Storage*>(base.storage)),
		base(&base)
	{
		this->base->add_ref();
	}
	~storage_shared_ptr() noexcept {
		if (base)
			base->release();
	}

	storage_shared_ptr(storage_shared_ptr&& o) noexcept : storage(o.storage), base(o.base) {
		o.base = nullptr;
	}
	storage_shared_ptr &operator=(storage_shared_ptr&& o) noexcept {
		if (base)
			base->release();

		storage = o.storage;
		base = o.base;
		o.base = nullptr;

		return *this;
	}
	storage_shared_ptr(const storage_shared_ptr& o) : storage(o.storage), base(o.base) {
		base->add_ref();
	}
	storage_shared_ptr &operator=(const storage_shared_ptr& o) noexcept {
		if (base)
			base->release();

		storage = o.storage;
		base = o.base;
		base->add_ref();

		return *this;
	}

	auto& operator*() { return *storage; }
	const auto& operator*() const { return *storage; }
	auto* operator->() { return storage; }
	const auto* operator->() const { return storage; }
};

}
}
