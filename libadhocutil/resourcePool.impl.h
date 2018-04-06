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
		incRef();
	}

	template <typename R>
	ResourceHandle<R>::ResourceHandle(const ResourceHandle & rh) :
		resource(rh.resource)
	{
		incRef();
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
		return std::get<2>(*resource);
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
		incRef();
	}

	template <typename R>
	void
	ResourceHandle<R>::incRef() const
	{
		ASSERT(resource);
		++std::get<2>(*resource);
	}

	template <typename R>
	void
	ResourceHandle<R>::decRef()
	{
		ASSERT(resource);
		if (!--std::get<2>(*resource)) {
			if (auto pool = std::get<1>(*resource)) {
				if (std::uncaught_exception()) {
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
		for (auto & r : available) {
			destroyResource(r.get());
		}
		for (auto & r : inUse) {
			destroyResource(std::get<0>(*r.second).get());
			std::get<1>(*r.second) = nullptr;
		}
	}

	template <typename R>
	void
	ResourcePool<R>::destroyResource(R const *) const throw()
	{
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
		for (auto & r : available) {
			destroyResource(r.get());
		}
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
				auto ro = std::make_shared<typename ResourceHandle<R>::Object>(r, this, 0);
				available.pop_front();
				inUse.insert({ std::this_thread::get_id(), ro });
				return ro;
			}
			catch (...) {
				destroyResource(r.get());
				available.pop_front();
			}
		}
		auto ro = std::make_shared<typename ResourceHandle<R>::Object>(createResource(), this, 0);
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
			else {
				destroyResource(r.get());
			}
		}
		catch (...) {
			destroyResource(r.get());
		}
		poolSize.notify();
	}

	template <typename R>
	void
	ResourcePool<R>::discard(const std::shared_ptr<R> & r)
	{
		Lock(lock);
		removeFrom(r, inUse);
		destroyResource(r.get());
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

