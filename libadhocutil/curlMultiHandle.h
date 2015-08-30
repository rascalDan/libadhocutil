#ifndef ADHOCUTIL_CURLMULTIHANDLE_H
#define ADHOCUTIL_CURLMULTIHANDLE_H

#include <boost/function.hpp>
#include <set>
#include <map>
#include "intrusivePtrBase.h"
#include "visibility.h"
#include "curlHandle.h"

class RunningCurl;
typedef boost::intrusive_ptr<RunningCurl> RunningCurlPtr;

class DLL_PUBLIC CurlMultiHandle : public IntrusivePtrBase {
	public:
		typedef std::set<RunningCurlPtr> CURLs;
		typedef boost::function<void(std::istream &)> Consumer;

		CurlMultiHandle();
		~CurlMultiHandle();

		CurlMultiHandle(const CurlMultiHandle &) = delete;
		void operator=(const CurlMultiHandle &) = delete;

		CurlHandlePtr addCurl(const std::string &, const Consumer &);
		void performAll();

	private:
		typedef std::map<CURL *, RunningCurlPtr> Running;

		DLL_PRIVATE void addRunner(CURLM * curlm, Running & running, CurlMultiHandle::CURLs & curls);

		CURLs curls;
};
typedef boost::intrusive_ptr<CurlMultiHandle> CurlMultiHandlePtr;

#endif

