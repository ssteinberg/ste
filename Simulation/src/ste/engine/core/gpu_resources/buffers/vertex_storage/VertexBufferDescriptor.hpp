// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gl_type_traits.hpp"
#include "typelist.hpp"
#include "generate_array.hpp"

namespace StE {
namespace Core {

class VertexBufferDescriptor {
public:
	std::size_t offset_to_attrib(int attrib) const noexcept{
		int offset, i;
		for (offset = i = 0; i < attrib; offset += attrib_size(i), ++i);
		return offset;
	}

public:
	virtual std::size_t attrib_count() const noexcept = 0;
	virtual std::size_t attrib_size(int attrib) const noexcept = 0;
	virtual std::size_t attrib_element_count(int attrib) const noexcept = 0;
	virtual GLenum attrib_element_type(int attrib) const noexcept = 0;
	virtual bool attrib_element_normalized(int attrib) const noexcept{ return false; };
};

template<typename... Ts>
class VBODescriptorWithTypes {
private:
	using size_type = std::size_t;

	template<int index> struct sizes_populator {
		using T = std::size_t;
		static constexpr T value = sizeof(typename typelist_type_at<index, Ts...>::type);
	};
	template<int index> struct elements_count_populator {
		using T = std::size_t;
		using ValT = typename typelist_type_at<index, Ts...>::type;
		static constexpr T value = gl_elements_count<ValT>::elements;
		static_assert(value > 0, "ValT is NOT a scalar or vector type");
	};
	template<int index> struct elements_type_name_populator {
		using T = GLenum;
		using ValT = typename typelist_type_at<index, Ts...>::type;
		static constexpr T value = gl_type_name_enum<ValT>::gl_enum;
		static_assert(value != 0, "ValT is NOT a valid OpenGL type");
	};

	class _descriptor : public VertexBufferDescriptor {
		using sizes_arr = typename generate_array<sizeof...(Ts), sizes_populator>::result;
		using elements_arr = typename generate_array<sizeof...(Ts), elements_count_populator>::result;
		using element_type_names_arr = typename generate_array<sizeof...(Ts), elements_type_name_populator>::result;
	public:
		std::size_t attrib_count() const noexcept override{ return sizeof...(Ts); };
		std::size_t attrib_size(int attrib) const noexcept override{ return sizes_arr::data[attrib]; };
		std::size_t attrib_element_count(int attrib) const noexcept override{ return elements_arr::data[attrib]; };
		GLenum attrib_element_type(int attrib) const noexcept override{ return element_type_names_arr::data[attrib]; };
	};
public:
	using descriptor = _descriptor;
};

}
}
