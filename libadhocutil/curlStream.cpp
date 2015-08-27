#include "curlStream.h"
#include <net.h>

CurlStreamSource::CurlStreamSource(const std::string & url) :
	curl_handle(curl_easy_init()),
	curl_headers(nullptr),
	buflen(0),
	buf(nullptr),
	res(CURLE_OK)
{
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &CurlStreamSource::recvWrapper);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, this);
}

CurlStreamSource::~CurlStreamSource()
{
	if (curl_headers) {
		curl_slist_free_all(curl_headers);
	}
	if (res != CURLE_OK) {
		AdHoc::Net::CurlException ce(res, curl_easy_strerror(res), IceUtil::Optional<Ice::Short>());
		long http_code = 0;
		if (curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code) == CURLE_OK) {
			ce.httpcode = http_code;
		}
		curl_easy_cleanup(curl_handle);
		throw ce;
	}
	curl_easy_cleanup(curl_handle);
}

std::streamsize
CurlStreamSource::read(char * target, std::streamsize targetSize)
{
	if (!buflen) {
		SwapContext();
	}
	size_t bytes = std::min<size_t>(buflen, targetSize);
	memcpy(target, buf, bytes);
	buflen -= bytes;
	buf += bytes;
	return bytes;
}

void
CurlStreamSource::setopt(CURLoption opt, const void * & val)
{
	curl_easy_setopt(curl_handle, opt, val);
}

void
CurlStreamSource::appendHeader(const char * header)
{
	curl_headers = curl_slist_append(curl_headers, header);
}

void
CurlStreamSource::Callback()
{
	if (curl_headers) {
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, curl_headers);
	}
	res = curl_easy_perform(curl_handle);
}

size_t
CurlStreamSource::recvWrapper(void * data, size_t sz, size_t nm, void * css)
{
	return static_cast<CurlStreamSource *>(css)->recv(data, sz * nm);
}

size_t
CurlStreamSource::recv(void * data, size_t datalen)
{
	buf = (char *)data;
	buflen = datalen;
	SwapContext();
	return datalen;
}

