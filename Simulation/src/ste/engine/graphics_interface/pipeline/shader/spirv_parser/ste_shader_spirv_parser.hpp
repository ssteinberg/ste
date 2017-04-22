// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_spirv_parsed_variable.hpp>
#include <ste_shader_stage_block_layout.hpp>
#include <ste_shader_spirv_parser_output.hpp>

#include <ste_shader_stage_binding_type.hpp>

#include <vector>
#include <string>

namespace StE {
namespace GL {

class ste_shader_spirv_parser {
private:
	enum class element_type : std::uint16_t {
		unknown,
		uniform,
		storage,
		push_constant,
		spec_constant,
		output,
		
	};

	struct parser_internal_element {
		std::uint32_t set_idx;
		std::uint32_t bind_idx;
		std::uint32_t location_idx;

		_internal::ste_shader_spirv_parsed_variable variable;
		element_type type{ element_type::unknown };
		ste_shader_stage_block_layout block_layout;

		bool is_variable{ false };
		bool bind_assigned{ false };
		bool location_assigned{ false };

		std::vector<const std::uint32_t*> struct_member_ops;

		bool is_binding() const {
			// Filter out those pesky non-block "push constants"
			if (type == element_type::push_constant &&
				block_layout == ste_shader_stage_block_layout::none)
				return false;

			return is_variable && 
				bind_assigned && 
				type != element_type::output;
		}
		bool is_attachment() const {
			return is_variable && 
				location_assigned &&
				type == element_type::output;
		}
	};

private:
	static void parse_constant_value(parser_internal_element &, const std::uint32_t *);
	static void parse_decoration(parser_internal_element &, const std::uint32_t *);
	static void parse_decoration(_internal::ste_shader_spirv_parsed_variable &, const std::uint32_t *);
	static void parse_storage_class(parser_internal_element &, std::uint32_t);
	static void consume_type(parser_internal_element &, const parser_internal_element &);
	static std::size_t process_spirv_op(std::vector<parser_internal_element> &, 
										const std::uint32_t *);
	static ste_shader_stage_binding_type element_type_to_binding_type(const element_type &);

public:
	static ste_shader_spirv_parser_output parse(const std::string &spirv_code);
};

}
}
