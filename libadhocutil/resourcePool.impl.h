#ifndef ADHOCUTIL_RESOURCEPOOL_IMPL_H
#define ADHOCUTIL_RESOURCEPOOL_IMPL_H

#include "resourcePool.h"
#include "lockHelpers.h"
#include "safeMapFind.h"

#define ASSERT(expr) if(!expr) throw std::runtime_error(#expr)

namespace AdHoc {
	//
	// ResourceHandle
	//

	template <typename R>
	ResourceHandle<R>::ResourceHandle(const std::shared_ptr<Object> & o) :
		resource(o)
	{
	}

	template <typename R>
	ResourceHandle<R>::ResourceHandle(const ResourceHandle & rh) :
		resource(rh.resource)
	{
	}

	template <typename R>
	ResourceHandle<R>::ResourceHandle(ResourceHandle && rh) :
		resource(rh.resource)
	{
		rh.decRef();
	}

	template <typename R>
	ResourceHandle<R>::~ResourceHandle()
	{
		if (resource) {
			decRef();
		}
	}

	template <typename R>
	unsigned int
	ResourceHandle<R>::handleCount() const
	{
		ASSERT(resource);
		// InUse has one, we don't count that
		return resource.use_count() - 1;
	}

	template <typename R>
	R *
	ResourceHandle<R>::get() const
	{
		ASSERT(resource);
		return std::get<0>(*resource).get();
	}

	template <typename R>
	void
	ResourceHandle<R>::release()
	{
		decRef();
	}

	template <typename R>
	ResourceHandle<R>::operator bool() const
	{
		return (bool)resource;
	}

	template <typename R>
	R *
	ResourceHandle<R>::operator->() const
	{
		ASSERT(resource);
		return std::get<0>(*resource).get();
	}

	template <typename R>
	void
	ResourceHandle<R>::operator=(const ResourceHandle & rh)
	{
		if (resource) {
			decRef();
		}
		resource = rh.resource;
	}

	template <typename R>
	void
	ResourceHandle<R>::operator=(ResourceHandle && rh)
	{
		if (resource) {
			decRef();
		}
		resource = rh.resource;
		rh.decRef();
	}

	template <typename R>
	void
	ResourceHandle<R>::decRef()
	{
		ASSERT(resource);
		// InUse has one, we don't count that
		if (resource.use_count() == 2) {
			if (auto pool = std::get<1>(*resource)) {
				if (std::uncaught_exceptions()) {
					pool->discard(std::get<0>(*resource));
				}
				else {
					pool->putBack(std::get<0>(*resource));
				}
			}
		}
		resource.reset();
	}

	//
	// ResourcePool
	//

	template <typename R>
	ResourcePool<R>::ResourcePool(unsigned int max, unsigned int k) :
		poolSize(max),
		keep(k)
	{
	}

	template <typename R>
	ResourcePool<R>::~ResourcePool()
	{
		for (auto & r : inUse) {
			std::get<1>(*r.second) = nullptr;
		}
	}

	template <typename R>
	void
	ResourcePool<R>::testResource(R const *) const
	{
	}

	template <typename R>
	void
	ResourcePool<R>::returnTestResource(R const *) const
	{
	}

	template <typename R>
	unsigned int
	ResourcePool<R>::inUseCount() const
	{
		SharedLock(lock);
		return inUse.size();
	}

	template <typename R>
	unsigned int
	ResourcePool<R>::availableCount() const
	{
		SharedLock(lock);
		return available.size();
	}

	template <typename R>
	unsigned int
	ResourcePool<R>::freeCount() const
	{
		SharedLock(lock);
		return poolSize.freeCount();
	}

	template <typename R>
	ResourceHandle<R>
	ResourcePool<R>::getMine()
	{
		Lock(lock);
		return safeMapLookup<NoCurrentResourceT<R>>(inUse, std::this_thread::get_id());
	}

	template <typename R>
	void
	ResourcePool<R>::idle()
	{
		Lock(lock);
		available.clear();
	}

	template <typename R>
	ResourceHandle<R>
	ResourcePool<R>::get()
	{
		poolSize.wait();
		try {
			return getOne();
		}
		catch(...) {
			poolSize.notify();
			throw;
		}
	}

	template <typename R>
	ResourceHandle<R>
	ResourcePool<R>::get(unsigned int timeout)
	{
		if (!poolSize.wait(timeout)) {
			throw TimeOutOnResourcePoolT<R>();
		}
		try {
			return getOne();
		}
		catch(...) {
			poolSize.notify();
			throw;
		}
	}

	template <typename R>
	ResourceHandle<R>
	ResourcePool<R>::getOne()
	{
		Lock(lock);
		while (!available.empty()) {
			auto & r = available.front();
			try {
				testResource(r.get());
				auto ro = std::make_shared<typename ResourceHandle<R>::Object>(r, this);
				available.pop_front();
				inUse.insert({ std::this_thread::get_id(), ro });
				return ro;
			}
			catch (...) {
				available.pop_front();
			}
		}
		auto ro = std::make_shared<typename ResourceHandle<R>::Object>(createResource(), this);
		inUse.insert({ std::this_thread::get_id(), ro });
		return ro;
	}

	template <typename R>
	void
	ResourcePool<R>::putBack(const std::shared_ptr<R> & r)
	{
		Lock(lock);
		removeFrom(r, inUse);
		try {
			returnTestResource(r.get());
			if (available.size() < keep) {
				available.push_back(r);
			}
		}
		catch (...) {
		}
		poolSize.notify();
	}

	template <typename R>
	void
	ResourcePool<R>::discard(const std::shared_ptr<R> & r)
	{
		Lock(lock);
		removeFrom(r, inUse);
		poolSize.notify();
	}

	template <typename R>
	void
	ResourcePool<R>::removeFrom(const std::shared_ptr<R> & r, InUse & inUse)
	{
		auto rs = inUse.equal_range(std::this_thread::get_id());
		for (auto & ri = rs.first; ri != rs.second; ri++) {
			if (std::get<0>(*ri->second) == r) {
				inUse.erase(ri);
				return;
			}
		}
	}

	template <typename R>
	TimeOutOnResourcePoolT<R>::TimeOutOnResourcePoolT() :
		TimeOutOnResourcePool(typeid(R).name())
	{
	}

	template <typename R>
	NoCurrentResourceT<R>::NoCurrentResourceT(const std::thread::id & id) :
		NoCurrentResource(id, typeid(R).name())
	{
	}

}

#endif

