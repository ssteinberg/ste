
#include <stdafx.hpp>
#include <ste_context.hpp>
#include <vk_descriptor_set_write_resource.hpp>

#include <vk_command_recorder.hpp>
#include <vk_cmd_pipeline_barrier.hpp>
#include <vk_pipeline_barrier.hpp>
#include <device_image_layout_transform.hpp>

#include <font.hpp>
#include <text_manager.hpp>
#include <glyph_point.hpp>
#include <text_renderer.hpp>

#include <optional.hpp>

using namespace StE::Text;

text_manager::text_manager(const StE::ste_context &context,
						   const font &default_font,
						   int default_size)
	: context(context),
	gm(context),
	default_font(default_font),
	default_size(default_size),
	vert(context, std::string("text_distance_map_contour.vert")),
	geom(context, std::string("text_distance_map_contour.geom")),
	frag(context, std::string("text_distance_map_contour.frag"))
{
	this->descriptor_set = std::make_unique<GL::vk_unique_descriptor_set>(create_descriptor_set(context.device().logical_device(), 0));
	this->descriptor_set->get().write({
		GL::vk_descriptor_set_write_resource(VK_DESCRIPTOR_TYPE_SAMPLER, 2, 0, GL::vk_descriptor_set_write_image(gm.sampler()))
	});
}

StE::GL::vk_unique_descriptor_set text_manager::create_descriptor_set(const StE::GL::vk_logical_device &device, 
																	  std::uint32_t texture_count) {
	GL::vk_descriptor_set_layout_binding glyph_data_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	GL::vk_descriptor_set_layout_binding glyph_textures_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 1, texture_count);
	GL::vk_descriptor_set_layout_binding glyph_sampler_binding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);
	return GL::vk_unique_descriptor_set(device, { glyph_data_binding, glyph_textures_binding, glyph_sampler_binding });
}

void text_manager::adjust_line(std::vector<glyph_point> &points, const attributed_wstring &wstr, unsigned line_start_index, float line_start, float line_height, const glm::vec2 &ortho_pos) {
	if (points.size() - line_start_index) {
		optional<const Attributes::align*> alignment_attrib = wstr.attrib_of_type(Attributes::align::attrib_type_s(), { line_start_index,points.size() - line_start_index });
		optional<const Attributes::line_height*> line_height_attrib = wstr.attrib_of_type(Attributes::line_height::attrib_type_s(), { line_start_index,points.size() - line_start_index });

		if (alignment_attrib && alignment_attrib->get() != Attributes::align::alignment::Left) {
			float line_len = ortho_pos.x - line_start;
			float offset = alignment_attrib->get() == Attributes::align::alignment::Center ? -line_len*.5f : -line_len;
			for (unsigned i = line_start_index; i < points.size(); ++i) {
				points[i].data.x += static_cast<std::uint16_t>(offset);
			}
		}

		if (line_height_attrib && line_height>0)
			line_height = line_height_attrib->get();
	}
	for (unsigned i = line_start_index; i < points.size(); ++i) {
		*(reinterpret_cast<std::uint16_t*>(&points[i].data.x) + 1) -= static_cast<std::uint16_t>(line_height);
	}
}

