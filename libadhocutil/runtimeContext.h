#ifndef ADHOCUTIL_RUNTIMECONTEXT_H
#define ADHOCUTIL_RUNTIMECONTEXT_H

#include <stdlib.h>
#include <ucontext.h>
#include "visibility.h"

class DLL_PUBLIC RuntimeContext {
	public:
		RuntimeContext(size_t stacksize = 16384);
		virtual ~RuntimeContext();

		void SwapContext();

	protected:
		DLL_PRIVATE virtual void Callback() = 0;

	private:
		DLL_PRIVATE static void ccallback(RuntimeContext * rc);

		void * stack;
		ucontext_t initial;
		ucontext_t callback;
		bool swapped;
};

#endif

