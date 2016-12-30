#include "runtimeContext.h"
#include <errno.h>
#include <string.h>
#include <sys.h>

namespace AdHoc {
namespace System {

RuntimeContext::RuntimeContext(size_t stacksize) :
	completed(false),
	swapped(false)
{
	stack = malloc(stacksize);
	if (getcontext(&ctxCallback) == -1)
		throw SystemException("getcontext(3) failed", strerror(errno), errno);
	ctxCallback.uc_stack.ss_sp = stack;
	ctxCallback.uc_stack.ss_size = stacksize;
	ctxCallback.uc_link = &ctxInitial;
	makecontext(&ctxCallback, (void (*)())&RuntimeContext::callbackWrapper, 1, this);
}

RuntimeContext::~RuntimeContext()
{
	free(stack);
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
			if (completed) {
				free(stack);
				stack = nullptr;
			}
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

