#ifndef ADHOCUTIL_SEMAPHORE_H
#define ADHOCUTIL_SEMAPHORE_H

// Borrowed from StackOverflow
// http://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads

#include "visibility.h"
#include <condition_variable>
#include <mutex>

namespace AdHoc {
	/// A portable semaphore with timeout support
	class DLL_PUBLIC Semaphore {
	public:
		/// Construct a new semaphore with optional initial count.
		explicit Semaphore(unsigned int initial = 0);

		/// Notify one waiting thread.
		void notify();
		/// Wait for a single count.
		void wait();
		/// Wait for a single count with timeout.
		/// @param ms Timeout in milliseconds.
		bool wait(unsigned int ms);
		/// Free
		[[nodiscard]] unsigned int freeCount() const;

	private:
		std::mutex mutex;
		std::condition_variable condition;
		unsigned long count;
	};
}

#endif
