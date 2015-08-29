#include "curlHandle.h"
#include <net.h>

static void cleanup() __attribute__((destructor));
static void cleanup()
{
	curl_global_cleanup();
}

CurlHandle::CurlHandle(const std::string & url) :
	curl_handle(curl_easy_init()),
	curl_headers(nullptr),
	postS(NULL), postE(NULL)
{
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
}

CurlHandle::~CurlHandle()
{
	if (curl_headers) {
		curl_slist_free_all(curl_headers);
	}
	if (postS) {
		curl_formfree(postS);
	}
	curl_easy_cleanup(curl_handle);
}

void
CurlHandle::setopt(CURLoption opt, const void * val)
{
	curl_easy_setopt(curl_handle, opt, val);
}

void
CurlHandle::appendHeader(const char * header)
{
	curl_headers = curl_slist_append(curl_headers, header);
}

void
CurlHandle::appendPost(const char * name, const char * value)
{
	CURLFORMcode r = curl_formadd(&postS, &postE, CURLFORM_PTRNAME, name, CURLFORM_PTRCONTENTS, value, CURLFORM_END);
	if (r == 0) {
		curl_easy_setopt(curl_handle, CURLOPT_HTTPPOST, postS);
	}
}

void
CurlHandle::perform()
{
	if (curl_headers) {
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, curl_headers);
	}
	checkCurlCode(curl_easy_perform(curl_handle));
}

void
CurlHandle::checkCurlCode(CURLcode res) const
{
	if (res != CURLE_OK) {
		AdHoc::Net::CurlException ce(res, curl_easy_strerror(res), IceUtil::Optional<Ice::Short>());
		long http_code = 0;
		if (curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code) == CURLE_OK) {
			ce.httpcode = http_code;
		}
		throw ce;
	}
}

