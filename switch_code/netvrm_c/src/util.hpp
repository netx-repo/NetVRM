#pragma once

#include <type_traits>
#include <vector>

namespace netvrm {
/* Removes all pointers from a type. This is the general template when the type doesn't include pointers, so we directly return itself. */
template<class Container>
struct remove_all_pointers {
    using type = Container;
};

/* Removes all pointers from a type. This is the specialized version for when the type contains pointers, so we recursively remove it layer by layer. */
template<class Container>
struct remove_all_pointers<Container*> {
    using type = typename remove_all_pointers<Container>::type;
};

/* Gives the type of content of a vector. This is the general version when the type doesn't contain pointers. */
template<class Container>
struct value_type {
    using type = typename std::decay<Container>::type::value_type;
};

/* Gives the type of content of a vector. This is the specialized version when the type contains pointers. */
template<class Container>
struct value_type<Container*> {
    using type = typename value_type<typename remove_all_pointers<Container*>::type>::type;
};

/* Given the type of a vector, returns the type of element inside that vector. This function accepts pointers, l and r-references, as well as values */
template<class Container>
using value_type_t = typename value_type<Container>::type;
}