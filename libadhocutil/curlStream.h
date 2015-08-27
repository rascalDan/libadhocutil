#ifndef ADHOCUTIL_CURLSTREAM_H
#define ADHOCUTIL_CURLSTREAM_H

#include <boost/iostreams/stream.hpp>
#include "runtimeContext.h"
#include <string>
#include <curl/curl.h>
#include "visibility.h"

class DLL_PUBLIC CurlStreamSource : public boost::iostreams::source, RuntimeContext {
	public:
		CurlStreamSource(const std::string & url);
		~CurlStreamSource();

		CurlStreamSource(const CurlStreamSource &) = delete;
		void operator=(const CurlStreamSource &) = delete;

		std::streamsize read(char * target, std::streamsize targetSize);

		void setopt(CURLoption opt, const void * & val);
		void appendHeader(const char * header);

	private:
		DLL_PRIVATE void Callback() override;

		DLL_PRIVATE static size_t recvWrapper(void * data, size_t sz, size_t nm, void * css);
		DLL_PRIVATE size_t recv(void * data, size_t datalen);

		CURL * curl_handle;
		struct curl_slist * curl_headers;
		size_t buflen;
		char * buf;
		CURLcode res;
};

typedef boost::reference_wrapper<CurlStreamSource> css_ref;

#endif

