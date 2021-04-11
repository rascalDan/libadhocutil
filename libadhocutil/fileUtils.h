#ifndef ADHOCUTIL_FILEUTILS_H
#define ADHOCUTIL_FILEUTILS_H

#include "c++11Helpers.h"
#include "visibility.h"
#include <fcntl.h> // IWYU pragma: export
#include <filesystem> // IWYU pragma: export
#include <string_view>
#include <sys/stat.h> // IWYU pragma: export

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
			explicit FileHandle(int fd) noexcept;

			/**
			 * Open a new file handle.
			 * @param path Path of file to open.
			 * @param flags File handle flags
			 */
			explicit FileHandle(const std::filesystem::path & path, int flags = O_RDONLY);

			/**
			 * Open a new file handle.
			 * @param path Path of file to open.
			 * @param flags File handle flags
			 * @param mode File handle mode
			 */
			FileHandle(const std::filesystem::path & path, int flags, int mode);

			virtual ~FileHandle() noexcept;

			/// Standard move/copy support
			SPECIAL_MEMBERS_COPY(FileHandle, delete);

			/// Standard move/copy support
			FileHandle & operator=(FileHandle &&) noexcept;

			/**
			 * Implicit conversion back to raw Unix file descriptor.
			 * @return The container file descriptor.
			 */
			// NOLINTNEXTLINE(hicpp-explicit-conversions)
			operator int() const noexcept;

			/// The file handle.
			const int fh;
		};

		/**
		 * An extension to FileHandle that automatically calls fstat on the opened handle.
		 */
		class DLL_PUBLIC FileHandleStat : public FileHandle {
		public:
			~FileHandleStat() override = default;

			/**
			 * Construct from an existing file descriptor.
			 * @param fd An open file descriptor.
			 */
			explicit FileHandleStat(int fd);

			/**
			 * Open a new file handle (with the default flags).
			 * @param path Path of file to open.
			 * @param flags File handle flags
			 */
			explicit FileHandleStat(const std::filesystem::path & path, int flags = O_RDONLY);

			/**
			 * Open a new file handle.
			 * @param path Path of file to open.
			 * @param flags File handle flags
			 * @param mode File handle mode
			 */
			FileHandleStat(const std::filesystem::path & path, int flags, int mode);

			/// Standard move/copy support
			SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(FileHandleStat);

			/**
			 * Get the stat structure.
			 * @return The stat structure.
			 */
			[[nodiscard]] const struct stat & getStat() const noexcept;

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
			MemMap(MemMap &&) noexcept = default;

			/**
			 * Construct from an existing file descriptor.
			 * @param fd An open file descriptor.
			 * @param flags File handle flags
			 */
			explicit MemMap(int fd, int flags = O_RDONLY);

			/**
			 * Open a new file handle (with the default flags).
			 * @param path Path of file to open.
			 * @param flags File handle flags
			 */
			explicit MemMap(const std::filesystem::path & path, int flags = O_RDONLY);

			/**
			 * Open a new file handle.
			 * @param path Path of file to open.
			 * @param flags File handle flags
			 * @param mode File handle mode
			 */
			MemMap(const std::filesystem::path & path, int flags, int mode);

			~MemMap() override;

			/// Standard move/copy support
			SPECIAL_MEMBERS_COPY(MemMap, delete);

			MemMap & operator=(MemMap &&) = delete;

			/// The file data.
			const void * const data;

#ifdef __cpp_lib_string_view
			/**
			 * Create a std::string_view of the mapped data.
			 */
			template<typename T = char>
			[[nodiscard]] auto
			sv() const
			{
				return std::basic_string_view<T>((const T *)data, st.st_size / sizeof(T));
			}
#endif

		private:
			[[nodiscard]] DLL_PRIVATE void * setupMapInt(int flags) const;
			[[nodiscard]] DLL_PRIVATE void * setupMap(int flags) const;
			[[nodiscard]] DLL_PRIVATE void * setupMap(const std::filesystem::path & path, int flags) const;
		};
	}
}

#endif
