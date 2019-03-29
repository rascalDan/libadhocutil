#include "runtimeContext.h"
#include <cerrno>
#include <cstring>
#include <sys.h>

// NOLINTNEXTLINE(modernize-concat-nested-namespaces)
namespace AdHoc {
namespace System {

RuntimeContext::RuntimeContext(size_t stacksize) :
	stack(stacksize),
	ctxInitial({}),
	ctxCallback({}),
	completed(false),
	swapped(false)
{
	if (getcontext(&ctxCallback) == -1) {
		throw SystemException("getcontext(3) failed", strerror(errno), errno);
	}
	ctxCallback.uc_stack.ss_sp = &stack.front();
	ctxCallback.uc_stack.ss_size = stacksize;
	ctxCallback.uc_link = &ctxInitial;
	makecontext(&ctxCallback, (void (*)())&RuntimeContext::callbackWrapper, 1, this);
}

void
RuntimeContext::swapContext()
{
	swapped = !swapped;
	if (swapped) {
		swapcontext(&ctxInitial, &ctxCallback);
	}
	else {
		if (!completed) {
			swapcontext(&ctxCallback, &ctxInitial);
		}
	}
}

bool
RuntimeContext::hasCompleted() const
{
	return completed;
}

void
RuntimeContext::callbackWrapper(RuntimeContext * rc)
{
	rc->callback();
	rc->completed = true;
}

}
}

