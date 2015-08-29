#ifndef ADHOCUTIL_CURLHANDLE_H
#define ADHOCUTIL_CURLHANDLE_H

#include <curl/curl.h>
#include "intrusivePtrBase.h"
#include "visibility.h"

class DLL_PUBLIC CurlHandle : public virtual IntrusivePtrBase {
	public:
		CurlHandle(const std::string & url);
		virtual ~CurlHandle();

		CurlHandle(const CurlHandle &) = delete;
		void operator=(const CurlHandle &) = delete;

		void setopt(CURLoption opt, const void * val);
		void getinto(CURLoption opt, void * & val);
		void appendHeader(const char *);
		void appendPost(const char *, const char *);
		void perform();

	protected:
		void checkCurlCode(CURLcode res) const;

		CURL * curl_handle;
		curl_slist * curl_headers;
		curl_httppost * postS, * postE;
};
typedef boost::intrusive_ptr<CurlHandle> CurlHandlePtr;

#endif

