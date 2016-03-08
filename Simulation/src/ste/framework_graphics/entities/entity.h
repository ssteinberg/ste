// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {
namespace Graphics {
	
class generic_entity {
public:
	virtual void set_model_matrix(const glm::mat4 &m) = 0;
	virtual const glm::mat4 &get_model_transform() const = 0;
};

class entity : public generic_entity {
protected:
	glm::mat4 model_mat{ 1.f };
	
public:
	virtual void set_model_matrix(const glm::mat4 &m) override { model_mat = m; }
	virtual const glm::mat4 &get_model_transform() const override { return model_mat; }
};

class entity_flagged : public entity {
private:
	bool dirty{ false };
	
public:
	virtual void set_model_matrix(const glm::mat4 &m) override {
		entity::set_model_matrix(m);
		dirty = true;
	}
	
	bool is_dirty() const { return dirty; }
	void clear_dirty_flag() { dirty = false; }
};
	
}
}
