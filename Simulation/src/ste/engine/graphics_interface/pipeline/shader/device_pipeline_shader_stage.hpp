//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>

#include <vulkan/vulkan.h>
#include <vk_shader_stage_descriptor.hpp>
#include <vk_shader.hpp>
#include <vk_pipeline_graphics.hpp>

#include <ste_shader_stage_binding.hpp>
#include <ste_shader_stage.hpp>
#include <ste_shader_blob_header.hpp>
#include <ste_shader_exceptions.hpp>

#include <string>
#include <vector>
#include <istream>
#include <allow_type_decay.hpp>

namespace StE {
namespace GL {

class device_pipeline_shader_stage 
	: ste_resource_deferred_create_trait, 
	public allow_type_decay<device_pipeline_shader_stage, vk_shader> 
{
public:
	using specializations_changed_signal_t = signal<const device_pipeline_shader_stage*>;

private:
	ste_shader_stage stage{ ste_shader_stage::none };
	std::vector<ste_shader_stage_binding> stage_bindings;

	vk_shader shader;
	const std::string name;

	vk_shader::spec_map specializations;

private:
	static void verify_blob_header_sanity(const ste_shader_blob_header &header) {
		bool valid_stage =
			header.type == ste_shader_stage::vertex_program ||
			header.type == ste_shader_stage::tesselation_control_program ||
			header.type == ste_shader_stage::tesselation_evaluation_program ||
			header.type == ste_shader_stage::geometry_program ||
			header.type == ste_shader_stage::fragment_program ||
			header.type == ste_shader_stage::compute_program;

		if (header.magic != ste_shader_blob_header::header_magic_value ||
			header.properties.version_major == 0 ||
			!valid_stage) {
			throw ste_shader_load_unrecognized_exception();
		}
	}

	static std::vector<ste_shader_stage_binding> verify_spirv_and_read_bindings(const std::string &code);

	static auto load_and_verify_shader_blob(const boost::filesystem::path &modules_path,
											const std::string &name,
											ste_shader_stage &out_stage,
											std::vector<ste_shader_stage_binding> &out_stage_bindings) {
		auto path = modules_path / name;

		ste_shader_blob_header header;
		std::string code;

		// Load SPIR-v code
		{
			std::ifstream fs;
			fs.exceptions(fs.exceptions() | std::ios::failbit | std::ifstream::badbit);
			fs.open(path.string(), std::ios::in | std::ios::binary);

			fs.read(reinterpret_cast<char*>(&header), sizeof(header));
			code = std::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
		}

		// Verify header
		verify_blob_header_sanity(header);

		// Parse SPIR-v, and read bound resources
		out_stage_bindings = verify_spirv_and_read_bindings(code);

		// If header checks out, write stage and return blob
		out_stage = header.type;
		return code;
	}

public:
	/**
	*	@brief	Load a shader module named 'name' from the shader modules directory.
	*			The shader modules directory path is specified by the context.
	*
	*	@throws std::ios_base::failure	On IO errors
	*	@throws ste_shader_load_unrecognized_exception	If the shader module blob is corrupted or unrecognized by the loader
	*	@throws vk_exception			On Vulkan errors
	*
	*	@param ctx		Context
	*	@param name		Module name
	*/
	device_pipeline_shader_stage(const ste_context &ctx,
								 const std::string &name)
		: shader(ctx.device(),
				 load_and_verify_shader_blob(ctx.engine().storage().shader_module_dir_path(), 
											 name,
											 this->stage,
											 this->stage_bindings)),
		name(name) 
	{
		assert(stage != ste_shader_stage::none);
	}
	~device_pipeline_shader_stage() noexcept {}

	device_pipeline_shader_stage(device_pipeline_shader_stage &&) = default;
	device_pipeline_shader_stage &operator=(device_pipeline_shader_stage &&) = default;
	device_pipeline_shader_stage(const device_pipeline_shader_stage &) = delete;
	device_pipeline_shader_stage &operator=(const device_pipeline_shader_stage &) = delete;

	auto &get() { return shader; }
	auto &get() const { return shader; }
	auto &get_name() const { return name; }
	auto &get_stage() const { return stage; }
	auto &get_stage_bindings() const { return stage_bindings; }

	/**
	 *	@brief	Provides the shader specialization constant map.
	 *			For new specializations to take affect, pipeline has to be recreated.
	 */
	void set_specializations(const vk_shader::spec_map &specializations) {
		this->specializations = specializations;
	}

	/**
	*	@brief	Retrieve the Vulkan stage flag for the shader module
	*/
	VkShaderStageFlagBits vk_shader_stage_flag() const {
		switch (stage) {
		case ste_shader_stage::vertex_program:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ste_shader_stage::tesselation_control_program:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case ste_shader_stage::tesselation_evaluation_program:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case ste_shader_stage::geometry_program:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case ste_shader_stage::fragment_program:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ste_shader_stage::compute_program:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			assert(false);
			return VK_SHADER_STAGE_ALL;
		}
	}
	/**
	*	@brief	Retrieve the Vulkan pipeline shader stage descriptor for the shader module
	*/
	auto pipeline_stage_descriptor() const {
		vk_shader_stage_descriptor desc;
		desc.stage = vk_shader_stage_flag();
		desc.shader = &shader;
		desc.specializations = &specializations;
		return desc;
	}
};

}
}
