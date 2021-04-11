#ifndef ADHOCUTIL_FPRINTBF_H
#define ADHOCUTIL_FPRINTBF_H

#include "buffer.h"
#include "visibility.h"
#include <boost/format.hpp> // IWYU pragma: keep
#include <boost/format/format_fwd.hpp>
#include <cstdio>
#include <filesystem>
#include <string>

DLL_PUBLIC size_t fprintss(FILE *, const std::string &);

DLL_PUBLIC size_t fprintbf(FILE *, const boost::format &);

DLL_PUBLIC FILE * fopen(const std::filesystem::path & path, const char * mode);

template<typename... Params> size_t inline fprintbf(FILE * f, const std::string & fmt, const Params &... p)
{
	auto bf = AdHoc::Buffer::getFormat(fmt);
	return fprintbf(f, bf, p...);
}

template<typename Param, typename... Params>
size_t inline fprintbf(FILE * f, boost::format & fmt, const Param & p, const Params &... ps)
{
	fmt % p;
	return fprintbf(f, fmt, ps...);
}

#endif
