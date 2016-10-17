#include "fileUtils.h"
#include <unistd.h>
#include <sys.h>
#include <sys/mman.h>

namespace AdHoc {
	namespace FileUtils {
		boost::filesystem::path operator/(const boost::filesystem::path & p, unsigned int n)
		{
			auto pp = p.begin();
			while (n--) ++pp;
			return *pp;
		}

		FileHandle::FileHandle(const boost::filesystem::path & path, int flags) :
			fh(open(path.c_str(), flags))
		{
			if (fh < 0) {
				throw SystemException("Failed to open " + path.string(), strerror(errno), errno);
			}
		}

		FileHandle::FileHandle(const boost::filesystem::path & path, int flags, int mode) :
			fh(open(path.c_str(), flags, mode))
		{
			if (fh < 0) {
				throw SystemException("Failed to open " + path.string(), strerror(errno), errno);
			}
		}

		FileHandle::~FileHandle()
		{
			close(fh);
		}

		FileHandleStat::FileHandleStat(const boost::filesystem::path & path) :
			FileHandle(path)
		{
			if (fstat(fh, &st)) {
				// LCOV_EXCL_START can't think of a way to test open succeeding and fstat failing
				throw SystemException("Failed to fstat " + path.string(), strerror(errno), errno);
				// LCOV_EXCL_STOP
			}
		}

		const struct stat &
		FileHandleStat::getStat() const
		{
			return st;
		}

		MemMap::MemMap(const boost::filesystem::path & path) :
			FileHandleStat(path),
			data(mmap(0, st.st_size, PROT_READ, MAP_SHARED, fh, 0))
		{
			if (data == (void*)-1) {
				throw SystemException("Failed to mmap " + path.string(), strerror(errno), errno);
			}
		}

		MemMap::~MemMap()
		{
			munmap(data, st.st_size);
		}
	}
}

