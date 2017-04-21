// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_variable_type.hpp>
#include <ste_shader_variable_layout_verification_exceptions.hpp>

#include <block_layout.hpp>
#include <sampler.hpp>
#include <image_view.hpp>
#include <texture.hpp>

#include <optional.hpp>
#include <string>
#include <memory>
#include <vector>
#include <error_type.hpp>

namespace StE {
namespace GL {

namespace _internal {

template <typename Src, typename Target0, typename... Targets>
struct ste_shader_stage_binding_variable_dispatcher {
	template <typename T, typename... Ts>
	static void validate(const Src *src, Ts&&... ts) {
		if (auto *ptr = dynamic_cast<const Target0*>(src)) {
			ptr->template validate<T>(std::forward<Ts>(ts)...);
			return;
		}
		ste_shader_stage_binding_variable_dispatcher<Src, Targets...>::template validate<T>(src,
																							std::forward<Ts>(ts)...);
	}
};
template <typename Src, typename Target0>
struct ste_shader_stage_binding_variable_dispatcher<Src, Target0> {
	template <typename T, typename... Ts>
	static void validate(const Src *src, Ts&&... ts) {
		if (auto *ptr = dynamic_cast<const Target0*>(src)) {
			ptr->template validate<T>(std::forward<Ts>(ts)...);
			return;
		}

		assert(false && "No target found");
	}
};

template <typename T, bool is_block>
struct ste_shader_stage_binding_variable_remove_blocks {};
template <typename T>
struct ste_shader_stage_binding_variable_remove_blocks<T, false> {
	using type = T;
};
template <typename B>
struct ste_shader_stage_binding_variable_remove_blocks<B, true> {
	static constexpr bool single_element_block = B::count == 1;
	using T = typename B::template type_at<0>;
	using type = std::conditional_t<
		single_element_block,
		typename ste_shader_stage_binding_variable_remove_blocks<T, is_block_layout_v<T>>::type,
		error_type
	>;
};
template <typename T>
using ste_shader_stage_binding_variable_remove_blocks_t = 
	typename ste_shader_stage_binding_variable_remove_blocks<T, is_block_layout_v<T>>::type;

}

class ste_shader_stage_binding_variable_opaque;
class ste_shader_stage_binding_variable_scalar;
class ste_shader_stage_binding_variable_matrix;
class ste_shader_stage_binding_variable_array;
class ste_shader_stage_binding_variable_struct;

/**
 *	@brief	Shader binding variable
 */
class ste_shader_stage_binding_variable {
private:
	std::string var_name;
	std::uint16_t offset_bytes{ 0 };
	std::string default_specialized_value;
	optional<std::string> specialized_value;

private:
	using dispatcher = _internal::ste_shader_stage_binding_variable_dispatcher<
		ste_shader_stage_binding_variable,
		ste_shader_stage_binding_variable_opaque,
		ste_shader_stage_binding_variable_scalar,
		ste_shader_stage_binding_variable_matrix,
		ste_shader_stage_binding_variable_array,
		ste_shader_stage_binding_variable_struct
	>;

protected:
	ste_shader_stage_binding_variable(std::string name,
									  std::uint16_t offset_bytes = 0)
		: var_name(name), offset_bytes(offset_bytes)
	{}

public:
	ste_shader_stage_binding_variable(ste_shader_stage_binding_variable&&) = default;
	ste_shader_stage_binding_variable &operator=(ste_shader_stage_binding_variable&&) = default;
	ste_shader_stage_binding_variable(const ste_shader_stage_binding_variable&) = default;
	ste_shader_stage_binding_variable &operator=(const ste_shader_stage_binding_variable&) = default;

	virtual ~ste_shader_stage_binding_variable() noexcept {}

	/**
	*	@brief	Variable type
	*/
	virtual ste_shader_stage_variable_type type() const { return ste_shader_stage_variable_type::unknown; }

	/**
	 *	@brief	Variable name
	 */
	auto& name() const { return var_name; }
	/**
	*	@brief	Variable offset in parent in bytes
	*/
	auto& offset() const { return offset_bytes; }

