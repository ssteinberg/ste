// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vk_vertex_input_attributes.hpp>

#include <vk_format_type_traits.hpp>
#include <ste_type_traits.hpp>

#include <typelist.hpp>
#include <generate_array.hpp>

namespace StE {
namespace GL {

template<typename... Ts>
class vertex_attributes_from_tuple {
private:
	using size_type = std::size_t;

	template<typename T, typename... Tail> 
	struct stride_counter {
		static constexpr size_type value = sizeof(T) + stride_counter<Tail...>::value;
	};
	template<typename T> 
	struct stride_counter<T>  {
		static constexpr size_type value = sizeof(T);
	};

	template<int index> struct sizes_populator {
		using T = size_type;
		static constexpr T value = sizeof(typename typelist_type_at<index, Ts...>::type);
	};
	template<int index> struct elements_format_populator {
		using T = VkFormat;
		using ValT = typename typelist_type_at<index, Ts...>::type;
		static constexpr T value = vk_format_for_type_v<ValT>;
		static_assert(value != 0, "ValT is NOT a valid type");
	};

	class _descriptor : public vk_vertex_input_attributes {
		using sizes_arr = typename generate_array<sizeof...(Ts), sizes_populator>::result;
		using element_type_names_arr = typename generate_array<sizeof...(Ts), elements_format_populator>::result;
	public:
		virtual ~_descriptor() noexcept {}

		size_type attrib_count() const noexcept override { return sizeof...(Ts); };
		size_type stride() const noexcept override { return stride_counter<Ts...>::value; };
		size_type attrib_size(int attrib) const noexcept override { return sizes_arr::data[attrib]; };
		VkFormat attrib_format(int attrib) const noexcept override { return element_type_names_arr::data[attrib]; };
	};
public:
	using descriptor = _descriptor;
};

}
}
