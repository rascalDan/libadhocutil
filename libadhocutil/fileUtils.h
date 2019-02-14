#ifndef ADHOCUTIL_FILEUTILS_H
#define ADHOCUTIL_FILEUTILS_H

#include <filesystem>
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
		DLL_PUBLIC std::filesystem::path operator/(const std::filesystem::path & p, unsigned int n);

		/**
		 * File handle wrapper to ensure closure on scope exit
		 */
		class DLL_PUBLIC FileHandle {
			public:
				/**
				 * Move constructor.
				 */
				FileHandle(FileHandle &&) noexcept;

				/**
				 * Construct from an existing file descriptor.
				 * @param fd An open file descriptor.
				 */
				FileHandle(int fd) noexcept;

				/**
				 * Open a new file handle.
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 */
				FileHandle(const std::filesystem::path & path, int flags = O_RDONLY);

				/**
				 * Open a new file handle.
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 * @param mode File handle mode
				 */
				FileHandle(const std::filesystem::path & path, int flags, int mode);

				virtual ~FileHandle() noexcept;

				FileHandle(const FileHandle &) = delete;
				void operator=(const FileHandle &) = delete;

				/**
				 * Implicit conversion back to raw Unix file descriptor.
				 * @return The container file descriptor.
				 */
				operator int() const noexcept;

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
				FileHandleStat(const std::filesystem::path & path, int flags = O_RDONLY);

				/**
				 * Open a new file handle.
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 * @param mode File handle mode
				 */
				FileHandleStat(const std::filesystem::path & path, int flags, int mode);

				/**
				 * Get the stat structure.
				 * @return The stat structure.
				 */
				const struct stat & getStat() const noexcept;

				/**
				 * Refresh and return the stat structure.
				 * @return The stat structure.
				 */
				const struct stat & refreshStat();

			protected:
				/// The stat structure.
				struct stat st;

			private:
				DLL_PRIVATE void refreshStat(const std::filesystem::path & path);
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
				MemMap(const std::filesystem::path & path, int flags = O_RDONLY);

				/**
				 * Open a new file handle.
				 * @param path Path of file to open.
				 * @param flags File handle flags
				 * @param mode File handle mode
				 */
				MemMap(const std::filesystem::path & path, int flags, int mode);

				~MemMap();

				/// The file data.
				void * const data;

#ifdef __cpp_lib_string_view
				/**
				 * Create a std::string_view of the mapped data.
				 */
				template<typename T = char>
				auto sv() const
				{
					return std::basic_string_view<T>((const T *)data, st.st_size / sizeof(T));
				}
#endif

			private:
				DLL_PRIVATE void * setupMapInt(int flags) const;
				DLL_PRIVATE void * setupMap(int flags) const;
				DLL_PRIVATE void * setupMap(const std::filesystem::path & path, int flags) const;
		};
	}
}

#endif