	/*
	*	@brief	Array elements. >1 for arrays or 0 for a run-time array.
	*/
	virtual std::uint32_t size() const { return 1; }

	/**
	*	@brief	Returns variable size in bytes
	*/
	virtual std::uint32_t size_bytes() const = 0;

	/**
	*	@brief	Checks if the binding variable is compatible with type T.
	*
	*	@throws	ste_shader_variable_layout_verification_exception	On different validation failures
	*/
	template <typename T>
	void validate(const T &t) const {
		validate<T>();
	}
	/**
	*	@brief	Checks if the binding variable is compatible with type T.
	*
	*	@throws	ste_shader_variable_layout_verification_exception	On different validation failures
	*/
	template <typename T>
	void validate() const {
		using Type = std::remove_cv_t<std::remove_reference_t<T>>;
		dispatcher::validate<Type>(this);
	}

	void set_default_specialized_value(std::string default_specialized_value) {
		this->default_specialized_value = default_specialized_value;
	}
	/**
	*	@brief	For specialization constants, resets the specializable constant
	*/
	void reset_specialization() {
		specialized_value = none;
	}
	/**
	*	@brief	For specialization constants, check if constant was specialized
	*/
	bool has_non_default_specialization() {
		return !!specialized_value;
	}
	/**
	*	@brief	For specialization constants, specializes the constant
	*/
	template <typename T>
	void specialize(const T &t) {
		static_assert(std::is_pod_v<T>, "T must be a POD");

		std::string data;
		data.resize(sizeof(T));
		memcpy(data.data(), &t, sizeof(T));

		specialized_value = data;
	}
	/**
	*	@brief	For specialization constants, specializes the constant
	*/
	void specialize_bin(const std::string &data) {
		specialized_value = data;
	}
	/**
	*	@brief	For specialization constants, returns the specialized value
	*/
	template <typename T>
	T read_specialized_value() const {
		static_assert(std::is_pod_v<T>, "T must be a POD");

		const std::string& data = specialized_value ?
			specialized_value.get() :
			default_specialized_value;

		if (sizeof(T) > data.size()) {
			assert(false);
			return T();
		}
		return *reinterpret_cast<const T*>(data.data());
	}

	/**
	*	@brief	Checks if the variables are compatible. (i.e. std::is_same_v<>)
	*/
	virtual bool compatible(const ste_shader_stage_binding_variable &var) const {
		return type() == var.type() &&
			size_bytes() == var.size_bytes();
	}

	/**
	 *	@brief	Checks if the variables are identical.
	 *			Identical variables are compatible, have same name and offset.
	 */
	virtual bool operator==(const ste_shader_stage_binding_variable &var) const {
		return this->compatible(var) &&
			offset() == var.offset() &&
			name() == var.name();
	}
	bool operator!=(const ste_shader_stage_binding_variable &var) const {
		return !(*this == var);
	}
};

/**
*	@brief	Shader binding variable
*			Opaque type
*/
class ste_shader_stage_binding_variable_opaque : public ste_shader_stage_binding_variable {
	using Base = ste_shader_stage_binding_variable;

private:
	ste_shader_stage_variable_type var_type;

public:
	ste_shader_stage_binding_variable_opaque(const ste_shader_stage_variable_type &type,
											 std::string name,
											 std::uint16_t offset_bytes)
		: Base(name, offset_bytes),
		var_type(type)
	{}

	ste_shader_stage_binding_variable_opaque(ste_shader_stage_binding_variable_opaque&&) = default;
	ste_shader_stage_binding_variable_opaque &operator=(ste_shader_stage_binding_variable_opaque&&) = default;
	ste_shader_stage_binding_variable_opaque(const ste_shader_stage_binding_variable_opaque&) = default;
	ste_shader_stage_binding_variable_opaque &operator=(const ste_shader_stage_binding_variable_opaque&) = default;

	virtual ~ste_shader_stage_binding_variable_opaque() noexcept {}

	ste_shader_stage_variable_type type() const override final { return var_type; }
	std::uint32_t size_bytes() const override final { return 0; }

