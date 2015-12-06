#ifndef ADHOCUTIL_FPRINTBF_H
#define ADHOCUTIL_FPRINTBF_H

#include "visibility.h"
#include "buffer.h"
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>

DLL_PUBLIC size_t fprintss(FILE *, const std::string &);

DLL_PUBLIC size_t fprintbf(FILE *, const boost::format &);

DLL_PUBLIC FILE * fopen(const boost::filesystem::path & path, const char * mode);

template <typename ... Params>
size_t inline fprintbf(FILE * f, const std::string & fmt, const Params & ... p)
{
	return fprintbf(f, *AdHoc::Buffer::getFormat(fmt), p...);
}

template <typename Param, typename ... Params>
size_t inline fprintbf(FILE * f, boost::format & fmt, const Param & p, const Params & ... ps)
{
	fmt % p;
	return fprintbf(f, fmt, ps...);
}

#endif

