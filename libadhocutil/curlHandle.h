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

		template <typename T>
		void setopt(CURLoption opt, const T val);
		void getinfo(CURLINFO info, long & val) const;
		void getinfo(CURLINFO info, int & val) const;
		void getinfo(CURLINFO info, double & val) const;
		void getinfo(CURLINFO info, char * & val) const;
		void appendHeader(const char *);
		void appendPost(const char *, const char *);
		void perform();

		operator CURL *() const;

	protected:
		void checkCurlCode(CURLcode res) const;

		CURL * curl_handle;
		curl_slist * curl_headers;
		curl_httppost * postS, * postE;
};
typedef boost::intrusive_ptr<CurlHandle> CurlHandlePtr;

template <>
void CurlHandle::setopt(CURLoption opt, const void * val);
template <>
void CurlHandle::setopt(CURLoption opt, int val);
template <>
void CurlHandle::setopt(CURLoption opt, long val);
template <typename T>
void CurlHandle::setopt(CURLoption opt, const T val)
{
	setopt(opt, (const void *)val);
}

#endif

