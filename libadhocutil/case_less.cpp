#include "case_less.h"
#include <string_view>

static_assert(&AdHoc::case_less::operator()<std::string_view, std::string_view>);
