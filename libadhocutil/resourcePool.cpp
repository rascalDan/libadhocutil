#include "resourcePool.h"
#include "compileTimeFormatter.h"
#include <semaphore>

namespace AdHoc {
	ResourcePoolBase::ResourcePoolBase(std::ptrdiff_t maxSize, std::size_t keep_) : keep {keep_}, poolSize {maxSize} { }

	void
	ResourcePoolBase::release()
	{
		poolSize.release();
	}

	void
	ResourcePoolBase::acquire()
	{
		poolSize.acquire();
	}

	bool
	ResourcePoolBase::try_acquire_for(std::chrono::milliseconds timeout)
	{
		return poolSize.try_acquire_for(timeout);
	}

	ResourcePoolBase::~ResourcePoolBase() = default;

	TimeOutOnResourcePool::TimeOutOnResourcePool(const char * const n) : name(n) { }

	AdHocFormatter(TimeOutOnResourcePoolMsg, "Timeout getting a resource from pool of %?");
	std::string
	TimeOutOnResourcePool::message() const noexcept
	{
		return TimeOutOnResourcePoolMsg::get(name);
	}

	NoCurrentResource::NoCurrentResource(const std::thread::id id, const char * const n) : threadId(id), name(n) { }

	AdHocFormatter(NoCurrentResourceMsg, "Thread %? has no current resource handle of type %?");
	std::string
	NoCurrentResource::message() const noexcept
	{
		return NoCurrentResourceMsg::get(threadId, name);
	}
}
