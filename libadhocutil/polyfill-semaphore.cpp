#include "polyfill-semaphore.h"
#include <chrono>

namespace AdHoc {
	Semaphore::Semaphore(std::ptrdiff_t initial) : count(initial) { }

	void
	Semaphore::release()
	{
		std::scoped_lock lock(mutex);
		++count;
		condition.notify_one();
	}

	void
	Semaphore::acquire()
	{
		std::unique_lock lock(mutex);
		while (!count) {
			condition.wait(lock);
		}
		--count;
	}

	bool
	Semaphore::try_acquire_for(std::chrono::milliseconds timeout)
	{
		std::unique_lock lock(mutex);
		while (!count) {
			if (condition.wait_for(lock, timeout) == std::cv_status::timeout) {
				return false;
			}
		}
		--count;
		return true;
	}

	std::ptrdiff_t
	Semaphore::freeCount() const
	{
		return count;
	}
}
