#ifndef ADHOCUTIL_SEMAPHORE_H
#define ADHOCUTIL_SEMAPHORE_H

// Borrowed from StackOverflow
// http://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads

#include "visibility.h"
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>

namespace AdHoc {
	/// A portable semaphore with timeout support
	class DLL_PUBLIC Semaphore {
	public:
		/// Construct a new semaphore with optional initial count.
		explicit Semaphore(std::ptrdiff_t initial);

		/// Notify one waiting thread.
		void release();
		/// Wait for a single count.
		void acquire();
		/// Wait for a single count with timeout.
		/// @param ms Timeout how long to wait.
		bool try_acquire_for(std::chrono::milliseconds ms);
		/// Wait for a single count with timeout.
		/// @param ms Timeout how long to wait.
		bool
		try_acquire_for(unsigned int ms)
		{
			return try_acquire_for(std::chrono::milliseconds(ms));
		}
		/// Wait for a single count with timeout.
		/// @param ms Timeout how long to wait.
		template<class Rep, class Period>
		bool
		try_acquire_for(const std::chrono::duration<Rep, Period> & rel_time)
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(rel_time);
		}

		/// Free
		[[nodiscard]] std::ptrdiff_t freeCount() const;

	private:
		std::mutex mutex;
		std::condition_variable condition;
		std::ptrdiff_t count;
	};
}

#endif
