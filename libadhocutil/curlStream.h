#ifndef ADHOCUTIL_CURLSTREAM_H
#define ADHOCUTIL_CURLSTREAM_H

#include <boost/iostreams/stream.hpp>
#include "runtimeContext.h"
#include <string>
#include <curl/curl.h>
#include "visibility.h"
#include "curlHandle.h"

class DLL_PUBLIC CurlStreamSource : public boost::iostreams::source, public CurlHandle, RuntimeContext {
	public:
		CurlStreamSource(const std::string & url);

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

typedef boost::iostreams::stream<boost::reference_wrapper<CurlStreamSource>> CurlStream;

#endif

