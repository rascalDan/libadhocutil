#ifndef ADHOCUTIL_RUNTIMECONTEXT_H
#define ADHOCUTIL_RUNTIMECONTEXT_H

#include "c++11Helpers.h"
#include "visibility.h"
#include <cstdlib>
#include <ucontext.h>
#include <vector>

namespace AdHoc::System {

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
		explicit RuntimeContext(size_t stacksize = 16384);
		virtual ~RuntimeContext() = default;
		/// Standard move/copy support
		SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(RuntimeContext);

		/** Swap to/from the contained stack. */
		void swapContext();

		/** Has the callback on the contained stack run to completion? */
		[[nodiscard]] bool hasCompleted() const;

	protected:
		/** Overridden in a sub-class to implement functionality in the alternate stack */
		DLL_PRIVATE virtual void callback() = 0;

	private:
		DLL_PRIVATE static void callbackWrapper(RuntimeContext * rc);

		std::vector<char> stack;
		ucontext_t ctxInitial {};
		ucontext_t ctxCallback {};
		bool completed {false};
		bool swapped {false};
	};

}

#endif
