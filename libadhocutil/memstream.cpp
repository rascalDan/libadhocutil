#include "memstream.h"
#include <cstdlib>

namespace AdHoc {
	MemStream::MemStream() :
		buf(nullptr),
		len(0),
		strm(open_memstream(&buf, &len))
	{
	}

	MemStream::~MemStream()
	{
		fclose(strm);
		free(buf);
	}

	MemStream::operator FILE * ()
	{
		return strm;
	}

	MemStream::operator const char * () const
	{
		fflush(strm);
		return buf;
	}

	MemStream::operator std::string_view() const
	{
		fflush(strm);
		return { buf, len };
	}

	const char * MemStream::buffer() const
	{
		return *this;
	}

	std::string_view MemStream::sv() const
	{
		return *this;
	}

	size_t MemStream::length() const
	{
		fflush(strm);
		return len;
	}
}

