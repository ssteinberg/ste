// StE
// © Shlomi Steinberg, 2015-2016

/**	@file	lru_cache_cacheable.hpp
 *	@brief	lru_cache cached object entry in cache index
 *
 *	@author	Shlomi Steinberg
 */

#pragma once

#include <boost/archive/basic_binary_iarchive.hpp>
#include <boost/archive/basic_binary_oarchive.hpp>

#include <filesystem>

#include <lib/string.hpp>
#include <fstream>
#include <chrono>
#include <atomic>
#include <lib/unique_ptr.hpp>
#include <optional.hpp>

namespace ste {

template <typename K, typename lru_iterator_type>
class lru_cache_cacheable {
private:
	static constexpr auto archive_extension = ".stearchive";

	K key;
	byte_t size{ 0 };
	lru_iterator_type lru_it;
	std::atomic<bool> live{ false };

	std::experimental::filesystem::path f;

public:
	lru_cache_cacheable() = default;
	lru_cache_cacheable(const K &k,
						const std::experimental::filesystem::path &path) : lru_cache_cacheable() {
		using namespace std::chrono;
		auto tp = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch());
		auto name = lib::to_string(tp.count()) + archive_extension;

		f = path / name;
		key = k;
	}
	lru_cache_cacheable(const K &k,
						const std::experimental::filesystem::path &path,
						const std::experimental::filesystem::path &file_name) : lru_cache_cacheable() {
		f = path / file_name;
		key = k;
	}
	~lru_cache_cacheable() noexcept {
		if (!f.empty() && !live.load(std::memory_order_acquire)) {
			std::error_code err;
			std::experimental::filesystem::remove(f, err);
		}
	}

	lru_cache_cacheable(lru_cache_cacheable &&c) noexcept
		: key(std::move(c.key)),
		size(c.size),
		lru_it(std::move(c.lru_it)),
		live(c.live.load(std::memory_order_acquire)),
		f(std::move(c.f))
	{}
	lru_cache_cacheable(const lru_cache_cacheable &c) noexcept
		: key(c.key),
		size(c.size),
		lru_it(c.lru_it),
		live(c.live.load(std::memory_order_acquire)),
		f(c.f)
	{}

	void replace(lru_cache_cacheable &&item) {
		size = item.size;
	}

	void mark_live(const lru_iterator_type &it) {
		lru_it = it;
		live.store(true, std::memory_order_release);

		std::error_code err;
		size = byte_t(std::experimental::filesystem::file_size(f, err));
	}
	void mark_for_deletion() {
		live.store(false, std::memory_order_release);
	}
	bool is_live() const { return live.load(std::memory_order_acquire); }
	const K &get_k() const { return key; }
	auto get_size() const { return size; }
	lru_iterator_type& get_lru_it() { return lru_it; }
	std::experimental::filesystem::path get_file_name() const { return f.filename(); }

	template <typename V>
	void archive(V &&v) {
		{
			std::ofstream ofs(f, std::ios::binary);
			boost::archive::binary_oarchive oa(ofs);
			oa << std::forward<V>(v);
		}

		std::error_code err;
		size = byte_t(std::experimental::filesystem::file_size(f, err));
	}
	template <typename V>
	optional<V> unarchive() const {
		std::ifstream ifs(f, std::ios::binary);
		if (ifs) {
			boost::archive::binary_iarchive ia(ifs);
			V v;
			ia >> v;

			return v;
		}

		return none;
	}
};

}