	/**
	*	@brief	Checks if the binding variable is compatible with type T.
	*/
	template <typename T>
	void validate() const {
		using Type = _internal::ste_shader_stage_binding_variable_remove_blocks_t<T>;

		if (var_type == ste_shader_stage_variable_type::sampler_t &&
			!std::is_convertible_v<Type, sampler>)
			throw ste_shader_variable_layout_verification_opaque_or_unknown_type("Expected a sampler type");

		if ((var_type == ste_shader_stage_variable_type::image_t ||
			 var_type == ste_shader_stage_variable_type::storage_image_t) &&
			!std::is_convertible_v<Type, image_view_generic>)
			throw ste_shader_variable_layout_verification_opaque_or_unknown_type("Expected an image_view_generic type");

		if (var_type == ste_shader_stage_variable_type::texture_t &&
			!std::is_convertible_v<Type, texture_generic>)
			throw ste_shader_variable_layout_verification_opaque_or_unknown_type("Expected a texture_generic type");
	}
};

/**
*	@brief	Shader binding variable
*			Scalar types, signed/unsigned integers and floating-points.
*/
class ste_shader_stage_binding_variable_scalar : public ste_shader_stage_binding_variable {
	using Base = ste_shader_stage_binding_variable;

private:
	ste_shader_stage_variable_type var_type;
	std::uint16_t var_width;

public:
	ste_shader_stage_binding_variable_scalar(const ste_shader_stage_variable_type &type,
											 std::string name,
											 std::uint16_t offset_bytes,
											 std::uint16_t width)
		: Base(name, offset_bytes),
		var_type(type),
		var_width(width)
	{}

	ste_shader_stage_binding_variable_scalar(ste_shader_stage_binding_variable_scalar&&) = default;
	ste_shader_stage_binding_variable_scalar &operator=(ste_shader_stage_binding_variable_scalar&&) = default;
	ste_shader_stage_binding_variable_scalar(const ste_shader_stage_binding_variable_scalar&) = default;
	ste_shader_stage_binding_variable_scalar &operator=(const ste_shader_stage_binding_variable_scalar&) = default;

	virtual ~ste_shader_stage_binding_variable_scalar() noexcept {}

	ste_shader_stage_variable_type type() const override final { return var_type; }
	std::uint32_t size_bytes() const override final { return var_width >> 3; }

	/**
	*	@brief	Scalar width, in bits.
	*/
	auto& width() const { return var_width; }

	/**
	*	@brief	Is a floating-point scalar trait
	*/
	bool is_floating_point() const { return var_type == ste_shader_stage_variable_type::float_t; }
	/**
	*	@brief	Is a signed scalar trait
	*/
	bool is_signed() const { return var_type != ste_shader_stage_variable_type::uint_t; }

	/**
	*	@brief	Checks if the binding variable is compatible with type T.
	*/
	template <typename T>
	void validate() const {
		using Type = _internal::ste_shader_stage_binding_variable_remove_blocks_t<T>;
		static constexpr bool type_is_signed = ::StE::GL::is_signed<Type>::value;
		static constexpr bool type_is_float =  ::StE::GL::is_floating_point<Type>::value;
		static constexpr bool type_is_scalar = ::StE::GL::is_scalar<Type>::value;
		
		if (sizeof(Type) != this->size_bytes())
			throw ste_shader_variable_layout_verification_type_mismatch("Size mismatch");
		if (!type_is_scalar)
			throw ste_shader_variable_layout_verification_type_mismatch("Expected a scalar type");
		if (type_is_float != this->is_floating_point())
			throw ste_shader_variable_layout_verification_type_mismatch("Scalar type mismatch");
		if (type_is_signed != this->is_signed())
			throw ste_shader_variable_layout_verification_type_mismatch("Scalar signess mismatch");
	}
};

/**
*	@brief	Shader binding variable
*			Matrix type
*/
class ste_shader_stage_binding_variable_matrix : public ste_shader_stage_binding_variable {
	using Base = ste_shader_stage_binding_variable;

private:
	std::uint16_t var_matrix_stride;
	std::uint32_t var_rows;
	std::uint32_t var_columns;
	std::unique_ptr<ste_shader_stage_binding_variable_scalar> scalar_var;

public:
	ste_shader_stage_binding_variable_matrix(std::unique_ptr<ste_shader_stage_binding_variable_scalar> &&scalar,
											 std::string name,
											 std::uint16_t offset_bytes,
											 std::uint32_t rows,
											 std::uint32_t columns = 1,
											 std::uint16_t matrix_stride = 0)
		: Base(name, offset_bytes),
		var_matrix_stride(matrix_stride),
		var_rows(rows),
		var_columns(columns),
		scalar_var(std::move(scalar))
	{
		assert(columns >= 1 && columns <= 4);
		assert(rows >= 1 && rows <= 4);
	}

