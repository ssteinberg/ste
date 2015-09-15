// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "llr_resource.h"
#include "bindable_resource.h"

#include <functional>
#include <vector>
#include <map>

namespace StE {
namespace LLR {

class FramebufferObjectAllocator : public llr_resource_stub_allocator {
public:
	static int allocate() { GLuint id;  glCreateFramebuffers(1, &id); return id; }
	static void deallocate(unsigned int &id) { glDeleteFramebuffers(1, reinterpret_cast<GLuint*>(&id)); id = 0; }
};

class FramebufferObjectBinder {
public:
	static void bind(unsigned int id, int sampler = 0) { glBindFramebuffer(GL_FRAMEBUFFER, id); }
	static void unbind(int sampler = 0) { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
};

class FramebufferObject : public bindable_resource<FramebufferObjectAllocator, FramebufferObjectBinder, int>, public bindable_generic_resource {
private:
	GLenum status;

	using Base = bindable_resource<FramebufferObjectAllocator, FramebufferObjectBinder, int>;

public:
	FramebufferObject() {}

	FramebufferObject(FramebufferObject &&t) = default;
	FramebufferObject &operator=(FramebufferObject &&t) = default;

	using Base::bind;
	using Base::unbind;
	void bind() const override final { bind(0); }
	void unbind() const override final { unbind(0); }

	bool set_attachments(const std::vector<std::reference_wrapper<const GenericResource>> color_attachments, const std::vector<int> &levels) {
		bind();
		assert(color_attachments.size() == levels.size());
		for (int i = 0; i < color_attachments.size(); ++i)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_attachments[i].get().get_resource_id(), levels[i]);
		return (status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE;
	}

	bool set_attachments(const GenericResource &depth_attachment, const std::vector<std::reference_wrapper<const GenericResource>> color_attachments, const std::vector<int> &levels) {
		bind();
		assert(color_attachments.size() == levels.size());
		for (int i = 0; i < color_attachments.size(); ++i)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_attachments[i].get().get_resource_id(), levels[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment.get_resource_id(), 0);
		return (status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE;
	}

	bool set_attachments(const GenericResource &depth_attachment, const std::vector<std::reference_wrapper<const GenericResource>> color_attachments) { return set_attachments(depth_attachment, color_attachments, std::vector<int>(color_attachments.size(), 0)); }

	bool set_attachments(const std::vector<std::reference_wrapper<const GenericResource>> color_attachments) {
		bind();
		for (int i = 0; i < color_attachments.size(); ++i)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_attachments[i].get().get_resource_id(), 0);
		return (status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE;
	}

	static void set_rendering_targets(const std::map<int,int> &color_output_to_attachment_map) {
		std::vector<GLenum> indices;
		for (auto &kv : color_output_to_attachment_map) {
			if (indices.size() <= kv.first) indices.resize(kv.first+1, GL_COLOR_ATTACHMENT0 + kv.second);
			else indices[kv.first] = GL_COLOR_ATTACHMENT0 + kv.second;
		}
		glDrawBuffers(indices.size(), &indices[0]);
	}

	bool is_fbo_complete() const { bind(); return is_valid() && status == GL_FRAMEBUFFER_COMPLETE; }
	GLenum get_status_code() const { return status; }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRFrameBufferObject; }
};

}
}
