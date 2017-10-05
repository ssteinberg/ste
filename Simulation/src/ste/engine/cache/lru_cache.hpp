//	StE
// © Shlomi Steinberg 2015-2017

/**	@file	lru_cache.hpp
 *	@brief	LRU general purpose disk caching class
 *
 *	@author	Shlomi Steinberg
 */

#pragma once

#include <lru_cache_cacheable.hpp>
#include <lru_cache_index.hpp>

#include <lib/concurrent_queue.hpp>
#include <interruptible_thread.hpp>

#include <condition_variable>
#include <atomic>
#include <mutex>
#include <anchored.hpp>

#include <lib/aligned_padded_ptr.hpp>

namespace ste {

/**
 *	@brief	LRU general purpose disk caching class
 *
 *	lru_cache provides thread safe, lock-free general purpose disk caching facilities. Fetching and sotiring operations can be ran asynchronously, however it is the consumer's responsibility to synchronize insertion of 
 *	items with identical keys.
 *	lru_cache (de)serializes using boost::serialization.
 *	lru_cache keeps an offline database (index) on disk.
 *
 *	@param K	key type
 */
template <typename K>
class lru_cache : anchored {
private:
	using key_type = K;
	using index_type = lru_cache_index<key_type>;
	using cacheable = typename index_type::val_type;

	static constexpr std::uint64_t write_index_every_ops = 10;

private:
	struct shared_data_t {
		std::atomic<std::uint64_t> ops{ 0 };

		mutable std::mutex m;
		mutable std::condition_variable cv;

		mutable lib::concurrent_queue<typename index_type::val_data_guard> accessed_queue;

		std::atomic<std::uint64_t> total_size{ 0 };
	};

private:
	const std::experimental::filesystem::path path;
	const byte_t quota;
	lib::aligned_padded_ptr<shared_data_t> shared_data;

	index_type index;

	interruptible_thread t;

private:
	void shutdown() {
		t.interrupt();
		do { shared_data->cv.notify_one(); } while (!shared_data->m.try_lock());
		shared_data->m.unlock();
		if (t.joinable())
			t.join();
	}

	void item_accessed(typename index_type::val_data_guard &&val_guard) const {
		shared_data->accessed_queue.push(std::move(val_guard));
		shared_data->cv.notify_one();
	}

public:
	lru_cache(lru_cache &&) = delete;
	lru_cache(const lru_cache &) = delete;
	lru_cache &operator=(lru_cache &&) = delete;
	lru_cache &operator=(const lru_cache &) = delete;

	/**
	*	@brief	lru_cache ctor
	*
	* 	@param path		Cache directory
	*	@param quota	Max size in bytes. 0 for unlimited.
	*/
	lru_cache(const std::experimental::filesystem::path &path, byte_t quota = 0_B)
		: path(path),
		quota(quota),
		index(path, shared_data->total_size),
		t([this] ()
	{
		for (;;) {
			if (interruptible_thread::is_interruption_flag_set()) return;

			{
				std::unique_lock<std::mutex> l(shared_data->m);
				shared_data->cv.wait(l);
			}

			auto accessed_item = shared_data->accessed_queue.pop();
			while (accessed_item != nullptr) {
				index.move_to_lru_front(**accessed_item);
				accessed_item = shared_data->accessed_queue.pop();
			}

			auto ts = byte_t(shared_data->total_size.load(std::memory_order_relaxed));
			while (ts > this->quota && this->quota) {
				auto size = this->index.erase_back();
				assert(size);
				if (!size)
					break;

				ts = byte_t(shared_data->total_size.fetch_sub(size, std::memory_order_relaxed));
			}

			if (shared_data->ops.load(std::memory_order_relaxed) > write_index_every_ops) {
				shared_data->ops.store(0, std::memory_order_release);

				this->index.write_index();
			}
		}
	}) 
	{
		std::experimental::filesystem::create_directory(path);
	}
	~lru_cache() noexcept { shutdown(); }

	/**
	*	@brief	Store an object in the cache
	*
	* 	@param k	key
	*	@param v	object to serialize
	*/
	template <typename V>
	void insert(const key_type &k, V &&v) {
		try {
			cacheable item(k, path);
			item.archive(std::forward<V>(v));
			index.insert(k, std::move(item));
		}
#ifdef _DEBUG
		catch (const std::exception &e) {
			std::cerr << "lru_cache: Failed archiving: " << e.what() << std::endl;
#else
		catch (const std::exception &) {
#endif
			return;
		}

		std::atomic_thread_fence(std::memory_order_acquire);
		auto val_guard = index.map.try_get(k);
		if (!val_guard.is_valid())
			return;

		auto item_size = val_guard->get_size();
		assert(item_size);
		shared_data->total_size.fetch_add(item_size, std::memory_order_relaxed);
		item_accessed(std::move(val_guard));

		++shared_data->ops;
	}

	/**
	*	@brief	Returns a lambda that read the object associated with key k from the cache, if any.
	*
	* 	@param k	key
	*/
	template <typename V>
	optional<V> get(const key_type &k) const {
		auto val_guard = this->index.map[k];
		if (!val_guard.is_valid())
			return none;

		try {
			auto v = val_guard->template unarchive<V>();
			if (v) {
				this->item_accessed(std::move(val_guard));
				return std::move(v);
			}

			return none;
		}
#ifdef _DEBUG
		catch (const std::exception &e) {
			std::cerr << "lru_cache: Failed unarchiving: " << e.what() << std::endl;
#else
		catch (const std::exception &) {
#endif
			return none;
		}
	}
};

}
