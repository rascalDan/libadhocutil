#include "scopeExit.h"

namespace AdHoc {

ScopeExit::ScopeExit(const Event & onexitpre, const Event & onsuccess, const Event & onfailure, const Event & onexitpost) :
	onExitPre(onexitpre),
	onSuccess(onsuccess),
	onFailure(onfailure),
	onExitPost(onexitpost)
{
}

ScopeExit::~ScopeExit()
{
	if (onExitPre) onExitPre();
	if (std::uncaught_exception()) {
		if (onFailure) onFailure();
	}
	else {
		if (onSuccess) onSuccess();
	}
	if (onExitPost) onExitPost();
}

}

