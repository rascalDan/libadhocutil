#ifndef ADHOCUTIL_FILEUTILS_H
#define ADHOCUTIL_FILEUTILS_H

#include <boost/filesystem/path.hpp>
#include <sys/stat.h>
#include <fcntl.h>
#include "visibility.h"

namespace AdHoc {
	namespace FileUtils {
		/**
		 * Extract an element of a path.
		 * @param p The source path.
		 * @param n The index of the element to extract.
		 * @return The path element.
		 */
		DLL_PUBLIC boost::filesystem::path operator/(const boost::filesystem::path & p, unsigned int n);

		/**
		 * File handle wrapper to ensure closure on scope exit
		 */
		class DLL_PUBLIC FileHandle {
			public:
				/**
				 * Open a new file handle.
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 */
				FileHandle(const boost::filesystem::path & path, int flags = O_RDONLY);
				virtual ~FileHandle();

				/// The file handle.
				const int fh;
		};

		/**
		 * An extension to FileHandle that automatically calls fstat on the opened handle.
		 */
		class DLL_PUBLIC FileHandleStat : public FileHandle {
			public:
				/**
				 * Open a new file handle (with the default flags).
				 * @param path Path of file to open.
				 */
				FileHandleStat(const boost::filesystem::path & path);

				/**
				 * Get the stat structure.
				 * @return The stat structure.
				 */
				const struct stat & getStat() const;

			protected:
				/// The stat structure.
				struct stat st;
		};

		/**
		 * Extension to FileHandle to automatically memmaps the file.
		 */
		class DLL_PUBLIC MemMap : public FileHandleStat {
			public:
				/**
				 * Open a new file handle (with the default flags).
				 * @param path Path of file to open.
				 */
				MemMap(const boost::filesystem::path & path);
				~MemMap();

				/// The file data.
				void * const data;
		};
	}
}

#endif


