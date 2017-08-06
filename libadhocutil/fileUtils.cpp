#include "fileUtils.h"
#include "compileTimeFormatter.h"
#include <unistd.h>
#include <sys.h>
#include <sys/mman.h>

namespace AdHoc {
	namespace FileUtils {
		AdHocFormatter(FD, "FD %?");

		boost::filesystem::path operator/(const boost::filesystem::path & p, unsigned int n)
		{
			auto pp = p.begin();
			while (n--) ++pp;
			return *pp;
		}

		FileHandle::FileHandle(int d) :
			fh(d)
		{
		}

		FileHandle::FileHandle(const boost::filesystem::path & path, int flags) :
			fh(open(path.c_str(), flags))
		{
			if (fh < 0) {
				throw SystemExceptionOn("open(2) failed", strerror(errno), errno, path.string());
			}
		}

		FileHandle::FileHandle(const boost::filesystem::path & path, int flags, int mode) :
			fh(open(path.c_str(), flags, mode))
		{
			if (fh < 0) {
				throw SystemExceptionOn("open(2) failed", strerror(errno), errno, path.string());
			}
		}

		FileHandle::~FileHandle()
		{
			close(fh);
		}

		FileHandle::operator int() const
		{
			return fh;
		}

		FileHandleStat::FileHandleStat(int fd) :
			FileHandle(fd)
		{
			refreshStat();
		}

		FileHandleStat::FileHandleStat(const boost::filesystem::path & path, int flags) :
			FileHandle(path, flags)
		{
			refreshStat(path);
		}

		FileHandleStat::FileHandleStat(const boost::filesystem::path & path, int flags, int mode) :
			FileHandle(path, flags, mode)
		{
			refreshStat(path);
		}

		const struct stat &
		FileHandleStat::getStat() const
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
		FileHandleStat::refreshStat(const boost::filesystem::path & path)
		{
			if (fstat(fh, &st)) {
				// LCOV_EXCL_START can't think of a way to test open succeeding and fstat failing
				throw SystemExceptionOn("fstat(2) failed", strerror(errno), errno, path.string());
				// LCOV_EXCL_STOP
			}
		}

		MemMap::MemMap(int d, int flags) :
			FileHandleStat(d),
			data(setupMap(flags))
		{
		}

		MemMap::MemMap(const boost::filesystem::path & path, int flags) :
			FileHandleStat(path, flags),
			data(setupMap(path, flags))
		{
		}

		MemMap::MemMap(const boost::filesystem::path & path, int flags, int mode) :
			FileHandleStat(path, flags, mode),
			data(setupMap(path, flags))
		{
		}

		MemMap::~MemMap()
		{
			munmap(data, st.st_size);
		}

		void *
		MemMap::setupMapInt(int flags) const
		{
			return mmap(0, st.st_size, flags & (O_WRONLY | O_RDWR) ? PROT_WRITE : PROT_READ, MAP_SHARED, fh, 0);
		}

		void *
		MemMap::setupMap(int flags) const
		{
			auto data = setupMapInt(flags);
			if (data == (void*)-1) {
				throw SystemExceptionOn("mmap(2) failed", strerror(errno), errno, FD::get(fh));
			}
			return data;
		}

		void *
		MemMap::setupMap(const boost::filesystem::path & path, int flags) const
		{
			auto data = setupMapInt(flags);
			if (data == (void*)-1) {
				throw SystemExceptionOn("mmap(2) failed", strerror(errno), errno, path.string());
			}
			return data;
		}
	}
}

