#include "curlHandle.h"
#include <net.h>
#include <boost/numeric/conversion/cast.hpp>
#include "compileTimeFormatter.h"

namespace AdHoc {
namespace Net {

static void cleanup() __attribute__((destructor));
static void cleanup()
{
	curl_global_cleanup();
}

CurlHandle::CurlHandle(const std::string & url) :
	curl_handle(curl_easy_init()),
	curl_headers(nullptr),
	postS(nullptr), postE(nullptr)
{
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
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

template <>
void
CurlHandle::setopt(CURLoption opt, const void * val)
{
	curl_easy_setopt(curl_handle, opt, val);
}

template <>
void
CurlHandle::setopt(CURLoption opt, int val)
{
	curl_easy_setopt(curl_handle, opt, val);
}

template <>
void
CurlHandle::setopt(CURLoption opt, long val)
{
	curl_easy_setopt(curl_handle, opt, val);
}

void
CurlHandle::getinfo(CURLINFO info, long & val) const
{
	curl_easy_getinfo(curl_handle, info, &val);
}

void
CurlHandle::getinfo(CURLINFO info, int & ival) const
{
	long val;
	curl_easy_getinfo(curl_handle, info, &val);
	ival = boost::numeric_cast<int>(val);
}

void
CurlHandle::getinfo(CURLINFO info, double & val) const
{
	curl_easy_getinfo(curl_handle, info, &val);
}

void
CurlHandle::getinfo(CURLINFO info, char * & val) const
{
	curl_easy_getinfo(curl_handle, info, &val);
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

CurlHandle::operator CURL *() const
{
	return curl_handle;
}

void
CurlHandle::checkCurlCode(CURLcode res) const
{
	if (res != CURLE_OK) {
		long http_code = 0;
		if (curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code) == CURLE_OK) {
			throw AdHoc::Net::CurlException(res, curl_easy_strerror(res), http_code);
		}
		throw AdHoc::Net::CurlException(res, curl_easy_strerror(res), IceUtil::None);
	}
}

AdHocFormatter(CurlExceptionMsg, "Network operation failed: %? (%?)");
AdHocFormatter(CurlExceptionMsgHttp, "HTTP operation failed: %?: %? (%?)");

void
CurlException::ice_print(std::ostream & s) const
{
	if (httpcode) {
		CurlExceptionMsgHttp::write(s, *httpcode, message, resultcode);
	}
	else {
		CurlExceptionMsg::write(s, message, resultcode);
	}
}

}
}

