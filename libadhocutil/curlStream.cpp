#include "curlStream.h"
#include <algorithm>
#include <cstring>

namespace AdHoc::Net {

	CurlStreamSource::CurlStreamSource(const std::string & url) : CurlHandle(url)
	{
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &CurlStreamSource::recvWrapper);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, this);
	}

	AdHoc::Net::CurlStreamSource::~CurlStreamSource() = default;

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
		auto bytes = std::min(buflen, targetSize);
		memcpy(target, buf, static_cast<size_t>(bytes));
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
		return static_cast<size_t>(
				static_cast<CurlStreamSource *>(css)->recv(data, static_cast<std::streamsize>(sz * nm)));
	}

	std::streamsize
	CurlStreamSource::recv(void * data, std::streamsize datalen)
	{
		buf = static_cast<char *>(data);
		buflen = datalen;
		swapContext();
		return datalen;
	}

}
