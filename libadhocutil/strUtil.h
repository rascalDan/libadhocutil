#include <string_view>

namespace AdHoc {
	constexpr auto &
	rtrim(std::string_view & in, const auto c)
	{
		if (const auto n = in.find_last_not_of(c); n != std::string_view::npos) {
			in.remove_suffix(in.length() - n - 1);
		}
		return in;
	}

	constexpr auto &
	ltrim(std::string_view & in, const auto c)
	{
		if (const auto n = in.find_first_not_of(c); n != std::string_view::npos) {
			in.remove_prefix(n);
		}
		return in;
	}

	constexpr auto &
	trim(std::string_view & in, const auto c)
	{
		ltrim(in, c);
		rtrim(in, c);
		return in;
	}
}
