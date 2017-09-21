// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <memory>
#include <functional>

namespace ste {

template <typename ... Ts>
class signal;

/**
*	@brief	Connection to a signal. 
*/
template <typename ... Ts>
class connection {
private:
	friend class signal<Ts...>;

	using lambda = std::function<void(Ts...)>;

private:
	lambda l;
	signal<Ts...>* sig{ nullptr };

	void disconnect() {
		if (sig)
			sig->disconnect(this);
		sig = nullptr;
	}

public:
	connection() = default;
	template <typename L>
	connection(L &&l) : l(std::forward<L>(l)) {}
	~connection() noexcept {
		disconnect();
	}

	connection(const connection &s) = delete;
	connection& operator=(const connection &s) = delete;

	connection(connection &&s) noexcept : l(std::move(s.l)) {
		if (s.sig) {
			sig = s.sig;
			s.disconnect();
			sig->connect(this);
		}
	}
	connection& operator=(connection &&s) noexcept {
		l = std::move(s.l);

		if (sig) {
			disconnect();
		}

		if (s.sig) {
			sig = s.sig;
			s.disconnect();
			sig->connect(this);
		}

		return *this;
	}

	bool connected() const { return sig != nullptr; }

	template <typename... Args>
	void operator()(Args&&...args) const {
		l(std::forward<Args>(args)...);
	}
};

}