	ste_shader_stage_binding_variable_matrix(ste_shader_stage_binding_variable_matrix&&) = default;
	ste_shader_stage_binding_variable_matrix &operator=(ste_shader_stage_binding_variable_matrix&&) = default;
	ste_shader_stage_binding_variable_matrix(const ste_shader_stage_binding_variable_matrix&) = default;
	ste_shader_stage_binding_variable_matrix &operator=(const ste_shader_stage_binding_variable_matrix&) = default;

	virtual ~ste_shader_stage_binding_variable_matrix() noexcept {}

	ste_shader_stage_variable_type type() const override final { return scalar_var->type(); }
	std::uint32_t size_bytes() const override final {
		auto var_size = scalar_var->size_bytes();
		auto stride = var_matrix_stride > 0 ? var_matrix_stride : var_rows * var_size;
		return stride * var_columns;
	}

	/**
	 *	@brief	Matrix/vector rows. 1 for scalars.
	 */
	auto& rows() const { return var_rows; }
	/*
	 *	@brief	Columns. >1 for matrices.
	 */
	auto& columns() const { return var_columns; }
	/*
	*	@brief	Stride between columns in bytes. 0 for tightly packed.
	*/
	auto& matrix_stride() const { return var_matrix_stride; }

	/*
	*	@brief	Get the underlying variable
	*/
	const auto* underlying_variable() const { return scalar_var.get(); }

	/**
	*	@brief	Checks if the binding variable is compatible with type T.
	*/
	template <typename T>
	void validate() const {
		using Type = _internal::ste_shader_stage_binding_variable_remove_blocks_t<T>;

		scalar_var->validate<typename remove_extents<Type>::type>();

		if (!is_matrix_v<Type> && !is_vector_v<Type>)
			throw ste_shader_variable_layout_verification_type_mismatch("Expected a vector or matrix type");
		if (matrix_columns_count_v<Type> != columns() ||
			matrix_rows_count_v<Type> != rows())
			throw ste_shader_variable_layout_verification_type_mismatch("Type rows or columns count mismatch");
	}

	/**
	*	@brief	Checks if the variables reference the same binding
	*/
	bool compatible(const ste_shader_stage_binding_variable &var) const override {
		auto *ptr = dynamic_cast<decltype(this)>(&var);
		if (!ptr)
			return false;
		return Base::compatible(var) &&
			columns() == ptr->columns() &&
			rows() == ptr->rows() &&
			matrix_stride() == ptr->matrix_stride();
	}
};

/**
*	@brief	Shader binding variable
*			Array of variables
*/
class ste_shader_stage_binding_variable_array : public ste_shader_stage_binding_variable {
	using Base = ste_shader_stage_binding_variable;

private:
	std::uint32_t array_elements{ 1 };
	std::uint16_t array_stride{ 0 };
	std::unique_ptr<ste_shader_stage_binding_variable> var;

	optional<const ste_shader_stage_binding_variable_scalar*> length_specialization_constant;

public:
	ste_shader_stage_binding_variable_array(std::unique_ptr<ste_shader_stage_binding_variable> &&var,
											std::string name,
											std::uint16_t offset_bytes,
											std::uint32_t array_elements,
											std::uint16_t array_stride,
											optional<const ste_shader_stage_binding_variable_scalar*> length_specialization_constant = none)
		: Base(name, offset_bytes),
		array_elements(array_elements),
		array_stride(array_stride),
		var(std::move(var)),
		length_specialization_constant(length_specialization_constant)
	{}

