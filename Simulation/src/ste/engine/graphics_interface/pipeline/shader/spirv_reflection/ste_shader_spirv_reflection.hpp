// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_spirv_reflected_variable.hpp>
#include <ste_shader_stage_block_layout.hpp>
#include <ste_shader_spirv_reflection_output.hpp>

#include <ste_shader_stage_binding_type.hpp>

#include <lib/vector.hpp>
#include <lib/string.hpp>

namespace ste {
namespace gl {

class ste_shader_spirv_reflection {
private:
	enum class storage_type : std::uint16_t {
		unknown,
		uniform,
		storage,
		push_constant,
		spec_constant,
	};

	enum class storage_policy : std::uint8_t {
		none,
		input,
		output
	};

	struct parser_internal_element {
		std::uint32_t set_idx;
		std::uint32_t bind_idx;
		std::uint32_t location_idx;

		_internal::ste_shader_spirv_reflected_variable variable;
		storage_type storage{ storage_type::unknown };
		storage_policy policy{ storage_policy::none };
		ste_shader_stage_block_layout block_layout;

		bool is_variable{ false };
		bool bind_assigned{ false };
		bool location_assigned{ false };

		lib::vector<const std::uint32_t*> struct_member_ops;

		bool is_binding() const {
			// Filter out those pesky non-block "push constants"
			if (storage == storage_type::push_constant &&
				block_layout == ste_shader_stage_block_layout::none)
				return false;

			return is_variable && 
				bind_assigned && 
				policy != storage_policy::output;
		}
		bool is_attachment() const {
			return is_variable && 
				location_assigned &&
				policy == storage_policy::output;
		}
	};

private:
	static void parse_constant_value(parser_internal_element &, const std::uint32_t *);
	static void parse_decoration(parser_internal_element &, const std::uint32_t *);
	static void parse_decoration(_internal::ste_shader_spirv_reflected_variable &, const std::uint32_t *);
	static void parse_storage_class(parser_internal_element &, std::uint32_t);
	static void consume_type(parser_internal_element &, const parser_internal_element &);
	static std::size_t process_spirv_op(lib::vector<parser_internal_element> &, 
										const std::uint32_t *);
	static ste_shader_stage_binding_type element_type_to_binding_type(const storage_type &);

public:
	static ste_shader_spirv_reflection_output parse(const lib::string &spirv_code);
};

}
}