std::vector<glyph_point> text_manager::create_points(glm::vec2 ortho_pos, const attributed_wstring &wstr) {
	float line_start = ortho_pos.x;
	int line_start_index = 0;
	float prev_line_height = 0;
	float line_height = 0;
	int num_lines = 1;

	std::vector<glyph_point> points;
	for (unsigned i = 0; i < wstr.length(); ++i) {
		if (wstr[i] == '\n') {
			adjust_line(points, wstr, line_start_index, line_start, prev_line_height, ortho_pos);

			ortho_pos.x = line_start;
			ortho_pos.y -= prev_line_height;
			prev_line_height = line_height;
			line_height = 0;
			line_start_index = points.size();

			++num_lines;

			continue;
		}

		optional<const Attributes::font*> font_attrib = wstr.attrib_of_type(Attributes::font::attrib_type_s(), { i,1 });
		optional<const Attributes::rgb*> color_attrib = wstr.attrib_of_type(Attributes::rgb::attrib_type_s(), { i,1 });
		optional<const Attributes::size*> size_attrib = wstr.attrib_of_type(Attributes::size::attrib_type_s(), { i,1 });
		optional<const Attributes::stroke*> stroke_attrib = wstr.attrib_of_type(Attributes::stroke::attrib_type_s(), { i,1 });
		optional<const Attributes::weight*> weight_attrib = wstr.attrib_of_type(Attributes::weight::attrib_type_s(), { i,1 });

		const font &font = font_attrib ? font_attrib->get() : default_font;
		int size = size_attrib ? size_attrib->get() : default_size;
		glm::u8vec4 color = color_attrib ? color_attrib->get() : glm::u8vec4{255, 255, 255, 255};

		auto g = gm.glyph_for_font(font, wstr[i]);

		float f = static_cast<float>(size) / static_cast<float>(glyph::ttf_pixel_size);
		float w = weight_attrib ? weight_attrib->get() : 400.f;
		float lh = (g->metrics.height + g->metrics.start_y) * f * 2 + 1;

		float advance = static_cast<float>(i + 1 < wstr.length() ? gm.spacing(font, { wstr[i], wstr[i + 1] }, size) : 0);

		glm::u32vec2 pos = { static_cast<std::uint32_t>(ortho_pos.x + .5f), static_cast<std::uint32_t>(ortho_pos.y + .5f) };
		std::uint32_t osize = static_cast<std::uint32_t>(f * 2.f * glyph_point::size_scale + .5f);
		std::uint32_t glyph_index = static_cast<std::uint32_t>(g->buffer_index);
		float weight = glm::clamp<float>(w - 400, -300, 500) * f * .003f;

		glyph_point p;
		p.data.x = (pos.x & 0xFFFF) | (pos.y << 16);
		p.data.y = (osize & 0xFFFF) | (glyph_index << 16);
		p.data.z = glm::packUnorm4x8(glm::vec4(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f, 
											   .5f + weight * glyph_point::weight_scale));
		p.data.w = 0;

		if (stroke_attrib) {
			float stroke_width = stroke_attrib->get_width();
			advance += glm::floor(stroke_width * .5f);

			auto c = stroke_attrib->get_color().get();
			p.data.w = glm::packUnorm4x8(glm::vec4(c.x / 255.0f,c.y / 255.0f,c.z / 255.0f, 
												   stroke_width * glyph_point::stroke_width_scale));
		}

		points.push_back(p);

		line_height = std::max(lh, line_height);
		ortho_pos.x += advance;
	}

	adjust_line(points, wstr, line_start_index, line_start, num_lines>1 ? line_height : 0, ortho_pos);

	return points;
}

bool text_manager::update_glyphs(StE::GL::vk_command_recorder &recorder) {
	auto updated_range = gm.update_pending_glyphs(recorder);
	if (!updated_range.length)
		return false;

	std::uint32_t texture_count = static_cast<std::uint32_t>(gm.textures().size());

	// Update fragment specialization constant
	static_cast<GL::vk_shader&>(frag).specialize_constant(0, texture_count);

	// Create new descriptor set and layout
	auto new_descriptor_set = std::make_unique<GL::vk_unique_descriptor_set>(create_descriptor_set(context.device().logical_device(), texture_count));
	std::vector<GL::vk_descriptor_set_write_image> image_writes;
	image_writes.reserve(texture_count);
	for (std::uint32_t i = updated_range.start; i < updated_range.start + updated_range.length; ++i) {
		auto &glyph_texture = gm.textures()[i];

		// Move image to correct layout
		auto image_barrier = GL::image_layout_transform_barrier(glyph_texture.texture,
																VK_ACCESS_TRANSFER_WRITE_BIT,
																glyph_texture.texture.layout(),
																VK_ACCESS_SHADER_READ_BIT,
																VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		recorder << GL::vk_cmd_pipeline_barrier(GL::vk_pipeline_barrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
																		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
																		image_barrier));

		// Write new descriptor
		auto texture_write = GL::vk_descriptor_set_write_image(glyph_texture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		image_writes.push_back(texture_write);
	}

	std::vector<GL::vk_descriptor_set_copy_resources> copies = { GL::vk_descriptor_set_copy_resources(*this->descriptor_set, 
																									  2, 0, 2, 0, 1) };
	if (updated_range.start > 0) {
		// Need to copy old descriptors
		copies.push_back(GL::vk_descriptor_set_copy_resources(*this->descriptor_set, 1, 0, 1, 0, updated_range.start));
	}

	// Copy and write out descriptors
	new_descriptor_set->get().update(
	{ // Writes
		GL::vk_descriptor_set_write_resource(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, 0, GL::vk_descriptor_set_write_buffer(gm.ssbo(), texture_count)),
		GL::vk_descriptor_set_write_resource(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, updated_range.start, image_writes)
	}, 
	copies);

	this->descriptor_set = std::move(new_descriptor_set);

	return true;
}

std::unique_ptr<text_renderer> text_manager::create_renderer(const StE::GL::vk_render_pass *renderpass) {
	return std::make_unique<text_renderer>(this,
										   renderpass);
}
