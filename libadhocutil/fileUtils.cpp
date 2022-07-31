#include "fileUtils.h"
#include "compileTimeFormatter.h"
#include <boost/assert.hpp>
#include <cerrno>
#include <cstring>
#include <memory>
#include <sys.h>
#include <sys/mman.h>
#include <unistd.h>

namespace AdHoc::FileUtils {
	AdHocFormatter(FD, "FD %?");

	std::filesystem::path
	operator/(const std::filesystem::path & p, unsigned int n)
	{
		auto pp = p.begin();
		while (n--) {
			++pp;
		}
		return *pp;
	}

	FileHandle::FileHandle(int d) noexcept : fh(d) { }

	FileHandle::FileHandle(FileHandle && o) noexcept : fh(o.fh)
	{
		const_cast<int &>(o.fh) = -1;
	}

	FileHandle &
	FileHandle::operator=(FileHandle && o) noexcept
	{
		const_cast<int &>(fh) = o.fh;
		const_cast<int &>(o.fh) = -1;
		return *this;
	}

	FileHandle::FileHandle(const std::filesystem::path & path, int flags) : fh(open(path.c_str(), flags))
	{
		if (fh < 0) {
			throw SystemExceptionOn("open(2) failed", strerror(errno), errno, path);
		}
	}

	FileHandle::FileHandle(const std::filesystem::path & path, int flags, int mode) :
		fh(open(path.c_str(), flags, mode))
	{
		if (fh < 0) {
			throw SystemExceptionOn("open(2) failed", strerror(errno), errno, path);
		}
	}

	FileHandle::~FileHandle() noexcept
	{
		if (fh >= 0) {
			BOOST_VERIFY(close(fh) == 0);
		}
	}

	FileHandle::operator int() const noexcept
	{
		return fh;
	}

	FileHandleStat::FileHandleStat(int fd) : FileHandle(fd), st({})
	{
		refreshStat();
	}

	FileHandleStat::FileHandleStat(const std::filesystem::path & path, int flags) : FileHandle(path, flags), st({})
	{
		refreshStat(path);
	}

	FileHandleStat::FileHandleStat(const std::filesystem::path & path, int flags, int mode) :
		FileHandle(path, flags, mode), st({})
	{
		refreshStat(path);
	}

	const struct stat &
	FileHandleStat::getStat() const noexcept
	{
		return st;
	}

	const struct stat &
	FileHandleStat::refreshStat()
	{
		if (fstat(fh, &st)) {
			// LCOV_EXCL_START can't think of a way to test open succeeding and fstat failing
			throw SystemExceptionOn("fstat(2) failed", strerror(errno), errno, FD::get(fh));
			// LCOV_EXCL_STOP
		}
		return st;
	}

	void
	FileHandleStat::refreshStat(const std::filesystem::path & path)
	{
		if (fstat(fh, &st)) {
			// LCOV_EXCL_START can't think of a way to test open succeeding and fstat failing
			throw SystemExceptionOn("fstat(2) failed", strerror(errno), errno, path);
			// LCOV_EXCL_STOP
		}
	}

	MemMap::MemMap(int d, int flags) : FileHandleStat(d), data(setupMap(flags)) { }

	MemMap::MemMap(const std::filesystem::path & path, int flags) :
		FileHandleStat(path, flags), data(setupMap(path, flags))
	{
	}

	MemMap::MemMap(const std::filesystem::path & path, int flags, int mode) :
		FileHandleStat(path, flags, mode), data(setupMap(path, flags))
	{
	}

	MemMap::~MemMap()
	{
		munmap(const_cast<void *>(data), static_cast<size_t>(st.st_size));
	}

	void *
	MemMap::setupMapInt(int flags) const
	{
		return mmap(nullptr, static_cast<size_t>(st.st_size), (flags & (O_WRONLY | O_RDWR)) ? PROT_WRITE : PROT_READ,
				MAP_SHARED, fh, 0);
	}

	void *
	MemMap::setupMap(int flags) const
	{
		auto newData = setupMapInt(flags);
		// NOLINTNEXTLINE(performance-no-int-to-ptr)
		if (newData == MAP_FAILED) {
			throw SystemExceptionOn("mmap(2) failed", strerror(errno), errno, FD::get(fh));
		}
		return newData;
	}

	void *
	MemMap::setupMap(const std::filesystem::path & path, int flags) const
	{
		auto newData = setupMapInt(flags);
		// NOLINTNEXTLINE(performance-no-int-to-ptr)
		if (newData == MAP_FAILED) {
			throw SystemExceptionOn("mmap(2) failed", strerror(errno), errno, path);
		}
		return newData;
	}
}
