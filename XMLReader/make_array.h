#pragma once
// code copy from make_array.h in repositry xml_text_cooking_quiz(Author:yumetodo)

#include <array>
#include <type_traits>
#if (_MSC_VER < 1900)
#define CONSTEXPR inline
#else
#define CONSTEXPR constexpr
#endif

template<class T> CONSTEXPR std::array<T, 0> make_array() {
	return std::array<T, 0>{ {}};
}
template<class... T> CONSTEXPR std::array<typename std::common_type<T...>::type, sizeof...(T)> make_array(T... args) {
	return std::array<typename std::common_type<T...>::type, sizeof...(T)>{ {args...}};
}
