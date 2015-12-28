#include "semaphore.h"

namespace AdHoc {
	Semaphore::Semaphore(unsigned int initial) : count(initial)
	{
	}

	void
	Semaphore::notify()
	{
		boost::mutex::scoped_lock lock(mutex);
		++count;
		condition.notify_one();
	}

	void
	Semaphore::wait()
	{
		boost::mutex::scoped_lock lock(mutex);
		while (!count) {
			condition.wait(lock);
		}
		--count;
	}

	bool
	Semaphore::wait(unsigned int timeout)
	{
		const boost::system_time expiry = boost::get_system_time() + boost::posix_time::milliseconds(timeout);
		boost::mutex::scoped_lock lock(mutex);
		while (!count) {
			if (!condition.timed_wait(lock, expiry)) {
				return false;
			}
		}
		--count;
		return true;
	}

	unsigned int
	Semaphore::freeCount() const
	{
		return count;
	}
}

