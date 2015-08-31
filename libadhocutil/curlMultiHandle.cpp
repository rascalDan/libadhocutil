#include "curlMultiHandle.h"
#include <boost/iostreams/stream.hpp>
#include <map>
#include "runtimeContext.h"
#include "curlStream.h"

class RunningCurl : public CurlStreamSource {
	public:
		RunningCurl(const std::string & url, const boost::function<void(std::istream &)> & c) :
			CurlStreamSource(url),
			consumer(c)
		{
		}

		void callback() override
		{
			typedef boost::reference_wrapper<RunningCurl> rc_ref;
			boost::iostreams::stream<rc_ref> curlstrm(boost::ref(*this));
			consumer(curlstrm);
		}

	private:
		const boost::function<void(std::istream &)> consumer;
};

CurlMultiHandle::CurlMultiHandle()
{
}

CurlMultiHandle::~CurlMultiHandle()
{
}

CurlHandlePtr
CurlMultiHandle::addCurl(const std::string & url, const boost::function<void(std::istream &)> & c)
{
	RunningCurl * css = new RunningCurl(url, c);
	curls.insert(css);
	return css;
}

void
CurlMultiHandle::addRunner(CURLM * curlm, Running & running, CurlMultiHandle::CURLs & curls)
{
	auto runner = *curls.begin();
	curl_multi_add_handle(curlm, *runner);
	running[*runner] = runner;
	runner->swapContext();
	curls.erase(runner);
}

void
CurlMultiHandle::performAll()
{
	if (!curls.empty()) {
		Running running;
		CURLM * curlm = curl_multi_init();

		while (!curls.empty() && running.size() < 5) {
			addRunner(curlm, running, curls);
		}
		CURLMcode code;
		int act = running.size();
		while (act) {
			while ((code = curl_multi_perform(curlm, &act)) == CURLM_CALL_MULTI_PERFORM) ;
			// Has anything finished
			CURLMsg * msg;
			int msgs = 0;
			while ((msg = curl_multi_info_read(curlm, &msgs))) {
				if (msg->msg == CURLMSG_DONE) {
					curl_multi_remove_handle(curlm, msg->easy_handle);
					auto ri = running.find(msg->easy_handle);
					ri->second->res = msg->data.result;
					ri->second->swapContext();
					running.erase(ri);
					if (!curls.empty()) {
						addRunner(curlm, running, curls);
						act += 1;
					}
				}
			}
			// Wait for something to happen
			fd_set r, w, e;
			int maxfd = 0;
			struct timeval to = { 0, 100000 };
			FD_ZERO(&r);
			FD_ZERO(&w);
			FD_ZERO(&e);
			curl_multi_fdset(curlm, &r, &w, &e, &maxfd);
			select(act, &r, &w, &e, &to);
		}
		curl_multi_cleanup(curlm);
	}
}

