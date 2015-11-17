#ifndef ADHOCUTIL_SEMAPHORE_H
#define ADHOCUTIL_SEMAPHORE_H

// Borrowed from StackOverflow
// http://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include "visibility.h"

namespace AdHoc {
	/// A portable semaphore with timeout support
	class DLL_PUBLIC Semaphore {
		public:
			Semaphore(unsigned int initial = 0);

			void notify();
			void wait();
			bool wait(unsigned int);

		private:
			boost::mutex mutex;
			boost::condition_variable condition;
			unsigned long count;
	};
}

#endif

