#ifndef ADHOCUTIL_CURLMULTIHANDLE_H
#define ADHOCUTIL_CURLMULTIHANDLE_H

#include <functional>
#include <set>
#include <map>
#include <memory>
#include "visibility.h"
#include "curlHandle.h"

namespace AdHoc {
namespace Net {

class RunningCurl;
typedef std::shared_ptr<RunningCurl> RunningCurlPtr;

/// Perform multiple CURL operations at once.
class DLL_PUBLIC CurlMultiHandle {
	public:
		/** A function that should consume the inbound byte stream. */
		typedef std::function<void(std::istream &)> Consumer;

		CurlMultiHandle();
		~CurlMultiHandle();

		/** Adds a new consumer for the given URL to the set of operations to perform. */
		CurlHandlePtr addCurl(const std::string &, const Consumer &);
		/** Perform all queued operations. */
		void performAll();

	private:
		typedef std::set<RunningCurlPtr> CURLs;
		typedef std::map<CURL *, RunningCurlPtr> Running;

		DLL_PRIVATE void addRunner(CURLM * curlm, Running & running, CURLs & curls);

		CURLs curls;
};
typedef std::shared_ptr<CurlMultiHandle> CurlMultiHandlePtr;

}
}

#endif

