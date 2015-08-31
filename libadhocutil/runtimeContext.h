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

	protected:
		DLL_PRIVATE virtual void callback() = 0;

	private:
		DLL_PRIVATE static void callbackWrapper(RuntimeContext * rc);

		void * stack;
		ucontext_t ctxInitial;
		ucontext_t ctxCallback;
		bool swapped;
};

#endif

