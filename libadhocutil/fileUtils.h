#ifndef ADHOCUTIL_FILEUTILS_H
#define ADHOCUTIL_FILEUTILS_H

#include <boost/filesystem/path.hpp>
#include <sys/stat.h>
#include <fcntl.h>
#include <string_view>
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
				 * Move constructor.
				 */
				FileHandle(FileHandle &&);

				/**
				 * Construct from an existing file descriptor.
				 * @param fd An open file descriptor.
				 */
				FileHandle(int fd);

				/**
				 * Open a new file handle.
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 */
				FileHandle(const boost::filesystem::path & path, int flags = O_RDONLY);

				/**
				 * Open a new file handle.
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 * @param mode File handle mode
				 */
				FileHandle(const boost::filesystem::path & path, int flags, int mode);

				virtual ~FileHandle();

				FileHandle(const FileHandle &) = delete;
				void operator=(const FileHandle &) = delete;

				/**
				 * Implicit conversion back to raw Unix file descriptor.
				 * @return The container file descriptor.
				 */
				operator int() const;

				/// The file handle.
				const int fh;
		};

		/**
		 * An extension to FileHandle that automatically calls fstat on the opened handle.
		 */
		class DLL_PUBLIC FileHandleStat : public FileHandle {
			public:
				/**
				 * Move constructor.
				 */
				FileHandleStat(FileHandleStat &&) = default;

				/**
				 * Construct from an existing file descriptor.
				 * @param fd An open file descriptor.
				 */
				FileHandleStat(int fd);

				/**
				 * Open a new file handle (with the default flags).
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 */
				FileHandleStat(const boost::filesystem::path & path, int flags = O_RDONLY);

				/**
				 * Open a new file handle.
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 * @param mode File handle mode
				 */
				FileHandleStat(const boost::filesystem::path & path, int flags, int mode);

				/**
				 * Get the stat structure.
				 * @return The stat structure.
				 */
				const struct stat & getStat() const;

				/**
				 * Refresh and return the stat structure.
				 * @return The stat structure.
				 */
				const struct stat & refreshStat();

			protected:
				/// The stat structure.
				struct stat st;

			private:
				DLL_PRIVATE void refreshStat(const boost::filesystem::path & path);
		};

		/**
		 * Extension to FileHandle to automatically memmaps the file.
		 */
		class DLL_PUBLIC MemMap : public FileHandleStat {
			public:
				/**
				 * Move constructor.
				 */
				MemMap(MemMap &&) = default;

				/**
				 * Construct from an existing file descriptor.
				 * @param fd An open file descriptor.
				 * @param flags File handle flags
				 */
				MemMap(int fd, int flags = O_RDONLY);

				/**
				 * Open a new file handle (with the default flags).
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 */
				MemMap(const boost::filesystem::path & path, int flags = O_RDONLY);

				/**
				 * Open a new file handle.
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 * @param mode File handle mode
				 */
				MemMap(const boost::filesystem::path & path, int flags, int mode);

				~MemMap();

				/// The file data.
				void * const data;

				/**
				 * Create a std::string_view of the mapped data.
				 */
				template<typename T = char>
				auto sv() const
				{
					return std::basic_string_view<T>((const T *)data, st.st_size / sizeof(T));
				}

			private:
				DLL_PUBLIC void * setupMapInt(int flags) const;
				DLL_PUBLIC void * setupMap(int flags) const;
				DLL_PUBLIC void * setupMap(const boost::filesystem::path & path, int flags) const;
		};
	}
}

#endif