	ste_shader_stage_binding_variable_array(ste_shader_stage_binding_variable_array&&) = default;
	ste_shader_stage_binding_variable_array &operator=(ste_shader_stage_binding_variable_array&&) = default;
	ste_shader_stage_binding_variable_array(const ste_shader_stage_binding_variable_array&) = default;
	ste_shader_stage_binding_variable_array &operator=(const ste_shader_stage_binding_variable_array&) = default;

	virtual ~ste_shader_stage_binding_variable_array() noexcept {}

	ste_shader_stage_variable_type type() const override final { return var->type(); }
	std::uint32_t size_bytes() const override final {
		if (array_stride > 0)
			return array_elements * array_stride;
		return array_elements * var->size_bytes();
	}

	/*
	 *	@brief	Array elements. >1 for arrays or 0 for a run-time array.
	 */
	std::uint32_t size() const override final {
		if (!length_spec_constant())
			return array_elements;

		return length_specialization_constant.get()->read_specialized_value<std::uint32_t>();
	}
	/*
	*	@brief	Returns true if array length is a constant that can be specialized.
	*/
	bool length_spec_constant() const { return !!length_specialization_constant; }
	/*
	*	@brief	Returns the length specialization constant variable
	*/
	auto get_length_spec_constant_var() const { return length_specialization_constant.get(); }
	/*
	 *	@brief	Array stride between elements. 0 for tightly packed.
	 */
	auto stride() const { return array_stride > 0 ? array_stride : var->size_bytes(); }

	/*
	*	@brief	Get the underlying variable
	*/
	const auto* underlying_variable() const { return var.get(); }

	/**
	*	@brief	Checks if the binding variable is compatible with type T.
	*/
	template <typename T>
	void validate() const {
		using T2 = _internal::ste_shader_stage_binding_variable_remove_blocks_t<T>;

		if (std::is_array_v<T>) {
			// T is an array
			using ElementT = std::remove_extent_t<T>;

			if (std::extent_v<T> != array_elements)
				throw ste_shader_variable_layout_verification_array_length_mismatch("Array length mismatch");
			if (sizeof(ElementT) != stride())
				throw ste_shader_variable_layout_verification_array_stride_mismatch("Array stride mismatch");

			using Type = _internal::ste_shader_stage_binding_variable_remove_blocks_t<ElementT>;
			var->validate<std::remove_extent_t<Type>>();
		}
		else if (std::is_array_v<T2>) {
			// T is a single item block layout containing an array
			using ElementT = std::remove_extent_t<T2>;

			if (std::extent_v<T2> != array_elements)
				throw ste_shader_variable_layout_verification_array_length_mismatch("Array length mismatch");
			if (sizeof(ElementT) != stride())
				throw ste_shader_variable_layout_verification_array_stride_mismatch("Array stride mismatch");

			using Type = _internal::ste_shader_stage_binding_variable_remove_blocks_t<ElementT>;
			var->validate<std::remove_extent_t<Type>>();
		}
		else {
			// Can't find an array
			throw ste_shader_variable_layout_verification_type_mismatch("Expected an array");
		}

		if (sizeof(T) != size_bytes())
			throw ste_shader_variable_layout_verification_type_mismatch("Array size doesn't match expected size");
	}

