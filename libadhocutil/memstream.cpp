#include "memstream.h"
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys.h>

namespace AdHoc {
	MemStream::MemStream() :
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

	MemStream::operator FILE * () noexcept
	{
		return strm;
	}

	MemStream::operator const char * () const noexcept
	{
		return buffer();
	}

	MemStream::operator std::string_view() const noexcept
	{
		return { buffer(), len };
	}

	const char * MemStream::buffer() const noexcept
	{
		fflush(strm);
		return buf;
	}

	std::string_view MemStream::sv() const noexcept
	{
		return { buffer(), len };
	}

	size_t MemStream::length() const noexcept
	{
		fflush(strm);
		return len;
	}
}

