
#include <stdafx.hpp>
#include <ste_shader_spirv_reflection.hpp>
#include <spirv_tokens.hpp>

#include <ste_shader_stage_variable.hpp>
#include <ste_shader_stage_block_layout.hpp>
#include <ste_shader_exceptions.hpp>

using namespace ste::gl;

void ste_shader_spirv_reflection::parse_constant_value(parser_internal_element &dst,
												   const std::uint32_t *op) {
	std::uint32_t op_length = op[0];
	spv::Op opcode = static_cast<spv::Op>(op_length & spv::OpCodeMask);
	std::uint16_t word_count = op_length >> spv::WordCountShift;

	if (opcode == spv::Op::OpSpecConstantTrue ||
		opcode == spv::Op::OpConstantTrue) {
		if (dst.variable.type != ste_shader_stage_variable_type::bool_t) {
			throw ste_shader_load_spirv_corrupt_or_incompatible();
		}
		dst.variable.constant_value = 1;
	}
	else if (opcode == spv::Op::OpConstantNull ||
			 opcode == spv::Op::OpSpecConstantFalse ||
			 opcode == spv::Op::OpConstantFalse) {
		if (dst.variable.type != ste_shader_stage_variable_type::bool_t) {
			throw ste_shader_load_spirv_corrupt_or_incompatible();
		}
		dst.variable.constant_value = 0;
	}
	else if (opcode == spv::Op::OpSpecConstant ||
			 opcode == spv::Op::OpConstant) {
		std::uint32_t val = 0;
		for (int i = 0; i < word_count - 3; ++i)
			val += op[3 + i] << i;
		dst.variable.constant_value = val;
	}
	else {}
}

void ste_shader_spirv_reflection::parse_storage_class(parser_internal_element &dst,
												  std::uint32_t storage_id) {
	spv::StorageClass storage = static_cast<spv::StorageClass>(storage_id);
	if (storage == spv::StorageClass::Uniform ||
		storage == spv::StorageClass::UniformConstant) {
		if (dst.storage == storage_type::unknown)
			dst.storage = storage_type::uniform;
	}
	else if (storage == spv::StorageClass::PushConstant) {
		dst.storage = storage_type::push_constant;
		dst.bind_assigned = true;
	}
	else if (storage == spv::StorageClass::Output) {
		dst.policy = storage_policy::output;
	}
	else if (storage == spv::StorageClass::Input) {
		dst.policy = storage_policy::input;
	}
}

void ste_shader_spirv_reflection::parse_decoration(parser_internal_element &dst,
											   const std::uint32_t *op) {
	spv::Decoration decoration = static_cast<spv::Decoration>(op[0]);

	switch (decoration) {
	case spv::Decoration::Binding:
	case spv::Decoration::SpecId:
		dst.bind_idx = op[1];
		dst.bind_assigned = true;
		break;
	case spv::Decoration::Location:
		dst.location_idx = op[1];
		dst.location_assigned = true;
		break;
	case spv::Decoration::DescriptorSet:
		dst.set_idx = op[1];
		break;
	case spv::Decoration::Block:
		dst.block_layout = ste_shader_stage_block_layout::std140;
		break;
	case spv::Decoration::BufferBlock:
		dst.storage = storage_type::storage;
		dst.block_layout = ste_shader_stage_block_layout::std430;
		break;
	default:
		parse_decoration(dst.variable, op);
		break;
	}
}

void ste_shader_spirv_reflection::parse_decoration(_internal::ste_shader_spirv_reflected_variable &variable,
											   const std::uint32_t *op) {
	spv::Decoration decoration = static_cast<spv::Decoration>(op[0]);

	switch (decoration) {
	case spv::Decoration::Offset:
		variable.offset = static_cast<std::uint16_t>(op[1]);
		break;
	case spv::Decoration::ArrayStride:
		variable.array_stride = static_cast<std::uint16_t>(op[1]);
		break;
	case spv::Decoration::MatrixStride:
		variable.matrix_stride = static_cast<std::uint16_t>(op[1]);
		break;
	default: {}
	}
}

void ste_shader_spirv_reflection::consume_type(parser_internal_element &dst,
										   const ste_shader_spirv_reflection::parser_internal_element &consume) {
	dst.variable.consume(consume.variable);
	if (consume.storage != storage_type::unknown)
		dst.storage = consume.storage;
	if (consume.policy != storage_policy::none)
		dst.policy = consume.policy;
	if (consume.block_layout != ste_shader_stage_block_layout::none)
		dst.block_layout = consume.block_layout;
	if (consume.bind_assigned)
		dst.bind_assigned = true;
}

