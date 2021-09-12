#include "semaphore.h"
#include <chrono>

namespace AdHoc {
	Semaphore::Semaphore(std::size_t initial) : count(initial) { }

	void
	Semaphore::notify()
	{
		std::scoped_lock lock(mutex);
		++count;
		condition.notify_one();
	}

	void
	Semaphore::wait()
	{
		std::unique_lock lock(mutex);
		while (!count) {
			condition.wait(lock);
		}
		--count;
	}

	bool
	Semaphore::wait(unsigned int timeout)
	{
		const auto expiry = std::chrono::milliseconds(timeout);
		std::unique_lock lock(mutex);
		while (!count) {
			if (condition.wait_for(lock, expiry) == std::cv_status::timeout) {
				return false;
			}
		}
		--count;
		return true;
	}

	std::size_t
	Semaphore::freeCount() const
	{
		return count;
	}
}
