#ifndef ADHOCUTIL_RUNTIMECONTEXT_H
#define ADHOCUTIL_RUNTIMECONTEXT_H

#include <stdlib.h>
#include <ucontext.h>
#include "visibility.h"
#include <vector>

namespace AdHoc {
namespace System {

/// Runtime Context
/**
 * Create an alternate stack for processing.
 */
class DLL_PUBLIC RuntimeContext {
	public:
		/**
		 * Create a new RuntimeContent
		 * @param stacksize The size in bytes of the new stack.
		 */
		RuntimeContext(size_t stacksize = 16384);
		virtual ~RuntimeContext() = default;

		/** Swap to/from the contained stack. */
		void swapContext();

		/** Has the callback on the contained stack run to completion? */
		bool hasCompleted() const;

	protected:
		/** Overridden in a sub-class to implement functionality in the alternate stack */
		DLL_PRIVATE virtual void callback() = 0;

	private:
		DLL_PRIVATE static void callbackWrapper(RuntimeContext * rc);

		std::vector<char> stack;
		ucontext_t ctxInitial;
		ucontext_t ctxCallback;
		bool completed;
		bool swapped;
};

}
}

#endif