std::size_t ste_shader_spirv_reflection::process_spirv_op(std::vector<ste_shader_spirv_reflection::parser_internal_element> &binds,
													  const std::uint32_t *op) {
	std::uint32_t op_length = op[0];
	spv::Op opcode = static_cast<spv::Op>(op_length & spv::OpCodeMask);
	std::uint16_t word_count = op_length >> spv::WordCountShift;

	if (opcode == spv::Op::OpName) {
		std::uint32_t id = op[1];
		binds[id].variable.name = reinterpret_cast<const char *>(&op[2]);
	}
	else if (opcode == spv::Op::OpMemberName) {
		std::uint32_t id = op[1];
		std::uint32_t member_id = op[2];
		if (binds[id].variable.type == ste_shader_stage_variable_type::struct_t)
			binds[id].variable.struct_members[member_id].name = reinterpret_cast<const char *>(&op[3]);
		else
			binds[id].struct_member_ops.push_back(op);
	}
	else if (opcode == spv::Op::OpDecorate) {
		std::uint32_t id = op[1];
		parse_decoration(binds[id], op + 2);
	}
	else if (opcode == spv::Op::OpMemberDecorate) {
		std::uint32_t id = op[1];
		std::uint32_t member_id = op[2];
		if (binds[id].variable.type == ste_shader_stage_variable_type::struct_t)
			parse_decoration(binds[id].variable.struct_members[member_id], op + 3);
		else
			binds[id].struct_member_ops.push_back(op);
	}
	else if (opcode == spv::Op::OpTypePointer) {
		std::uint32_t id = op[1];
		std::uint32_t consume_id = op[3];
		consume_type(binds[id], binds[consume_id]);
		parse_storage_class(binds[id], op[2]);
	}
	else if (opcode == spv::Op::OpTypeForwardPointer) {
		// Forward pointers not supported at the moment
		assert(false);
	}
	else if (opcode == spv::Op::OpTypeVoid) {
		std::uint32_t id = op[1];
		binds[id].variable.type = ste_shader_stage_variable_type::void_t;
	}
	else if (opcode == spv::Op::OpTypeBool) {
		std::uint32_t id = op[1];
		binds[id].variable.type = ste_shader_stage_variable_type::bool_t;
	}
	else if (opcode == spv::Op::OpTypeInt) {
		std::uint32_t id = op[1];
		bool is_signed = op[3] == 1;
		binds[id].variable.width = static_cast<std::uint16_t>(op[2]);
		binds[id].variable.type = is_signed ?
			ste_shader_stage_variable_type::int_t :
			ste_shader_stage_variable_type::uint_t;
	}
	else if (opcode == spv::Op::OpTypeFloat) {
		std::uint32_t id = op[1];
		binds[id].variable.width = static_cast<std::uint16_t>(op[2]);
		binds[id].variable.type = ste_shader_stage_variable_type::float_t;
	}
	else if (opcode == spv::Op::OpTypeVector) {
		std::uint32_t id = op[1];
		std::uint32_t consume_id = op[2];
		auto elements = op[3];
		consume_type(binds[id], binds[consume_id]);
		binds[id].variable.rows = elements;
	}
	else if (opcode == spv::Op::OpTypeMatrix) {
		std::uint32_t id = op[1];
		std::uint32_t consume_id = op[2];
		auto columns = op[3];
		consume_type(binds[id], binds[consume_id]);
		binds[id].variable.columns = columns;
	}
	else if (opcode == spv::Op::OpTypeImage) {
		assert(word_count > 7);

		std::uint32_t id = op[1];
		auto sampled = word_count > 7 ? op[7] : 0;

		// 'sampled' defines if it is a storage image or regular image to use with a sampler
		switch (sampled) {
		case 1:
			// Indicates will be used with sampler
			binds[id].variable.type = ste_shader_stage_variable_type::image_t;
			break;
		case 2:
			// Indicates will be used without a sampler (a storage image)
			binds[id].variable.type = ste_shader_stage_variable_type::storage_image_t;
			break;
		case 0:
			// Unknown
		default:
			assert(false && "Unknown sampled status");
			binds[id].variable.type = ste_shader_stage_variable_type::unknown;
		}
	}
	else if (opcode == spv::Op::OpTypeSampler) {
		std::uint32_t id = op[1];
		binds[id].variable.type = ste_shader_stage_variable_type::sampler_t;
	}
	else if (opcode == spv::Op::OpTypeSampledImage) {
		std::uint32_t id = op[1];
		std::uint32_t consume_id = op[2];
		consume_type(binds[id], binds[consume_id]);
		binds[id].variable.type = ste_shader_stage_variable_type::texture_t;
	}
	else if (opcode == spv::Op::OpTypeArray) {
		std::uint32_t id = op[1];
		std::uint32_t consume_id = op[2];
		consume_type(binds[id], binds[consume_id]);

		auto element_id = op[3];
		auto elements = binds[element_id].variable.constant_value;
		if (!elements) {
			throw ste_shader_load_spirv_corrupt_or_incompatible();
		}
		if (binds[element_id].variable.array_elements != 1) {
			assert(false && "Arrays of arrays are unsupported");
		}
		binds[id].variable.array_elements = static_cast<std::uint32_t>(elements.get());

		if (binds[element_id].storage == storage_type::spec_constant) {
			binds[id].variable.array_length_specialization_constant_id = binds[element_id].bind_idx;
		}
	}
	else if (opcode == spv::Op::OpTypeRuntimeArray) {
		std::uint32_t id = op[1];
		std::uint32_t consume_id = op[2];
		consume_type(binds[id], binds[consume_id]);
		binds[id].variable.array_elements = 0;
	}
	else if (opcode == spv::Op::OpTypeStruct) {
		std::uint32_t id = op[1];
		binds[id].variable.type = ste_shader_stage_variable_type::struct_t;
		for (int i = 2; i < word_count; ++i)
			binds[id].variable.struct_members.push_back(binds[op[i]].variable);

		for (auto &struct_op : binds[id].struct_member_ops)
			process_spirv_op(binds, struct_op);
		binds[id].struct_member_ops.clear();
	}
	else if (opcode == spv::Op::OpTypeOpaque) {
		std::uint32_t id = op[1];
		binds[id].variable.type = ste_shader_stage_variable_type::opaque_t;
	}
	else if (opcode == spv::Op::OpVariable) {
		std::uint32_t id = op[2];
		std::uint32_t consume_id = op[1];
		consume_type(binds[id], binds[consume_id]);
		parse_storage_class(binds[id], op[2]);

		binds[id].is_variable = true;
	}
	else if (opcode == spv::Op::OpSpecConstantTrue ||
			 opcode == spv::Op::OpSpecConstantFalse ||
			 opcode == spv::Op::OpSpecConstant ||
			 opcode == spv::Op::OpSpecConstantComposite ||
			 opcode == spv::Op::OpConstantTrue ||
			 opcode == spv::Op::OpConstantFalse ||
			 opcode == spv::Op::OpConstant ||
			 opcode == spv::Op::OpConstantNull ||
			 opcode == spv::Op::OpConstantComposite) {
		std::uint32_t id = op[2];
		std::uint32_t consume_id = op[1];
		consume_type(binds[id], binds[consume_id]);
		parse_constant_value(binds[id], op);

		if (opcode == spv::Op::OpSpecConstantTrue ||
			opcode == spv::Op::OpSpecConstantFalse ||
			opcode == spv::Op::OpSpecConstant ||
			opcode == spv::Op::OpSpecConstantComposite)
			binds[id].storage = storage_type::spec_constant;

		binds[id].is_variable = true;
	}
	else if (opcode == spv::Op::OpSpecConstantOp) {
		// Op constants unsupported.
		assert(false);
	}

	return word_count;
}

