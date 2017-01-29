// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <memory>
#include <thread>
#include <future>
#include <exception>

namespace StE {

class interruptible_thread {
private:
	class interruptible_thread_flag {
	private:
		std::atomic<bool> flag{ false };

	public:
		void set() { flag = true; }
		bool is_set() const { return flag; }
	};

private:
	static thread_local interruptible_thread_flag *interruption_flag;

public:
	static bool is_interruption_flag_set() { return interruption_flag->is_set(); }

private:
	std::unique_ptr<interruptible_thread_flag> flag;
	std::unique_ptr<std::promise<bool>> promise;
	std::future<bool> future;
	std::thread t;

public:
	template <typename Functor>
	interruptible_thread(Functor&& func) : flag(std::make_unique<interruptible_thread_flag>()),
										   promise(std::make_unique<std::promise<bool>>()),
										   future(promise->get_future()),
										   t([f = std::move(func), ptr_flag = flag.get(), ptr_promise = promise.get()]() {
		interruption_flag = ptr_flag;
		try {
			f();
			ptr_promise->set_value(true);
		}
		catch (...) {
			ptr_promise->set_exception(std::current_exception());
		}
	}) {}
	~interruptible_thread() {
		if (flag != nullptr) {
			interrupt();
			if (joinable())
				join();
		}
	}

	interruptible_thread(interruptible_thread &&other)  noexcept : flag(std::move(other.flag)),
																   promise(std::move(other.promise)),
																   future(std::move(other.future)),
																   t(std::move(other.t)) {
		other.flag = nullptr;
	}

	interruptible_thread &operator=(interruptible_thread &&other) noexcept {
		flag = std::move(other.flag);
		promise = std::move(other.promise);
		future = std::move(other.future);
		t = std::move(other.t);

		other.flag = nullptr;

		return *this;
	}

	void interrupt() { flag->set(); }
	void join() { t.join(); }
	bool joinable() const { return t.joinable(); }
	void detach() { t.detach(); }
	auto &get_future() { return future; }
	auto &get_future() const { return future; }

	std::thread &get_thread() { return t; }
};

}
