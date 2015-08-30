#include "runtimeContext.h"
#include <stdexcept>

RuntimeContext::RuntimeContext(size_t stacksize) :
	swapped(false)
{
	stack = malloc(stacksize);
	if (getcontext(&callback) == -1)
		throw std::runtime_error("Failed to getcontext");
	callback.uc_stack.ss_sp = stack;
	callback.uc_stack.ss_size = stacksize;
	callback.uc_link = &initial;
	makecontext(&callback, (void (*)())&RuntimeContext::ccallback, 1, this);
}

RuntimeContext::~RuntimeContext()
{
	free(stack);
}

void
RuntimeContext::SwapContext()
{
	swapped = !swapped;
	if (swapped) {
		swapcontext(&initial, &callback);
	}
	else {
		if (stack) {
			swapcontext(&callback, &initial);
		}
	}
}

void
RuntimeContext::ccallback(RuntimeContext * rc)
{
	rc->Callback();
	free(rc->stack);
	rc->stack = nullptr;
}

