#include "resourcePool.h"
#include "buffer.h"

namespace AdHoc {
	TimeOutOnResourcePool::TimeOutOnResourcePool(const char * const n) :
		name(n)
	{
	}

	std::string
	TimeOutOnResourcePool::message() const throw()
	{
		return stringbf("Timeout getting a resource from pool of %s", name);
	}

	NoCurrentResource::NoCurrentResource(const std::thread::id & id, const char * const n) :
		threadId(id),
		name(n)
	{
	}

	std::string
	NoCurrentResource::message() const throw()
	{
		return stringbf("Thread %s has no current resource handle of type %s", threadId, name);
	}
}

