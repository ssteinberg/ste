// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "Texture2D.h"

#include <functional>
#include <vector>
#include <map>

namespace StE {
namespace LLR {

class FramebufferObject {
private:
	GLuint id;
	GLenum status;

public:
	FramebufferObject(FramebufferObject &&t) = default;
	FramebufferObject(const FramebufferObject &t) = delete;

	FramebufferObject &operator=(FramebufferObject &&t) = default;
	FramebufferObject &operator=(const FramebufferObject &t) = delete;

	FramebufferObject() { glGenFramebuffers(1, &id); }
	virtual ~FramebufferObject() { if (this->is_valid()) glDeleteFramebuffers(1, &id); }

	bool set_attachments(const std::vector<std::reference_wrapper<const Texture2D>> color_attachments, const std::vector<int> &levels) {
		bind();
		assert(color_attachments.size() == levels.size());
		for (int i = 0; i < color_attachments.size(); ++i)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_attachments[i].get().get_texture_id(), levels[i]);
		return (status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE;
	}

	bool set_attachments(const Texture2D &depth_attachment, const std::vector<std::reference_wrapper<const Texture2D>> color_attachments, const std::vector<int> &levels) {
		bind();
		assert(color_attachments.size() == levels.size());
		for (int i = 0; i < color_attachments.size(); ++i)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_attachments[i].get().get_texture_id(), levels[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment.get_texture_id(), 0);
		return (status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE;
	}

	bool set_attachments(const Texture2D &depth_attachment, const std::vector<std::reference_wrapper<const Texture2D>> color_attachments) { return set_attachments(depth_attachment, color_attachments, std::vector<int>(color_attachments.size(), 0)); }

	bool set_attachments(const std::vector<std::reference_wrapper<const Texture2D>> color_attachments) {
		bind();
		for (int i = 0; i < color_attachments.size(); ++i)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_attachments[i].get().get_texture_id(), 0);
		return (status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE;
	}

	void bind() const { assert(is_valid()); glBindFramebuffer(GL_FRAMEBUFFER, id); }
	static void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
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

	bool is_valid() const { return id != 0; }

protected:
	GLuint get_fbo_id() const { return id; }
};

}
}
