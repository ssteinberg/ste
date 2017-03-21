
#include <stdafx.hpp>
#include <text_renderer.hpp>
#include <text_manager.hpp>

#include <attributed_string.hpp>

using namespace StE::Text;

text_renderer::text_renderer(text_manager *tr)
	: tr(tr),
	vbo(tr->context, 100, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
{}

void text_renderer::set_text(const glm::vec2 &ortho_pos, const attributed_wstring &wstr) {
	points = tr->create_points(ortho_pos, wstr);
}