ste_shader_stage_binding_type ste_shader_spirv_reflection::element_type_to_binding_type(const storage_type &t) {
	switch (t) {
	case storage_type::push_constant:
		return ste_shader_stage_binding_type::push_constant;
	case storage_type::storage:
		return ste_shader_stage_binding_type::storage;
	case storage_type::uniform:
		return ste_shader_stage_binding_type::uniform;
	case storage_type::spec_constant:
		return ste_shader_stage_binding_type::spec_constant;
	default:
		assert(false);
		return ste_shader_stage_binding_type::unknown;
	}
}

ste_shader_spirv_reflection_output ste_shader_spirv_reflection::parse(const std::string &code) {
	if (code.size() % 4 > 0) {
		throw ste_shader_load_spirv_corrupt_or_incompatible();
	}

	std::vector<std::uint32_t> spirv;
	spirv.resize(code.size() / 4);
	memcpy(spirv.data(), code.data(), code.size());

	// Verify magic and version
	if (spirv[0] != spv::MagicNumber ||
		(spirv[1] >> 16) != (spv::Version >> 16)) {
		throw ste_shader_load_spirv_corrupt_or_incompatible();
	}

	std::uint32_t bound_count = spirv[3];
	std::vector<parser_internal_element> binds;
	binds.resize(bound_count);

	std::vector<const std::uint32_t *> postponed_ops;

	// Read and parse bindings
	for (std::size_t offset = 5; offset < spirv.size();) {
		const std::uint32_t *op = &spirv[offset];
		auto word_count = process_spirv_op(binds,
										   op);

		offset += word_count;
	}

	// Create output
	ste_shader_spirv_reflection_output output;

	// Write out only relevant bindings and attachments
	for (auto &b : binds) {
		bool is_attachment = b.is_attachment();
		bool is_binding = b.is_binding();
		if (!is_attachment && !is_binding)
			continue;

		assert(b.storage != storage_type::unknown || b.policy == storage_policy::output);
		assert(b.variable.type != ste_shader_stage_variable_type::unknown);

		// Generate the underlying variable
		auto variable = b.variable.generate_variable(b.storage == storage_type::spec_constant,
													 output.bindings);

		if (is_attachment) {
			// Output attachment
			ste_shader_stage_attachment attachment;
			attachment.location = b.location_idx;
			attachment.block_layout = b.block_layout;
			attachment.variable = std::move(variable);

			output.attachments.push_back(std::move(attachment));
		}
		else if (is_binding) {
			// Resource binding
			auto binding_type = element_type_to_binding_type(b.storage);

			ste_shader_stage_binding binding;
			binding.set_idx = b.set_idx;
			binding.bind_idx = b.bind_idx;
			binding.binding_type = binding_type;
			binding.block_layout = b.block_layout;
			binding.variable = std::move(variable);

			output.bindings.push_back(std::move(binding));
		}
	}

	return output;
}

