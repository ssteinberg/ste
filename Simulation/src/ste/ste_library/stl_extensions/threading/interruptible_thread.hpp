//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <memory>
#include <thread>
#include <exception>

#include <boundary.hpp>

namespace ste {

/*
 *	@brief	Simple std::thread wrapper object that adds an interruption flag.
 *			Running thread needs to check the is_interruption_flag_set() thread local flag.
 */
class interruptible_thread {
private:
	struct interruptible_thread_data {
		std::atomic<bool> flag{ false };
		std::atomic<bool> interrupted{ false };
		boundary<bool> promise;

		void interrupt() { flag = true; }
		bool is_interrupted() const { return flag; }
	};

private:
	static thread_local interruptible_thread_data *interruption_flag;

public:
	static bool is_interruption_flag_set() { return interruption_flag->is_interrupted(); }

private:
	std::shared_ptr<interruptible_thread_data> data{ nullptr };
	std::thread t;

public:
	template <typename Functor>
	interruptible_thread(Functor &&func)
		: data(std::make_shared<interruptible_thread_data>()),
		  t([f = std::move(func), data = data]() mutable {
			  // Set thread local flag pointer
			  interruption_flag = data.get();

			  // Run
			  f();
			  data->promise.signal_at_thread_exit(true);
		  })
	{}

	~interruptible_thread() {
		if (data != nullptr) {
			interrupt();
			if (joinable())
				join();
		}
	}

	interruptible_thread(interruptible_thread &&) = default;
	interruptible_thread &operator=(interruptible_thread &&) = default;

	/*
	 *	@brief	Sets the interruption flag
	 */
	void interrupt() { data->interrupt(); }

	/*
	*	@brief	Checks if thread's interruption signal was set
	*/
	bool is_interrupted() const { return data->is_interrupted(); }

	/*
	*	@brief	Checks if thread has terminated
	*/
	bool is_terminated() const { return data->promise.is_signaled(); }

	void join() { t.join(); }
	bool joinable() const { return t.joinable(); }

	void detach() && {
		t.detach();
		data = nullptr;
	}

	std::thread &get_thread() { return t; }
};

}
