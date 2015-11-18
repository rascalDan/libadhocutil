#ifndef ADHOCUTIL_RESOURCEPOOL_H
#define ADHOCUTIL_RESOURCEPOOL_H

#include <boost/tuple/tuple.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <atomic>
#include <thread>
#include <list>
#include <map>
#include "semaphore.h"
#include "exception.h"
#include "visibility.h"

namespace AdHoc {
	template <typename Resource>
	class ResourcePool;

	/// A handle to a resource allocated from a ResourcePool.
	template <typename Resource>
	class DLL_PUBLIC ResourceHandle {
		public:
			/// Handle to an allocated resource, the pool it belongs to and a count of active references.
			typedef boost::tuple<Resource *, ResourcePool<Resource> *, std::atomic<unsigned int>> Object;

			/// Create a reference to a new resource.
			ResourceHandle(Object *);
			/// Create a reference to an existing resource.
			ResourceHandle(const ResourceHandle &);
			~ResourceHandle();

			/// Reference to an existing resource.
			void operator=(const ResourceHandle &);
			/// Access to the resource.
			Resource * operator->() const;
			/// Access to the resource.
			Resource * get() const;

			/// Get number of handles to this resource.
			unsigned int handleCount() const;

		private:
			DLL_PRIVATE void incRef() const;
			DLL_PRIVATE void decRef();
			Object * resource;
	};

	/// A fully featured resource pool for sharing and reusing a finite set of
	/// resources, possibly across multiple threads.
	template <typename Resource>
	class DLL_PUBLIC ResourcePool {
		public:
			friend class ResourceHandle<Resource>;

			/// Create a new resource pool.
			/// @param maxSize The upper limit of how many concurrent active resources there can be.
			/// @param keep The number of resources to cache for reuse.
			ResourcePool(unsigned int maxSize, unsigned int keep);
			virtual ~ResourcePool();

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

		protected:
			/// Create a new resource instance to add to the pool.
			virtual Resource * createResource() const = 0;
			/// Destroy an existing resource (defaults to delete).
			virtual void destroyResource(Resource *) const;
			/// Test a cached resource is still suitable for use (defaults to no-op).
			virtual void testResource(const Resource *) const;

		private:
			typedef std::list<Resource *> Available;
			typedef std::multimap<std::thread::id, typename ResourceHandle<Resource>::Object *> InUse;

			void putBack(Resource *);
			void discard(Resource *);

			DLL_PRIVATE static void removeFrom(Resource *, InUse &);
			DLL_PRIVATE ResourceHandle<Resource> getOne();

			mutable boost::upgrade_mutex lock;
			Semaphore poolSize;
			unsigned int keep;
			Available available;
			InUse inUse;
	};

	/// Represents a failure to acquire a new resource within the given timeout.
	class DLL_PUBLIC TimeOutOnResourcePool : public AdHoc::StdException {
		public:
			/// Constrcut a new timeout exception for the given resource type.
			TimeOutOnResourcePool(const char * const type);

			std::string message() const throw() override;

		private:
			const char * const name;
	};

	/// Represents a failure to acquire a new resource of type R within the given timeout.
	template <typename R>
	class DLL_PUBLIC TimeOutOnResourcePoolT : public TimeOutOnResourcePool {
		public:
			TimeOutOnResourcePoolT();
	};

	/// Represents a request for the current thread's previous allocated resource
	/// when one has not been allocated.
	class DLL_PUBLIC NoCurrentResource : public AdHoc::StdException {
		public:
			/// Construct for a specific thread and resource type.
			NoCurrentResource(const std::thread::id &, const char * const type);

			std::string message() const throw() override;

		private:
			const std::thread::id threadId;
			const char * const name;
	};

	/// Represents a request for the current thread's previous allocated resource
	/// of type R when one has not been allocated.
	template <typename R>
	class DLL_PUBLIC NoCurrentResourceT : public NoCurrentResource {
		public:
			/// Construct for a specific thread and resource type R.
			NoCurrentResourceT(const std::thread::id &);
	};

}

#endif

