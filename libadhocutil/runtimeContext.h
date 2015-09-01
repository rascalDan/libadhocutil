#ifndef ADHOCUTIL_RUNTIMECONTEXT_H
#define ADHOCUTIL_RUNTIMECONTEXT_H

#include <stdlib.h>
#include <ucontext.h>
#include "visibility.h"

class DLL_PUBLIC RuntimeContext {
	public:
		RuntimeContext(size_t stacksize = 16384);
		virtual ~RuntimeContext();

		void swapContext();
		bool hasCompleted() const;

	protected:
		DLL_PRIVATE virtual void callback() = 0;

	private:
		DLL_PRIVATE static void callbackWrapper(RuntimeContext * rc);

		void * stack;
		ucontext_t ctxInitial;
		ucontext_t ctxCallback;
		bool completed;
		bool swapped;
};

#endif

