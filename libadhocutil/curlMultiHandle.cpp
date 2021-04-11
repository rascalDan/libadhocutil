#include "curlMultiHandle.h"
#include "curlStream.h"
#include <boost/core/ref.hpp>
#include <boost/core/typeinfo.hpp>
#include <boost/iostreams/stream.hpp>
#include <map>
#include <sys/select.h>
#include <sys/time.h>
#include <utility>

namespace AdHoc::Net {

	class RunningCurl : public CurlStreamSource {
	public:
		RunningCurl(const std::string & url, std::function<void(std::istream &)> c) :
			CurlStreamSource(url), consumer(std::move(c))
		{
		}

		void
		callback() override
		{
			using rc_ref = boost::reference_wrapper<RunningCurl>;
			boost::iostreams::stream<rc_ref> curlstrm(std::ref(*this));
			consumer(curlstrm);
		}

	private:
		const std::function<void(std::istream &)> consumer;
	};

	CurlMultiHandle::CurlMultiHandle() = default;

	CurlMultiHandle::~CurlMultiHandle() = default;

	CurlHandlePtr
	CurlMultiHandle::addCurl(const std::string & url, const std::function<void(std::istream &)> & c)
	{
		return *curls.insert(std::make_shared<RunningCurl>(url, c)).first;
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
			int act = running.size();
			while (act) {
				while (curl_multi_perform(curlm, &act) == CURLM_CALL_MULTI_PERFORM) { }
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
				struct timeval to = {0, 100000};
				// NOLINTNEXTLINE(hicpp-no-assembler)
				FD_ZERO(&r);
				// NOLINTNEXTLINE(hicpp-no-assembler)
				FD_ZERO(&w);
				// NOLINTNEXTLINE(hicpp-no-assembler)
				FD_ZERO(&e);
				curl_multi_fdset(curlm, &r, &w, &e, &maxfd);
				select(act, &r, &w, &e, &to);
			}
			curl_multi_cleanup(curlm);
		}
	}

}
