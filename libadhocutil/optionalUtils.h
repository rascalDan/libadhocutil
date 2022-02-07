#pragma once

#include <type_traits>

namespace AdHoc {
	template<typename A, typename B>
	auto
	operator/(const A & a, const B & b) -> typename std::enable_if<std::is_constructible<bool, A>::value
					&& !std::is_pointer<B>::value && std::is_convertible<decltype(*a), B>::value,
			decltype(a ? *a : b)>::type
	{
		return (a ? *a : b);
	}

	template<typename A, typename B>
	auto
	operator/(const A & a, const B & b) ->
			typename std::enable_if<std::is_constructible<bool, A>::value && std::is_pointer<B>::value,
					decltype(a ? &*a : b)>::type
	{
		return (a ? &*a : b);
	}
}
