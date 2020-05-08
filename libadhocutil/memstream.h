#ifndef LIBADHOC_MEMSTREAM_H
#define LIBADHOC_MEMSTREAM_H

#include <visibility.h>
#include <cstdio>
#include <string_view>
#include "c++11Helpers.h"

namespace AdHoc {
	/**
	 * Wrapper around open_memstream(3)
	 */
	class DLL_PUBLIC MemStream {
		public:
			MemStream();
			~MemStream();

			/// Standard move/copy support
			SPECIAL_MEMBERS_COPY(MemStream, delete);

			/// Standard move constructor
			MemStream(MemStream &&) noexcept;

			/// Standard move assignment
			MemStream & operator=(MemStream &&) noexcept;

			/// Implicit conversion to use as FILE* for writes
			// NOLINTNEXTLINE(hicpp-explicit-conversions)
			operator FILE * () noexcept;
			/// Implicit conversion to use as const char * for reads
			// NOLINTNEXTLINE(hicpp-explicit-conversions)
			operator const char * () const noexcept;
			/// Implicit conversion to use as std::string_view for reads
			// NOLINTNEXTLINE(hicpp-explicit-conversions)
			operator std::string_view () const noexcept;

			/// Get buffer contents
			[[nodiscard]] const char * buffer() const noexcept;
			/// Get buffer contents view
			[[nodiscard]] std::string_view sv() const noexcept;
			/// Get buffer length
			[[nodiscard]] size_t length() const noexcept;

		private:
			char * buf { nullptr };
			size_t len { 0 };
			FILE * strm;
	};
}

#endif

