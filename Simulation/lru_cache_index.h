// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "lru_cache_cacheable.h"

#include "concurrent_unordered_map.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/list.hpp>

#include <list>
#include <string>
#include <iostream>

namespace StE {

template <typename K>
class lru_cache;

template <typename K>
class lru_cache_index {
private:
	friend class lru_cache<K>;

	static constexpr auto index_file = "index.dat";

	using key_type = K;

	struct lru_node {
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version) {
			ar & k;
			ar & name;
		}

		key_type k;
		std::string name;
	};

private:
	using lru_list_type = std::list<lru_node>;
	using lru_list_iterator_type = lru_list_type::const_iterator;
	using val_type = lru_cache_cacheable<key_type, lru_list_iterator_type>;

	using res_kv_map = concurrent_unordered_map<key_type, val_type>;
	using val_data_guard = res_kv_map::value_data_guard_type;

	lru_list_type lru_list;
	res_kv_map map;

	boost::filesystem::path index_path;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & lru_list;
	}

	std::size_t populate_map(const boost::filesystem::path &path) {
		std::size_t size = 0;
		for (lru_list_iterator_type it = lru_list.begin(); it != lru_list.end(); ++it) {
			auto &k = it->k;
			val_type v(k, path, it->name);
			v.mark_live(it);
			size += v.get_size();
			map.emplace(k, std::move(v));
		}

		return size;
	}

	lru_cache_index(const boost::filesystem::path &path, std::atomic<std::size_t> &total_size) : index_path(path / index_file) {
		if (boost::filesystem::exists(index_path)) {
			std::ifstream ifs(index_path.string(), std::ios::binary);
			boost::archive::binary_iarchive ia(ifs);
			ia >> *this;
			total_size = populate_map(path);
		}
	}
	~lru_cache_index() {
		std::ofstream ofs(index_path.string(), std::ios::binary);
		boost::archive::binary_oarchive oa(ofs);
		oa << *this;
	}

	void move_to_lru_front(val_type &v) {
		if (v.is_live())
			lru_list.splice(lru_list.cbegin(), lru_list, v.get_lru_it());
		else {
			lru_list.push_back(lru_node({ v.get_k(), v.get_file_name().string() }));
			v.mark_live(lru_list.cbegin());
		}
	}

	void insert(const key_type &k, val_type &&v) {
		erase(k);
		map.emplace(k, std::move(v));
	}

	std::size_t erase(const key_type &k) {
		auto val_guard = map[k];
		if (!val_guard.is_valid())
			return 0;

		map.remove(k);
		lru_list.erase(val_guard->get_lru_it());
		val_guard->mark_for_deletion();
		return val_guard->get_size();
	}

	std::size_t erase_back() {
		if (!lru_list.size()) return 0;
		auto k = lru_list.back().k;

		return erase(k);
	}
};

}
