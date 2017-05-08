// StE
// © Shlomi Steinberg, 2015-2016

/**	@file	lru_cache_cacheable.hpp
 *	@brief	lru_cache cached object entry in cache index
 *
 *	@author	Shlomi Steinberg
 */

#pragma once

#include <boost_binary_ioarchive.hpp>

#include <boost_filesystem.hpp>

#include <string>
#include <iostream>
#include <chrono>
#include <atomic>

namespace ste {

template <typename K, typename lru_iterator_type>
class lru_cache_cacheable {
private:
	static constexpr auto archive_extension = ".stearchive";

private:
	K key;
	std::uint64_t size{ 0 };
	lru_iterator_type lru_it;
	std::atomic<bool> live{ false };

	boost::filesystem::path f;

public:
	lru_cache_cacheable() = default;
	lru_cache_cacheable(const K &k, 
						const boost::filesystem::path &path) : lru_cache_cacheable() {
		using namespace std::chrono;
		auto tp = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch());
		auto name = std::to_string(tp.count()) + archive_extension;

		f = path / name;
		key = k;
	}
	lru_cache_cacheable(const K &k,
						const boost::filesystem::path &path,
						const boost::filesystem::path &file_name) : lru_cache_cacheable() {
		f = path / file_name;
		key = k;
	}
	~lru_cache_cacheable() {
		if (!live.load(std::memory_order_acquire) && !f.empty()) {
			boost::system::error_code err;
			boost::filesystem::remove(f, err);
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

	void mark_live(const lru_iterator_type &it) {
		lru_it = it;
		live = true;

		boost::system::error_code err;
		size = static_cast<std::uint64_t>(boost::filesystem::file_size(f, err));
	}
	void mark_for_deletion() { live = false; }
	bool is_live() const { return live; }
	const K &get_k() const { return key; }
	auto get_size() const { return size; }
	lru_iterator_type& get_lru_it() { return lru_it; }
	boost::filesystem::path get_file_name() const { return f.filename(); }

	template <typename V>
	void archive(V &&v) {
		{
			std::ofstream ofs(f.string(), std::ios::binary);
			boost::archive::binary_oarchive oa(ofs);
			oa << std::forward<V>(v);
		}

		boost::system::error_code err;
		size = boost::filesystem::file_size(f, err);
	}
	template <typename V>
	V unarchive() const {
		std::ifstream ifs(f.string(), std::ios::binary);
		boost::archive::binary_iarchive ia(ifs);
		V v;
		ia >> v;

		return v;
	}
};

}
