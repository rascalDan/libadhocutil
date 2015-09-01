#ifndef ADHOCUTIL_SCOPEEXIT_H
#define ADHOCUTIL_SCOPEEXIT_H

#include <boost/function.hpp>
#include "visibility.h"

namespace AdHoc {

/// Run code at scope exit.
class DLL_PUBLIC ScopeExit {
	public:
		/** Callback for code to be run. */
		typedef boost::function<void()> Event;

		/** Construct a trigger for running code at scope exit.
		 * @param pre Run this code (unconditionally) first.
		 * @param success Only run this if the scope is exitted cleanly.
		 * @param failure Only run this if the scope is exitted because of an uncaught exception.
		 * @param post Run this code (unconditionally) last.
		 */
		ScopeExit(const Event & pre, const Event & success = Event(), const Event & failure = Event(), const Event & post = Event());
		~ScopeExit();

		ScopeExit(const ScopeExit &) = delete;
		void operator=(const ScopeExit &) = delete;

	private:
		const Event onExitPre;
		const Event onSuccess;
		const Event onFailure;
		const Event onExitPost;
};

}

#endif

