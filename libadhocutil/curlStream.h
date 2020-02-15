#ifndef ADHOCUTIL_CURLSTREAM_H
#define ADHOCUTIL_CURLSTREAM_H

#include <boost/iostreams/stream.hpp>
#include "runtimeContext.h"
#include <string>
#include <curl/curl.h>
#include "visibility.h"
#include "curlHandle.h"
#include "c++11Helpers.h"

namespace AdHoc {
namespace Net {

/// boost::iostreams::source implementation for CURL downloads.
class DLL_PUBLIC CurlStreamSource : public boost::iostreams::source, public CurlHandle, ::AdHoc::System::RuntimeContext {
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
		DLL_PRIVATE size_t recv(void * data, size_t datalen);

		size_t buflen;
		char * buf;
		CURLcode res;
};

using CurlStream = boost::iostreams::stream<boost::reference_wrapper<CurlStreamSource>>;

}
}

#endif

