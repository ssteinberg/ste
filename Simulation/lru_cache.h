// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "lru_cache_cacheable.h"
#include "lru_cache_index.h"

#include "concurrent_queue.h"
#include "task.h"
#include "interruptible_thread.h"

#include <string>

#include <atomic>
#include <mutex>
#include <condition_variable>

#include <memory>

namespace StE {

template <typename K>
class lru_cache {
private:
	using key_type = K;
	using index_type = lru_cache_index<key_type>;
	using cacheable = typename index_type::val_type;
	
	static constexpr std::uint64_t write_index_every_ops = 10;

private:
	index_type index;
	
	std::atomic<std::uint64_t> ops{ 0 };

	mutable std::mutex m;
	mutable std::condition_variable cv;

	mutable concurrent_queue<typename index_type::val_data_guard> accessed_queue;

	std::atomic<std::size_t> total_size{ 0 };
	boost::filesystem::path path;
	std::size_t quota;

	interruptible_thread t;

private:
	void shutdown() {
		t.interrupt();
		do { cv.notify_one(); } while (!m.try_lock());
		m.unlock();
		t.join();
	}

	void item_accessed(typename index_type::val_data_guard &&val_guard) const {
		accessed_queue.push(std::move(val_guard));
		cv.notify_one();
	}

public:
	lru_cache(lru_cache &&) = delete;
	lru_cache(const lru_cache &) = delete;
	lru_cache &operator=(lru_cache &&) = delete;
	lru_cache &operator=(const lru_cache &) = delete;

	lru_cache(const boost::filesystem::path &path, std::size_t quota = 0) : index(path, total_size), path(path), quota(quota), t([this] (){
		auto flag = interruptible_thread::interruption_flag;
		for (;;) {
			if (flag->is_set()) return;
			
			{
				std::unique_lock<std::mutex> l(this->m);
				this->cv.wait(l);
			}

			auto accessed_item = accessed_queue.pop();
			while (accessed_item != nullptr) {
				index.move_to_lru_front(**accessed_item);
				accessed_item = accessed_queue.pop();
			}

			std::size_t ts = this->total_size.load(std::memory_order_relaxed);
			while (ts > this->quota && this->quota) {
				auto size = this->index.erase_back();
				assert(size);
				if (!size)
					break;

				ts = this->total_size.fetch_sub(size, std::memory_order_relaxed);
			}
			
			if (this->ops.load(std::memory_order_relaxed) > write_index_every_ops) {
				this->ops.store(0, std::memory_order_release);
				
				this->index.write_index();
			}
		}
	}) {
		boost::filesystem::create_directory(path);
	}
	~lru_cache() { shutdown(); }

	template <typename V>
	void insert(const key_type &k, V &&v) {
		try {
			cacheable item(k, path);
			item.archive(std::forward<V>(v));
			index.insert(k, std::move(item));
		}
		catch (const std::exception &ex) {
			return;
		}

		std::atomic_thread_fence(std::memory_order_acquire);
		auto val_guard = index.map.try_get(k);
		if (!val_guard.is_valid())
			return;

		auto item_size = val_guard->get_size();
		assert(item_size);
		total_size.fetch_add(item_size, std::memory_order_relaxed);
		item_accessed(std::move(val_guard));
		
		ops++;
	}

	template <typename V>
	task<optional<V>> get(const key_type &k) const {
		return [=]() -> optional<V> {
			auto val_guard = this->index.map[k];
			if (!val_guard.is_valid())
				return none;

			try {
				auto v = val_guard->template unarchive<V>();
				this->item_accessed(std::move(val_guard));
				return optional<V>(std::move(v));
			}
			catch (const std::exception &ex) {
				return none;
			}
		};
	}
};

}
