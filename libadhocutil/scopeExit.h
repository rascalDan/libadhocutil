#ifndef ADHOCUTIL_SCOPEEXIT_H
#define ADHOCUTIL_SCOPEEXIT_H

#include <boost/function.hpp>
#include "visibility.h"

class DLL_PUBLIC ScopeExit {
	public:
		typedef boost::function<void()> Event;
		ScopeExit(const Event &, const Event & = Event(), const Event & = Event(), const Event & = Event());
		~ScopeExit();

		ScopeExit(const ScopeExit &) = delete;
		void operator=(const ScopeExit &) = delete;

	private:
		const Event onExitPre;
		const Event onSuccess;
		const Event onFailure;
		const Event onExitPost;
};

#endif

