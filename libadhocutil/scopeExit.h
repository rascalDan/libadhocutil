#ifndef ADHOCUTIL_SCOPEEXIT_H
#define ADHOCUTIL_SCOPEEXIT_H

#include <functional>
#include <vector>
#include "visibility.h"
#include "c++11Helpers.h"

namespace AdHoc {

/// Run code at scope exit.
class DLL_PUBLIC ScopeExit {
	public:
		/** Callback for code to be run. */
		using Event = std::function<void()>;

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
		explicit ScopeExit(const Event & pre, const Event & success = Event(), const Event & failure = Event(), const Event & post = Event());
		~ScopeExit();

		/// Standard move/copy support
		SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(ScopeExit);

		/// @cond
		std::vector<Event> onExitPre;
		std::vector<Event> onSuccess;
		std::vector<Event> onFailure;
		std::vector<Event> onExitPost;
		/// @endcond
};

}

#endif

