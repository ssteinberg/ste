// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "Font.h"
#include "glyph_manager.h"

#include "AttributedString.h"

#include "StEngineControl.h"
#include "task.h"

#include "VertexArrayObject.h"
#include "VertexBufferObject.h"
#include "GLSLProgram.h"
#include "Sampler.h"

#include <memory>
#include <string>
#include <vector>

namespace StE {
namespace Text {

class TextRenderer {
private:
	struct glyph_point {
		struct {
			glm::vec2 pos;
			float glyph, size;
		};
		glm::vec4 color;
		float weight;

		using descriptor = LLR::VBODescriptorWithTypes<glm::vec4, glm::vec4, float>::descriptor;
	};

private:
	std::unique_ptr<LLR::GLSLProgram> text_distance_mapping;

	static constexpr int vbo_ring_size = 4096;
	static constexpr LLR::BufferUsage::buffer_usage buffer_usage = static_cast<LLR::BufferUsage::buffer_usage>(LLR::BufferUsage::BufferUsageMapCoherent | LLR::BufferUsage::BufferUsageMapWrite | LLR::BufferUsage::BufferUsageMapPersistent);
	using vbo_type = LLR::VertexBufferObject<glyph_point, glyph_point::descriptor, buffer_usage>;
	using vbo_mapped_ptr_type = LLR::mapped_buffer_object_unique_ptr<vbo_type::T, vbo_type::access_usage>;

	std::unique_ptr<vbo_type> vbo;
	vbo_mapped_ptr_type vbo_mapped_ptr;
	int vbo_ring_current_offset{ 0 };

	LLR::VertexArrayObject vao;
	LLR::Sampler text_glyph_sampler;

	glyph_manager gm;
	Font default_font;
	int default_size;
	const StEngineControl &context;

	void adjust_line(std::vector<glyph_point> &, const AttributedWString &, int , float , float , const glm::vec2 &);
	std::vector<glyph_point> create_points(glm::vec2 , const AttributedWString &);

public:
	TextRenderer(const StEngineControl &context, const Font &default_font, int default_size = 32);

	void render(glm::vec2 ortho_pos, const AttributedWString &wstr);
};

}
}
