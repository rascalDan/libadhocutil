#ifndef ADHOCUTIL_SCOPEEXIT_H
#define ADHOCUTIL_SCOPEEXIT_H

#include <functional>
#include <vector>
#include "visibility.h"

namespace AdHoc {

/// Run code at scope exit.
class DLL_PUBLIC ScopeExit {
	public:
		/** Callback for code to be run. */
		typedef std::function<void()> Event;

		/**
		 * Construct an empty trigger for running code yet to be determined at scope exit
		 */
		ScopeExit();
		/** Construct a trigger for running code at scope exit.
		 * @param pre Run this code (unconditionally) first.
		 * @param success Only run this if the scope is exitted cleanly.
		 * @param failure Only run this if the scope is exitted because of an uncaught exception.
		 * @param post Run this code (unconditionally) last.
		 */
		ScopeExit(const Event & pre, const Event & success = Event(), const Event & failure = Event(), const Event & post = Event());
		~ScopeExit();

		/// Copying construction is disabled
		ScopeExit(const ScopeExit &) = delete;
		/// Assignment is disabled
		void operator=(const ScopeExit &) = delete;

		/// @cond
		std::vector<Event> onExitPre;
		std::vector<Event> onSuccess;
		std::vector<Event> onFailure;
		std::vector<Event> onExitPost;
		/// @endcond
};

}

#endif

