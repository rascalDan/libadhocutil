#pragma once

#include "c++11Helpers.h"
#include "visibility.h"
#include <curl/curl.h> // IWYU pragma: export
#include <memory>
#include <string>

namespace AdHoc::Net {

	/// libcurl handle wrapper.
	/** Wraps a libcurl CURL * object in a C++ friendly manner. */
	class DLL_PUBLIC CurlHandle {
	public:
		/**
		 * Create a new CurlHandle.
		 * @param url Set the required CURLOPT_URL property to the given url.
		 */
		explicit CurlHandle(const std::string & url);
		/// Standard move/copy support
		SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(CurlHandle);
		virtual ~CurlHandle();

		/** Set option wrapper. */
		template<typename T>
		void
		setopt(CURLoption opt, const T & val)
		{
			curl_easy_setopt(curl_handle, opt, val);
		}
		/** Get info for long values */
		void getinfo(CURLINFO info, long & val) const;
		/** Get info for int values (avoids ambiguous call errors for ease of use) */
		void getinfo(CURLINFO info, int & val) const;
		/** Get info for double values */
		void getinfo(CURLINFO info, double & val) const;
		/** Get info for char * values */
		void getinfo(CURLINFO info, char *& val) const;
		/** Append the given HTTP header */
		void appendHeader(const char *);
		/** Append the given HTTP post content */
		void appendPost(const char *, const char *);
		/** Perform the CURL transfer. */
		void perform();

		/** Get the underlying CURL * handle. @warning Make changes at your own risk. */
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		operator CURL *() const;

	protected:
		/// @cond
		void checkCurlCode(CURLcode res) const;

		CURL * curl_handle;
		curl_slist * curl_headers;
		curl_httppost *postS, *postE;
		/// @endcond
	};
	using CurlHandlePtr = std::shared_ptr<CurlHandle>;
}
