#include "curlStream.h"

CurlStreamSource::CurlStreamSource(const std::string & url) :
	CurlHandle(url),
	buflen(0),
	buf(nullptr),
	res(CURLE_OK)
{
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &CurlStreamSource::recvWrapper);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, this);
}

std::streamsize
CurlStreamSource::read(char * target, std::streamsize targetSize)
{
	if (!buflen) {
		swapContext();
		checkCurlCode(res);
		if (!buflen) {
			return 0;
		}
	}
	size_t bytes = std::min<size_t>(buflen, targetSize);
	memcpy(target, buf, bytes);
	buflen -= bytes;
	buf += bytes;
	return bytes;
}

void
CurlStreamSource::callback()
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
	swapContext();
	return datalen;
}

