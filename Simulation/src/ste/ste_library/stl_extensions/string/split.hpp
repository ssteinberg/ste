// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <string>
#include <sstream>

namespace ste {

/**
 *	@brief	Splits a string into substrings based on a delimiter
 *	
 *	@param	s		String to split
 *	@param	delim	Delimiter
 *	@param	out_it	Output iterator
 *	@param	remove_empty		Ignore empty substrings
 */
template <typename Out, class CharT, class Traits, class Allocator>
void split(const std::basic_string<CharT, Traits, Allocator> &s, 
		   const CharT &delim, 
		   Out out_it,
		   bool remove_empty = true) {
	std::basic_stringstream<CharT, Traits, Allocator> ss;
	ss.str(s);

	std::basic_string<CharT, Traits, Allocator> item;
	while (std::getline(ss, item, delim)) {
		if (remove_empty && item.empty())
			continue;
		*(out_it++) = item;
	}
}

/**
*	@brief	Convenience method to splits a string into a container of substrings based on a delimiter.
*			The container type and output iterator are template parameters.
*
*	@param	s		String to split
*	@param	delim	Delimiter
 *	@param	remove_empty		Ignore empty substrings
*	
*	@return	Container of substrings
*/
template <
	template<typename...> class Container, 
	template<typename> class OutputIterator, 
	class CharT, 
	class Traits, 
	class Allocator
>
auto split(const std::basic_string<CharT, Traits, Allocator> &s, 
		   const CharT &delim,
		   bool remove_empty = true) {
	using T = std::basic_string<CharT, Traits, Allocator>;
	using container_type = Container<T>;

	container_type elems;
	split(s, 
		  delim, 
		  OutputIterator<container_type>(elems),
		  remove_empty);
	return elems;
}

}
