#ifndef ADHOCUTIL_RESOURCEPOOL_IMPL_H
#define ADHOCUTIL_RESOURCEPOOL_IMPL_H

#include "resourcePool.h"
#include "lockHelpers.h"
#include "safeMapFind.h"

namespace AdHoc {
	//
	// ResourceHandle
	//

	template <typename R>
	ResourceHandle<R>::ResourceHandle(Object * o) :
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
		decRef();
	}

	template <typename R>
	unsigned int
	ResourceHandle<R>::handleCount() const
	{
		return boost::get<2>(*resource);
	}

	template <typename R>
	R *
	ResourceHandle<R>::get() const
	{
		return boost::get<0>(*resource);
	}

	template <typename R>
	R *
	ResourceHandle<R>::operator->() const
	{
		return boost::get<0>(*resource);
	}

	template <typename R>
	void
	ResourceHandle<R>::incRef() const
	{
		++boost::get<2>(*resource);
	}

	template <typename R>
	void
	ResourceHandle<R>::decRef()
	{
		if (!--boost::get<2>(*resource)) {
			if (std::uncaught_exception()) {
				boost::get<1>(*resource)->discard(boost::get<0>(*resource));
			}
			else {
				boost::get<1>(*resource)->putBack(boost::get<0>(*resource));
			}
			delete resource;
		}
		resource = nullptr;
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
			destroyResource(r);
		}
		for (auto & r : inUse) {
			destroyResource(boost::get<0>(*r.second));
		}
	}

	template <typename R>
	void
	ResourcePool<R>::destroyResource(R * r) const
	{
		delete r;
	}

	template <typename R>
	void
	ResourcePool<R>::testResource(const R *) const
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
			destroyResource(r);
		}
		available.clear();
	}

	template <typename R>
	ResourceHandle<R>
	ResourcePool<R>::get()
	{
		poolSize.wait();
		return getOne();
	}

	template <typename R>
	ResourceHandle<R>
	ResourcePool<R>::get(unsigned int timeout)
	{
		if (!poolSize.wait(timeout)) {
			throw TimeOutOnResourcePoolT<R>();
		}
		return getOne();
	}

	template <typename R>
	ResourceHandle<R>
	ResourcePool<R>::getOne()
	{
		UpgradableLock(lock, ulock);
		if (available.empty()) {
			auto ro = new typename ResourceHandle<R>::Object(createResource(), this);
			UpgradeLock(ulock);
			inUse.insert({ std::this_thread::get_id(), ro });
			return ro;
		}
		else {
			UpgradeLock(ulock);
			auto ro = new typename ResourceHandle<R>::Object(available.front(), this);
			available.pop_front();
			inUse.insert({ std::this_thread::get_id(), ro });
			return ro;
		}
	}

	template <typename R>
	void
	ResourcePool<R>::putBack(R * r)
	{
		Lock(lock);
		removeFrom(r, inUse);
		if (available.size() < keep) {
			available.push_back(r);
		}
		else {
			destroyResource(r);
		}
		poolSize.notify();
	}

	template <typename R>
	void
	ResourcePool<R>::discard(R * r)
	{
		Lock(lock);
		removeFrom(r, inUse);
		destroyResource(r);
		poolSize.notify();
	}

	template <typename R>
	void
	ResourcePool<R>::removeFrom(R * r, InUse & inUse)
	{
		auto rs = inUse.equal_range(std::this_thread::get_id());
		for (auto & ri = rs.first; ri != rs.second; ri++) {
			if (boost::get<0>(*ri->second) == r) {
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

