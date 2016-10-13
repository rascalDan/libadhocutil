#ifndef ADHOCUTIL_FILEUTILS_H
#define ADHOCUTIL_FILEUTILS_H

#include <boost/filesystem/path.hpp>
#include <sys/stat.h>
#include "visibility.h"

namespace AdHoc {
	namespace FileUtils {
		DLL_PUBLIC boost::filesystem::path operator/(const boost::filesystem::path & p, unsigned int n);

		class DLL_PUBLIC FileHandle {
			public:
				FileHandle(const boost::filesystem::path & path);
				virtual ~FileHandle();

				const int fh;
		};

		class DLL_PUBLIC FileHandleStat : public FileHandle {
			public:
				FileHandleStat(const boost::filesystem::path & path);

				const struct stat & getStat() const;

			protected:
				struct stat st;
		};

		class DLL_PUBLIC MemMap : public FileHandleStat {
			public:
				MemMap(const boost::filesystem::path & path);
				~MemMap();

				void * const data;
		};
	}
}

#endif


