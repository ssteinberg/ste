// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "entity.hpp"

#include "signal.hpp"

namespace StE {
namespace Graphics {

template <typename O>
class entity_signalling : public entity_dquat {
	using Base = entity_dquat;

private:
	signal<O> model_change_signal;
	O obj;

public:
	using signal_type = decltype(model_change_signal);

public:
	entity_signalling(O obj) : obj(obj) {}
	~entity_signalling() noexcept {}

	const signal_type &signal_model_change() const { return model_change_signal; }

	virtual void set_model_transform(const glm::dualquat &q) override {
		Base::set_model_transform(q);
		model_change_signal.emit(obj);
	}
};

}
}
