#ifndef ADHOCUTIL_CURLSTREAM_H
#define ADHOCUTIL_CURLSTREAM_H

#include "c++11Helpers.h"
#include "curlHandle.h"
#include "runtimeContext.h"
#include "visibility.h"
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <cstddef>
#include <iosfwd>
#include <string>
namespace boost {
	template<class T> class reference_wrapper;
}

namespace AdHoc::Net {

	/// boost::iostreams::source implementation for CURL downloads.
	class DLL_PUBLIC CurlStreamSource :
		public boost::iostreams::source,
		public CurlHandle,
		::AdHoc::System::RuntimeContext {
	public:
		/** Construct a new stream source for the given URL. */
		explicit CurlStreamSource(const std::string & url);
		/// Standard move/copy support
		SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(CurlStreamSource);
		~CurlStreamSource() override;

		/** Required member function for reading of the stream source by boost::iostreams::stream. */
		std::streamsize read(char * target, std::streamsize targetSize);

	private:
		friend class CurlMultiHandle;
		DLL_PRIVATE void callback() override;

		DLL_PRIVATE static size_t recvWrapper(void * data, size_t sz, size_t nm, void * css);
		DLL_PRIVATE std::streamsize recv(void * data, std::streamsize datalen);

		std::streamsize buflen;
		char * buf;
		CURLcode res;
	};

	using CurlStream = boost::iostreams::stream<boost::reference_wrapper<CurlStreamSource>>;
}

#endif
