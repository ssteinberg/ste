//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>

#include <ste_shader_object.hpp>
#include <vk_pipeline_graphics.hpp>

#include <ste_shader_stage_binding.hpp>
#include <ste_shader_program_stage.hpp>
#include <ste_shader_blob_header.hpp>
#include <ste_shader_exceptions.hpp>

#include <ste_shader_spirv_reflection_output.hpp>

#include <lib/string.hpp>
#include <lib/vector.hpp>
#include <istream>
#include <lib/unique_ptr.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class device_pipeline_shader_stage 
	: ste_resource_deferred_create_trait, 
	public allow_type_decay<device_pipeline_shader_stage, vk::vk_shader<>>
{
public:
	using specializations_changed_signal_t = signal<const device_pipeline_shader_stage*>;

private:
	lib::string name;
	lib::unique_ptr<const ste_shader_object> shader;

	vk::vk_shader<>::spec_map specializations;

private:
	static void verify_blob_header_sanity(const ste_shader_blob_header &header) {
		bool valid_stage =
			header.type == ste_shader_program_stage::vertex_program ||
			header.type == ste_shader_program_stage::tessellation_control_program ||
			header.type == ste_shader_program_stage::tessellation_evaluation_program ||
			header.type == ste_shader_program_stage::geometry_program ||
			header.type == ste_shader_program_stage::fragment_program ||
			header.type == ste_shader_program_stage::compute_program;

		if (header.magic != ste_shader_blob_header::header_magic_value ||
			header.properties.version_major == 0 ||
			!valid_stage) {
			throw ste_shader_load_unrecognized_exception();
		}
	}

	static ste_shader_spirv_reflection_output verify_spirv_and_read_bindings(const lib::string &code);

	static auto load_and_verify_shader_blob(const ste_context &ctx,
											const lib::string &name) {
		const auto &modules_path = ctx.engine().storage().shader_module_dir_path();
		auto path = modules_path / name;

		ste_shader_blob_header header;
		lib::string code;

		// Load SPIR-v code
		{
			std::ifstream fs;
			fs.exceptions(fs.exceptions() | std::ios::failbit | std::ifstream::badbit);
			fs.open(path.string(), std::ios::in | std::ios::binary);

			fs.read(reinterpret_cast<char*>(&header), sizeof(header));
			code = lib::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
		}

		// Verify header
		verify_blob_header_sanity(header);

		// Parse SPIR-v, and read resources' bindings and output attachments.
		auto parse_output = verify_spirv_and_read_bindings(code);
		auto &stage_bindings = parse_output.bindings;
		auto &parser_attachments = parse_output.attachments;

		// If header checks out, create the shader object
		ste_shader_program_stage stage = header.type;

		// Output attachments are relevant only for a fragment shader
		lib::vector<ste_shader_stage_attachment> stage_attachments = {};
		if (stage == ste_shader_program_stage::fragment_program) {
			stage_attachments = std::move(parser_attachments);
		}

		return lib::allocate_unique<const ste_shader_object>(ctx.device(),
															 code,
															 stage,
															 std::move(stage_bindings),
															 std::move(stage_attachments));
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
								 const lib::string &name)
		: name(name),
		shader(load_and_verify_shader_blob(ctx,
										   name))
	{}
	~device_pipeline_shader_stage() noexcept {}

	device_pipeline_shader_stage(device_pipeline_shader_stage &&) = default;
	device_pipeline_shader_stage &operator=(device_pipeline_shader_stage &&) = default;
	device_pipeline_shader_stage(const device_pipeline_shader_stage &) = delete;
	device_pipeline_shader_stage &operator=(const device_pipeline_shader_stage &) = delete;

	auto &get() { return shader->shader; }
	auto &get() const { return shader->shader; }
	auto &get_name() const { return name; }
	auto &get_stage() const { return shader->stage; }
	auto &get_stage_bindings() const { return shader->stage_bindings; }
	auto &get_stage_attachments() const { return shader->stage_attachments; }

	/**
	 *	@brief	Provides the shader specialization constant map.
	 *			For new specializations to take affect, pipeline has to be recreated.
	 */
	void set_specializations(const vk::vk_shader<>::spec_map &specializations) {
		this->specializations = specializations;
	}

	/**
	*	@brief	Retrieve the Vulkan stage flag for the shader module
	*/
	stage_flag vk_shader_stage_flag() const {
		return shader->shader_stage_flag();
	}

	/**
	*	@brief	Retrieve the Vulkan pipeline shader stage descriptor for the shader module
	*/
	auto pipeline_stage_descriptor() const {
		vk::vk_shader_stage_descriptor<> desc;
		desc.stage = static_cast<VkShaderStageFlagBits>(vk_shader_stage_flag());
		desc.shader = &get();
		desc.specializations = &specializations;
		return desc;
	}
};

}
}
