
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
{}

void text_manager::create_rendering_pipeline() {
	auto texture_count = gm.textures().size();

	GL::vk_descriptor_set_layout_binding glyph_data_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	GL::vk_descriptor_set_layout_binding glyph_textures_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 1, texture_count);
	GL::vk_descriptor_set_layout_binding glyph_sampler_binding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);
	GL::vk_unique_descriptor_set descriptor_set(context.device().logical_device(), { glyph_data_binding, glyph_textures_binding, glyph_sampler_binding });
	GL::vk_pipeline_layout pipeline_layout(context.device().logical_device(), { descriptor_set.get_layout() }, {});

	// Swapchain attachment
	GL::vk_render_pass_attachment swapchain_attachment = GL::vk_render_pass_attachment::clear_and_store(context.device().get_surface().format(),
																										VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	VkClearValue swapchain_attachment_clear_value = {};
	GL::vk_blend_op_descriptor attachment0_blend_op = GL::vk_blend_op_descriptor(VK_BLEND_FACTOR_SRC_ALPHA,
																				 VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
																				 VK_BLEND_OP_ADD);

	GL::vk_render_pass_subpass_descriptor presentation_subpass0({ VkAttachmentReference{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } });
	GL::vk_render_pass_subpass_dependency presentation_subpass0_dependency(VK_SUBPASS_EXTERNAL,
																		   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
																		   0,
																		   0,
																		   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
																		   VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

	glm::u32vec2 swapchain_size = context.device().get_surface().size();
	renderpass = std::make_unique<GL::vk_render_pass>(GL::vk_render_pass{ context.device().logical_device(),
	{ swapchain_attachment },
	{ presentation_subpass0 },
	{ presentation_subpass0_dependency } });
	auto swapchain_images_count = context.device().get_surface().get_swap_chain_images().size();
	presentation_framebuffers.reserve(swapchain_images_count);
	for (std::size_t i = 0; i < swapchain_images_count; ++i)
		presentation_framebuffers.emplace_back(context.device().logical_device(),
											   *renderpass,
											   std::vector<VkImageView>{ context.device().get_surface().get_swap_chain_images()[i].view },
											   swapchain_size);

	VkViewport viewport = { 0, 0,
		static_cast<float>(swapchain_size.x), static_cast<float>(swapchain_size.y),
		1.f, 0.f };
	VkRect2D scissor = { { 0,0 },{ swapchain_size.x, swapchain_size.y } };

	static_cast<GL::vk_shader&>(vert).specialize_constant(0, texture_count);

	GL::vk_pipeline_graphics pipeline(GL::vk_pipeline_graphics{ context.device().logical_device(),{ vert->graphics_pipeline_stage_descriptor(),
		geom->graphics_pipeline_stage_descriptor(),
		frag->graphics_pipeline_stage_descriptor() },
		pipeline_layout,
		*renderpass,
		0,
		viewport,
		scissor,
		{ { 0, glyph_point::descriptor() } },
		VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		GL::vk_rasterizer_op_descriptor(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE),
		GL::vk_depth_op_descriptor(),
		{ attachment0_blend_op },
		glm::vec4{ .0f } });

	this->pipeline = std::make_unique<pipeline_t>(pipeline_t{ std::move(descriptor_set), 
											std::move(pipeline_layout), std::move(pipeline) });
}

void text_manager::adjust_line(std::vector<glyph_point> &points, const attributed_wstring &wstr, unsigned line_start_index, float line_start, float line_height, const glm::vec2 &ortho_pos) {
	if (points.size() - line_start_index) {
		optional<const Attributes::align*> alignment_attrib = wstr.attrib_of_type(Attributes::align::attrib_type_s(), { line_start_index,points.size() - line_start_index });
		optional<const Attributes::line_height*> line_height_attrib = wstr.attrib_of_type(Attributes::line_height::attrib_type_s(), { line_start_index,points.size() - line_start_index });

		if (alignment_attrib && alignment_attrib->get() != Attributes::align::alignment::Left) {
			float line_len = ortho_pos.x - line_start;
			float offset = alignment_attrib->get() == Attributes::align::alignment::Center ? -line_len*.5f : -line_len;
			for (unsigned i = line_start_index; i < points.size(); ++i)
				points[i].pos.x += offset;
		}

		if (line_height_attrib && line_height>0)
			line_height = line_height_attrib->get();
	}
	for (unsigned i = line_start_index; i < points.size(); ++i)
		points[i].pos.y -= line_height;
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

		glyph_point p;
		p.pos = decltype(p.pos){ ortho_pos.x, ortho_pos.y };
		p.glyph = static_cast<float>(g->buffer_index) + .5f;
		p.size = f * 2;
		p.color = glm::vec4(color) / 255.0f;
		p.weight = glm::clamp<float>(w - 400, -300, 500) * f * .003f;

		if (stroke_attrib) {
			p.stroke_color = glm::vec4(stroke_attrib->get_color().get()) / 255.0f;
			p.stroke_width = stroke_attrib->get_width();
			advance += glm::floor(p.stroke_width * .5f);
		}
		else
			p.stroke_width = .0f;

		points.push_back(p);

		line_height = std::max(lh, line_height);
		ortho_pos.x += advance;
	}

	adjust_line(points, wstr, line_start_index, line_start, num_lines>1 ? line_height : 0, ortho_pos);

	return points;
}

void text_manager::update_glyphs(StE::GL::vk_command_recorder &recorder) {
	auto updated_range = gm.update_pending_glyphs(recorder);

	if (updated_range.length) {
		create_rendering_pipeline();

		std::vector<GL::vk_descriptor_set_write_image> image_writes;

		for (std::size_t i = 0; i < gm.textures().size(); ++i) {
			auto &glyph_texture = gm.textures()[i];

			auto texture_write = GL::vk_descriptor_set_write_image(glyph_texture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			image_writes.push_back(texture_write);

			if (i < updated_range.length) {
				// Move image to correct layout
				auto image_barrier = GL::image_layout_transform_barrier(glyph_texture.texture,
																		VK_ACCESS_TRANSFER_WRITE_BIT,
																		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
																		VK_ACCESS_SHADER_READ_BIT,
																		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				recorder << GL::vk_cmd_pipeline_barrier(GL::vk_pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
																				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
																				image_barrier));
			}
		}

		pipeline->descriptor_set.get().write({
			GL::vk_descriptor_set_write_resource(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, 0, GL::vk_descriptor_set_write_buffer(gm.ssbo(), gm.ssbo().size())),
			GL::vk_descriptor_set_write_resource(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, 0, image_writes),
			GL::vk_descriptor_set_write_resource(VK_DESCRIPTOR_TYPE_SAMPLER, 2, 0, GL::vk_descriptor_set_write_image(gm.sampler()))
		});
	}
}

std::unique_ptr<text_renderer> text_manager::create_renderer() {
	return std::make_unique<text_renderer>(this);
}
