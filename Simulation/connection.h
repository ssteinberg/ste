// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <memory>
#include <functional>

#include "signal.h"

namespace StE {

template <typename ... Ts>
class signal;

template <typename ... Ts>
class connection {
private:
	friend class signal<Ts...>;

	using lambda = std::function<void(Ts...)>;

private:
	int id;
	const signal<Ts...>* sig;
	lambda l;

	void operator()(const Ts&...args) { l(args...); }

public:
	connection(const connection &s) = delete;
	connection& operator=(const connection &s) = delete;
	connection(connection &&s) = default;
	connection& operator=(connection &&s) = default;

	connection(lambda &&l) : l(std::move(l)) {}
	~connection() {
		if (connected())
			sig->disconnect(id);
		break_connection();
	}

	void break_connection() { sig = nullptr; }
	bool connected() const { return sig != nullptr; }
};

}
