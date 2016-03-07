// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "renderable.h"

#include "optional.h"

#include "gl_current_context.h"

#include <memory>
#include <vector>
#include <list>

namespace StE {
namespace Graphics {

class rendering_queue {
private:
	std::list<renderable*> q;
	std::vector<std::unique_ptr<renderable>> ptrs;

public:
	void push_back(renderable* p) {
		if (p)
			q.push_back(p);
	}

	void push_back(std::unique_ptr<renderable> &&p) {
		if (p != nullptr) {
			q.push_back(p.get());
			ptrs.push_back(std::move(p));
		}
	}

	void push_front(renderable* p) {
		if (p)
			q.push_front(p);
	}

	void push_front(std::unique_ptr<renderable> &&p) {
		if (p != nullptr) {
			q.push_front(p.get());
			ptrs.push_back(std::move(p));
		}
	}

	void render(const LLR::GenericFramebufferObject *default_fbo) {
		auto p = q.begin();
		decltype(p) prev = q.end();

		while (p != q.end()) {
			auto *prev_states = prev != q.end() ? &(*prev)->get_requested_states() : nullptr;
			auto *states = &(*p)->get_requested_states();

			for (auto &s : *states) {
				optional<bool> prev_state = none;
				if (prev_states) {
					auto it = prev_states->find(s.first);
					if (it != prev_states->end())
						prev_state = it->second;
				}
				if (!prev_state || *prev_state != s.second)
					LLR::gl_current_context::get()->set_state(s.first, s.second);
			}

			if (prev_states) {
				for (auto &s : *prev_states) {
					auto it = states->find(s.first);
					if (it != states->end())
						continue;

					LLR::gl_current_context::get()->restore_default_state(s.first);
				}
			}

			bool reset_fbo = false;
			if (!(*p)->get_output_fbo()) {
				(*p)->set_output_fbo(default_fbo);
				reset_fbo = true;
			}

			(**p)();

			if (reset_fbo)
				(*p)->set_output_fbo(nullptr);

			prev = std::move(p);
			++p;

			if (p == q.end()) {
				for (auto &s : *states) 
					LLR::gl_current_context::get()->restore_default_state(s.first);
			}
		}

		q.clear();
		ptrs.clear();
	}
};

}
}
