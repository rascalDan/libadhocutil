#ifndef ADHOCUTIL_CASELESS_H
#define ADHOCUTIL_CASELESS_H

#include <algorithm> // IWYU pragma: keep
#include <cstring>

namespace AdHoc {
	/// STL compatible case-insentive less comparator wrapping strncasecmp
	struct case_less {
		/// Compare a and b
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
