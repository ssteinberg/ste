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
#include <memory>
#include <atomic>

namespace StE {

template <typename K, typename lru_iterator_type>
class lru_cache_cacheable {
private:
	struct data {
		K k;
		std::size_t size{ 0 };
		lru_iterator_type lru_it;
		std::atomic<bool> live{ false };

		boost::filesystem::path f;
	};

	std::shared_ptr<data> pdata;

	static constexpr auto archive_extension = ".stearchive";

public:
	lru_cache_cacheable() : pdata(std::make_shared<data>()) {}
	lru_cache_cacheable(const K &k, const boost::filesystem::path &path) : lru_cache_cacheable() {
		using namespace std::chrono;
		auto tp = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch());
		auto name = std::to_string(tp.count()) + archive_extension;

		pdata->f = path / name;
		pdata->k = k;
	}
	lru_cache_cacheable(const K &k, const boost::filesystem::path &path, const boost::filesystem::path &file_name) : lru_cache_cacheable() {
		pdata->f = path / file_name;
		pdata->k = k;
	}
	~lru_cache_cacheable() {
		if (pdata!=nullptr && !pdata->live.load() && !pdata->f.empty()) {
			boost::system::error_code err;
			boost::filesystem::remove(pdata->f, err);
		}
	}

	lru_cache_cacheable(lru_cache_cacheable &&c) = default;
	lru_cache_cacheable(const lru_cache_cacheable &c) = default;

	void mark_live(const lru_iterator_type &it) {
		pdata->lru_it = it;
		pdata->live = true;

		boost::system::error_code err;
		pdata->size = boost::filesystem::file_size(pdata->f, err);
	}
	void mark_for_deletion() { pdata->live = false; }
	bool is_live() const { return pdata->live; }
	const K &get_k() const { return pdata->k; }
	std::size_t get_size() const { return pdata->size; }
	lru_iterator_type& get_lru_it() { return pdata->lru_it; }
	boost::filesystem::path get_file_name() const { return pdata->f.filename(); }

	template <typename V>
	void archive(V &&v) {
		{
			std::ofstream ofs(pdata->f.string(), std::ios::binary);
			boost::archive::binary_oarchive oa(ofs);
			oa << std::forward<V>(v);
		}

		boost::system::error_code err;
		pdata->size = boost::filesystem::file_size(pdata->f, err);
	}
	template <typename V>
	V unarchive() const {
		std::ifstream ifs(pdata->f.string(), std::ios::binary);
		boost::archive::binary_iarchive ia(ifs);
		V v;
		ia >> v;

		return v;
	}
};

}
