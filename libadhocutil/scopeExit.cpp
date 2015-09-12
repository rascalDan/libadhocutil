#include "scopeExit.h"

namespace AdHoc {

ScopeExit::ScopeExit()
{
}

ScopeExit::ScopeExit(const Event & onexitpre, const Event & onsuccess, const Event & onfailure, const Event & onexitpost)
{
	if (onexitpre) onExitPre.push_back(onexitpre);
	if (onsuccess) onSuccess.push_back(onsuccess);
	if (onfailure) onFailure.push_back(onfailure);
	if (onexitpost) onExitPost.push_back(onexitpost);
}

ScopeExit::~ScopeExit()
{
	for(const auto & e : onExitPre) e();
	if (std::uncaught_exception()) {
		for(const auto & e : onFailure) e();
	}
	else {
		for(const auto & e : onSuccess) e();
	}
	for(const auto & e : onExitPost) e();
}

}

