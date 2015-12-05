#include "fprintbf.h"
#include <stdio.h>
#include <system_error>

size_t
fprintss(FILE * f, const std::string & s)
{
	auto r = fwrite(s.c_str(), 1, s.length(), f);
	if (r < s.length()) {
		throw std::system_error(errno, std::system_category());
	}
	return s.length();
}

size_t
fprintbf(FILE * f, const boost::format & fmt)
{
	auto r = fwrite(fmt.str().c_str(), 1, fmt.size(), f);
	if (r < fmt.size()) {
		throw std::system_error(errno, std::system_category());
	}
	return fmt.size();
}

FILE *
fopen(const boost::filesystem::path & path, const char * mode)
{
	auto f = fopen(path.c_str(), mode);
	if (!f) {
		throw std::system_error(errno, std::system_category());
	}
	return f;
}