	/**
	*	@brief	Checks if the variables reference the same binding
	*/
	bool compatible(const ste_shader_stage_binding_variable &var) const override {
		auto *ptr = dynamic_cast<decltype(this)>(&var);
		if (!ptr)
			return false;
		return Base::compatible(var) &&
			*underlying_variable() == *ptr->underlying_variable() &&
			size() == ptr->size() &&
			length_spec_constant() == ptr->length_spec_constant() &&
			stride() == ptr->stride();
	}
};

namespace _internal {

template <typename T>
void ste_shader_stage_binding_variable_type_validator(std::uint32_t offset,
													  const ste_shader_stage_binding_variable *var) {
	if (var->offset() != offset)
		throw ste_shader_variable_layout_verification_offset_mismatch("Struct member offset mismatch");
	var->validate<T>();
}

template <typename Var, int N, int Len, typename B>
struct ste_shader_stage_binding_variable_struct_validator {
	void operator()(const Var *var) {
		using T = typename B::template type_at<N>;
		auto offset = block_offset_of<N, B>();

		try {
			ste_shader_stage_binding_variable_type_validator<T>(offset, (*var)[N].get());
		}
		catch (GL::ste_shader_variable_layout_verification_exception &e) {
			// Prepend a error location message and rethrow
			e.prepend("At struct member [" + std::to_string(N) + "]:");
			throw;
		}
		ste_shader_stage_binding_variable_struct_validator<Var, N + 1, Len, B>()(var);
	}
};
template <typename Var, int Len, typename B>
struct ste_shader_stage_binding_variable_struct_validator<Var, Len, Len, B> {
	void operator()(const Var *) {}
};

struct ste_shader_stage_binding_variable_struct_validate {
	template <typename Var, typename T>
	static void validate(const Var *var,
						 std::enable_if_t<!is_block_layout_v<T>>* = nullptr) {
		if (var->count() == 1) {
			(*var)[0]->template validate<T>();
			return;
		}
		throw ste_shader_variable_layout_verification_type_mismatch("Expected a block_layout");
	}
	template <typename Var, typename B>
	static void validate(const Var *var,
						 std::enable_if_t<is_block_layout_v<B>>* = nullptr) {
		if (var->count() == 1) {
			(*var)[0]->template validate<B>();
			return;
		}
		ste_shader_stage_binding_variable_struct_validator<Var, 0, B::count, B>()(var);
	}
};

}

/**
*	@brief	Shader binding variable
*			Composite struct of variables
*/
class ste_shader_stage_binding_variable_struct : public ste_shader_stage_binding_variable {
	using Base = ste_shader_stage_binding_variable;

private:
	std::vector<std::unique_ptr<ste_shader_stage_binding_variable>> elements;

public:
	ste_shader_stage_binding_variable_struct(std::vector<std::unique_ptr<ste_shader_stage_binding_variable>> &&elements,
											 std::string name,
											 std::uint16_t offset_bytes)
		: Base(name, offset_bytes),
		elements(std::move(elements))
	{}

	ste_shader_stage_binding_variable_struct(ste_shader_stage_binding_variable_struct&&) = default;
	ste_shader_stage_binding_variable_struct &operator=(ste_shader_stage_binding_variable_struct&&) = default;
	ste_shader_stage_binding_variable_struct(const ste_shader_stage_binding_variable_struct&) = default;
	ste_shader_stage_binding_variable_struct &operator=(const ste_shader_stage_binding_variable_struct&) = default;

	virtual ~ste_shader_stage_binding_variable_struct() noexcept {}

	ste_shader_stage_variable_type type() const override final { return ste_shader_stage_variable_type::struct_t; }
	std::uint32_t size_bytes() const override final {
		if (elements.size())
			return elements.back()->offset() + elements.back()->size_bytes();
		return static_cast<std::uint32_t>(0);
	}

	/*
	*	@brief	Struct elements
	*/
	auto count() const { return elements.size(); }
	
	auto& operator[](std::size_t idx) const { return elements[idx]; }
	auto begin() const { return elements.begin(); }
	auto end() const { return elements.end(); }

	/**
	*	@brief	Checks if the binding variable is compatible with type T.
	*/
	template <typename T>
	void validate() const {
		using Validator = _internal::ste_shader_stage_binding_variable_struct_validate;
		Validator::validate<ste_shader_stage_binding_variable_struct, T>(this);
	}

	/**
	*	@brief	Checks if the variables reference the same binding
	*/
	bool compatible(const ste_shader_stage_binding_variable &var) const override {
		auto *ptr = dynamic_cast<decltype(this)>(&var);
		if (!ptr)
			return false;
		if (!(Base::compatible(var) &&
			  count() == ptr->count()))
			return false;

		for (std::size_t i = 0; i < count(); ++i)
			if (*(*this)[i] != *(*ptr)[i])
				return false;

		return true;
	}
};

}
}
