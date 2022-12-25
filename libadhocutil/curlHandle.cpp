#include "curlHandle.h"
#include "compileTimeFormatter.h"
#include <Ice/Optional.h>
#include <boost/numeric/conversion/cast.hpp>
#include <net.h>

namespace AdHoc::Net {

	static void cleanup() __attribute__((destructor));
	static void
	cleanup()
	{
		curl_global_cleanup();
	}

	CurlHandle::CurlHandle(const std::string & url) : curl_handle(curl_easy_init())
	{
		curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
	}

	CurlHandle::~CurlHandle()
	{
		if (curl_headers) {
			curl_slist_free_all(curl_headers);
		}
		if (mime) {
			curl_mime_free(mime);
		}
		curl_easy_cleanup(curl_handle);
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
	CurlHandle::getinfo(CURLINFO info, char *& val) const
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
		if (auto part = curl_mime_addpart(mime)) {
			curl_mime_name(part, name);
			curl_mime_data(part, value, CURL_ZERO_TERMINATED);
			curl_easy_setopt(curl_handle, CURLOPT_MIMEPOST, mime);
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
				throw AdHoc::Net::CurlException(res, curl_easy_strerror(res), static_cast<short>(http_code));
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
