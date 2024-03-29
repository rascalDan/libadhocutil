#pragma once

#include "c++11Helpers.h"
#include "curlHandle.h"
#include "visibility.h"
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace AdHoc::Net {
	class RunningCurl;
	using RunningCurlPtr = std::shared_ptr<RunningCurl>;

	/// Perform multiple CURL operations at once.
	class DLL_PUBLIC CurlMultiHandle {
	public:
		/** A function that should consume the inbound byte stream. */
		using Consumer = std::function<void(std::istream &)>;

		CurlMultiHandle();
		~CurlMultiHandle();

		/// Standard move/copy support
		SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(CurlMultiHandle);

		/** Adds a new consumer for the given URL to the set of operations to perform. */
		CurlHandlePtr addCurl(const std::string &, const Consumer &);
		/** Perform all queued operations. */
		void performAll();

	private:
		using CURLs = std::set<RunningCurlPtr>;
		using Running = std::map<CURL *, RunningCurlPtr>;

		DLL_PRIVATE void addRunner(CURLM * curlm, Running & running, CURLs & curls);

		CURLs curls;
	};
	using CurlMultiHandlePtr = std::shared_ptr<CurlMultiHandle>;
}
