#include <stdafx.hpp>
#include <ste_gl_context.hpp>

#include <ste_glfw_handle.hpp>

using namespace ste;
using namespace ste::gl;

vk::vk_instance<> ste_gl_context::create_vk_instance(const char *app_name,
													 unsigned app_version,
													 bool debug_context,
													 lib::vector<const char*> instance_extensions,
													 lib::vector<const char*> instance_layers) {
	std::uint32_t count;
	const char **extensions = glfwGetRequiredInstanceExtensions(&count);
	for (unsigned i = 0; i < count; ++i)
		instance_extensions.push_back(extensions[i]);

	// Add debug layers and extensions
	if (debug_context) {
		auto instance_validation_layers = vk_instance_validation_layers();
		instance_layers.insert(instance_layers.begin(), instance_validation_layers.begin(), instance_validation_layers.end());

		instance_extensions.push_back("VK_EXT_debug_report");
	}

	auto instance = vk::vk_instance<>(app_name,
									  app_version,
									  instance_extensions,
									  instance_layers);
	return instance;
}

lib::vector<const char*> ste_gl_context::vk_instance_validation_layers() {
	return lib::vector<const char*>{ "VK_LAYER_LUNARG_standard_validation" };
}
