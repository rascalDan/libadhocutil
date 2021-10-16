#include "case_less.h"
#include <string_view>

static_assert(
		std::is_member_function_pointer_v<decltype(&AdHoc::case_less::operator()<std::string_view, std::string_view>)>);
