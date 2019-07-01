#include "memstream.h"
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys.h>

namespace AdHoc {
	MemStream::MemStream() :
		buf(nullptr),
		len(0),
		strm(open_memstream(&buf, &len))
	{
		if (!strm) {
			// LCOV_EXCL_START no sensible way to make open_memstream fail
			throw SystemException("open_memstream failed", strerror(errno), errno);
			// LCOV_EXCL_STOP
		}
	}

	MemStream::MemStream(MemStream && o) noexcept :
		buf(o.buf),
		len(o.len),
		strm(o.strm)
	{
		o.buf = nullptr;
		o.len = 0;
		o.strm = nullptr;
	}

	MemStream::~MemStream()
	{
		if (strm) {
			fclose(strm);
		}
		// NOLINTNEXTLINE(hicpp-no-malloc)
		free(buf);
	}

	MemStream & MemStream::operator=(MemStream && o) noexcept
	{
		if (strm) {
			fclose(strm);
		}
		// NOLINTNEXTLINE(hicpp-no-malloc)
		free(buf);

		buf = o.buf;
		len = o.len;
		strm = o.strm;

		o.buf = nullptr;
		o.len = 0;
		o.strm = nullptr;

		return *this;
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

