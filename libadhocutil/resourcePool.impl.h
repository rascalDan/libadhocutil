#ifndef ADHOCUTIL_RESOURCEPOOL_IMPL_H
#define ADHOCUTIL_RESOURCEPOOL_IMPL_H

#include "lockHelpers.h"
#include "resourcePool.h" // IWYU pragma: export
#include "safeMapFind.h"
#include "semaphore.h"
#include <boost/assert.hpp>
#include <cstddef>
#include <exception>
#include <memory>
#include <thread>
#include <typeinfo>

namespace AdHoc {
	//
	// ResourceHandle
	//

	template<typename R> ResourceHandle<R>::ResourceHandle(std::shared_ptr<Object> o) noexcept : resource(std::move(o))
	{
	}

	template<typename R> ResourceHandle<R>::~ResourceHandle() noexcept
	{
		if (resource) {
			decRef();
		}
	}

	template<typename R>
	std::size_t
	ResourceHandle<R>::handleCount() const
	{
		BOOST_ASSERT(resource);
		// InUse has one, we don't count that
		return static_cast<std::size_t>(resource.use_count() - 1);
	}

	template<typename R>
	R *
	ResourceHandle<R>::get() const noexcept
	{
		BOOST_ASSERT(resource);
		return std::get<0>(*resource).get();
	}

	template<typename R>
	void
	ResourceHandle<R>::release() noexcept
	{
		decRef();
	}

	template<typename R> ResourceHandle<R>::operator bool() const noexcept
	{
		return resource.operator bool();
	}

	template<typename R>
	R *
	ResourceHandle<R>::operator->() const noexcept
	{
		BOOST_ASSERT(resource);
		return std::get<0>(*resource).get();
	}

	template<typename R>
	ResourceHandle<R> &
	ResourceHandle<R>::operator=(const ResourceHandle & rh) noexcept
	{
		if (&rh != this) {
			if (resource) {
				decRef();
			}
			resource = rh.resource;
		}
		return *this;
	}

	template<typename R>
	ResourceHandle<R> &
	ResourceHandle<R>::operator=(ResourceHandle && rh) noexcept
	{
		if (&rh != this) {
			if (resource) {
				decRef();
			}
			resource = std::move(rh.resource);
		}
		return *this;
	}

	template<typename R>
	void
	ResourceHandle<R>::decRef() noexcept
	{
		BOOST_ASSERT(resource);
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

	template<typename R> ResourcePool<R>::ResourcePool(std::size_t max, std::size_t k) : poolSize(max), keep(k) { }

	template<typename R> ResourcePool<R>::~ResourcePool()
	{
		for (auto & r : inUse) {
			std::get<1>(*r.second) = nullptr;
		}
	}

	template<typename R>
	void
	ResourcePool<R>::testResource(R const *) const
	{
	}

	template<typename R>
	void
	ResourcePool<R>::returnTestResource(R const *) const
	{
	}

	template<typename R>
	std::size_t
	ResourcePool<R>::inUseCount() const
	{
		SharedLock(lock);
		return inUse.size();
	}

	template<typename R>
	std::size_t
	ResourcePool<R>::availableCount() const
	{
		SharedLock(lock);
		return available.size();
	}

	template<typename R>
	std::size_t
	ResourcePool<R>::freeCount() const
	{
		SharedLock(lock);
		return poolSize.freeCount();
	}

	template<typename R>
	ResourceHandle<R>
	ResourcePool<R>::getMine()
	{
		Lock(lock);
		return ResourceHandle(safeMapLookup<NoCurrentResourceT<R>>(inUse, std::this_thread::get_id()));
	}

	template<typename R>
	void
	ResourcePool<R>::idle()
	{
		Lock(lock);
		available.clear();
	}

	template<typename R>
	ResourceHandle<R>
	ResourcePool<R>::get()
	{
		poolSize.wait();
		try {
			return ResourceHandle(getOne());
		}
		catch (...) {
			poolSize.notify();
			throw;
		}
	}

	template<typename R>
	ResourceHandle<R>
	ResourcePool<R>::get(unsigned int timeout)
	{
		if (!poolSize.wait(timeout)) {
			throw TimeOutOnResourcePoolT<R>();
		}
		try {
			return getOne();
		}
		catch (...) {
			poolSize.notify();
			throw;
		}
	}

	template<typename R>
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
				inUse.insert({std::this_thread::get_id(), ro});
				return ResourceHandle(ro);
			}
			catch (...) {
				available.pop_front();
			}
		}
		auto ro = std::make_shared<typename ResourceHandle<R>::Object>(createResource(), this);
		inUse.insert({std::this_thread::get_id(), ro});
		return ResourceHandle(ro);
	}

	template<typename R>
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

	template<typename R>
	void
	ResourcePool<R>::discard(const std::shared_ptr<R> & r)
	{
		Lock(lock);
		removeFrom(r, inUse);
		poolSize.notify();
	}

	template<typename R>
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

	template<typename R> TimeOutOnResourcePoolT<R>::TimeOutOnResourcePoolT() : TimeOutOnResourcePool(typeid(R).name())
	{
	}

	template<typename R>
	NoCurrentResourceT<R>::NoCurrentResourceT(const std::thread::id & id) : NoCurrentResource(id, typeid(R).name())
	{
	}

}

#endif
