#include "resourcePool.h"
#include "compileTimeFormatter.h"

namespace AdHoc {
	TimeOutOnResourcePool::TimeOutOnResourcePool(const char * const n) :
		name(n)
	{
	}

	AdHocFormatter(TimeOutOnResourcePoolMsg, "Timeout getting a resource from pool of %?");
	std::string
	TimeOutOnResourcePool::message() const noexcept
	{
		return TimeOutOnResourcePoolMsg::get(name);
	}

	NoCurrentResource::NoCurrentResource(const std::thread::id & id, const char * const n) :
		threadId(id),
		name(n)
	{
	}

	AdHocFormatter(NoCurrentResourceMsg, "Thread %? has no current resource handle of type %?");
	std::string
	NoCurrentResource::message() const noexcept
	{
		return NoCurrentResourceMsg::get(threadId, name);
	}
}

