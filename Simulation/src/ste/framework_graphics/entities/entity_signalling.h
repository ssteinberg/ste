// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "entity.h"

#include "signal.h"

namespace StE {
namespace Graphics {
	
template <typename O>
class entity_signalling : public entity_affine {
	using Base = entity_affine;
	
private:
	signal<O> model_change_signal;
	O obj;
	
public:
	using signal_type = decltype(model_change_signal);
	
public:
	entity_signalling(O obj) : obj(obj) {}
	
	const signal_type &signal_model_change() const { return model_change_signal; }
	
	virtual void set_model_matrix(const glm::mat4 &m) override {
		Base::model_mat = m;
		
		model_change_signal.emit(obj);
	}
};

}
}
