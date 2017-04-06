#pragma once

#include <stdafx.hpp>

#include <spirv_tokens.hpp>
#include <ste_shader_stage_binding.hpp>
#include <ste_shader_stage_binding_variable.hpp>
#include <ste_shader_exceptions.hpp>

#include <vector>
#include <string>
#include <optional.hpp>

namespace StE {
namespace GL {

namespace _internal {

struct ste_shader_spirv_bindings_parsed_variable {
	ste_shader_stage_variable_type type{ ste_shader_stage_variable_type::unknown };
	std::string name;

	std::uint32_t rows{ 1 };						// Matrix/vector rows. 1 for scalars.
	std::uint32_t columns{ 1 };						// Columns. >1 for matrix types.
	std::uint16_t matrix_stride{ 0 };				// Array stride between elements. 0 for tightly packed.

	std::uint32_t array_elements{ 1 };				// Array elements. >1 for arrays or 0 for a run-time array.
	bool array_length_spec_constant{ false };		// True if array length is a constant that can be specialized.
	std::uint16_t array_stride{ 0 };				// Array stride between elements. 0 for tightly packed.

	std::uint16_t offset{ 0 };						// Member offset in a struct.
	std::uint16_t width{ 0 };						// Integer/float width, only used for int_t and uint_t.
	optional<std::uint64_t> constant_value;			// Constant integer value, if available.

	std::vector<ste_shader_spirv_bindings_parsed_variable> struct_members;

	void consume(const ste_shader_spirv_bindings_parsed_variable &src) {
		// Consume only non-default attributes

		std::string name = std::move(src.name.size() ?
									 src.name :
									 this->name);

		this->name = std::move(name);
		this->rows = src.rows;
		this->columns = src.columns;
		if (src.matrix_stride != 0)
			this->matrix_stride = src.matrix_stride;

		if (src.type != ste_shader_stage_variable_type::unknown) {
			this->type = src.type;
			this->width = src.width;
		}

		if (src.array_elements != 1) {
			this->array_elements = src.array_elements;
			this->array_length_spec_constant = src.array_length_spec_constant;
		}
		if (src.array_stride > 0)
			this->array_stride = src.array_stride;

		if (src.constant_value)
			this->constant_value = std::move(src.constant_value);
		if (src.struct_members.size())
			this->struct_members = std::move(src.struct_members);
	}

	/**
	*	@brief	Checks if type is known
	*/
	bool has_type() const {
		return type != ste_shader_stage_variable_type::unknown;
	}
	/**
	*	@brief	Checks if type is opaque
	*/
	bool is_opaque() const {
		return ste_shader_stage_variable_type_is_opaque(type);
	}
	/**
	*	@brief	Checks if variable is an array
	*/
	bool is_array() const {
		return array_elements != 1;
	}
	/**
	*	@brief	Checks if variable is a run-time sized array
	*/
	bool is_runtime_array() const {
		return array_elements == 0;
	}
	/**
	*	@brief	Checks if variable is a struct
	*/
	bool is_struct() const {
		return struct_members.size() > 0;
	}
	/**
	*	@brief	Returns number of elements in the struct.
	*/
	std::size_t struct_size() const {
		if (is_struct())
			return struct_members.size();
		return 0;
	}
	/**
	*	@brief	Checks if variable is a matrix
	*/
	bool is_matrix() const {
		return columns > 1;
	}

	std::unique_ptr<ste_shader_stage_binding_variable> generate_variable() const {
		// Unknown type is an internal error
		if (!has_type()) {
			throw ste_shader_opaque_or_unknown_type();
		}

		// Arrays, matrices and vectors are composite types.
		// Create underlying types first.
		std::unique_ptr<ste_shader_stage_binding_variable> var;

		if (is_opaque()) {
			var = std::unique_ptr<ste_shader_stage_binding_variable>(std::make_unique<ste_shader_stage_binding_variable_opaque>(type,
																																name, 
																																offset));
		}
		else if (is_struct()) {
			// Handle structs recursively
			std::vector<std::unique_ptr<ste_shader_stage_binding_variable>> elements;
			for (auto &e : struct_members)
				elements.push_back(e.generate_variable());

			var = std::unique_ptr<ste_shader_stage_binding_variable>(std::make_unique<ste_shader_stage_binding_variable_struct>(std::move(elements),
																																name,
																																offset));
		}
		else {
			// Handle scalars, vectors and matrices
			auto scalar_var = std::make_unique<ste_shader_stage_binding_variable_scalar>(type,
																						 name,
																						 offset,
																						 width);

			if (rows > 1 || columns > 1) {
				// Vector/matrix
				auto matrix_var = std::make_unique<ste_shader_stage_binding_variable_matrix>(std::move(scalar_var),
																							 name,
																							 offset,
																							 rows,
																							 columns,
																							 matrix_stride);
				var = std::unique_ptr<ste_shader_stage_binding_variable>(std::move(matrix_var));
			}
			else {
				// Scalar
				var = std::unique_ptr<ste_shader_stage_binding_variable>(std::move(scalar_var));
			}
		}

		// Arrays
		if (is_array()) {
			auto array_var = std::make_unique<ste_shader_stage_binding_variable_array>(std::move(var),
																					   name,
																					   offset,
																					   array_elements,
																					   array_length_spec_constant,
																					   matrix_stride);
			var = std::unique_ptr<ste_shader_stage_binding_variable>(std::move(array_var));
		}

		return std::move(var);
	}
};

}

class ste_shader_spirv_bindings_parser {
private:
	struct ste_shader_binding_internal {
		std::uint32_t set_idx;
		std::uint32_t bind_idx;

		_internal::ste_shader_spirv_bindings_parsed_variable variable;
		ste_shader_stage_binding_type binding_type{ ste_shader_stage_binding_type::unknown };
		ste_shader_stage_block_layout block_layout;

		bool is_complete{ false };
		bool is_binding{ false };

		std::vector<const std::uint32_t*> struct_member_ops;
	};

private:
	static void parse_constant_value(ste_shader_binding_internal &, const std::uint32_t *);
	static void parse_decoration(ste_shader_binding_internal &, const std::uint32_t *);
	static void parse_decoration(_internal::ste_shader_spirv_bindings_parsed_variable &, const std::uint32_t *);
	static void parse_storage_class(ste_shader_binding_internal &, std::uint32_t);
	static void consume_type(ste_shader_binding_internal &, const ste_shader_binding_internal &);
	static std::size_t process_spirv_op(std::vector<ste_shader_binding_internal> &, 
										const std::uint32_t *);

public:
	static std::vector<ste_shader_stage_binding> parse_bindings(const std::string &spirv_code);
};

}
}
