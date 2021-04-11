#ifndef NAGIOS_H
#define NAGIOS_H

#include "visibility.h"
#include <cstdint>
#include <iosfwd>
#include <string_view>

namespace AdHoc {
	enum class NagiosStatusCode : uint8_t {
		OK = 0,
		Warning = 1,
		Critical = 2,
		Unknown = 3,
	};

	DLL_PUBLIC bool submitNagiosPassiveServiceCheck(
			const std::string_view &, NagiosStatusCode, const std::string_view &);
	DLL_PUBLIC bool submitNagiosPassiveServiceCheck(
			std::ostream &, const std::string_view &, NagiosStatusCode, const std::string_view &);
}

#endif
