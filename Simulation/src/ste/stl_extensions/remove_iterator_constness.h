// StE
// � Shlomi Steinberg, 2015

#pragma once

namespace StE {

template <typename Container, typename ConstIterator>
typename Container::iterator remove_iterator_constness(Container& c, ConstIterator it) {
	return c.erase(it, it);
}

}
