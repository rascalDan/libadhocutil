#ifndef LIBADHOC_MEMSTREAM_H
#define LIBADHOC_MEMSTREAM_H

#include <visibility.h>
#include <cstdio>
#include <string_view>

namespace AdHoc {
	class DLL_PUBLIC MemStream {
		public:
			MemStream();
			~MemStream();

			MemStream(const MemStream &) = delete;
			MemStream(MemStream &&) = delete;

			void operator=(const MemStream &) = delete;
			void operator=(MemStream &&) = delete;

			operator FILE * ();
			operator const char * () const;
			operator std::string_view () const;

			const char * buffer() const;
			std::string_view sv() const;
			size_t length() const;

		private:
			char * buf;
			size_t len;
			FILE * strm;
	};
}

#endif

