#ifndef LIBADHOC_MEMSTREAM_H
#define LIBADHOC_MEMSTREAM_H

#include <visibility.h>
#include <cstdio>
#include <string_view>

namespace AdHoc {
	/**
	 * Wrapper around open_memstream(3)
	 */
	class DLL_PUBLIC MemStream {
		public:
			MemStream();
			~MemStream();

			MemStream(const MemStream &) = delete;
			/// Standard move constructor
			MemStream(MemStream &&) noexcept;

			void operator=(const MemStream &) = delete;
			/// Standard move assignment
			MemStream & operator=(MemStream &&) noexcept;

			/// Implicit conversion to use as FILE* for writes
			operator FILE * ();
			/// Implicit conversion to use as const char * for reads
			operator const char * () const;
			/// Implicit conversion to use as std::string_view for reads
			operator std::string_view () const;

			/// Get buffer contents
			const char * buffer() const;
			/// Get buffer contents view
			std::string_view sv() const;
			/// Get buffer length
			size_t length() const;

		private:
			char * buf;
			size_t len;
			FILE * strm;
	};
}

#endif

