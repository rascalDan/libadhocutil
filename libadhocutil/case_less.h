#ifndef ADHOCUTIL_CASELESS_H
#define ADHOCUTIL_CASELESS_H

#include <algorithm>
#include <cstring>

namespace AdHoc {
	struct case_less {
		template<typename A, typename B>
		inline bool
		operator()(const A & a, const B & b) const
		{
			const auto cmp = strncasecmp(a.data(), b.data(), std::min(a.length(), b.length()));
			return (cmp < 0) || (!cmp && a.length() < b.length());
		}
	};
}

#endif
