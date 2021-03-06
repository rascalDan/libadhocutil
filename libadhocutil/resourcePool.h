#ifndef ADHOCUTIL_RESOURCEPOOL_H
#define ADHOCUTIL_RESOURCEPOOL_H

#include "c++11Helpers.h"
#include "exception.h"
#include "semaphore.h"
#include "visibility.h"
#include <list>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <thread>
#include <tuple>

namespace AdHoc {
	template<typename Resource> class ResourcePool;

	/// A handle to a resource allocated from a ResourcePool.
	template<typename Resource> class DLL_PUBLIC ResourceHandle {
	public:
		/// Handle to an allocated resource, the pool it belongs to and a count of active references.
		using Object = std::tuple<std::shared_ptr<Resource>, ResourcePool<Resource> *>;

		/// Create a reference to a new resource.
		explicit ResourceHandle(std::shared_ptr<Object>) noexcept;
		/// Standard move/copy support
		SPECIAL_MEMBERS_CONS(ResourceHandle, default);
		~ResourceHandle() noexcept;

		/// Reference to an existing resource.
		ResourceHandle & operator=(const ResourceHandle &) noexcept;
		/// Move reference to an existing resource.
		ResourceHandle & operator=(ResourceHandle &&) noexcept;
		/// Access to the resource.
		Resource * operator->() const noexcept;
		/// Access to the resource.
		Resource * get() const noexcept;
		/// Release the resource back to the pool.
		void release() noexcept;
		/// Cast to bool.
		explicit operator bool() const noexcept;

		/// Get number of handles to this resource.
		[[nodiscard]] unsigned int handleCount() const;

	private:
		DLL_PRIVATE void decRef() noexcept;
		std::shared_ptr<Object> resource;
	};

	/// A fully featured resource pool for sharing and reusing a finite set of
	/// resources, possibly across multiple threads.
	template<typename Resource> class DLL_PUBLIC ResourcePool {
	public:
		friend class ResourceHandle<Resource>;

		/// Create a new resource pool.
		/// @param maxSize The upper limit of how many concurrent active resources there can be.
		/// @param keep The number of resources to cache for reuse.
		ResourcePool(unsigned int maxSize, unsigned int keep);
		virtual ~ResourcePool();

		/// Standard move/copy support
		SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(ResourcePool);

		/// Get a resource from the pool (maybe cached, maybe constructed afresh)
		ResourceHandle<Resource> get();
		/// Get a resource from the pool (with timeout on max size of pool)
		/// @param ms Timeout in milliseconds.
		ResourceHandle<Resource> get(unsigned int ms);
		/// Get a new handle to the resource previous allocated to the current.
		ResourceHandle<Resource> getMine();
		/// Go idle; destroy all cached resources, currently active instances are untouched.
		void idle();

		/// Get number of active resources.
		unsigned int inUseCount() const;
		/// Get number of available cached resources.
		unsigned int availableCount() const;
		/// Get number of free slots.
		unsigned int freeCount() const;

	protected:
		/// Create a new resource instance to add to the pool.
		virtual std::shared_ptr<Resource> createResource() const = 0;
		/// Test a cached resource is still suitable for use before re-use (defaults to no-op).
		virtual void testResource(Resource const *) const;
		/// Test a cached resource is still suitable for use on return (defaults to no-op).
		virtual void returnTestResource(Resource const *) const;

	private:
		using Available = std::list<std::shared_ptr<Resource>>;
		using InUse = std::multimap<std::thread::id, std::shared_ptr<typename ResourceHandle<Resource>::Object>>;

		void putBack(const std::shared_ptr<Resource> &);
		void discard(const std::shared_ptr<Resource> &);

		DLL_PRIVATE static void removeFrom(const std::shared_ptr<Resource> &, InUse &);
		DLL_PRIVATE ResourceHandle<Resource> getOne();

		mutable std::shared_mutex lock;
		Semaphore poolSize;
		unsigned int keep;
		Available available;
		InUse inUse;
	};

	/// Represents a failure to acquire a new resource within the given timeout.
	class DLL_PUBLIC TimeOutOnResourcePool : public AdHoc::StdException {
	public:
		/// Constrcut a new timeout exception for the given resource type.
		explicit TimeOutOnResourcePool(const char * const type);

		std::string message() const noexcept override;

	private:
		const char * const name;
	};

	/// Represents a failure to acquire a new resource of type R within the given timeout.
	template<typename R> class DLL_PUBLIC TimeOutOnResourcePoolT : public TimeOutOnResourcePool {
	public:
		TimeOutOnResourcePoolT();
	};

	/// Represents a request for the current thread's previous allocated resource
	/// when one has not been allocated.
	class DLL_PUBLIC NoCurrentResource : public AdHoc::StdException {
	public:
		/// Construct for a specific thread and resource type.
		NoCurrentResource(const std::thread::id &, const char * const type);

		std::string message() const noexcept override;

	private:
		const std::thread::id threadId;
		const char * const name;
	};

	/// Represents a request for the current thread's previous allocated resource
	/// of type R when one has not been allocated.
	template<typename R> class DLL_PUBLIC NoCurrentResourceT : public NoCurrentResource {
	public:
		/// Construct for a specific thread and resource type R.
		explicit NoCurrentResourceT(const std::thread::id &);
	};

}

#endif
